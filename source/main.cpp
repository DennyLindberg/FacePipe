#include "application/application.h"

namespace fs = std::filesystem;

/*
	Application
*/
int main(int argc, char* args[])
{
	App::settings = {
		.vsync = true,
		.fullscreen = 0,
		.windowWidth = 1280,
		.windowHeight = 720,
		.maxFPS = 0,
		.sleepWhenFpsLimited = true,
		.clearColor = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f),
		.defaultCameraFOV = 45.0f,
		.viewportMouseSensitivity = 0.25f,
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
	WeakPtr<GLTriangleMesh> headmesh = GLTriangleMesh::Pool.CreateWeak();
	WeakPtr<GLTriangleMesh> armesh = GLTriangleMesh::Pool.CreateWeak();
	GLMesh::LoadPLY(App::Path("content/meshes/cube.ply"), *cubemesh);
	GLMesh::LoadPLY(App::Path("content/meshes/blender_suzanne.ply"), *headmesh);
	GLMesh::LoadPLY(App::Path("content/meshes/ARFaceGeometry.ply"), *armesh);

	WeakPtr<Object> cube = Object::Pool.CreateWeak();
	WeakPtr<Object> head = Object::Pool.CreateWeak();
	WeakPtr<Object> arhead = Object::Pool.CreateWeak();
	cube->name = "Cube";
	head->name = "Head";
	arhead->name = "ARhead";

	App::world->AddChild(cube);
	App::world->AddChild(head);
	App::world->AddChild(arhead);

	cube->AddComponent(cubemesh);
	cube->AddComponent(headmesh);
	cube->AddComponent(Camera::Pool.CreateWeak());
	head->AddComponent(headmesh);
	arhead->AddComponent(armesh);

	cube->transform.scale = glm::vec3(0.1f);
	head->transform.scale = glm::vec3(0.1f);
	//head->transform.position.x = -0.25f;
	arhead->transform.scale = glm::vec3(0.001f);

	WeakPtr<GLTexture> DefaultTexture = GLTexture::Pool.CreateWeak();
	DefaultTexture->LoadPNG(App::Path("content/textures/default.png"));
	DefaultTexture->CopyToGPU();

	App::ui.selected_object = cube;

	/*
		Main application loop
	*/
	bool quit = false;
	while (!quit)
	{
		if (!App::ReadyToTick())
		{
			continue;
		}

		App::Tick();

		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			if (event.type == SDL_QUIT)
				quit = true;

			if (App::ui.HandleInputEvent(&event))
				continue;

			SDL_Keymod mod = SDL_GetModState();
			bool bCtrlModifier = mod & KMOD_CTRL;
			bool bShiftModifier = mod & KMOD_SHIFT;
			bool bAltModifier = mod & KMOD_ALT;

			if (event.type == SDL_KEYDOWN)
			{
				auto key = event.key.keysym.sym;

				// global keys here
			}

			if (Viewport* activeViewport = App::ui.GetActiveViewport())
			{
				CameraController& controller = activeViewport->input;

				if (event.type == SDL_KEYDOWN)
				{
					auto key = event.key.keysym.sym;

					if      (key == SDLK_s) GLFramebuffers::SaveScreenshot(activeViewport->framebuffer);
					else if (key == SDLK_f) controller.SnapToOrigin();

					else if (key == SDLK_KP_7) controller.SetCameraView(CameraView::OrthographicY);
					else if (key == SDLK_KP_1) controller.SetCameraView(bCtrlModifier ? CameraView::OrthographicZneg : CameraView::OrthographicZ);
					else if (key == SDLK_KP_9) controller.SetCameraView(CameraView::OrthographicYneg);
					else if (key == SDLK_KP_3) controller.SetCameraView(bCtrlModifier ? CameraView::OrthographicXneg : CameraView::OrthographicX);
					else if (key == SDLK_KP_5) controller.SetCameraView(CameraView::Perspective);
				}

				if (event.type == SDL_MOUSEBUTTONDOWN)
				{
					App::ui.viewportCaptureMouse = true;
					App::ui.viewportCaptureMouseBeginX = event.motion.x;
					App::ui.viewportCaptureMouseBeginY = event.motion.y;
					SDL_ShowCursor(0);
					SDL_SetRelativeMouseMode(SDL_TRUE);

					auto button = event.button.button;
					if (button == SDL_BUTTON_LEFT)			controller.inputState = TurntableInputState::Rotate;
					else if (button == SDL_BUTTON_MIDDLE)	controller.inputState = TurntableInputState::Translate;
					else if (button == SDL_BUTTON_RIGHT)	controller.inputState = TurntableInputState::Zoom;

					controller.OnBeginInput();
				}
				else if (event.type == SDL_MOUSEBUTTONUP)
				{
					App::ui.viewportCaptureMouse = false;
					SDL_ShowCursor(1);
					SDL_SetRelativeMouseMode(SDL_FALSE);
				}
				else if (event.type == SDL_MOUSEMOTION && App::ui.viewportCaptureMouse)
				{
					controller.ApplyMouseInput(-event.motion.xrel, event.motion.yrel, App::settings.viewportMouseSensitivity);
					SDL_WarpMouseInWindow(App::window.GetSDLWindow(), App::ui.viewportCaptureMouseBeginX, App::ui.viewportCaptureMouseBeginY);
				}
			}
		}

		/*
			Render scene
		*/
		GLFramebuffers::ClearActive();

		// Background color gradient
		backgroundShader.Use();
		App::geometry.quad.Draw();
		GLFramebuffers::ClearActiveDepth();

		WeakPtr<Camera> camera = App::ui.applicationViewport->input.camera;
		WeakPtr<Camera> cameraPreview = App::ui.previewViewport->input.camera;
		
		// Set scene render properties
		glPolygonMode(GL_FRONT_AND_BACK, (App::ui.renderWireframe? GL_LINE : GL_FILL));
		App::shaders.UpdateCameraUBO(camera);
		App::shaders.UpdateLightUBOPosition(App::ui.lightFollowsCamera? camera->GetPosition() : glm::fvec3{ 999999.0f });

		// Debug: Test changing the mesh transform over time
		if (cube) cube->transform.rotation.y = sinf((float)App::clock.time);
		head->transform.position.z = abs(sinf((float)App::clock.time));
		head->transform.rotation.x = (float)App::clock.time*2.0f;
		//head->transform.scale = glm::vec3(0.1f*abs(sinf((float)App::clock.time*0.5f)));

		pointCloudShader.Use();
		pointCloudShader.SetUniformMat4("model", arhead->ComputeWorldMatrix());
		pointCloudShader.SetUniformFloat("size", App::settings.pointCloudSize);
		armesh->Draw(GL_POINTS);

		meshShader.Use();
		meshShader.SetUniformMat4("model", head->ComputeWorldMatrix());
		meshShader.SetUniformInt("useTexture", 0);
		meshShader.SetUniformInt("useFlatShading", 0);
		DefaultTexture->UseForDrawing();
		headmesh->Draw();
			
		// Grid
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		if (camera->GetView() == CameraView::Perspective)
			App::geometry.grid.Draw(App::geometry.quad, camera->ViewProjectionMatrix());
		else
			App::geometry.grid.Draw(App::geometry.quad, camera->ViewProjectionMatrix(), camera->ForwardVector(), camera->SideVector());
		
		// Test debug lines
		App::debuglines->AddLine({ 0.0f, 0.0f, 0.0f }, Transform::Position(head->ComputeWorldMatrix()), { 0.0f, 1.0f, 0.0f, 1.0f });
		GLMesh::AppendCoordinateAxis(*App::debuglines, head->ComputeWorldMatrix());
		App::Render(App::ui.applicationViewport->input.camera);

		if (auto F = GLFramebuffers::BindScoped(App::ui.previewViewport->framebuffer))
		{
			GLFramebuffers::ClearActive();
			App::shaders.UpdateCameraUBO(cameraPreview);
			App::shaders.UpdateLightUBOPosition(App::ui.lightFollowsCamera? cameraPreview->GetPosition() : glm::fvec3{ 999999.0f });

			if (cube)
			{
				meshShader.SetUniformMat4("model", cube->transform.Matrix());
				meshShader.SetUniformInt("useTexture", 0);
				cubemesh->Draw();
			}
		}

		// Done
		App::window.RenderImgui();
		App::window.SwapFramebuffer();
	}

	App::Shutdown();

	return 0;
}
