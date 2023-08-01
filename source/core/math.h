#pragma once

#include "glad/glad.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/euler_angles.hpp"

typedef glm::ivec4 Color;
typedef glm::vec4 FColor;

namespace Math
{
	template<typename T>
	constexpr T tPi = T(3.14159265358979323846264338327950288);

	constexpr double dPi = tPi<double>;
	constexpr double dTwoPi = 2.0*tPi<double>;
	constexpr float Pi = tPi<float>;
	constexpr float TwoPi = 2.0f*tPi<float>;
}
