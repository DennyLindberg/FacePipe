#pragma once

#include "core/objectpool.h"
#include "core/input.h"
#include "opengl/mesh.h"
#include "opengl/framebuffer.h"

class Viewport : public ObjectPoolInterface<Viewport, ObjectType_Viewport>
{
protected:
	static bool hasInitialViewport;

public:
	CameraController input;
	GLuint framebuffer = 0;
	WeakPtr<GLLine> debuglines;
	
	Viewport() {}
	~Viewport() {}

	void Initialize();
	void Destroy();

	void Clear(EGLFramebufferClear clear = EGLFramebufferClear::All);

	void UseForRendering(EGLFramebufferClear clear = EGLFramebufferClear::None);

	void RenderScoped(EGLFramebufferClear clear, std::function<void()> func);

	void Render(std::function<void(Viewport&)> func);

	void Resize(GLuint newWidth, GLuint newHeight);

	void HandleInputEvent(const void* eventPtr);
};
