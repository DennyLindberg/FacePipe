#include "application/application.h"

namespace fs = std::filesystem;

#define DEBUG_SHOW_SUZANNE true

/*
	Application
*/
int main(int argc, char* args[])
{
	App::settings = {
		.showImguiDemo = false,
		.showConsole = false,
		.vsync = true,
		.fullscreen = false,
		.windowWidth = 1280,
		.windowHeight = 720,
		.windowTitle = "FacePipe",
		.maxFPS = 0,
		.sleepWhenFpsLimited = true,
		.clearColor = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f),
		.defaultCameraFOV = 45.0f,
		.maintainVerticalFOV = true,
		.viewportMouseSensitivity = 0.25f,
		.skyLightDirection = glm::normalize(glm::fvec3(1.0f)),
		.skyLightColor = glm::fvec4(1.0f),
		.receiveDataSocketPort = 9000,
	};

	App::Initialize();

	/*
		Load meshes and add to scene
	*/
	WeakPtr<GLTriangleMesh> cubemesh = GLTriangleMesh::Pool.CreateWeak();
	WeakPtr<GLTriangleMesh> suzannemesh = GLTriangleMesh::Pool.CreateWeak();
	WeakPtr<GLTriangleMesh> armesh = GLTriangleMesh::Pool.CreateWeak();
	WeakPtr<GLTriangleMesh> mpmesh = GLTriangleMesh::Pool.CreateWeak();
	GLMesh::LoadPLY(App::Path("content/meshes/cube.ply"), *cubemesh);
	GLMesh::LoadPLY(App::Path("content/meshes/blender_suzanne.ply"), *suzannemesh);					// Blender Suzanne (monkey head)
	GLMesh::LoadPLY(App::Path("content/thirdparty/arkit/ARFaceGeometry.ply"), *armesh);				// Apple ARKit
	GLMesh::LoadPLY(App::Path("content/thirdparty/mediapipe/canonical_face_model.ply"), *mpmesh);	// Google MediaPipe

	mpmesh->SetColors(glm::fvec4(0.0f, 0.0f, 0.0f, 1.0f));
	mpmesh->SetUsage(GL_DYNAMIC_DRAW, true);

#if DEBUG_SHOW_SUZANNE
	WeakPtr<Object> suzanne = Object::Pool.CreateWeak();
	suzanne->name = "Head";
	suzanne->AddComponent(suzannemesh);
	suzanne->transform.scale = glm::vec3(0.1f);
	suzanne->transform.position = glm::vec3(-1.0f, 0.0f, -1.0f);
	App::world->AddChild(suzanne);
	App::ui.selected_object = suzanne;
