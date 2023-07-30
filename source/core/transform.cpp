#include "transform.h"

glm::fvec3 Transform::GetDirectionVector(TransformDirection direction)
{
	switch (direction)
	{
		case TransformDirection::Forward:	{ return Transform::Forward; }
		case TransformDirection::Backward:	{ return Transform::Backward; }
		case TransformDirection::Left:		{ return Transform::Left; }
		case TransformDirection::Right:		{ return Transform::Right; }
		case TransformDirection::Up:		{ return Transform::Up; }
		case TransformDirection::Down:		{ return Transform::Down; }
		default:							{ return glm::fvec3(0.0f); }
	}
}

glm::fvec3 Transform::Position(glm::mat4 matrix, glm::fvec3 position)
{
	return matrix*glm::fvec4(position.x, position.y, position.z, 1.0f);
}

glm::fvec3 Transform::Vector(glm::mat4 matrix, glm::fvec3 vector)
{
	return matrix*glm::fvec4(vector.x, vector.y, vector.z, 0.0f);
}

glm::fvec3 Transform::Direction(glm::mat4 matrix, TransformDirection direction)
{
	return Transform::Vector(matrix, GetDirectionVector(direction));
}
