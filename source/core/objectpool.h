#pragma once

#include <stdint.h>
#include <limits>
#include <vector>
#include <cassert>

#include "objectptr.h"

namespace ObjectPoolInternals
{
	void RegisterPool(ObjectType type, std::function<void()> emptyPoolFunc);
	void ShutdownPools();
}

template<typename T, size_t OT>
class ObjectPool
{
protected:
	static const ObjectType Type = ObjectType(OT);
	static bool bIsRegistered;

public:
	friend struct WeakPtr<T>;

	ObjectPool() { }

	~ObjectPool() { }

	void EmptyPool()
	{
		for (T& object : objects)
		{
			object.Destroy();
		}
		objects.clear();
	}

	T& operator[](ObjectId id) 
	{ 
		return objects[id];
	}

	T* Get(WeakPtrGeneric generic)
	{
		return generic.type == Type && IsValid(generic.id, generic.safeguard)? &objects[generic.id] : nullptr;
	}

	T* Get(ObjectId id, uint32_t safeguard)
	{
		return IsValid(id, safeguard)? &objects[id] : nullptr;
	}

	const std::vector<T>& Objects() const
	{ 
		return objects; 
	}

	bool IsValid(ObjectId id)
	{
		return id < objects.size() && safeguards[id] != 0;
	}

	bool IsValid(ObjectId id, uint32_t safeguard)
	{
		return id < objects.size() && safeguards[id] != 0 && safeguards[id] == safeguard;
	}

	WeakPtr<T> CreateWeak()
	{
		return GetWeakPtr(Create());
	}

	WeakPtr<T> GetWeakPtr(ObjectId id)
	{
		WeakPtr<T> newPtr;
		newPtr.id = id;
		newPtr.safeguard = 0; // invalid until object is found
		
		if (id < safeguards.size())
		{
			newPtr.safeguard = safeguards[id];
		}

		return newPtr;
	}

	ObjectId Create()
	{
		if (!bIsRegistered)
		{
			bIsRegistered = true;
			ObjectPoolInternals::RegisterPool(Type, [this]() -> void { EmptyPool(); });
		}

		ObjectId newId = (ObjectId) nextFreeSlot;
		++safeguardCounter;

		if (newId >= objects.size())
		{
			safeguards.push_back(safeguardCounter);
			objects.emplace_back(T{});
		}
		else
		{
			safeguards[newId] = safeguardCounter;
			objects[newId] = T{};
		}

		UpdateNextFreeSlot();

		objects[newId].Initialize();
		objects[newId].poolId = newId;

		return newId;
	}

	void Destroy(WeakPtr<T>& weakPtr)
	{
		Destroy(weakPtr.id, weakPtr.safeguard);
		weakPtr.id = 0;
		weakPtr.safeguard = 0;
	}

	// safeguard should be used if you want to ensure the id you are removing refers to the same object and not a new one that occupies the same slot
	void Destroy(ObjectId id, uint32_t safeguard = 0)
	{
		if (id >= objects.size())
			return; // invalid id

		if (safeguard != 0 && safeguards[id] != safeguard)
			return; // this is not the same object

		safeguards[id] = 0;
		objects[id].Destroy();
		//objects[id].data = T{}; // leave stale

		if (id < nextFreeSlot)
			nextFreeSlot = id;
	}

protected:
	void UpdateNextFreeSlot()
	{
		// find next gap in vector
		while (nextFreeSlot < safeguards.size() && safeguards[nextFreeSlot] != 0)
		{
			++nextFreeSlot;
		}

		assert(nextFreeSlot <= std::numeric_limits<uint32_t>::max());
	}

protected:
	std::vector<T> objects;				// object pool
	std::vector<uint32_t> safeguards;	// safeguard values are used to ensure the object occupying a slot is the same that a WeakObjectPtr is referencing
	uint32_t safeguardCounter = 0;		// used to generate safeguard values
	size_t nextFreeSlot = 0;			// used for reduced iteration times (the vector can have holes in it)
};

template<typename T, size_t OT>
bool ObjectPool<T, OT>::bIsRegistered = false;

// use as base class to add pool to any class
template<typename T, size_t OT>
class ObjectPoolInterface
{
protected:
	ObjectId poolId = 0;

public:
	friend class ObjectPool<T, OT>;
	static ObjectPool<T, OT> Pool;
	static const ObjectType Type = ObjectType(OT);

	ObjectPoolInterface() 
	{
		static_assert(OT < std::numeric_limits<ObjectType>::max(), "Template argument 'size_t OT' must have a value supported by ObjectType (see objecttypes.h)");
	}

	~ObjectPoolInterface() {}

	ObjectId Id() const { return poolId; }
	WeakPtr<T> GetWeakPtr() const { return Pool.GetWeakPtr(poolId); }
};

template<typename T, size_t OT>
ObjectPool<T, OT> ObjectPoolInterface<T, OT>::Pool;