#endif

	WeakPtr<Object> arhead = Object::Pool.CreateWeak();
	arhead->name = "ARKitHead";
	arhead->AddComponent(armesh);
	arhead->transform.scale = glm::vec3(0.01f);
	App::world->AddChild(arhead);

	WeakPtr<Object> mphead = Object::Pool.CreateWeak();
	mphead->name = "MediaPipeHead";
	mphead->AddComponent(mpmesh);
	mphead->transform.scale = glm::vec3(1.0f);
	App::world->AddChild(mphead);

	WeakPtr<GLTexture> DefaultTexture = GLTexture::Pool.CreateWeak();
	DefaultTexture->LoadPNG(App::Path("content/textures/default.png"));
	DefaultTexture->CopyToGPU();

	NetAddressIP4 unrealAddress(9001); // Unreal test
	NetAddressIP4 blenderAddress(9002); // Blender test

	App::OnTickEvent = [&](float time, float dt, const SDL_Event& event) -> void 
	{
		
	};

	App::OnTickScene = [&](float time, float dt) -> void 
	{
#if DEBUG_SHOW_SUZANNE
		// naive bounce just to see some movement
		static float yvel = 0.0f;
		yvel -= 9.8f*dt;
		suzanne->transform.position.x += 0.1f*dt;
		suzanne->transform.position.y += yvel*dt;
		suzanne->transform.rotation.z -= (float) (2.0*time);

		if (suzanne->transform.position.y < 0.0f)
		{
			suzanne->transform.position.y = 0.0f;
			yvel = abs(yvel);
		}

		// Show Suzanne's coordinate frame
		App::ui.sceneViewport->debuglines->AddLine({ 0.0f, 0.0f, 0.0f }, Transform::Position(suzanne->ComputeWorldMatrix()), { 0.0f, 1.0f, 0.0f, 1.0f });
		GLMesh::AppendCoordinateAxis(*App::ui.sceneViewport->debuglines, suzanne->ComputeWorldMatrix());
#endif

		// Sending packets
		//std::string send = std::format("Time: {}", time);
		//Logf(LOG_NET_SEND, "[{}:{}]: {}\n", sendTarget.ip, sendTarget.port, send);
		//udp.Send(send, sendTarget);
		
		// Receiving packets
		UDPDatagram datagram;
		while (App::datagramsQueue.Pop(datagram))
		{
			if (!FacePipe::ParseHeader(datagram.message, datagram.metaData))
			{
				App::lastReceivedDatagram = UDPDatagram();
				continue;
			}

			switch (datagram.metaData.DataType)
			{
			case FacePipe::EFacepipeData::Blendshapes:
			{
				FacePipe::GetBlendshapes(datagram.message, datagram.metaData, App::latestFrame.Blendshapes);
				break;
			}
			case FacePipe::EFacepipeData::Landmarks2D:
			case FacePipe::EFacepipeData::Landmarks3D:
			{
				// TODO: Landmarks2D is a bit problematic when we store it as latestFrame, we expect 3D there
				FacePipe::GetLandmarks(datagram.message, datagram.metaData, App::latestFrame.Landmarks, App::latestFrame.ImageWidth, App::latestFrame.ImageHeight);
				break;
			}
			case FacePipe::EFacepipeData::Mesh:
			{
				break;
			}
			case FacePipe::EFacepipeData::Matrices4x4:
			{
				FacePipe::GetMatrices(datagram.message, datagram.metaData, App::latestFrame.Matrices);
				break;
			}
			}

			App::lastReceivedDatagram = datagram;

			// forward to next application
			App::receiveDataSocket.Send(datagram, unrealAddress);
			App::receiveDataSocket.Send(datagram, blenderAddress);
		}

		float w = (float) App::latestFrame.ImageWidth;
		float h = (float) App::latestFrame.ImageHeight;
		if (w == 0.0f || h == 0.0f)
		{
			w = 1.0f;
			h = 1.0f;
		}
		float ratio = w/h;

		// Draw debug transforms
		for (auto& Pair : App::latestFrame.Matrices)
		{
			auto& m = Pair.second;

			if (m.size() != 16)
				continue;

			glm::mat4 mat(
				m[0], m[1], m[2], m[3],
				m[4], m[5], m[6], m[7],
				m[8], m[9], m[10], m[11],
				m[12], m[13], m[14], m[15]
			);

			// TODO: Why is the scale off from MediaPipe? Wrong documentation on units?
			w *= 0.1f;
			h *= 0.1f;
			
			// negative sign is used to rotate the matrix so that Z+ is forward (just for easier visualization)
			glm::fvec3 origin{ mat[0][3]/w, mat[1][3]/h, mat[2][3]/w }; // translation vector x,y,z
			glm::fvec3 x{ -mat[0] };
			glm::fvec3 y{ mat[1] };
			glm::fvec3 z{ -mat[2] };

			App::ui.sceneViewport->debuglines->AddLine(origin, origin+x, glm::fvec4(1.0f, 0.0f, 0.0f, 1.0f));
			App::ui.sceneViewport->debuglines->AddLine(origin, origin+y, glm::fvec4(0.0f, 1.0f, 0.0f, 1.0f));
			App::ui.sceneViewport->debuglines->AddLine(origin, origin+z, glm::fvec4(0.0f, 0.0f, 1.0f, 1.0f));
		}
		
		if (size_t mpcount = App::latestFrame.Landmarks.size()/3)
		{
			// fill missing points (eyes, etc) - MediaPipe added additional points to the original canonical head
			while (mpmesh->positions.size() < mpcount)
			{
				unsigned int i = (unsigned int)mpmesh->positions.size();
				mpmesh->AddVertex(glm::fvec3(0.0f), glm::fvec3(1.0f, 0.0f, 0.0f), glm::fvec4(1.0f, 0.0f, 0.0f, 1.0f), glm::fvec4(1.0f));
				mpmesh->DefineNewTriangle(i, i, i);
			}

			for (size_t i=0; i<mpcount; ++i)
			{
				mpmesh->positions[i].x = App::latestFrame.Landmarks[i*3];
				mpmesh->positions[i].y = App::latestFrame.Landmarks[i*3+1]; // divide by ratio to restore image proportions
				mpmesh->positions[i].z = App::latestFrame.Landmarks[i*3+2];
			}
			mpmesh->SendToGPU();
		}
	};

	App::OnTickRender = [&](float time, float dt) -> void 
	{	
		App::ui.sceneViewport->Render([&](Viewport& viewport) {
			GLProgram& pointCloudShader = App::shaders.pointCloudShader;

			// Draw ARKit canonical head
			//pointCloudShader.Use();
			//pointCloudShader.SetUniformMat4("model", arhead->ComputeWorldMatrix());
			//pointCloudShader.SetUniformFloat("size", App::settings.pointCloudSize);
			//armesh->Draw(GL_POINTS);

			// Draw MediaPipe canonical head
			pointCloudShader.Use();
			pointCloudShader.SetUniformMat4("model", mphead->ComputeWorldMatrix());
			pointCloudShader.SetUniformFloat("size", App::settings.pointCloudSize);
			mpmesh->Draw(GL_POINTS);

#if DEBUG_SHOW_SUZANNE
			GLProgram& meshShader = App::shaders.defaultMeshShader;
			meshShader.Use();
			meshShader.SetUniformMat4("model", suzanne->ComputeWorldMatrix());
			meshShader.SetUniformInt("useTexture", 0);
			meshShader.SetUniformInt("useFlatShading", 0);
			DefaultTexture->UseForDrawing();
			suzannemesh->Draw();
#endif
		});
	};

	App::Run();

	return 0;
}
