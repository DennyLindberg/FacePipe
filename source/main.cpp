#include "core/core.h"
#include "opengl/opengl.h"
#include "python/python.h"
#include "imnodes.h"

namespace fs = std::filesystem;

/*
	Program configurations
*/
const float CAMERA_FOV = 45.0f;

const ApplicationSettings settings = {
	.vsync = true,
	.fullscreen = 0,
	.windowWidth = 1280,
	.windowHeight = 720,
	.fpsLimit = 0,
	.windowRatio = 1280.0f / 720.0f,
	.contentPath = fs::current_path().parent_path() / "content"
};

const fs::path textureFolder = settings.contentPath / "textures";
const fs::path shaderFolder = settings.contentPath / "shaders";
const fs::path meshFolder = settings.contentPath / "meshes";
const fs::path curvesFolder = settings.contentPath / "curves";
const fs::path scriptsFolder = settings.contentPath / "scripts";

/*
	Application
*/
int main(int argc, char* args[])
{
	InitializeApplication(settings);

	UniformRandomGenerator uniformGenerator;
	ApplicationClock clock;

	OpenGLWindow window;
	window.SetTitle("FacePipe");
	window.SetClearColor(0.5f, 0.5f, 0.5f, 1.0f);

	GLuint defaultVao = 0;
	glGenVertexArrays(1, &defaultVao);
	glBindVertexArray(defaultVao);

	FileListener fileListener;
	fileListener.StartThread(curvesFolder);

	PythonInterpreter Python;
	PythonScript PythonTestScript(scriptsFolder / "test.py");

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

	GLQuad backgroundQuad;
	GLGrid grid;
	grid.size = 5.0f;
	grid.gridSpacing = 0.1f;

	TurntableController turntable(camera);
	turntable.position = glm::vec3{0.0f, 0.15f, 0.0f};
	turntable.sensitivity = 0.25f;
	turntable.Set(-65.0f, 15.0f, 1.0f);

	/*
		Load and initialize shaders
	*/
	GLTexture defaultTexture{ textureFolder / "default.png" };
	defaultTexture.UseForDrawing();

	// Uniform Buffer Object containing matrices and light information
	GLUBO CameraUBO, LightUBO;
	CameraUBO.Bind(1);
	CameraUBO.Allocate(16 * 8 + 16); // 2 matrices => 8 columns => 16 bytes per column, +vec3 16 bytes
	LightUBO.Bind(2);
	LightUBO.Allocate(16 * 2);

	// Change each LoadShader call to LoadLiveShader for live editing
	GLProgram lineShader, backgroundShader, headShader, bezierLinesShader;
	ShaderManager shaderManager;
	shaderManager.InitializeFolder(shaderFolder);
	shaderManager.LoadShader(lineShader, L"line_vertex.glsl", L"line_fragment.glsl");
	shaderManager.LoadShader(backgroundShader, L"background_vertex.glsl", L"background_fragment.glsl");

	shaderManager.LoadLiveShader(headShader, L"head_vertex.glsl", L"head_fragment.glsl", L"head_geometry.glsl");
	shaderManager.LoadLiveShader(bezierLinesShader, L"bezier_vertex.glsl", L"line_fragment.glsl", L"bezier_lines_geometry.glsl");

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
	GLTriangleMesh headmesh;
	GLMesh::LoadOBJ(meshFolder/"blender_suzanne.obj", headmesh);
	headmesh.transform.scale = glm::vec3(0.25f);

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
	window.imguiLayout = [&]() -> void {
		ImGui::SetNextWindowSize(ImVec2(settings.windowWidth * 0.25f, settings.windowHeight * 1.0f));
		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::Begin("Settings");
		{
			ImGui::Text("Scene");
			ImGui::Checkbox("Wireframe", &renderWireframe);
			ImGui::Checkbox("Light follows camera", &lightFollowsCamera);
			if (ImGui::Button("Run Python test script"))
			{
				if (Python.Execute(PythonTestScript) != PythonScriptError::None)
				{
					std::cout << Python.GetLastException().what() << std::endl;
				}
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
		if (settings.fpsLimit > 0 && clock.TimeSinceLastTick() < 1.0/settings.fpsLimit)
		{
			continue;
		}

		clock.Tick();
		SetThreadedTime(clock.tickTime);

		window.SetTitle("FPS: " + FpsString(clock.deltaTime));
		shaderManager.CheckLiveShaders();
		fileListener.ProcessCallbacksOnMainThread();

		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			quit = (event.type == SDL_QUIT) || (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE);
			if (quit) break;

			window.HandleImguiEvent(&event);

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
			else if (event.type == SDL_MOUSEBUTTONDOWN)
			{
				if (bAltModifier)
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

		/*
			Render scene
		*/
		window.Clear();

		// Background color gradient
		backgroundShader.Use();
		backgroundQuad.Draw();
		glClear(GL_DEPTH_BUFFER_BIT);
		
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
			//headmesh.transform.rotation = glm::vec3(0.0f, 360.0f*sinf((float) clock.tickTime), 0.0f);

			headShader.Use();
			headShader.SetUniformMat4("model", headmesh.transform.ModelMatrix());
			headmesh.Draw();
		}

		// Grid
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		grid.Draw(projectionmatrix * viewmatrix);
		
		// Clear depth so that we can draw lines on top of everything
		glClear(GL_DEPTH_BUFFER_BIT);
		
		// Coordinate axis'
		lineShader.Use();
		lineShader.SetUniformFloat("useUniformColor", false);
		coordinateReferenceLines.Draw();

		// Done
		window.RenderImgui();
		window.SwapFramebuffer();
	}

	exit(0);
}
