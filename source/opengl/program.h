#pragma once
#include <string>
#include <map>
#include "glad/glad.h"
#include "../core/math.h"

struct UniformInt
{
	GLint id = 0;
	GLint value = 0;

	void Upload()
	{
		glUniform1i(id, value);
	}
};

struct UniformFloat
{
	GLint id = 0;
	float value = 0.0f;

	void Upload()
	{
		glUniform1f(id, value);
	}
};

struct UniformVec2
{
	GLint id = 0;
	glm::fvec2 value = glm::fvec2{ 0.0f };

	void Upload()
	{
		glUniform2fv(id, 1, glm::value_ptr(value));
	}
};

struct UniformVec3
{
	GLint id = 0;
	glm::fvec3 value = glm::fvec3{0.0f};

	void Upload()
	{
		glUniform3fv(id, 1, glm::value_ptr(value));
	}
};

struct UniformVec4
{
	GLint id = 0;
	glm::fvec4 value = glm::fvec4{ 0.0f };

	void Upload()
	{
		glUniform4fv(id, 1, glm::value_ptr(value));
	}
};

struct UniformMat4
{
	GLint id = 0;
	glm::mat4 value = glm::mat4{ 1.0f };

	void Upload()
	{
		glUniformMatrix4fv(id, 1, GL_FALSE, glm::value_ptr(value));
	}
};

class GLUBO
{
protected:
	GLuint uboId = 0;
	GLuint allocated_size = 0;

public:
	GLUBO() {}
	~GLUBO() {}

	void Initialize();
	void Shutdown();

	// Set point to the same binding value as used in the glsl shader
	void Bind(GLuint point);

	void Allocate(GLuint numbytes);

	template<typename T>
	void SetData(T* data, GLuint offset, GLuint numbytes)
	{
		assert((offset+numbytes) <= allocated_size);

		glBindBuffer(GL_UNIFORM_BUFFER, uboId);
		glBufferSubData(GL_UNIFORM_BUFFER, offset, numbytes, data);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}

	/*
		See this article for a more efficient way of setting up
		an UBO with a struct and memcpy.
		https://www.geeks3d.com/20140704/gpu-buffers-introduction-to-opengl-3-1-uniform-buffers-objects/
	*/
};

class GLProgram
{
protected:
	static GLuint activeProgramId;

	GLuint programId = 0;
	GLint vertex_shader_id = 0;
	GLint fragment_shader_id = 0;
	GLint geometry_shader_id = -1; // optional

	std::map<std::string, UniformInt> intUniforms;
	std::map<std::string, UniformFloat> floatUniforms;
	std::map<std::string, UniformVec2> vec2Uniforms;
	std::map<std::string, UniformVec3> vec3Uniforms;
	std::map<std::string, UniformVec4> vec4Uniforms;
	std::map<std::string, UniformMat4> mat4Uniforms;

public:
	GLProgram();
	~GLProgram();

	bool HasGeometryShader() { return geometry_shader_id != -1; }
	void LoadFragmentShader(const std::string& shaderText);
	void LoadVertexShader(const std::string& shaderText);
	void LoadGeometryShader(const std::string& shaderText);
	GLint LinkAndPrintStatus();
	void CompileAndLink();
	void Use();
	GLuint Id();

	template<typename T, typename U>
	void SetUniform(const std::string& name, const T value, std::map<std::string, U>& uniformMap)
	{
		Use();

		auto it = uniformMap.find(name);
		if (it == uniformMap.end())
		{
			auto pair = uniformMap.emplace(name, U{
				.id = glGetUniformLocation(programId, name.c_str()),
				.value = value
			});
			it = pair.first;
		}

		it->second.value = value;
		it->second.Upload();
	}

	inline void SetUniformInt(const std::string& name, int value)			{ SetUniform<int, UniformInt>(name, value, intUniforms); }
	inline void SetUniformFloat(const std::string& name, float value)		{ SetUniform<float, UniformFloat>(name, value, floatUniforms); }
	inline void SetUniformVec2(const std::string& name, glm::fvec2 value)	{ SetUniform<glm::fvec2, UniformVec2>(name, value, vec2Uniforms); }
	inline void SetUniformVec3(const std::string& name, glm::fvec3 value)	{ SetUniform<glm::fvec3, UniformVec3>(name, value, vec3Uniforms); }
	inline void SetUniformVec4(const std::string& name, glm::fvec4 value)	{ SetUniform<glm::fvec4, UniformVec4>(name, value, vec4Uniforms); }
	inline void SetUniformMat4(const std::string& name, glm::mat4 value)	{ SetUniform<glm::mat4, UniformMat4>(name, value, mat4Uniforms); }

	void ReloadUniforms();
};
