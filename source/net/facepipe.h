#pragma once

#include <stdint.h>
#include <string>
#include <string_view>

#include "nlohmann/json.hpp"

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
		Transforms = 4,

		INVALID = 255
	};

	enum class EDatagramType : uint8_t
	{
		Invalid = 0,
		Bytes = 1,
		String = 2,
		JSON = 3,
		MAX = 4
	};

	struct MetaData
	{
		int API = 0;			// version of protocol
		int Scene = 0;			// Scene of camera and subject
		int Camera = 0;			// Camera the subject was captured in
		int Subject = 0;		// The subject the data belongs to

		std::string Source = "None";						// Which application / capture method produced this data
		EFacepipeData DataType = EFacepipeData::INVALID;	// What the data contains
		double Time = 0.0;									// When the message was sent on the source side
	};

	struct Transform
	{
		std::string Name = "";
		std::vector<float> Matrix;
	};

	struct Frame
	{
		MetaData Meta;
		std::vector<std::string> BlendshapeNames;
		std::vector<float> BlendshapeValues;
		std::vector<float> Landmarks;
		std::vector<FacePipe::Transform> Transforms;

		int ImageWidth = 0;
		int ImageHeight = 0;
	};
}

namespace FacePipe
{
	EDatagramType ToType(char FirstByte);

	bool ParseJSON(const std::string_view& Message, nlohmann::json& OutJSON);
	bool ParseJSON(const std::string& Message, nlohmann::json& OutJSON);
	bool ParseMetaData(const nlohmann::json& Message, MetaData& OutMeta);

	bool GetBlendshapes(const MetaData& MessageMeta, const nlohmann::json& Message, std::vector<std::string>& OutNames, std::vector<float>& OutValues);
	bool GetLandmarks(const MetaData& MessageMeta, const nlohmann::json& Message, std::vector<float>& OutValues, int& ImageWidth, int& ImageHeight);
	bool GetTransforms(const MetaData& MessageMeta, const nlohmann::json& Message, std::vector<FacePipe::Transform>& OutTransforms);
}

namespace FacePipe
{
	template<typename T>
	bool ParseJSON(const T& Message, nlohmann::json& OutJSON)
	{
		if (Message.size() == 0 || ToType(Message[0]) != EDatagramType::JSON)
			return false;

		std::string_view Data(Message.begin() + 1, Message.end()); // data starts from the second byte
		return ParseJSON(Data, OutJSON);
	}

	template<typename T>
	bool Parse(const T& Message, MetaData& OutMeta, nlohmann::json& OutJSON)
	{
		return ParseJSON(Message, OutJSON) && ParseMetaData(OutJSON, OutMeta);
	}
}
