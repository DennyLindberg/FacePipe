#include "application/application.h"

#include "nlohmann/json.hpp"

namespace fs = std::filesystem;
using json = nlohmann::json;

#define INVALID_TYPE '\0'
#define BYTES_TYPE 'b'
#define STRING_TYPE 's'
#define JSON_TYPE 'j'

enum class EFacepipeData : uint8_t
{
	Landmarks = 0,
	Blendshapes = 1,
	Transforms = 2
};

enum class EDatagramType : uint8_t
{
	Invalid = 0,
	Bytes = 1,
	String = 2,
	JSON = 3,
	MAX = 4
};

void decode_header(const UDPDatagram& datagram, int& scene, int& camera, int& subject, EDatagramType& datatype)
{
	scene = 0;
	camera = 0;
	subject = 0;
	datatype = EDatagramType::Invalid;

	if (datagram.message.size() >= 4)
	{
		scene = (int)datagram.message[0];
		camera = (int)datagram.message[1];
		subject = (int)datagram.message[2];

		switch (datagram.message[3])
		{
		case 'b': { datatype = EDatagramType::Bytes; break; }
		case 's': { datatype = EDatagramType::String; break; }
		case 'j': { datatype = EDatagramType::JSON; break; }
		default:  { datatype = EDatagramType::Invalid; break; }
		}
	}
}

std::string data_as_str(UDPDatagram& datagram)
{
	return std::string(datagram.message.begin() + 4, datagram.message.end());
}

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
	};
	auto& settings = App::settings;

	App::Initialize();
	App::window.SetTitle("FacePipe");

	//NetSocket sendTarget(Net::LocalHost, 1337);
	UDPSocket udp(9000);
	udp.Start();

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
		if (udp.Receive(datagrams))
		{
			for (UDPDatagram& datagram : datagrams)
			{
				int scene, camera, subject;
				EDatagramType data_type;

				decode_header(datagram, scene, camera, subject, data_type);

				switch (data_type)
				{
				case EDatagramType::Bytes:	
				{ 
					break; 
				}
				case EDatagramType::String:	
				{ 
					break; 
				}
				case EDatagramType::JSON:	
				{
					try {
						json mpdata = json::parse(data_as_str(datagram));

						json& type = mpdata["type"];
						json& data = mpdata["data"];

						if (!type.is_number_integer())
							continue;

						EFacepipeData facedata = (EFacepipeData) type.get<int>();

						switch (facedata)
						{
						case EFacepipeData::Landmarks:
						{
							if (data.is_array())
							{
								App::mediapipeLandmarks = data.get<std::vector<float>>();
							}
							break;
						}
						case EFacepipeData::Blendshapes:
						{
							if (data.is_array())
							{
								App::arkitBlendshapes = data.get<std::vector<float>>();
							}
							break;
						}
						case EFacepipeData::Transforms:
						{
							break;
						}
						default: {}
						}
					}
					catch (std::exception e)
					{
					}
					break;	
				}
				default: {}
				}

				//Logf(LOG_NET_RECEIVE, "[{}:{}]: {}\n", datagram.source.ip, datagram.source.port, datagram.message);
			}
		}

		//Logf(LOG_NET_RECEIVE, "{} vs {}\n", mpmesh->positions.size(), App::mediapipeLandmarks.size()/3);
		
		size_t mpcount = App::mediapipeLandmarks.size()/3;
		size_t maxcount = mpmesh->positions.size();
		if (maxcount > mpcount)
			maxcount = mpcount;

		for (size_t i=0; i<maxcount; ++i)
		{
			mpmesh->positions[i].x = -App::mediapipeLandmarks[i*3];
			mpmesh->positions[i].y = App::mediapipeLandmarks[i*3+1];
			mpmesh->positions[i].z = App::mediapipeLandmarks[i*3+2];
		}
		mpmesh->SendToGPU();
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

	udp.Close();

	return 0;
}
