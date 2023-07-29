#include "geometrymanager.h"

void GeometryManager::Initialize()
{
	GLuint defaultVao = 0;
	glGenVertexArrays(1, &defaultVao);
	glBindVertexArray(defaultVao);

	coordinateAxis.Initialize();
	glm::fvec3 origin(0.0f);
	coordinateAxis.AddLine(origin, origin + glm::fvec3(1.0f, 0.0f, 0.0f), glm::fvec4(1.0f, 0.0f, 0.0f, 1.0f)); // x is red
	coordinateAxis.AddLine(origin, origin + glm::fvec3(0.0f, 1.0f, 0.0f), glm::fvec4(0.0f, 1.0f, 0.0f, 1.0f)); // y is green
	coordinateAxis.AddLine(origin, origin + glm::fvec3(0.0f, 0.0f, 1.0f), glm::fvec4(0.0f, 0.0f, 1.0f, 1.0f)); // z is blue
	coordinateAxis.SendToGPU();
}

void GeometryManager::Shutdown()
{
	coordinateAxis.Shutdown();
}
