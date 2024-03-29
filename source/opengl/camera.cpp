#include "camera.h"
#include "application/application.h"

void Camera::Initialize()
{
	fieldOfView = App::settings.defaultCameraFOV;
}

void Camera::Destroy()
{

}

void Camera::SetView(CameraView newView)
{
	if (view == newView)
		return;
	view = newView;

	UpdateVectors();
}

glm::mat4 Camera::ProjectionMatrix(float aspectRatio) const
{
	if (view == CameraView::Perspective)
	{
		glm::mat4 m = glm::perspective(
			glm::radians(fieldOfView),
			aspectRatio,
			nearClipPlane, farClipPlane
		);

		if (!App::settings.maintainVerticalFOV)
		{
			// Quickfix
			// https://community.khronos.org/t/horizontal-vertical-fov-glm-projection-matrix/73854
			float a = m[0][0];
			float b = m[1][1];
			m[0][0] = b;
			m[1][1] = a;
		}

		return m;
	}
	else
	{
		return glm::ortho(
			-orthographicZoom*aspectRatio,
			orthographicZoom*aspectRatio,
			-orthographicZoom,
			orthographicZoom,
			-farClipPlane, farClipPlane
		);
	}
}

void Camera::UpdateVectors()
{
	// Note that the direction is where we are looking FROM. This means the forward vector is inverted.
	// The directions are set to match Blender.
	const float OrthoDistance = 50.0f;
	const glm::fvec3 x(1.0f, 0.0f, 0.0f);
	const glm::fvec3 y(0.0f, 1.0f, 0.0f);
	const glm::fvec3 z(0.0f, 0.0f, 1.0f);

	if (view != CameraView::Perspective)
	{
		position = focusPoint;
	}

	switch (view)
	{
		case CameraView::OrthographicX:
		{
			focusPoint.x = 0.0f;
			position.x = OrthoDistance;
			forwardVector = -x;
			upVector = y;
			sideVector = -z;
			break;
		}
		case CameraView::OrthographicY:
		{
			focusPoint.y = 0.0f;
			position.y = OrthoDistance;
			forwardVector = -y;
			upVector = -z;
			sideVector = x;
			break;
		}
		case CameraView::OrthographicZ:
		{
			focusPoint.z = 0.0f;
			position.z = OrthoDistance;
			forwardVector = -z;
			upVector = y;
			sideVector = x;
			break;
		}
		case CameraView::OrthographicXneg:
		{
			focusPoint = position;
			focusPoint.x = 0.0f;
			position.x = -OrthoDistance;
			forwardVector = x;
			upVector = y;
			sideVector = z;
			break;
		}
		case CameraView::OrthographicYneg:
		{
			focusPoint.y = 0.0f;
			position.y = -OrthoDistance;
			forwardVector = y;
			upVector = z;
			sideVector = x;
			break;
		}
		case CameraView::OrthographicZneg:
		{
			focusPoint.z = 0.0f;
			position.z = -OrthoDistance;
			forwardVector = z;
			upVector = y;
			sideVector = -x;
			break;
		}
		case CameraView::Perspective:
		default: 
		{
			forwardVector = glm::normalize(focusPoint - position);

			glm::vec3 worldUp = glm::vec3(0.0f, flipUpDirection ? -1.0f : 1.0f, 0.0f);
			if (!glm::all(glm::equal(forwardVector, worldUp)))
			{
				sideVector = glm::cross(forwardVector, worldUp);
				upVector = glm::cross(sideVector, forwardVector);

				sideVector = glm::normalize(sideVector);
				upVector = glm::normalize(upVector);
			}
		}
	}
}
