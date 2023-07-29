#include "core/application.h"

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

	GLuint defaultVao = 0;
	glGenVertexArrays(1, &defaultVao);
	glBindVertexArray(defaultVao);

	fs::path PythonTestScript = App::Path("content/scripts/test_cv2_webcam.py");

printf(R"(
====================================================================
	
    FacePipe

    Controls:
        Control the camera
        Alt + LMB: Rotate
        Alt + MMB: Move
        Alt + RMB: Zoom

        S:         Take screenshot
        F:         Re-center camera on origin			       
			       
        ESC:       Close the application

====================================================================
)");

	/*
		Setup scene and controls
	*/
	Camera camera;
	camera.fieldOfView = CAMERA_FOV;

	GLQuad quadMesh;
	GLGrid grid;
	grid.size = 5.0f;
	grid.gridSpacing = 0.1f;

	TurntableController turntable(camera);
	turntable.position = glm::vec3{0.0f, 0.15f, 0.0f};
	turntable.sensitivity = 0.25f;
	turntable.Set(-65.0f, 15.0f, 1.0f);

	GLuint RenderTarget = GLFramebuffers::Create(GLuint(settings.windowWidth*0.25f), GLuint(settings.windowHeight*0.25f), App::settings.clearColor);

	/*
		Load and initialize shaders
	*/
	GLTexture defaultTexture{ App::Path("content/textures/default.png") };
	defaultTexture.UseForDrawing();

	// Uniform Buffer Object containing matrices and light information
	GLUBO CameraUBO, LightUBO;
	CameraUBO.Bind(1);
	CameraUBO.Allocate(16 * 8 + 16); // 2 matrices => 8 columns => 16 bytes per column, +vec3 16 bytes
	LightUBO.Bind(2);
	LightUBO.Allocate(16 * 2);

	// Change each LoadShader call to LoadLiveShader for live editing
	GLProgram lineShader, backgroundShader, headShader, bezierLinesShader;
	App::shaders.LoadShader(lineShader, L"line_vertex.glsl", L"line_fragment.glsl");
	App::shaders.LoadShader(backgroundShader, L"background_vertex.glsl", L"background_fragment.glsl");

	App::shaders.LoadLiveShader(headShader, L"head_vertex.glsl", L"head_fragment.glsl");//, L"head_geometry.glsl");
	App::shaders.LoadLiveShader(bezierLinesShader, L"bezier_vertex.glsl", L"line_fragment.glsl", L"bezier_lines_geometry.glsl");

	// Initialize model values
	glm::mat4 identity_transform{ 1.0f };
	lineShader.Use();
	lineShader.SetUniformMat4("model", identity_transform);
	headShader.Use(); 
	headShader.SetUniformMat4("model", identity_transform);
	bezierLinesShader.Use();
	bezierLinesShader.SetUniformMat4("model", identity_transform);

	// Initialize light source in shaders
	glm::vec4 lightColor{ 1.0f, 1.0f, 1.0f, 1.0f };
	glm::vec3 lightPosition{ 999999.0f };
	LightUBO.SetData(glm::value_ptr(lightPosition), 0, 12);
	LightUBO.SetData(glm::value_ptr(lightColor), 16, 16);

	/*
		Load head mesh
	*/
	GLTriangleMesh cubemesh, headmesh;
	GLMesh::LoadPLY(App::Path("content/meshes/cube.ply"), cubemesh);
	GLMesh::LoadPLY(App::Path("content/meshes/blender_suzanne.ply"), headmesh);
	//GLMesh::LoadPLY(meshFolder/"ARFaceGeometry.ply", headmesh);
	cubemesh.transform.scale = glm::vec3(0.25f);
	headmesh.transform.scale = glm::vec3(0.25f);

	GLLine cubeMeshNormals;
	GLMesh::LoadLinesFromMeshNormals(cubemesh, cubeMeshNormals, 0.2f);
	cubeMeshNormals.transform.scale = cubemesh.transform.scale;

	/*
		Coordinate Axis Lines
	*/
	GLLine coordinateReferenceLines;
	GLMesh::AppendCoordinateAxis(
		coordinateReferenceLines, 
		glm::fvec3{ 0.0f, 0.0f, 0.0f }, 
		glm::fvec3{ 1.0f, 0.0f, 0.0f }, 
		glm::fvec3{ 0.0f, 1.0f, 0.0f }, 
		glm::fvec3{ 0.0f, 0.0f, 1.0f },
		0.1f
	);
	coordinateReferenceLines.SendToGPU();

	/*
		User interaction parameters in the UI
	*/
	bool renderHead = true;
	bool renderWireframe = false;
	bool renderBezierLines = false;
	bool lightFollowsCamera = false;
	bool drawDebugNormals = false;
	
	/*
		IMGUI callback
	*/
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
			if (ImGui::Button("Run Python test script"))
			{
				App::python.Execute(PythonTestScript);
			}

			ImGui::InputTextMultiline( "ScriptInput", &input_field_string, ImVec2(0.0f, 200.0f) );
			if (ImGui::Button("Execute"))
			{
				App::python.Execute(input_field_string);
			}

			GLuint Texture, TextureWidth, TextureHeight;
			if (GLFramebuffers::GetTexture(RenderTarget, Texture, TextureWidth, TextureHeight))
			{
				ImGui::Image((ImTextureID)(intptr_t)Texture, ImVec2((float)TextureWidth, (float)TextureHeight), {0, 1}, {1, 0});
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
			App::window.HandleImguiEvent(&event);
			if (ImGui::GetIO().WantCaptureKeyboard)
			{
				continue;
			}

			quit = (event.type == SDL_QUIT) || (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE);

			SDL_Keymod mod = SDL_GetModState();
			bool bCtrlModifier = mod & KMOD_CTRL;
			bool bShiftModifier = mod & KMOD_SHIFT;
			bool bAltModifier = mod & KMOD_ALT;

			if (event.type == SDL_KEYDOWN)
			{
				auto key = event.key.keysym.sym;

				if      (key == SDLK_s) TakeScreenshot("screenshot.png", settings.windowWidth, settings.windowHeight);
				else if (key == SDLK_f) turntable.SnapToOrigin();
			}

			if (!ImGui::GetIO().WantCaptureMouse)
			{
				if (event.type == SDL_MOUSEBUTTONDOWN)
				{
					//if (bAltModifier)
					{
						captureMouse = true;
						SDL_ShowCursor(0);
						SDL_SetRelativeMouseMode(SDL_TRUE);

						auto button = event.button.button;
						if (button == SDL_BUTTON_LEFT)   turntable.inputState = TurntableInputState::Rotate;
						else if (button == SDL_BUTTON_MIDDLE) turntable.inputState = TurntableInputState::Translate;
						else if (button == SDL_BUTTON_RIGHT)  turntable.inputState = TurntableInputState::Zoom;

						turntable.OnBeginInput();
					}
				}
				else if (event.type == SDL_MOUSEBUTTONUP)
				{
					captureMouse = false;
					SDL_ShowCursor(1);
					SDL_SetRelativeMouseMode(SDL_FALSE);
				}
				else if (event.type == SDL_MOUSEMOTION && captureMouse)
				{
					turntable.ApplyMouseInput(-event.motion.xrel, event.motion.yrel);
				}
			}
		}

		/*
			Render scene
		*/
		GLFramebuffers::ClearActive();

		// Background color gradient
		backgroundShader.Use();
		quadMesh.Draw();
		GLFramebuffers::ClearActiveDepth();
		
		// Set scene render properties
		glPolygonMode(GL_FRONT_AND_BACK, (renderWireframe? GL_LINE : GL_FILL));
		glm::mat4 viewmatrix = camera.ViewMatrix();
		glm::mat4 projectionmatrix = camera.ProjectionMatrix();
		CameraUBO.SetData(glm::value_ptr(projectionmatrix), 0, 64);
		CameraUBO.SetData(glm::value_ptr(viewmatrix), 64, 64);
		CameraUBO.SetData(glm::value_ptr(camera.GetPosition()), 128, 16);

		// Update light source
		LightUBO.SetData(glm::value_ptr(lightFollowsCamera? camera.GetPosition() : lightPosition), 0, 12);

		if (renderHead)
		{
			// Debug: Test changing the mesh transform over time
			//headmesh.transform.rotation = glm::vec3(0.0f, 360.0f*sinf((float) App::clock.time), 0.0f);
			
			headShader.Use();
			headShader.SetUniformMat4("model", headmesh.transform.ModelMatrix());
			headmesh.Draw();

			if (auto F = GLFramebuffers::BindScoped(RenderTarget))
			{
				GLFramebuffers::ClearActive();
				cubemesh.Draw();
			}
		}

		// Grid
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		grid.Draw(quadMesh, projectionmatrix * viewmatrix);
		
		// Coordinate axis' xray on top of scene
		GLFramebuffers::ClearActiveDepth();
		lineShader.Use();
		lineShader.SetUniformFloat("useUniformColor", false);
		coordinateReferenceLines.Draw();

		//lineShader.SetUniformMat4("model", cubeMeshNormals.transform.ModelMatrix());
		//cubeMeshNormals.Draw();

		// Done
		App::window.RenderImgui();
		App::window.SwapFramebuffer();
	}

	App::Shutdown();

	return 0;
}
