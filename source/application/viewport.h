#pragma once

#include "core/objectpool.h"
#include "core/input.h"

class Viewport : public ObjectPoolInterface<Viewport, ObjectType_Viewport>
{
protected:
	static bool hasInitialViewport;
	GLuint width = 0;
	GLuint height = 0;

public:
	CameraController input;
	GLuint framebuffer = 0;
	
	Viewport() {}
	~Viewport() {}

	void Initialize();
	void Destroy();

	void Resize(GLuint newWidth, GLuint newHeight);
};
