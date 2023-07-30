#pragma once

#include <stdint.h>
#include <limits>
#include <vector>
#include <cassert>

#include "objectptr.h"

template<typename T, ObjectType OT>
class ObjectPool
{
public:
	friend struct WeakPtr<T>;
	static const ObjectType Type = OT;

	ObjectPool() {}
	~ObjectPool() { EmptyPool(); }

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

	T* GetSafe(ObjectId id, uint32_t safeguard)
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
