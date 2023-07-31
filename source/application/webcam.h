#pragma once

#include "core/core.h"
#include "opengl/opengl.h"
#include <thread>
#include <mutex>

class WebCam
{
protected:
	std::thread thread;
	std::atomic<bool> bRunThread = false;
	std::atomic<bool> bStarted = false;
	std::atomic<bool> bTextureDirty = true;
	std::mutex mutex;

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

	void Thread_Loop();

	void Thread_CaptureFrame();
	
	void UpdateTextureWhenDirty();

	std::string DebugString();
};
