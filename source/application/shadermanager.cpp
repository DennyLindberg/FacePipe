#include "shadermanager.h"
#include "opengl/camera.h"
#include "core/utilities.h"

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
}

void ShaderManager::Shutdown()
{
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
