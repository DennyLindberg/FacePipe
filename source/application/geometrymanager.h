#pragma once

#include "opengl/mesh.h"

class GeometryManager
{
public:
	GeometryManager() {}
	~GeometryManager() {}

	void Initialize();
	void Shutdown();

public:
	GLLine coordinateAxis;
};
