#pragma once

#include <functional>
#include <vector>
#include "core/transform.h"

typedef uint8_t ObjectType;
typedef uint32_t ObjectId;

#define OBJECTTYPE_UNKNOWN 0
#define OBJECTTYPE_CAMERA 1
#define OBJECTTYPE_LIGHT 2
#define OBJECTTYPE_MESH 3
#define OBJECTTYPE_TEXTURE 4

// Contains a vector index and generator safeguard to ensure that the id references the same object
// safeguard 0 means uninitialized ptr
struct GenericWeakObjectPtr
{
public:
	GenericWeakObjectPtr(ObjectType t) : type(t) {}

	ObjectType type = OBJECTTYPE_UNKNOWN;	// what type of object this refers to
	ObjectId id = 0;						// index to vector
	uint32_t safeguard = 0;					// based on generator

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

class Object
{
public:
	Object() {}
	~Object() {}

	Transform transform;

	void AddChild(GenericWeakObjectPtr weakPtr)
	{
		for (const GenericWeakObjectPtr& child : children)
		{
			if (child == weakPtr)
				return; // already added
		}

		children.push_back(weakPtr);
	}

	template<class T>
	void AddChild(WeakObjectPtr<T> weakPtr)
	{
		AddChild(GenericWeakObjectPtr(weakPtr));
	}

	// TODO: Scene graph must check generic objects against known object pools
	//void CleanupDestroyedChildren(std::function<void(const GenericWeakObjectPtr& child)> IsInvalidChild)
	//{
	//	children.erase(
	//		std::remove_if(children.begin(), children.end(), IsInvalidChild),
	//		children.end()
	//	);
	//}

protected:
	std::vector<GenericWeakObjectPtr> children;
};
