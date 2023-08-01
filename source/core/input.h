#pragma once

#include "math.h"
#include "objectptr.h"
#include "opengl/camera.h"

enum class TurntableInputState
{
	Rotate,
	Zoom,
	Translate,
	COUNT
};

class CameraController
{
protected:
	float yaw = 0.0f;
	float pitch = 0.0f;
	float distance = 1.0f;
	bool bFlipYaw = false; // used to reverse direction when up-side down

public:
	WeakPtr<Camera> camera;
	glm::vec3 turntablePivot = glm::vec3{ 0.0f };
	bool clampPitch = false;
	
	TurntableInputState inputState = TurntableInputState::Rotate;

	CameraController() {}
	~CameraController() {};

	void Initialize();
	void Shutdown();

	void SetCameraView(CameraView view);

	void OnBeginInput();
	void SetDistance(float newDistance);
	void Set(float newYaw, float newPitch, float newDistance);
	void Offset(float yawOffset, float pitchOffset, float distanceOffset);
	void ApplyMouseInput(int deltaX, int deltaY, float sensitivity);
	void SnapToOrigin();

protected:
	void UpdateCamera();
};