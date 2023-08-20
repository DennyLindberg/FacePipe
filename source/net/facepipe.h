#pragma once

#include <stdint.h>
#include <string>
#include <string_view>
#include <vector>
#include <map>

namespace FacePipe
{
	enum class EAxis : uint8_t
	{
		X = 0,	// +X
		Y = 1,	// +Y
		Z = 2,	// +Z
		nX = 3,	// -X
		nY = 4,	// -Y
		nZ = 5, // -Z
	};

	enum class EFacepipeData : uint8_t
	{
		Blendshapes = 0,
		Landmarks2D = 1,
		Landmarks3D = 2,
		Mesh = 3,
		Matrices4x4 = 4,

		INVALID = 255
	};

	enum class EDatagramType : uint8_t
	{
		Invalid = 0,
		ASCII = 1,
		Bytes = 2,
		String = 3,
		WString = 4,
		Encoded = 5,
		MAX = 6
	};

	struct MetaData
	{
		int Scene = 0;			// Scene of camera and subject
		int Camera = 0;			// Camera the subject was captured in
		int Subject = 0;		// The subject the data belongs to

		std::string Source = "None";						// Which application / capture method produced this data
		EFacepipeData DataType = EFacepipeData::INVALID;	// What the data contains
		double Time = 0.0;									// When the message was sent on the source side
	};

	struct Frame
	{
		MetaData Meta;
		std::map<std::string, float> Blendshapes;
		std::map<std::string, std::vector<float>> Matrices;
		std::vector<float> Landmarks;

		int ImageWidth = 0;
		int ImageHeight = 0;
	};
}

namespace FacePipe
{
	bool ParseHeader(const std::vector<char>& Message, MetaData& OutMeta, std::string_view& OutContent);

	bool GetBlendshapes(const MetaData& MessageMeta, const std::string_view Content, std::map<std::string, float>& OutBlendshapes);
	bool GetLandmarks(const MetaData& MessageMeta, const std::string_view Content, std::vector<float>& OutValues, int& ImageWidth, int& ImageHeight);
	bool GetTransforms(const MetaData& MessageMeta, const std::string_view Content, std::map<std::string, std::vector<float>>& OutMatrices);
}
