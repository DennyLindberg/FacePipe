#include "texture.h"

#include <iostream>
#include <memory>
#include <algorithm>
#include <fstream>
#include "lodepng.h"

#define INTERNAL_PIXEL_FORMAT GL_RGBA
#define PIXEL_FORMAT GL_RGBA
#define PIXEL_TYPE GL_UNSIGNED_INT_8_8_8_8_REV

void GLTexture::Initialize()
{
	if (!textureId)
	{
		glGenTextures(1, &textureId);
	}
}

void GLTexture::Destroy()
{
	if (textureId)
	{
		glDeleteTextures(1, &textureId);
		textureId = 0;
	}

	size = 0;
	numPixels = 0;
	width = 0;
	height = 0;
}

void GLTexture::SetSize(int textureWidth, int textureHeight)
{
	if (!textureId || width == textureWidth && height == textureHeight)
		return;

	width = textureWidth;
	height = textureHeight;

	numPixels = width * height;
	size = numPixels * 4;
	glData.resize(size);

	for (int i = 0; i < size; ++i)
	{
		glData[i] = 0;
	}

	UpdateParameters();
}

void GLTexture::UpdateParameters()
{
	if (!textureId) return;

	glBindTexture(GL_TEXTURE_2D, textureId);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexImage2D(GL_TEXTURE_2D, 0, INTERNAL_PIXEL_FORMAT, width, height, 0, PIXEL_FORMAT, PIXEL_TYPE, (GLvoid*)glData.data());
}

inline void GLTexture::SetPixel(unsigned int pixelIndex, GLubyte r, GLubyte g, GLubyte b, GLubyte a)
{
	glData[pixelIndex + 0] = r;
	glData[pixelIndex + 1] = g;
	glData[pixelIndex + 2] = b;
	glData[pixelIndex + 3] = a;
}

void GLTexture::SetPixel(unsigned int x, unsigned int y, GLubyte r, GLubyte g, GLubyte b, GLubyte a)
{
	SetPixel(PixelArrayIndex(x, y), r, g, b, a);
}

void GLTexture::SetPixel(unsigned int x, unsigned int y, double r, double g, double b, double a)
{
	r = std::max(std::min(1.0, r), 0.0);
	g = std::max(std::min(1.0, g), 0.0);
	b = std::max(std::min(1.0, b), 0.0);
	a = std::max(std::min(1.0, a), 0.0);
	SetPixel(x, y, GLubyte(r*255.0), GLubyte(g*255.0), GLubyte(b*255.0), GLubyte(a*255.0));
}

void GLTexture::SetPixel(unsigned int x, unsigned int y, const Color& color)
{
	unsigned int pixelIndex = PixelArrayIndex(x, y);
	glData[pixelIndex + 0] = color.r;
	glData[pixelIndex + 1] = color.g;
	glData[pixelIndex + 2] = color.b;
	glData[pixelIndex + 3] = color.a;
}

void GLTexture::SetPixel(unsigned int x, unsigned int y, const FColor& color)
{
	unsigned int pixelIndex = PixelArrayIndex(x, y);
	glData[pixelIndex + 0] = GLubyte(std::max(std::min(1.0f, color.r), 0.0f) * 255);
	glData[pixelIndex + 1] = GLubyte(std::max(std::min(1.0f, color.g), 0.0f) * 255);
	glData[pixelIndex + 2] = GLubyte(std::max(std::min(1.0f, color.b), 0.0f) * 255);
	glData[pixelIndex + 3] = GLubyte(std::max(std::min(1.0f, color.a), 0.0f) * 255);
}

void GLTexture::SetPixelSafe(int x, int y, const Color& color)
{
	if (x > 0 && y > 0 && x < width && y < height)
	{
		SetPixel(x, y, color);
	}
}

void GLTexture::SetPixelSafe(int x, int y, const FColor& color)
{
	if (x > 0 && y > 0 && x < width && y < height)
	{
		SetPixel(x, y, color);
	}
}

unsigned int GLTexture::PixelArrayIndex(unsigned int x, unsigned int y)
{
	return y * width * 4 + x * 4;
}


void GLTexture::UseForDrawing(unsigned int TextureUnit)
{
	if (!textureId) return;
	glActiveTexture(GL_TEXTURE0 + TextureUnit);
	glBindTexture(GL_TEXTURE_2D, textureId);
}

