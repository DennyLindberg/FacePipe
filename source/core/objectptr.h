#pragma once

#include <functional>
#include <vector>
#include "core/transform.h"

typedef uint8_t ObjectType;
typedef uint32_t ObjectId;

#define OBJECTTYPE_UNKNOWN 0
#define OBJECTTYPE_OBJECT 1
#define OBJECTTYPE_CAMERA 2
#define OBJECTTYPE_LIGHT 3
#define OBJECTTYPE_MESH 4
#define OBJECTTYPE_TEXTURE 5
#define OBJECTTYPE_GLLine 6

// Contains a vector index and generator safeguard to ensure that the id references the same object
// safeguard 0 means uninitialized ptr
struct GenericWeakObjectPtr
{
public:
	GenericWeakObjectPtr(ObjectType t) : type(t) {}

	ObjectType type = OBJECTTYPE_UNKNOWN;	// what type of object this refers to
	ObjectId id = 0;						// index to vector
	uint32_t safeguard = 0;					// based on generator

	void Clear()
	{
		type = OBJECTTYPE_UNKNOWN;
		id = 0;
		safeguard = 0;
	}

	bool operator==(const GenericWeakObjectPtr& b) const { return type == b.type && id == b.id && safeguard == b.safeguard; }
};

template<typename T>
struct WeakObjectPtr : public GenericWeakObjectPtr
{
public:
	WeakObjectPtr() : GenericWeakObjectPtr(T::Pool.Type) {}

public:
	void Destroy() { T::Pool.Destroy(*this); }

	inline ObjectId GetId() const { return id; }
	inline uint32_t GetSafeguard() const { return safeguard; }
	inline T* Get() const { return T::Pool.GetSafe(id, safeguard); }

	operator bool() const { return Get() != nullptr; }
	T* operator->() const { return Get(); }
	T& operator*(void) const { return *Get(); }

	operator T* () const { return Get(); }
};
