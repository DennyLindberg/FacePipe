#pragma once

#include "core/objectpool.h"
#include "core/input.h"
#include "opengl/framebuffer.h"

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

	void Clear(EGLFramebufferClear clear = EGLFramebufferClear::All);

	void UseForRendering(EGLFramebufferClear clear = EGLFramebufferClear::None);

	void RenderScoped(EGLFramebufferClear clear, std::function<void()> func);

	void Resize(GLuint newWidth, GLuint newHeight);
};
