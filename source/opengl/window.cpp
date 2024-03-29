#include "window.h"
#include "glad/glad.h"
#include "SDL2/SDL_syswm.h"
#include <string>

// IMGUI support
#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_opengl3.h"

#include "imnodes.h"
#include "framebuffer.h"

#include "application/application.h"

extern "C" {
	/*
		Laptops with discrete GPUs tend to auto-select the integrated graphics instead of the
		discrete GPU. (such as Nvidia or AMD)

		These declarations tell the GPU driver to pick the discrete GPU if available.

		https://gist.github.com/statico/6809850727c708f08458
		http://developer.download.nvidia.com/devzone/devcenter/gamegraphics/files/OptimusRenderingPolicies.pdf
		http://developer.amd.com/community/blog/2015/10/02/amd-enduro-system-for-developers/
	*/
	__declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;			// Nvidia
	__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;		// AMD
}

std::function<void(EGLWindowEvent, int, int)> OpenGLWindow::OnWindowChanged = [](auto&&...) {};

// window proc override for SDL2
#define SIZEMOVETIMER 100
WNDPROC sdl2WndProc;
LRESULT CALLBACK FacepipeWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) 
{
	// We use a event timer to keep pumping updates while moving or resizing the window,
	// otherwise the window freezes until the user lets go of their input.
	// Related but not needed: WM_MOVE, WM_PAINT

	switch (uMsg) 
	{
	case WM_ENTERSIZEMOVE:
	{
		SetTimer(hWnd, SIZEMOVETIMER, 1, NULL);
		break;
	}
	case WM_EXITSIZEMOVE:
	{
		KillTimer(hWnd, SIZEMOVETIMER);
		break;
	}
	case WM_TIMER:
	{
		if (wParam == SIZEMOVETIMER)
		{
			OpenGLWindow::OnWindowChanged(EGLWindowEvent::SizeMoveTimer, 0, 0);
			return 0;
		}
		break;
	}
	case WM_SIZE:
	{
		OpenGLWindow::OnWindowChanged(EGLWindowEvent::Resize, LOWORD(lParam),  HIWORD(lParam));
		break;
	}
	default: {}
	}

	// Trickle events to SDL2
	return CallWindowProc(sdl2WndProc, hWnd, uMsg, wParam, lParam);
}

void InitIMGUI(SDL_Window* window, SDL_GLContext gl_context)
{
	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();

	ImNodes::CreateContext();
	ImNodes::StyleColorsDark();

	// IO is only used for setting the config flags
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	//io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

	// Setup Platform/Renderer bindings
	//SDL_Window* window = SDL_CreateWindow("Dear ImGui SDL2+OpenGL3 example", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, window_flags);
	//SDL_GLContext gl_context = SDL_GL_CreateContext(window);
	ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
	ImGui_ImplOpenGL3_Init(nullptr);
}

// Called before SDL shuts down
void ShutdownIMGUI()
{
	ImNodes::DestroyContext();
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();
}

void OpenGLWindow::SetTitle(std::string newCaption)
{
	SDL_SetWindowTitle(window, newCaption.c_str());
}

void OpenGLWindow::SwapFramebuffer()
{
	SDL_GL_SwapWindow(window);
}

void OpenGLWindow::HandleImguiEvent(const SDL_Event* event)
{
	ImGui_ImplSDL2_ProcessEvent(event);

	//if (event->type == SDL_WINDOWEVENT && event->window.event == SDL_WINDOWEVENT_RESIZED)
	//{
	//	onWindowResized(event->window.data1, event->window.data2);
	//}
}

void OpenGLWindow::RenderImgui()
{
	{
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplSDL2_NewFrame(window);
		ImGui::NewFrame();
	}

	if (App::settings.showImguiDemo)
	{
		ImGui::ShowDemoWindow();
	}
	else
	{
		drawImguiCallback();
	}

	{
		ImGui::Render();
		//glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
		//glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
		//glClear(GL_COLOR_BUFFER_BIT);
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	}
}

void OpenGLWindow::Initialize(int width, int height, bool fullscreen, bool vsync, bool showConsole)
{
	auto sdl_die = [](const char* message) {
		fprintf(stderr, "%s: %s\n", message, SDL_GetError());
		exit(2);
	};

	// Initialize SDL 
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		sdl_die("Couldn't initialize SDL");
	}

	atexit(SDL_Quit);
	SDL_GL_LoadLibrary(NULL); // Default OpenGL is fine.

	// Request an OpenGL context (should be core)
	//SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	//SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
	//SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 16);

	// Create the window
	if (fullscreen)
	{
		window = SDL_CreateWindow(
			"",
			SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
			0, 0, SDL_WINDOW_FULLSCREEN_DESKTOP | SDL_WINDOW_OPENGL
		);
	}
	else
	{
		window = SDL_CreateWindow(
			"",
			SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
			width, height, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE
		);
	}
	if (window == NULL) sdl_die("Couldn't set video mode");

	HWND consoleWindow = GetConsoleWindow();
	ShowWindow(consoleWindow, (int) (showConsole && !fullscreen)); // 0 = SW_HIDE, 1 = SW_SHOW

	maincontext = SDL_GL_CreateContext(window);

	if (maincontext == NULL)
	{
		sdl_die("Failed to create OpenGL context");
	}

	// Check OpenGL properties
	printf("OpenGL loaded\n");
	gladLoadGLLoader(SDL_GL_GetProcAddress);
	printf("Vendor:   %s\n", glGetString(GL_VENDOR));
	printf("Renderer: %s\n", glGetString(GL_RENDERER));
	printf("Version:  %s\n", glGetString(GL_VERSION));

	// Use v-sync
	SDL_GL_SetSwapInterval(vsync ? 1 : 0);

	glEnable(GL_DEPTH_TEST);
	//glEnable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	//glEnable(GL_MULTISAMPLE);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	int w, h;
	SDL_GetWindowSize(window, &w, &h);
	glViewport(0, 0, w, h);
	glScissor(0, 0, w, h);

	InitIMGUI(window, maincontext);

	// Everything is ready - now hook the Win32 events
	SDL_SysWMinfo wmInfo;
	SDL_VERSION(&wmInfo.version);
	if (SDL_GetWindowWMInfo(window, &wmInfo)) 
	{
		sdl2WndProc = (WNDPROC)SetWindowLongPtr(wmInfo.info.win.window, GWLP_WNDPROC, (LONG_PTR)FacepipeWndProc);
	}
}

void OpenGLWindow::Destroy()
{
	ShutdownIMGUI();
	if (window)
	{
		if (sdl2WndProc) 
		{
			SDL_SysWMinfo wmInfo;
			SDL_GetWindowWMInfo(window, &wmInfo);
			SetWindowLongPtr(wmInfo.info.win.window, GWLP_WNDPROC, (LONG_PTR)sdl2WndProc);
		}

		SDL_GL_DeleteContext(maincontext);
		SDL_DestroyWindow(window);
	}
}

