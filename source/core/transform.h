#pragma once

#include "core/math.h"

struct Transform
{
	glm::fvec3 position{ 0.0f };
	glm::fvec3 rotation{ 0.0f };	// (x,y,z) = (pitch,yaw,roll) - clockwise rotation (right hand rule)
	glm::fvec3 scale{ 1.0f };

	inline glm::mat4 Matrix() const { return TranslateMatrix() * RotateMatrix() * ScaleMatrix(); }
	inline glm::mat4 TranslateMatrix() const { return glm::translate(glm::mat4{ 1.0f }, position); }
	inline glm::mat4 ScaleMatrix() const { return glm::scale(glm::mat4{ 1.0f }, scale); }
	inline glm::mat4 RotateMatrix() const { return glm::eulerAngleYXZ(rotation.y, rotation.x, rotation.z); }
};
