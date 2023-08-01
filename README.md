# FacePipe
Application for processing face tracking data from various sources and exposing them to other applications.

Created by Denny Lindberg and Lisa Gren.

# Platform

- C++17
- For Windows
- OpenGL 4.6

# Building the code

Get **premake5** for generating makefiles or solutions for your IDE - https://premake.github.io

After downloading it, place the `premake5.exe` in the root folder of the project (... or place it in `Windows/System32` to install it globally on the system). Run `premake5 vs2022` in the terminal or command line to generate a Visual Studio solution (the solution ends up in the temp folder). Open the solution and you're good to go.

# Third party libraries used

**glad** for OpenGL bindings - https://github.com/Dav1dde/glad

**SDL2** for creating a cross-platform OpenGL window with input - https://www.libsdl.org - 2.26.x release

**glm** for vector and matrix data types compatible with OpenGL - https://glm.g-truc.net/0.9.9/index.html

**lodepng** for loading and saving PNG files - https://lodev.org/lodepng/

**tinyobjloader** for loading obj files - https://github.com/syoyo/tinyobjloader

**Dear ImGUI** for the UI - https://github.com/ocornut/imgui

# Algorithms copied or referenced
**Various GLSL noise algorithms** - Copied: https://gist.github.com/patriciogonzalezvivo/670c22f3966e662d2f83

**Evan's grid shader** - Slightly modified: http://madebyevan.com/shaders/grid/

## Folder structure

**binaries/** - contains compiled executable, dlls, images, configs or audio. (screenshots end up here)

**content/** - mesh, curves, textures and shader files.

**include/** - thirdparty includes.

**libs/** - windows specific libs.

**source/** - main folder for source code.

**temp/** - this folder is generated by premake5 and contains the solution. This folder can be deleted at any time.