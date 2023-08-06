#pragma once
#include <vector>
#include <filesystem>
#include "glad/glad.h"
#include "core/math.h"
#include "core/objectpool.h"

#define FLIP_IMAGES_VERTICALLY false								// set false to keep [0,0] in the top left corner (like most image applications...)
#define FLIP_TEXTURE_VERTICALLY_IN_SHADER !FLIP_IMAGES_VERTICALLY	// OpenGL expects images to start in the bottom left, the texcoord.y coordinate must be flipped when the image is not otherwise the UV coordinates are wrong (we don't touch the mesh data)

class GLTexture : public ObjectPoolInterface<GLTexture, ObjectType_GLTexture>
{
public:
	std::vector<GLubyte> glData; // vector is used to simplify load/save with lodepng
	GLuint textureId = 0;
	
	int size = 0;
	int numPixels = 0;
	int width = 0;
	int height = 0;

public:
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
	void LoadPNG(std::filesystem::path filepath, bool bFlipVertically = FLIP_IMAGES_VERTICALLY);
	static void FlipVertically(std::vector<GLubyte>& glData, int width, int height, int channelCount = 4);
};
