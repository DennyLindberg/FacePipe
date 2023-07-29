#pragma once
#include "opengl/program.h"
#include "core/filelistener.h"
#include <string>
#include <filesystem>

enum class ShaderType
{
	VERTEX = 0,
	FRAGMENT = 1,
	GEOMETRY = 2,
	UNKNOWN = 3
};

class ShaderManager
{
protected:
	std::filesystem::path rootFolder;
	FileListener fileListener;

	GLUBO cameraUBO;
	GLUBO lightUBO;

public:
	static const GLuint positionAttribId = 0;
	static const GLuint normalAttribId = 1;
	static const GLuint colorAttribId = 2;
	static const GLuint texCoordAttribId = 3;

public:
	ShaderManager() = default;
	~ShaderManager() = default;

	void Initialize(std::filesystem::path shaderFolder);
	void InitializeDefaultShaders();
	void Shutdown();

	void LoadLiveShader(GLProgram& targetProgram, std::wstring vertexFilename, std::wstring fragmentFilename, std::wstring geometryFilename = L"");
	void LoadShader(GLProgram& targetProgram, std::wstring vertexFilename, std::wstring fragmentFilename, std::wstring geometryFilename = L"");
	void UpdateShader(GLProgram& targetProgram, std::filesystem::path filePath, ShaderType type);
	void CheckLiveShaders();

	void UpdateCameraUBO(class Camera& camera);
	void UpdateLightUBOPosition(const glm::fvec3& position);
	void UpdateLightUBOColor(const glm::fvec4& color);

public:
	GLProgram screenspaceQuadShader;
	GLProgram canvasShader;
	GLProgram gridShader;
};