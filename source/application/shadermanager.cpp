#include "shadermanager.h"
#include "opengl/camera.h"
#include "core/utilities.h"
#include "application/application.h"

namespace fs = std::filesystem;

void ShaderManager::Initialize(std::filesystem::path shaderFolder)
{
	rootFolder = shaderFolder;
	fileListener.Initialize(shaderFolder);
	
	cameraUBO.Initialize();
	cameraUBO.Bind(1);
	cameraUBO.Allocate(16 * 8 + 16); // 2 matrices => 8 columns => 16 bytes per column, +vec3 16 bytes

	lightUBO.Initialize();
	lightUBO.Bind(2);
	lightUBO.Allocate(16 * 2);

	// defaults until user changes it
	UpdateLightUBOPosition(glm::fvec3{ 999999.0f });
	UpdateLightUBOColor(glm::fvec4{ 1.0f });

	InitializeDefaultShaders();
}

void ShaderManager::InitializeDefaultShaders()
{
	screenspaceQuadShader.Initialize();
	screenspaceQuadShader.LoadVertexShader(R"(
		#version 330

		layout(location = 0) in vec3 vertexPosition;
		layout(location = 3) in vec4 vertexTCoord;

		uniform vec2 uPos;
		uniform vec2 uSize;
		uniform bool uFlipY;

		out vec2 TexCoords;

		void main()
		{
			// Scale based on center
			gl_Position.x = vertexPosition.x*uSize.x + uPos.x - 1.0f;
			gl_Position.y = vertexPosition.y*uSize.y - uPos.y + 1.0f;
			gl_Position.z = vertexPosition.z;
			gl_Position.w = 1.0f;

			if (uFlipY)
				TexCoords = vec2(vertexTCoord.x, vertexTCoord.y);
			else
				TexCoords = vec2(vertexTCoord.x, 1.0f - vertexTCoord.y);
		}
	)");
	screenspaceQuadShader.LoadFragmentShader(R"(
		#version 330

		in vec2 TexCoords;

		uniform sampler2D quadTexture;
		uniform float uOpacity;

		out vec4 FragColor; 

		void main()
		{
			FragColor = texture(quadTexture, TexCoords);
			FragColor.a *= uOpacity;
		}
	)");
	screenspaceQuadShader.CompileAndLink();
	glBindAttribLocation(screenspaceQuadShader.Id(), ShaderManager::positionAttribId, "vertexPosition");
	glBindAttribLocation(screenspaceQuadShader.Id(), ShaderManager::texCoordAttribId, "vertexTCoord");
	screenspaceQuadShader.SetUniformFloat("uOpacity", 1.0f);
	screenspaceQuadShader.SetUniformVec2("uPos", glm::fvec2(1.0f));
	screenspaceQuadShader.SetUniformVec2("uSize", glm::fvec2(1.0f));
	screenspaceQuadShader.SetUniformInt("uFlipY", 0);

	canvasShader.Initialize();
	std::string fragment, vertex;
	if (LoadText(App::Path("content/shaders/basic_fragment.glsl"), fragment) && LoadText(App::Path("content/shaders/basic_vertex.glsl"), vertex))
	{
		canvasShader.LoadFragmentShader(fragment);
		canvasShader.LoadVertexShader(vertex);
		canvasShader.CompileAndLink();
	}

	gridShader.Initialize();
	gridShader.LoadFragmentShader(R"glsl(
		#version 330

		in vec4 TCoord;
		in vec3 position;

		layout(location = 0) out vec4 color;
		uniform float gridSpacing;
		uniform float opacity;
			
		void main() 
		{
			// Antialiased grid, slightly modified
			// http://madebyevan.com/shaders/grid/

			float gridScaling = 0.5f/gridSpacing;
			vec2 lineCoords = position.xy * gridScaling;
			vec2 grid = abs(fract(lineCoords-0.5)-0.5) / fwidth(lineCoords);
			float lineMask = min(1.0, min(grid.x, grid.y));

			color = vec4(lineMask, lineMask, lineMask, lineMask*opacity);
		}
	)glsl");

	gridShader.LoadVertexShader(R"glsl(
		#version 330

		layout(location = 0) in vec3 vertexPosition;
		layout(location = 3) in vec4 vertexTCoord;
		uniform mat4 mvp;
		uniform float size;

		out vec4 TCoord;
		out vec3 position;

		void main()
		{
			gl_Position = mvp * vec4(vertexPosition*size, 1.0f);
			TCoord = vertexTCoord;
			position = vertexPosition*size;
		}
	)glsl");
	gridShader.CompileAndLink();
	glBindAttribLocation(gridShader.Id(), ShaderManager::positionAttribId, "vertexPosition");
	glBindAttribLocation(gridShader.Id(), ShaderManager::texCoordAttribId, "vertexTCoord");
}

