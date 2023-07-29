#pragma once
#include <algorithm>
#include <memory>
#include "texture.h"
#include "mesh.h"
#include "../core/math.h"

class Canvas2D
{
protected:
	bool bDirty = true;
	GLTexture* texture = nullptr;

public:
	Canvas2D() {}
	~Canvas2D() { Shutdown(); }

	void Initialize(int width, int height);
	void Shutdown();

	GLTexture* GetTexture() const { return texture; }

	void RenderToScreen();

/*
	Canvas drawing methods
*/
public:
	void Fill(Color& color);
	void DrawLine(glm::fvec2 start, glm::fvec2 end, Color& color);
};