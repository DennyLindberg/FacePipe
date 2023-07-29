#include "canvas.h"
#include "application/application.h"

void DrawBresenhamLine(GLTexture& texture, float x1, float y1, float x2, float y2, Color& color)
{
	// Taken from Rosetta Code
	// https://rosettacode.org/wiki/Bitmap/Bresenham%27s_line_algorithm#C.2B.2B

	// Bresenham's line algorithm
	const bool steep = (fabs(y2 - y1) > fabs(x2 - x1));
	if (steep)
	{
		std::swap(x1, y1);
		std::swap(x2, y2);
	}

	if (x1 > x2)
	{
		std::swap(x1, x2);
		std::swap(y1, y2);
	}

	const float dx = x2 - x1;
	const float dy = fabs(y2 - y1);

	float error = dx / 2.0f;
	const int ystep = (y1 < y2) ? 1 : -1;
	int y = (int)y1;

	const int maxX = (int)x2;

	for (int x = (int)x1; x < maxX; x++)
	{
		if (steep)
		{
			texture.SetPixelSafe(y, x, color);
		}
		else
		{
			texture.SetPixelSafe(x, y, color);
		}

		error -= dy;
		if (error < 0)
		{
			y += ystep;
			error += dx;
		}
	}
}

void Canvas2D::RenderToScreen()
{
	if (bDirty)
	{
		bDirty = false;
		texture->CopyToGPU();
	}

	App::shaders.canvasShader.Use();
	texture->UseForDrawing();

	// TODO: Transform quad by setting uniforms in shader
	App::geometry.quad.Draw();
}

void Canvas2D::Initialize(int width, int height)
{
	texture = new GLTexture(width, height);
}

void Canvas2D::Shutdown()
{
	if (texture)
	{
		delete texture;
		texture = nullptr;
	}
}

/*
	Canvas drawing methods
*/
void Canvas2D::Fill(Color& color)
{
	bDirty = true;
	texture->Fill(color);
}

void Canvas2D::DrawLine(glm::fvec2 start, glm::fvec2 end, Color& color)
{
	bDirty = true;
	DrawBresenhamLine(*texture, start.x, start.y, end.x, end.y, color);
}