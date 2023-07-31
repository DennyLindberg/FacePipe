#pragma once

#include "core/core.h"
#include "opengl/opengl.h"

class WebCam
{
protected:
	bool bStarted = false;
	void* cameraPtr = nullptr;
	GLTexture texture;
	std::vector<GLubyte> buffer;

public:
	WebCam();

	~WebCam();

	void Initialize();

	void Shutdown();

	int TextureWidth() { return texture.width; }

	int TextureHeight() { return texture.height; }

	GLuint Texture() { return texture.textureId; }

	bool IsActive() const { return bStarted; }

	void Start();

	void Stop();

	void CaptureFrame();

	std::string DebugString();
};
