#include "input.h"

CameraController::CameraController(WeakPtr<Camera> cam)
	: camera(cam)
{
}

void CameraController::SetCameraView(CameraView view)
{
	camera->SetView(view);
	SetDistance(view == CameraView::Perspective? 1.0f : 1.0f);
}

void CameraController::OnBeginInput()
{
	bFlipYaw = camera->flipUpDirection;
}

void CameraController::SetDistance(float newDistance)
{
	distance = newDistance;
	UpdateCamera();
}

void CameraController::Set(float newYaw, float newPitch, float newDistance)
{
	yaw = newYaw;
	pitch = newPitch;
	distance = newDistance;
	UpdateCamera();
}

void CameraController::Offset(float yawOffset, float pitchOffset, float distanceOffset)
{
	yaw += bFlipYaw? -yawOffset : yawOffset;
	pitch += pitchOffset;
	distance += distanceOffset;
	UpdateCamera();
}

void CameraController::ApplyMouseInput(int deltaX, int deltaY)
{
	float relativeSensitivity = 0.1f*log(1.0f + distance);

	switch (inputState)
	{
	case TurntableInputState::Zoom:
	{
		float normalizedMovement = (deltaX - deltaY) / 2.0f;
		Offset(0.0f, 0.0f, normalizedMovement*sensitivity*relativeSensitivity);
		break;
	}
	case TurntableInputState::Translate:
	{
		glm::vec3 inputOffset = camera->UpVector()*float(deltaY) + camera->SideVector()*float(deltaX);
		if (camera->GetView() == CameraView::Perspective)
			inputOffset *= 0.1f*sensitivity*relativeSensitivity;
		else
			inputOffset *= 0.01f*sensitivity*distance;
		turntablePivot += inputOffset;
		UpdateCamera();
		break;
	}
	case TurntableInputState::Rotate:
	default:
	{
		Offset(deltaX*sensitivity, deltaY*sensitivity, 0.0f);
	}
	}
}

void CameraController::SnapToOrigin()
{
	turntablePivot = glm::vec3{ 0.0f };
	UpdateCamera();
}

glm::vec3 PlacementVector(float yaw, float pitch)
{
	// Angle is inverted to create a right handed system
	float yawRad = -yaw * Math::Pi / 180.0f;
	float pitchRad = pitch * Math::Pi / 180.0f;

	float b = cosf(pitchRad);
	return glm::vec3{
		b*cosf(yawRad),
		sinf(pitchRad),
		b*sinf(yawRad)
	};
}

void CameraController::UpdateCamera()
{
	if (camera->GetView() == CameraView::Perspective)
	{
		yaw = fmod(yaw, 360.0f);
		pitch = fmod(pitch, 360.0f);

		if (distance < camera->nearClipPlane)
		{
			distance = camera->nearClipPlane;
		}

		if (clampPitch)
		{
			if (pitch > 90.0f)  pitch = 90.0f;
			if (pitch < -90.0f) pitch = -90.0f;
		}
		else
		{
			camera->flipUpDirection = (abs(pitch) >= 90.0f && abs(pitch) <= 270.0f);
		}

		camera->SetPosition(turntablePivot + PlacementVector(yaw, pitch) * distance);
		camera->SetFocusPoint(turntablePivot);
	}
	else
	{
		camera->SetPosition(turntablePivot);
		camera->SetOrthographicZoom(distance);
	}
}

