#include "application/application.h"

namespace fs = std::filesystem;

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
	auto& settings = App::settings;

	App::Initialize();
	App::window.SetTitle("FacePipe");

	/*
		Load and initialize shaders
	*/
	GLProgram& meshShader = App::shaders.defaultMeshShader;
	GLProgram& lineShader = App::shaders.lineShader;
	GLProgram& backgroundShader = App::shaders.backgroundShader;
	GLProgram& bezierLinesShader = App::shaders.bezierLinesShader;
	GLProgram& pointCloudShader = App::shaders.pointCloudShader;

	/*
		Load head mesh
	*/
	WeakPtr<GLTriangleMesh> cubemesh = GLTriangleMesh::Pool.CreateWeak();
	WeakPtr<GLTriangleMesh> suzannemesh = GLTriangleMesh::Pool.CreateWeak();
	WeakPtr<GLTriangleMesh> armesh = GLTriangleMesh::Pool.CreateWeak();
	WeakPtr<GLTriangleMesh> mpmesh = GLTriangleMesh::Pool.CreateWeak();
	GLMesh::LoadPLY(App::Path("content/meshes/cube.ply"), *cubemesh);
	GLMesh::LoadPLY(App::Path("content/meshes/blender_suzanne.ply"), *suzannemesh);
	GLMesh::LoadPLY(App::Path("content/thirdparty/arkit/ARFaceGeometry.ply"), *armesh);
	GLMesh::LoadPLY(App::Path("content/thirdparty/mediapipe/canonical_face_model.ply"), *mpmesh);

	mpmesh->SetColors(glm::fvec4(0.0f, 0.0f, 0.0f, 1.0f));
	mpmesh->SetUsage(GL_DYNAMIC_DRAW); // uploads as a side-effect

	WeakPtr<Object> suzanne = Object::Pool.CreateWeak();
	WeakPtr<Object> arhead = Object::Pool.CreateWeak();
	WeakPtr<Object> mphead = Object::Pool.CreateWeak();
	suzanne->name = "Head";
	arhead->name = "ArkitHead";
	mphead->name = "MediaPipeHead";

	App::world->AddChild(suzanne);
	App::world->AddChild(arhead);
	App::world->AddChild(mphead);

	suzanne->AddComponent(suzannemesh);
	arhead->AddComponent(armesh);
	mphead->AddComponent(mpmesh);

	suzanne->transform.scale = glm::vec3(0.1f);
	suzanne->transform.position = glm::vec3(-1.0f, 0.0f, -1.0f);
	arhead->transform.scale = glm::vec3(0.01f);
	mphead->transform.scale = glm::vec3(1.0f);

	WeakPtr<GLTexture> DefaultTexture = GLTexture::Pool.CreateWeak();
	DefaultTexture->LoadPNG(App::Path("content/textures/default.png"));
	DefaultTexture->CopyToGPU();

	NetAddressIP4 OutgoingAddress(9001); // to Unreal or Blender

	App::ui.selected_object = suzanne;

	App::OnTickEvent = [&](float time, float dt, const SDL_Event& event) -> void 
	{
		
	};

	App::OnTickScene = [&](float time, float dt) -> void 
	{
		static float yvel = -0.0f;
		yvel += -9.8f*dt;

		// bounce
		suzanne->transform.position.y += yvel*dt;
		if (suzanne->transform.position.y < 0.0f)
		{
			suzanne->transform.position.y = 0.0f;
			yvel = abs(yvel);
		}

		suzanne->transform.position.x += 0.1f*dt;
		suzanne->transform.rotation.z = -(float)time * 2.0f;

		//viewport.debuglines->AddLine({ 0.0f, 0.0f, 0.0f }, Transform::Position(suzanne->ComputeWorldMatrix()), { 0.0f, 1.0f, 0.0f, 1.0f });
		//GLMesh::AppendCoordinateAxis(*viewport.debuglines, suzanne->ComputeWorldMatrix());

		// Sending packets
		//std::string send = std::format("Time: {}", time);
		//Logf(LOG_NET_SEND, "[{}:{}]: {}\n", sendTarget.ip, sendTarget.port, send);
		//udp.Send(send, sendTarget);
		
		// Receiving packets
		std::vector<UDPDatagram> datagrams;
		if (App::receiveDataSocket.Receive(datagrams))
		{
			for (UDPDatagram& datagram : datagrams)
			{
				nlohmann::json jsonData;
				if (!FacePipe::Parse(datagram.message, datagram.metaData, jsonData))
				{
					App::lastReceivedDatagram = UDPDatagram();
					continue;
				}
				
				switch (datagram.metaData.DataType)
				{
				case FacePipe::EFacepipeData::Blendshapes:
				{
					FacePipe::GetBlendshapes(datagram.metaData, jsonData, App::latestFrame.BlendshapeNames, App::latestFrame.BlendshapeValues);
					break;
				}
				case FacePipe::EFacepipeData::Landmarks:
				{
					FacePipe::GetLandmarks(datagram.metaData, jsonData, App::latestFrame.Landmarks);
					break;
				}
				case FacePipe::EFacepipeData::Transforms:
				{
					break;
				}
				}

				App::lastReceivedDatagram = datagram;

				// forward to next application
				App::receiveDataSocket.Send(datagram, OutgoingAddress);
			}
		}
		
		if (size_t mpcount = App::latestFrame.Landmarks.size()/3)
		{
			// fill missing points (eyes, etc)
			while (mpmesh->positions.size() < mpcount)
			{
				unsigned int i = (unsigned int)mpmesh->positions.size();
				mpmesh->AddVertex(glm::fvec3(0.0f), glm::fvec3(1.0f, 0.0f, 0.0f), glm::fvec4(1.0f, 0.0f, 0.0f, 1.0f), glm::fvec4(1.0f));
				mpmesh->DefineNewTriangle(i, i, i);
			}

			for (size_t i=0; i<mpcount; ++i)
			{
				mpmesh->positions[i].x = -App::latestFrame.Landmarks[i*3];
				mpmesh->positions[i].y = App::latestFrame.Landmarks[i*3+1];
				mpmesh->positions[i].z = App::latestFrame.Landmarks[i*3+2];
			}
			mpmesh->SendToGPU();
		}
	};

	App::OnTickRender = [&](float time, float dt) -> void 
	{
		App::ui.sceneViewport->Render([&](Viewport& viewport) {
			//pointCloudShader.Use();
			//pointCloudShader.SetUniformMat4("model", arhead->ComputeWorldMatrix());
			//pointCloudShader.SetUniformFloat("size", App::settings.pointCloudSize);
			//armesh->Draw(GL_POINTS);

			pointCloudShader.Use();
			pointCloudShader.SetUniformMat4("model", mphead->ComputeWorldMatrix());
			pointCloudShader.SetUniformFloat("size", App::settings.pointCloudSize);
			mpmesh->Draw(GL_POINTS);

			meshShader.Use();
			meshShader.SetUniformMat4("model", suzanne->ComputeWorldMatrix());
			meshShader.SetUniformInt("useTexture", 0);
			meshShader.SetUniformInt("useFlatShading", 0);
			DefaultTexture->UseForDrawing();
			suzannemesh->Draw();
		});
	};

	App::Run();

	return 0;
}
