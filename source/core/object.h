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
	bool visible = true;

	Object() {}
	~Object() {}

	void Initialize() {}
	void Destroy() {}

	ObjectId GetObjectId() const { return poolId; }
	WeakPtr<Object> GetWeakPtr() const { return Object::Pool.GetWeakPtr(poolId); }

	WeakPtr<Object> GetParent() const { return parent; }
	const std::vector<WeakPtr<Object>>& GetChildren() { return children; }
	const std::vector<WeakPtrGeneric>& GetComponents() { return components; }

	void DetachFromParent();

	void AttachTo(WeakPtr<Object> newParent);

	void AddChild(WeakPtr<Object> newChild);

	void RemoveChild(WeakPtr<Object> child);

	void AddComponent(WeakPtrGeneric weakPtr);

	glm::mat4 ComputeWorldMatrix();
};
