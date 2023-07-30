#include "application/application.h"

#include "core/objectpool.h"

namespace fs = std::filesystem;

const float CAMERA_FOV = 45.0f;

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
		.clearColor = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)
	};
	auto& settings = App::settings;

	App::Initialize();
	App::window.SetTitle("FacePipe");

	fs::path PythonTestScript = App::Path("content/scripts/test_cv2_webcam.py");

	std::string helpString = R"(Controls:
- Mouse buttons: Camera
- S: Screenshot
- F: Re-center camera
)";
	
	// Testing of object pool with weak ptr support
	ObjectId cameraId = Camera::Pool.Create();
	WeakObjectPtr<Camera> cameraTest = Camera::Pool.CreateWeak();

	/*
		Setup scene and controls
	*/
	Camera& camera = Camera::Pool[cameraId];
	camera.fieldOfView = CAMERA_FOV;

	Camera cameraCube;
	cameraCube.fieldOfView = CAMERA_FOV;

	TurntableController turntable(camera);
	TurntableController turntableCube(cameraCube);
	turntable.position = glm::vec3{-0.15f, 0.0f, 0.0f};
	turntable.sensitivity = 0.25f;
	turntable.Set(-65.0f, 15.0f, 0.75f);
	turntableCube.position = turntable.position;
	turntableCube.sensitivity = turntable.sensitivity;
	turntableCube.Set(-65.0f, 15.0f, 1.0f);

	auto SetCameraView = [](Camera& camera, TurntableController& controller, CameraView view) -> void {
		camera.SetView(view);
		controller.SetDistance(view == CameraView::Perspective? 1.0f : 1.0f);
	};

	GLuint RenderTarget = GLFramebuffers::Create(GLuint(settings.windowWidth*0.25f), GLuint(settings.windowHeight*0.25f), App::settings.clearColor);

	/*
		Load and initialize shaders
	*/
	GLTexture defaultTexture{ App::Path("content/textures/default.png") };
	defaultTexture.UseForDrawing();

	GLProgram& meshShader = App::shaders.defaultMeshShader;
	GLProgram& lineShader = App::shaders.lineShader;
	GLProgram& backgroundShader = App::shaders.backgroundShader;
	GLProgram& bezierLinesShader = App::shaders.bezierLinesShader;
	GLProgram& pointCloudShader = App::shaders.pointCloudShader;

	/*
		Load head mesh
	*/
	GLTriangleMesh cubemesh, headmesh, armesh;
	GLMesh::LoadPLY(App::Path("content/meshes/cube.ply"), cubemesh);
	GLMesh::LoadPLY(App::Path("content/meshes/blender_suzanne.ply"), headmesh);
	GLMesh::LoadPLY(App::Path("content/meshes/ARFaceGeometry.ply"), armesh);
	cubemesh.transform.scale = glm::vec3(0.1f);
	headmesh.transform.scale = glm::vec3(0.1f);
	headmesh.transform.position.x = -0.25f;
	armesh.transform.scale = glm::vec3(0.001f);

	GLTexture DefaultTexture(App::Path("content/textures/default.png"));
	DefaultTexture.CopyToGPU();

	GLLine cubeMeshNormals;
	GLMesh::LoadLinesFromMeshNormals(cubemesh, cubeMeshNormals, 0.2f);
	cubeMeshNormals.transform.scale = cubemesh.transform.scale;

	/*
		User interaction parameters in the UI
	*/
	bool renderWireframe = false;
	bool lightFollowsCamera = false;
	bool drawDebugNormals = false;

	/*
		IMGUI callback
	*/
	bool interactingWithPreview = false;
	std::string input_field_string{"default text"};
	App::window.imguiLayout = [&]() -> void {
		ImGui::SetNextWindowSize(ImVec2(settings.windowWidth * 0.25f, settings.windowHeight * 1.0f));
		ImGui::SetNextWindowPos(ImVec2(0, 0));

		ImGui::Begin("App");
		{
			ImGui::Text("Scene");
			ImGui::Text(("App - FPS: " + FpsString(App::clock.deltaTime)).c_str());
			ImGui::Text(("App - Time: " + std::to_string(App::clock.time)).c_str());
			ImGui::InputInt("MaxFPS", &App::settings.maxFPS, 0);
			ImGui::Checkbox("Wireframe", &renderWireframe);
			ImGui::Checkbox("Light follows camera", &lightFollowsCamera);
			ImGui::SliderFloat("PointSize", &App::settings.pointCloudSize, 0.0005f, 0.01f, "%.5f");
			if (ImGui::Button("Run Python test script"))
			{
				App::python.Execute(PythonTestScript);
			}

			ImGui::InputTextMultiline( "ScriptInput", &input_field_string, ImVec2(0.0f, 100.0f) );
			if (ImGui::Button("Execute"))
			{
				App::python.Execute(input_field_string);
			}

			ImGui::InputTextMultiline( "Help", &helpString, ImVec2(0.0f, 100.0f), ImGuiInputTextFlags_ReadOnly );

			GLuint Texture, TextureWidth, TextureHeight;
			if (GLFramebuffers::GetTexture(RenderTarget, Texture, TextureWidth, TextureHeight))
			{
				ImGui::Image((ImTextureID)(intptr_t)Texture, ImVec2((float)TextureWidth, (float)TextureHeight), {0, 1}, {1, 0});
				interactingWithPreview = ImGui::IsItemHovered();
			}
		}
		ImGui::End();

		ImGui::SetNextWindowSize(ImVec2(settings.windowWidth * 1.0f, settings.windowHeight * 0.25f));
		ImGui::SetNextWindowPos(ImVec2(0, settings.windowHeight * 0.75f));
		ImGui::Begin("Nodes");
		{
			ImNodes::BeginNodeEditor();
			
			{
				ImNodes::BeginNode(1);

				ImNodes::BeginNodeTitleBar();
				ImGui::TextUnformatted("simple node :)");
				ImNodes::EndNodeTitleBar();

				ImNodes::BeginInputAttribute(2);
				ImGui::Text("input");
				ImNodes::EndInputAttribute();

				ImNodes::BeginOutputAttribute(3);
				ImGui::Indent(40);
				ImGui::Text("output");
				ImNodes::EndOutputAttribute();

				ImNodes::EndNode();
			}

			{
				ImNodes::BeginNode(4);

				ImNodes::BeginNodeTitleBar();
				ImGui::TextUnformatted("simple node 2");
				ImNodes::EndNodeTitleBar();

				ImNodes::BeginInputAttribute(5);
				ImGui::Text("input");
				ImNodes::EndInputAttribute();

				ImNodes::BeginOutputAttribute(6);
				ImGui::Indent(40);
				ImGui::Text("output");
				ImNodes::EndOutputAttribute();

				ImNodes::EndNode();
			}

			ImNodes::EndNodeEditor();
		}
		ImGui::End();
	};

	/*
		Main application loop
	*/
	bool quit = false;
	bool captureMouse = false;
	int captureMouseX, captureMouseY;
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

			App::window.HandleImguiEvent(&event);
			if (ImGui::GetIO().WantCaptureKeyboard && !interactingWithPreview)
			{
				continue;
			}

			SDL_Keymod mod = SDL_GetModState();
			bool bCtrlModifier = mod & KMOD_CTRL;
			bool bShiftModifier = mod & KMOD_SHIFT;
			bool bAltModifier = mod & KMOD_ALT;

			if (event.type == SDL_KEYDOWN)
			{
				auto key = event.key.keysym.sym;

				if      (key == SDLK_s) GLFramebuffers::SaveScreenshot();
				else if (key == SDLK_f) turntable.SnapToOrigin();

				else if (key == SDLK_KP_7) SetCameraView(camera, turntable, CameraView::OrthographicY);
				else if (key == SDLK_KP_1) SetCameraView(camera, turntable, bCtrlModifier? CameraView::OrthographicZneg : CameraView::OrthographicZ);
				else if (key == SDLK_KP_9) SetCameraView(camera, turntable, CameraView::OrthographicYneg);
				else if (key == SDLK_KP_3) SetCameraView(camera, turntable, bCtrlModifier ? CameraView::OrthographicXneg : CameraView::OrthographicX);
				else if (key == SDLK_KP_5) SetCameraView(camera, turntable, CameraView::Perspective);
			}

			if (!ImGui::GetIO().WantCaptureMouse || interactingWithPreview)
			{
				auto& activeTurntable = interactingWithPreview? turntableCube : turntable;

				if (event.type == SDL_MOUSEBUTTONDOWN)
				{
					captureMouseX = event.motion.x;
					captureMouseY = event.motion.y;
					captureMouse = true;
					SDL_ShowCursor(0);
					SDL_SetRelativeMouseMode(SDL_TRUE);

					auto button = event.button.button;
					if (button == SDL_BUTTON_LEFT)			activeTurntable.inputState = TurntableInputState::Rotate;
					else if (button == SDL_BUTTON_MIDDLE)	activeTurntable.inputState = TurntableInputState::Translate;
					else if (button == SDL_BUTTON_RIGHT)	activeTurntable.inputState = TurntableInputState::Zoom;

					activeTurntable.OnBeginInput();
				}
				else if (event.type == SDL_MOUSEBUTTONUP)
				{
					captureMouse = false;
					SDL_ShowCursor(1);
					SDL_SetRelativeMouseMode(SDL_FALSE);
				}
				else if (event.type == SDL_MOUSEMOTION && captureMouse)
				{
					activeTurntable.ApplyMouseInput(-event.motion.xrel, event.motion.yrel);
					SDL_WarpMouseInWindow(App::window.GetSDLWindow(), captureMouseX, captureMouseY);
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
		
		// Set scene render properties
		glPolygonMode(GL_FRONT_AND_BACK, (renderWireframe? GL_LINE : GL_FILL));
		App::shaders.UpdateCameraUBO(camera);
		App::shaders.UpdateLightUBOPosition(lightFollowsCamera? camera.GetPosition() : glm::fvec3{ 999999.0f });

		// Debug: Test changing the mesh transform over time
		//headmesh.transform.rotation = glm::vec3(0.0f, 360.0f*sinf((float) App::clock.time), 0.0f);

		pointCloudShader.Use();
		pointCloudShader.SetUniformMat4("model", armesh.transform.ModelMatrix());
		pointCloudShader.SetUniformFloat("size", App::settings.pointCloudSize);
		armesh.Draw(GL_POINTS);

		meshShader.Use();
		meshShader.SetUniformMat4("model", headmesh.transform.ModelMatrix());
		meshShader.SetUniformInt("useTexture", 0);
		meshShader.SetUniformInt("useFlatShading", 0);
		DefaultTexture.UseForDrawing();
		headmesh.Draw();
			
		// Grid
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		if (camera.GetView() == CameraView::Perspective)
			App::geometry.grid.Draw(App::geometry.quad, camera.ViewProjectionMatrix());
		else
			App::geometry.grid.Draw(App::geometry.quad, camera.ViewProjectionMatrix(), camera.ForwardVector(), camera.SideVector());
		
		// Coordinate axis' xray on top of scene
		GLFramebuffers::ClearActiveDepth();
		lineShader.Use();
		App::geometry.coordinateAxis.Draw();

		lineShader.SetUniformMat4("model", cubeMeshNormals.transform.ModelMatrix());
		//cubeMeshNormals.Draw();

		if (auto F = GLFramebuffers::BindScoped(RenderTarget))
		{
			GLFramebuffers::ClearActive();
			App::shaders.UpdateCameraUBO(cameraCube);
			App::shaders.UpdateLightUBOPosition(lightFollowsCamera? cameraCube.GetPosition() : glm::fvec3{ 999999.0f });
			meshShader.SetUniformMat4("model", cubemesh.transform.ModelMatrix());
			meshShader.SetUniformInt("useTexture", 0);
			cubemesh.Draw();
		}

		// Done
		App::window.RenderImgui();
		App::window.SwapFramebuffer();
	}

	App::Shutdown();

	return 0;
}
