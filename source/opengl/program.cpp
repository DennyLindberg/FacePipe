#include "program.h"

#include <string>
#include <iostream>

GLuint GLProgram::activeProgramId = 0;

void GLUBO::Initialize()
{
	glGenBuffers(1, &uboId);
}

void GLUBO::Shutdown()
{
	glDeleteBuffers(1, &uboId);
}

void GLUBO::Bind(GLuint point)
{
	glBindBufferBase(GL_UNIFORM_BUFFER, point, uboId);
}

void GLUBO::Allocate(GLuint numbytes)
{
	glBindBuffer(GL_UNIFORM_BUFFER, uboId);
	glBufferData(GL_UNIFORM_BUFFER, numbytes, NULL, GL_STATIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	allocated_size = numbytes;
}

GLProgram::GLProgram()
{
	programId = glCreateProgram();
	fragment_shader_id = glCreateShader(GL_FRAGMENT_SHADER);
	vertex_shader_id = glCreateShader(GL_VERTEX_SHADER);
	// geometry_shader_id created on LoadGeometryShader (as it is optional)
}

GLProgram::~GLProgram()
{
	glDeleteProgram(programId);
	glDeleteShader(vertex_shader_id);
	glDeleteShader(fragment_shader_id);

	if (HasGeometryShader())
	{
		glDeleteShader(geometry_shader_id);
	}
}

void GLProgram::LoadFragmentShader(const std::string& shaderText)
{
	GLint sourceLength = (GLint)shaderText.size();
	const char *fragmentSourcePtr = shaderText.c_str();
	glShaderSource(fragment_shader_id, 1, &fragmentSourcePtr, &sourceLength);
}

void GLProgram::LoadVertexShader(const std::string& shaderText)
{
	GLint sourceLength = (GLint)shaderText.size();
	const char *vertexSourcePtr = shaderText.c_str();
	glShaderSource(vertex_shader_id, 1, &vertexSourcePtr, &sourceLength);
}

void GLProgram::LoadGeometryShader(const std::string& shaderText)
{
	if (!HasGeometryShader())
	{
		geometry_shader_id = glCreateShader(GL_GEOMETRY_SHADER);
	}

	GLint sourceLength = (GLint)shaderText.size();
	const char* geometrySourcePtr = shaderText.c_str();
	glShaderSource(geometry_shader_id, 1, &geometrySourcePtr, &sourceLength);
}

GLint CompileAndPrintStatus(GLuint glShaderId)
{
	GLint compileStatus = 0;
	glCompileShader(glShaderId);
	glGetShaderiv(glShaderId, GL_COMPILE_STATUS, &compileStatus);

	if (compileStatus == GL_FALSE)
	{
		std::string message("");

		int infoLogLength = 0;
		glGetShaderiv(glShaderId, GL_INFO_LOG_LENGTH, &infoLogLength);
		if (infoLogLength == 0)
		{
			message = "Message is empty (GL_INFO_LOG_LENGTH == 0)";
		}
		else
		{
			std::unique_ptr<GLchar[]> infoLog(new GLchar[infoLogLength]);
			int charsWritten = 0;
			glGetShaderInfoLog(glShaderId, infoLogLength, &charsWritten, infoLog.get());
			message = std::string(infoLog.get());
		}

		std::cout << "GL_INFO_LOG: " << message;
	}

	return compileStatus;
}

GLint GLProgram::LinkAndPrintStatus()
{
	glAttachShader(programId, vertex_shader_id);
	glAttachShader(programId, fragment_shader_id);
	if (HasGeometryShader())
	{
		glAttachShader(programId, geometry_shader_id);
	}
	glLinkProgram(programId);

	GLint linkStatus = 0;
	glGetProgramiv(programId, GL_LINK_STATUS, &linkStatus);
	if (linkStatus == GL_FALSE)
	{
		std::string message("");

		int infoLogLength = 0;
		std::unique_ptr<GLchar[]> infoLog(new GLchar[infoLogLength]);
		glGetProgramiv(programId, GL_INFO_LOG_LENGTH, &infoLogLength);
		if (infoLogLength == 0)
		{
			message = "Message is empty (GL_INFO_LOG_LENGTH == 0)";
		}
		else
		{
			std::unique_ptr<GLchar[]> infoLog(new GLchar[infoLogLength]);
			int charsWritten = 0;
			glGetProgramInfoLog(programId, infoLogLength, &charsWritten, infoLog.get());
			message = std::string(infoLog.get());
		}

		std::cout << "GL_INFO_LOG: " << message;
		return 0;
	} 

	glDetachShader(programId, vertex_shader_id);
	glDetachShader(programId, fragment_shader_id);
	if (HasGeometryShader())
	{
		glDetachShader(programId, geometry_shader_id);
	}

	return linkStatus;
}

void GLProgram::CompileAndLink()
{
	bool VertexShaderCompiled   = CompileAndPrintStatus(vertex_shader_id)   == GL_TRUE;
	bool FragmentShaderCompiled = CompileAndPrintStatus(fragment_shader_id) == GL_TRUE;
	bool GeometryShaderCompiled = true; // optional
	if (HasGeometryShader())
	{
		GeometryShaderCompiled = CompileAndPrintStatus(geometry_shader_id) == GL_TRUE;
	}

	if (!VertexShaderCompiled || !FragmentShaderCompiled || !GeometryShaderCompiled)
	{
		std::wcout << L"Failed to compile shaders\n";
	}
	else if (LinkAndPrintStatus() == GL_TRUE)
	{
		// These attributes are bound by default
		glBindAttribLocation(programId, 0, "vertexPosition");
		glBindAttribLocation(programId, 1, "vertexTCoord");

		ReloadUniforms();
	}
}

void GLProgram::Use()
{
	if (programId != activeProgramId)
	{
		activeProgramId = activeProgramId;
		glUseProgram(programId);
	}
}

GLuint GLProgram::Id()
{
	return programId;
}

void GLProgram::ReloadUniforms()
{
	Use();

	for (auto& u : intUniforms)
	{
		u.second.id = glGetUniformLocation(programId, u.first.c_str());
		u.second.Upload();
	}

	for (auto& u : floatUniforms)
	{
		u.second.id = glGetUniformLocation(programId, u.first.c_str());
		u.second.Upload();
	}

	for (auto& u : vec3Uniforms)
	{
		u.second.id = glGetUniformLocation(programId, u.first.c_str());
		u.second.Upload();
	}

	for (auto& u : vec4Uniforms)
	{
		u.second.id = glGetUniformLocation(programId, u.first.c_str());
		u.second.Upload();
	}

	for (auto& u : mat4Uniforms)
	{
		u.second.id = glGetUniformLocation(programId, u.first.c_str());
		u.second.Upload();
	}
}
