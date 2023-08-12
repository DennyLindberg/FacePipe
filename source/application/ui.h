#pragma once

#include "core/math.h"
#include "core/object.h"
#include "viewport.h"
#include "ui_logger.h"

class UIManager
{
public:
	bool fullscreenViewport = false;
	bool renderWireframe = false;
	bool lightFollowsCamera = false;
	bool drawDebugNormals = false;

	bool viewportCaptureMouse = false;
	int viewportCaptureMouseBeginX = 0;
	int viewportCaptureMouseBeginY = 0;

	WeakPtr<Object> selected_object;

	WeakPtr<Viewport> applicationViewport;
	WeakPtr<Viewport> activeViewport;
	WeakPtr<Viewport> sceneViewport;
	std::vector<WeakPtr<Viewport>> viewports;

public:
	bool displayQuitDialog = false;

public:
	UILoggerManager logging;

public:
	void Initialize();
	void Shutdown();

	bool HandleInputEvent(const void* event);
	void RenderUI();

	WeakPtr<Viewport> CreateViewport();

	void UpdateActiveViewport(WeakPtr<Viewport> viewport, bool bActiveByMouse);

	Viewport* GetActiveViewport();

public:
	bool HasKeyboardFocus() const;
	bool HasMouseFocus() const;

public:
	void HandleQuit();
};
