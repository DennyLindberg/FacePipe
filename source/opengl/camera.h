#pragma once
#include "core/math.h"
#include "core/objectpool.h"

enum class CameraView
{
	Perspective,
	OrthographicX,
	OrthographicY,
	OrthographicZ,
	OrthographicXneg,
	OrthographicYneg,
	OrthographicZneg
};

// This camera uses a position and a focus point to determine orientation.
// The getters and setters are used to ensure that the internals update.
class Camera
{
public:
	friend class ObjectPool<Camera>;
	static ObjectPool<Camera> Pool;

protected:
	ObjectId poolId = 0;

	glm::vec3 forwardVector = { 0.0f, 0.0f, 1.0f };
	glm::vec3 upVector = { 0.0f, 1.0f, 0.0f };
	glm::vec3 sideVector = { 1.0f, 0.0f, 0.0f };

	glm::vec3 position = glm::vec3{ 0.0f, 0.0f, 1.0f };
	glm::vec3 focusPoint = glm::vec3{ 0.0f };

	float orthographicZoom = 1.0f;
	CameraView view = CameraView::Perspective;

public:
	Transform transform;

	bool flipUpDirection = false;
	float fieldOfView = 90.0f;
	float nearClipPlane = 0.1f;
	float farClipPlane = 100.0f;

	Camera() = default;
	~Camera() = default;

	void Initialize() {}
	void Destroy() {}

	CameraView GetView() const { return view; }
	void SetView(CameraView newView);

	glm::fvec3 GetPosition()
	{
		return position;
	}

	void SetOrthographicZoom(float zoom)
	{
		orthographicZoom = zoom;
	}

	void SetPosition(const glm::vec3& newPosition)
	{
		position = newPosition;

		if (view != CameraView::Perspective)
		{
			focusPoint = newPosition;
		}

		UpdateVectors();
	}

	void SetFocusPoint(const glm::vec3& newFocusPoint)
	{
		focusPoint = newFocusPoint;

		if (view != CameraView::Perspective)
		{
			position = focusPoint;
		}

		UpdateVectors();
	}

	inline glm::mat4 ViewMatrix() const { return glm::lookAt(position, focusPoint, upVector); }

	glm::mat4 ProjectionMatrix() const;

	inline glm::mat4 ViewProjectionMatrix()
	{
		return ProjectionMatrix() * ViewMatrix();
	}

	inline glm::vec3 UpVector() { return upVector; }
	inline glm::vec3 SideVector() { return sideVector; }
	inline glm::vec3 ForwardVector() { return forwardVector; }

protected:
	void UpdateVectors();
};