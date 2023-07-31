#include "objectpool.h"
#include <array>
#include <iostream>

static std::array<std::function<void()>, ObjectTypeMax> RegisteredPools;

void ObjectPoolInternals::RegisterPool(ObjectType type, std::function<void()> emptyPoolFunc)
{
	//std::cout << "Registering pool " << uint32_t(type) << std::endl;
	RegisteredPools[type] = emptyPoolFunc;
}

void ObjectPoolInternals::ShutdownPools()
{
	for (size_t i=0; i<ObjectTypeMax; i++)
	{
		if (RegisteredPools[i])
		{
			//std::cout << "Emptying pool " << i << std::endl;
			RegisteredPools[i]();
		}
	}
}