void ShaderManager::Shutdown()
{
	canvasShader.Shutdown();
	screenspaceQuadShader.Shutdown();

	cameraUBO.Shutdown();
	lightUBO.Shutdown();
	fileListener.Shutdown();
}

void ShaderManager::LoadLiveShader(GLProgram& targetProgram, std::wstring vertexFilename, std::wstring fragmentFilename, std::wstring geometryFilename)
{
	LoadShader(targetProgram, vertexFilename, fragmentFilename, geometryFilename);

	fileListener.Bind(
		vertexFilename,
		[this, &targetProgram](fs::path filePath) -> void
		{
			this->UpdateShader(targetProgram, filePath, ShaderType::VERTEX);
		}
	);

	fileListener.Bind(
		fragmentFilename,
		[this, &targetProgram](fs::path filePath) -> void
		{
			this->UpdateShader(targetProgram, filePath, ShaderType::FRAGMENT);
		}
	);

	if (geometryFilename != L"")
	{
		fileListener.Bind(
			geometryFilename,
			[this, &targetProgram](fs::path filePath) -> void
			{
				this->UpdateShader(targetProgram, filePath, ShaderType::GEOMETRY);
			}
		);
	}
}

void ShaderManager::LoadShader(GLProgram& targetProgram, std::wstring vertexFilename, std::wstring fragmentFilename, std::wstring geometryFilename)
{
	std::string fragment, vertex, geometry;
	if (!LoadText(rootFolder/vertexFilename, vertex))
	{
		wprintf(L"\r\nFailed to read shader: %Ls\r\n", vertexFilename.c_str());
		return;
	}

	if (!LoadText(rootFolder/fragmentFilename, fragment))
	{
		wprintf(L"\r\nFailed to read shader: %Ls\r\n", fragmentFilename.c_str());
		return;
	}

	bool bShouldLoadGeometryShader = (geometryFilename != L"");
	if (bShouldLoadGeometryShader)
	{
		if (!LoadText(rootFolder/geometryFilename, geometry))
		{
			wprintf(L"\r\nFailed to read shader: %Ls\r\n", geometryFilename.c_str());
			return;
		}
	}

	targetProgram.Initialize();
	targetProgram.LoadFragmentShader(fragment);
	targetProgram.LoadVertexShader(vertex);
	if (bShouldLoadGeometryShader)
	{
		targetProgram.LoadGeometryShader(geometry);
	}
	targetProgram.CompileAndLink();
}

void ShaderManager::UpdateShader(GLProgram& targetProgram, fs::path filePath, ShaderType type)
{
	std::string text;
	if (!LoadText(filePath, text))
	{
		wprintf(L"\r\nUpdateShader failed to read file: %Ls\r\n", filePath.c_str());
		return;
	}

	//printf("\r\n=======\r\n%s\r\n=======\r\n\r\n", text.c_str());
	std::wstring MessageType = L"";
	switch (type)
	{
	case ShaderType::VERTEX:
	{
		MessageType = L"Vertex";
		targetProgram.LoadVertexShader(text);
		break;
	}
	case ShaderType::FRAGMENT:
	{
		MessageType = L"Fragment";
		targetProgram.LoadFragmentShader(text);
		break;
	}
	case ShaderType::GEOMETRY:
	{
		MessageType = L"Geometry";
		targetProgram.LoadGeometryShader(text);
		break;
	}
	}
	targetProgram.CompileAndLink();

	wprintf(
		L"\r\n%Ls shader updated: %Ls\r\n", 
		MessageType.c_str(),
		filePath.c_str()
	);
}

void ShaderManager::CheckLiveShaders()
{
	fileListener.ProcessCallbacksOnMainThread();
}

void ShaderManager::UpdateCameraUBO(Camera& camera)
{
	/*glm::mat4 viewmatrix = camera.ViewMatrix();
	glm::mat4 projectionmatrix = camera.ProjectionMatrix();*/
	cameraUBO.SetData(glm::value_ptr(camera.ProjectionMatrix()), 0, 64);
	cameraUBO.SetData(glm::value_ptr(camera.ViewMatrix()), 64, 64);
	cameraUBO.SetData(glm::value_ptr(camera.GetPosition()), 128, 16);
}

void ShaderManager::UpdateLightUBOPosition(const glm::fvec3& position)
{
	lightUBO.SetData(glm::value_ptr(position), 0, 12);
}

void ShaderManager::UpdateLightUBOColor(const glm::fvec4& color)
{
	lightUBO.SetData(glm::value_ptr(color), 16, 16);
}
