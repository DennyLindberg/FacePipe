#pragma once

#include <string>
#include "objectpool.h"

class Object
{
public:
	friend class ObjectPool<Object, OBJECTTYPE_OBJECT>;
	static ObjectPool<Object, OBJECTTYPE_OBJECT> Pool;

	std::string name = "Object";
	Transform transform;

protected:
	ObjectId poolId = 0;
	WeakPtr<Object> parent;
	std::vector<WeakPtr<Object>> children;
	std::vector<WeakPtrGeneric> components;

public:
	Object() {}
	~Object() {}

	void Initialize() {}
	void Destroy() {}

	WeakPtr<Object> GetWeakPtr() const { return Object::Pool.GetWeakPtr(poolId); }

	WeakPtr<Object> GetParent() const { return parent; }

	void DetachFromParent();

	void AttachTo(WeakPtr<Object> newParent);

	void AddChild(WeakPtr<Object> newChild);

	void RemoveChild(WeakPtr<Object> child);

	void AddComponent(WeakPtrGeneric weakPtr);

	glm::mat4 ComputeWorldMatrix();
};
