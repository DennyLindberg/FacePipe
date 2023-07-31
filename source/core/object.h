#pragma once

#include <string>
#include "objectpool.h"

class Object : public ObjectPoolInterface<Object, ObjectType_Object>
{
public:
	std::string name = "Object";
	Transform transform;

protected:
	WeakPtr<Object> parent;
	std::vector<WeakPtr<Object>> children;
	std::vector<WeakPtrGeneric> components;

public:
	bool visible = true;

	Object() {}
	~Object() {}

	void Initialize() {}
	void Destroy() {}

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
