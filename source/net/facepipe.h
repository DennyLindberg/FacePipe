#pragma once

#include "nlohmann/json.hpp"

namespace FacePipe
{
	enum class EFacepipeData : uint8_t
	{
		Landmarks = 0,
		Blendshapes = 1,
		Transforms = 2,

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

		std::string SourceName;			// Which application / capture method produced this data
		std::string SourceVersion;		// Version of the source

		EFacepipeData DataType;			// What the data contains
		double TimeStamp = 0.0;			// When the message was sent on the source side
	};
}

namespace FacePipe
{
	EDatagramType ToType(char FirstByte);

	bool ParseJSON(const std::string_view& Message, nlohmann::json& OutJSON);
	bool ParseMetaData(const nlohmann::json& Message, MetaData& OutMeta);

	bool GetBlendshapes(const MetaData& MessageMeta, const nlohmann::json& Message, std::vector<std::string>& OutNames, std::vector<float>& OutValues);
	bool GetLandmarks(const MetaData& MessageMeta, const nlohmann::json& Message, std::vector<float>& OutValues);
	bool GetTransforms(const MetaData& MessageMeta, const nlohmann::json& Message, std::vector<float>& OutValues);
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
