#pragma once

#include <string>
#include "objectpool.h"

class Object
{
public:
	friend class ObjectPool<Object, OBJECTTYPE_OBJECT>;
	static ObjectPool<Object, OBJECTTYPE_OBJECT> Pool;

	Object() {}
	~Object() {}

	void Initialize() {}
	void Destroy() {}

	Transform transform;

	void AddChild(WeakObjectPtr<Object> newChild)
	{
		if (Object* obj = newChild.Get())
		{
			obj->parent = Pool.GetWeakPtr(poolId);

			for (WeakObjectPtr<Object>& child : children)
			{
				if (child == newChild)
					return; // already a child
			}

			children.push_back(newChild);
		}
	}

	void RemoveChild(WeakObjectPtr<Object> child)
	{
		if (Object* obj = child.Get())
		{
			obj->parent.Clear();
		}

		for (size_t i=0; i<children.size(); ++i)
		{
			if (children[i] == child)
			{
				children[i].Clear();
			}
		}
	}

	void AddComponent(GenericWeakObjectPtr weakPtr)
	{
		for (const GenericWeakObjectPtr& child : components)
		{
			if (child == weakPtr)
				return; // already added
		}

		components.push_back(weakPtr);
	}

	template<class T>
	void AddComponent(WeakObjectPtr<T> weakPtr)
	{
		AddComponent(GenericWeakObjectPtr(weakPtr));
	}

	glm::mat4 ComputeWorldMatrix()
	{
		glm::mat4 mat = transform.Matrix();

		Object* p = parent.Get();
		while (p)
		{
			mat = p->transform.Matrix() * mat;
			p = p->parent.Get();
		}

		return mat;
	}

	// TODO: Scene graph must check generic objects against known object pools
	//void CleanupDestroyedComponents(std::function<void(const GenericWeakObjectPtr& child)> IsInvalidChild)
	//{
	//	children.erase(
	//		std::remove_if(children.begin(), children.end(), IsInvalidChild),
	//		children.end()
	//	);
	//}

public:
	std::string name = "Object";

protected:
	ObjectId poolId = 0;
	WeakObjectPtr<Object> parent;
	std::vector<WeakObjectPtr<Object>> children;
	std::vector<GenericWeakObjectPtr> components;
};
