#pragma once

#include "core/math.h"

enum TransformDirection
{
	Forward,
	Backward,
	Left,
	Right,
	Up,
	Down
};

struct Transform
{
	static inline const glm::fvec3 Forward = glm::fvec3(0.0f, 0.0f, 1.0f);
	static inline const glm::fvec3 Backward = glm::fvec3(0.0f, 0.0f, -1.0f);
	static inline const glm::fvec3 Left = glm::fvec3(1.0f, 0.0f, 0.0f);
	static inline const glm::fvec3 Right = glm::fvec3(-1.0f, 0.0f, 0.0f);
	static inline const glm::fvec3 Up = glm::fvec3(0.0f, 1.0f, 0.0f);
	static inline const glm::fvec3 Down = glm::fvec3(0.0f, -1.0f, 0.0f);

	glm::fvec3 position{ 0.0f };
	glm::fvec3 rotation{ 0.0f };	// (x,y,z) = (pitch,yaw,roll) - clockwise rotation (right hand rule)
	glm::fvec3 scale{ 1.0f };

	static glm::fvec3 GetDirectionVector(TransformDirection direction);
	static glm::fvec3 Position(glm::mat4 matrix, glm::fvec3 position = glm::fvec3(0.0f));
	static glm::fvec3 Vector(glm::mat4 matrix, glm::fvec3 vector = Transform::Forward);
	static glm::fvec3 Direction(glm::mat4 matrix, TransformDirection direction);

	inline glm::mat4 Matrix() const { return TranslateMatrix() * RotateMatrix() * ScaleMatrix(); }
	inline glm::mat4 TranslateMatrix() const { return glm::translate(glm::mat4{ 1.0f }, position); }
	inline glm::mat4 ScaleMatrix() const { return glm::scale(glm::mat4{ 1.0f }, scale); }
	inline glm::mat4 RotateMatrix() const { return glm::eulerAngleYXZ(rotation.y, rotation.x, rotation.z); }
};