void GLTexture::CopyToGPU()
{
	if (!textureId) return;
	glBindTexture(GL_TEXTURE_2D, textureId);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, PIXEL_FORMAT, PIXEL_TYPE, (GLvoid*)glData.data());
}

void GLTexture::Fill(const Color& color)
{
	for (int x = 0; x < width; ++x)
	{
		for (int y = 0; y < height; ++y)
		{
			SetPixel(x, y, color);
		}
	}
}

void GLTexture::Fill(const FColor& color)
{
	Color remapped;
	remapped.r = GLubyte(std::max(std::min(1.0f, color.r), 0.0f) * 255);
	remapped.g = GLubyte(std::max(std::min(1.0f, color.g), 0.0f) * 255);
	remapped.b = GLubyte(std::max(std::min(1.0f, color.b), 0.0f) * 255);
	remapped.a = GLubyte(std::max(std::min(1.0f, color.a), 0.0f) * 255);

	for (int x = 0; x < width; ++x)
	{
		for (int y = 0; y < height; ++y)
		{
			SetPixel(x, y, remapped);
		}
	}
}

void GLTexture::FillDebug()
{
	for (int x = 0; x < width; ++x)
	{
		for (int y = 0; y < height; ++y)
		{
			GLubyte r = (GLubyte)(x / (float)width * 255);
			GLubyte g = (GLubyte)(y / (float)height * 255);
			GLubyte b = 0;
			GLubyte a = 255;
			SetPixel(x, y, r, g, b, a);
		}
	}
}

void GLTexture::SaveAsPNG(std::vector<GLubyte>& glData, int width, int height, std::string filepath, bool incrementNewFile)
{
	auto remove_extension = [](const std::string& filename) -> std::string {
		size_t lastdot = filename.find_last_of(".");
		if (lastdot == std::string::npos) return filename;
		return filename.substr(0, lastdot);
	};

	auto file_exists = [](std::string filename) -> bool {
		std::ifstream infile(filename);
		return infile.good();
	};

	int count = 0;
	std::string baseName = remove_extension(filepath);
	do
	{
		count++;
		filepath = baseName + std::string(5 - std::to_string(count).length(), '0') + std::to_string(count) + ".png";
	} while (file_exists(filepath));

	unsigned error = lodepng::encode(filepath, glData, (unsigned int)width, (unsigned int)height);
	if (error)
	{
		std::cout << "encoder error " << error << ": " << lodepng_error_text(error) << std::endl;
	}
}

void GLTexture::LoadPNG(std::filesystem::path filepath, bool bFlipVertically)
{
	unsigned sourceWidth, sourceHeight;

	glData.clear();
	glData.shrink_to_fit();

	std::vector<unsigned char> png;
	lodepng::State state;
	unsigned error = lodepng::load_file(png, filepath.string());
	if (!error)
	{
		error = lodepng::decode(glData, sourceWidth, sourceHeight, state, png);
	}

	if (error)
	{
		std::cout << "decoder error " << error << ": " << lodepng_error_text(error) << std::endl;
	}
	else
	{
		const LodePNGColorMode& color = state.info_png.color;
		//switch (color.colortype)
		//{
		//case LCT_RGB:
		//	PIXEL_FORMAT = GL_RGB;
		//	break;
		//case LCT_RGBA:
		//default:
		//	PIXEL_FORMAT = GL_RGBA;
		//	break;
		//}

		width = sourceWidth;
		height = sourceHeight;

		size = width * height * lodepng_get_channels(&color);

		if (bFlipVertically)
		{
			GLTexture::FlipVertically(glData, width, height);
		}

		UpdateParameters();
	}
}

void GLTexture::FlipVertically(std::vector<GLubyte>& glData, int width, int height, int channelCount)
{
	int pixelSourceId = 0;
	int pixelTargetId = 0;
	std::vector<GLubyte> original = glData;
	for (int x = 0; x < width; ++x)
	{
		for (int y = 0; y < height; ++y)
		{
			pixelSourceId = x * channelCount + y * width * channelCount;
			pixelTargetId = x * channelCount + (height - y - 1) * width * channelCount;

			glData[pixelTargetId] = original[pixelSourceId];
			glData[pixelTargetId + 1] = original[pixelSourceId + 1];
			glData[pixelTargetId + 2] = original[pixelSourceId + 2];
			glData[pixelTargetId + 3] = original[pixelSourceId + 3];
		}
	}
}
