#include "camera.h"
#include "application/application.h"

glm::mat4 Camera::ProjectionMatrix() const
{
	return glm::perspective(
		glm::radians(fieldOfView),
		App::settings.windowRatio,
		nearClipPlane, farClipPlane
	);
}
