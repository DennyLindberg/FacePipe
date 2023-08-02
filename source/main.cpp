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
		.viewportMouseSensitivity = 0.25f,
		.skyLightDirection = glm::normalize(glm::fvec3(1.0f)),
		.skyLightColor = glm::fvec4(1.0f),
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
	GLMesh::LoadPLY(App::Path("content/meshes/cube.ply"), *cubemesh);
	GLMesh::LoadPLY(App::Path("content/meshes/blender_suzanne.ply"), *suzannemesh);
	GLMesh::LoadPLY(App::Path("content/meshes/ARFaceGeometry.ply"), *armesh);

	WeakPtr<Object> suzanne = Object::Pool.CreateWeak();
	WeakPtr<Object> arhead = Object::Pool.CreateWeak();
	suzanne->name = "Head";
	arhead->name = "ARhead";

	App::world->AddChild(suzanne);
	App::world->AddChild(arhead);

	suzanne->AddComponent(suzannemesh);
	arhead->AddComponent(armesh);

	suzanne->transform.scale = glm::vec3(0.1f);
	//head->transform.position.x = -0.25f;
	arhead->transform.scale = glm::vec3(0.001f);

	WeakPtr<GLTexture> DefaultTexture = GLTexture::Pool.CreateWeak();
	DefaultTexture->LoadPNG(App::Path("content/textures/default.png"));
	DefaultTexture->CopyToGPU();

	App::ui.selected_object = suzanne;

	App::OnTickEvent = [&](double time, double dt, const SDL_Event& event) -> void 
	{
		
	};

	App::OnTickScene = [&](double time, double dt) -> void 
	{
		suzanne->transform.position.z = abs(sinf((float)time));
		suzanne->transform.rotation.x = (float)time*2.0f;
		//head->transform.scale = glm::vec3(0.1f*abs(sinf((float)time*0.5f)));
	};

	App::OnTickRender = [&](double time, double dt) -> void 
	{
		App::ui.sceneViewport->Render([&](Viewport& viewport) {
			viewport.debuglines->AddLine({ 0.0f, 0.0f, 0.0f }, Transform::Position(suzanne->ComputeWorldMatrix()), { 0.0f, 1.0f, 0.0f, 1.0f });
			GLMesh::AppendCoordinateAxis(*viewport.debuglines, suzanne->ComputeWorldMatrix());

			pointCloudShader.Use();
			pointCloudShader.SetUniformMat4("model", arhead->ComputeWorldMatrix());
			pointCloudShader.SetUniformFloat("size", App::settings.pointCloudSize);
			armesh->Draw(GL_POINTS);

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
