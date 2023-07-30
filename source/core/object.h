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
	WeakObjectPtr<Object> parent;
	std::vector<WeakObjectPtr<Object>> children;
	std::vector<GenericWeakObjectPtr> components;

public:
	Object() {}
	~Object() {}

	void Initialize() {}
	void Destroy() {}

	WeakObjectPtr<Object> GetParent() const { return parent; }

	void DetachFromParent();

	void AddChild(WeakObjectPtr<Object> newChild);

	void RemoveChild(WeakObjectPtr<Object> child);

	void AddComponent(GenericWeakObjectPtr weakPtr);

	glm::mat4 ComputeWorldMatrix();
};
