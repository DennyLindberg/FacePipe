#pragma once

#include "core/math.h"
#include "core/object.h"

class UIManager
{
public:
	GLuint previewFramebuffer = 0;

	bool interactingWithPreview = false;
	bool renderWireframe = false;
	bool lightFollowsCamera = false;
	bool drawDebugNormals = false;

	bool viewportCaptureMouse = false;
	int viewportCaptureMouseBeginX = 0;
	int viewportCaptureMouseBeginY = 0;

	WeakPtr<Object> selected_object;

public:
	void Initialize();
	void Shutdown();

	bool HandleInputEvent(const void* event);
	void DrawUI();

public:
	bool HasKeyboardFocus() const;
	bool HasMouseFocus() const;
};
