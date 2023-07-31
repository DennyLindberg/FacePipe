#pragma once
#include <vector>
#include <filesystem>
#include "glad/glad.h"
#include "core/math.h"
#include "core/objectpool.h"

class GLTexture
{
public:
	ObjectId poolId = 0;
	std::vector<GLubyte> glData; // vector is used to simplify load/save with lodepng
	GLuint textureId = 0;
	
	int size = 0;
	int numPixels = 0;
	int width = 0;
	int height = 0;

public:
	friend class ObjectPool<GLTexture>;
	static ObjectPool<GLTexture> Pool;

	GLTexture() {}
	~GLTexture() {}

	void Initialize();
	void Destroy();

	void SetSize(int textureWidth, int textureHeight);

	void UpdateParameters();

	inline GLubyte& operator[] (unsigned int i) { return glData[i]; }

	inline void SetPixel(unsigned int pixelIndex, GLubyte r, GLubyte g, GLubyte b, GLubyte a);
	void SetPixel(unsigned int x, unsigned int y, GLubyte r, GLubyte g, GLubyte b, GLubyte a);
	void SetPixel(unsigned int x, unsigned int y, double r, double g, double b, double a);

	void SetPixel(unsigned int x, unsigned int y, const Color& color);
	void SetPixel(unsigned int x, unsigned int y, const FColor& color);

	void SetPixelSafe(int x, int y, const Color& color);
	void SetPixelSafe(int x, int y, const FColor& color);

	unsigned int PixelArrayIndex(unsigned int x, unsigned int y);

	void UseForDrawing(unsigned int TextureUnit = 0);
	void CopyToGPU();

	void Fill(const Color& color);
	void Fill(const FColor& color);

	void FillDebug();
	static void SaveAsPNG(std::vector<GLubyte>& glData, int width, int height, std::string filepath, bool incrementNewFile = false);
	void LoadPNG(std::filesystem::path filepath, bool bFlipVertically = true);
	static void FlipVertically(std::vector<GLubyte>& glData, int width, int height, int channelCount = 4);
};
