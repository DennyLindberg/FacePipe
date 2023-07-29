#pragma once
#include "mesh.h"
#include "program.h"
#include <memory>

class GLGrid
{
protected:
	GLProgram gridShaderProgram;

	GLuint mvpUniform = 0;
	GLuint gridUniform = 0;
	GLuint sizeUniform = 0;
	GLuint opacityUniform = 0;

public:
	float size = 1.0f;
	float gridSpacing = 0.5f;
	float opacity = 0.2f;

	GLGrid();
	~GLGrid() = default;

	void Draw(GLQuad& mesh, const glm::mat4& mvp);
};