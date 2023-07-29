#include "grid.h"
#include "application/application.h"

void GLGrid::Draw(GLQuad& mesh, const glm::mat4& mvp)
{
	const glm::mat4 planeRotationMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3{ 1.0f, 0.0f, 0.0f });
	glm::mat4 mvpOffset = mvp * planeRotationMatrix;
	
	auto& shader = App::shaders.gridShader;
	shader.Use();
	shader.SetUniformFloat("gridSpacing", gridSpacing);
	shader.SetUniformFloat("size", size);
	shader.SetUniformFloat("opacity", opacity);
	shader.SetUniformMat4("mvp", mvpOffset);
	mesh.Draw();
}
