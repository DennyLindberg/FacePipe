#pragma once

#include "core/math.h"
#include "camera.h"

class Camera;

class Scene
{
public:
	Scene() {}
	~Scene() {}

	Camera camera;
};
