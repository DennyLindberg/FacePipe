#include "object.h"

void Object::DetachFromParent()
{
	if (Object* p = parent.Get())
	{
		p->RemoveChild(GetWeakPtr());
	}
}

void Object::AttachTo(WeakPtr<Object> newParent)
{
	DetachFromParent();

	if (Object* p = newParent.Get())
	{
		p->AddChild(GetWeakPtr());
	}
}

void Object::AddChild(WeakPtr<Object> newChild)
{
	if (Object* obj = newChild.Get())
	{
		obj->parent = GetWeakPtr();

		for (WeakPtr<Object>& child : children)
		{
			if (child == newChild)
				return; // already a child
		}

		children.push_back(newChild);
	}
}

void Object::RemoveChild(WeakPtr<Object> child)
{
	if (Object* obj = child.Get())
	{
		obj->parent.Clear();
	}

	for (size_t i = 0; i < children.size(); ++i)
	{
		if (children[i] == child)
		{
			children.erase(children.begin() + i);
			return;
		}
	}
}

void Object::AddComponent(WeakPtrGeneric weakPtr)
{
	for (const WeakPtrGeneric& child : components)
	{
		if (child == weakPtr)
			return; // already added
	}

	components.push_back(weakPtr);
}

glm::mat4 Object::ComputeWorldMatrix()
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
