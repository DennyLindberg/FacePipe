#pragma once

#include <functional>
#include <vector>
#include "core/transform.h"

typedef uint8_t ObjectType;
typedef uint32_t ObjectId;

#define ObjectType_Unknown 0

// Contains a vector index and generator safeguard to ensure that the id references the same object
// safeguard 0 means uninitialized ptr
struct WeakPtrGeneric
{
public:
	WeakPtrGeneric(ObjectType objectType) : type(objectType) {}

	ObjectType type = ObjectType_Unknown;	// what type of object this refers to
	ObjectId id = 0;					// index to vector
	uint32_t safeguard = 0;				// based on generator

	void Clear()
	{
		type = ObjectType_Unknown;
		id = 0;
		safeguard = 0;
	}

	template<typename T>
	inline T* As() { return TryGet<T>(); }

	template<typename T>
	T* TryGet()
	{
		return T::Pool.Get(*this);
	}

	bool Is(ObjectType objectType) const { return type == objectType; }

	bool operator==(const WeakPtrGeneric& b) const { return type == b.type && id == b.id && safeguard == b.safeguard; }
};

template<typename T>
struct WeakPtr : public WeakPtrGeneric
{
public:
	WeakPtr() : WeakPtrGeneric(T::Pool.Type) {}

	WeakPtr(WeakPtrGeneric ptr)
		: WeakPtrGeneric(T::Pool.Type)
	{
		if (ptr.type == T::Pool.Type)
		{
			id = ptr.id;
			safeguard = ptr.safeguard;
		}
	}

public:
	void Destroy() { T::Pool.Destroy(*this); }

	inline ObjectId GetId() const { return id; }
	inline uint32_t GetSafeguard() const { return safeguard; }
	inline T* Get() const { return T::Pool.Get(id, safeguard); }

	operator bool() const { return Get() != nullptr; }
	T* operator->() const { return Get(); }
	T& operator*(void) const { return *Get(); }

	operator T* () const { return Get(); }
};
