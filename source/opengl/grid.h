#pragma once
#include "mesh.h"
#include "program.h"
#include <memory>

class GLGrid
{
public:
	float size = 1.0f;
	float gridSpacing = 0.5f;
	float opacity = 0.2f;

	GLGrid() {}
	~GLGrid() {};

	void Draw(GLQuad& mesh, const glm::mat4& mvp);
};
