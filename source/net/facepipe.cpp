#include "facepipe.h"

/*
* Protocol layout
* 
* First byte of datagram.message[0] is the type
*	'b' = bytes
*	'j' = json
*	's' = string
* 
* Remainder datagram.message[1] is handled differently per type
* 
* bytes
*	not implemented
* 
* string
*	only treated as log
* 
* json
*   {
*		'channel': [0,0,0,0],								# api version, scene, camera, subject
*		'header': ['mediapipe','1.0.0.0','blendshapes'],	# source, version, data type
*		'time': 0.0,										# time in seconds at the source (application start, since epoch, does not matter)
*		'data': {}											# data that changes with type
*	}
*/

using json = nlohmann::json;

namespace FacePipe
{
	EDatagramType ToType(char FirstByte)
	{
		switch (FirstByte)
		{
		case 'b': { return EDatagramType::Bytes; }
		case 's': { return EDatagramType::String; }
		case 'j': { return EDatagramType::JSON; }
		default:  { return EDatagramType::Invalid; }
		}
	}

	bool ParseJSON(const std::string_view& Message, nlohmann::json& OutJSON)
	{
		try
		{
			OutJSON = json::parse(Message);
			return true;
		}
		catch (std::exception e)
		{
		}

		return false;
	}

	bool ParseMetaData(const nlohmann::json& Message, MetaData& OutMeta)
	{
		try 
		{
			const json& channel = Message["channel"];
			{
				if (!channel.is_array())
					return false;

				auto values = channel.get<std::vector<int>>();			// [0,0,0,0] api version, scene, camera, subject
				if (values.size() < 4)
					return false;

				OutMeta.API = values[0];
				OutMeta.Scene = values[1];
				OutMeta.Camera = values[2];
				OutMeta.Subject = values[3];
			}

			const json& header = Message["header"];
			{
				if (!header.is_array())
					return false;

				auto values = header.get<std::vector<std::string>>();	// ['source', 'version', 'datatype'] e.g. ['mediapipe', '1.0.0.0', 'blendshapes']
				if (values.size() < 3)
					return false;

				OutMeta.SourceName = values[0];
				OutMeta.SourceVersion = values[1];

				auto& DataType = values[2];
				if (DataType == "blendshapes")
					OutMeta.DataType = EFacepipeData::Blendshapes;
				else if (DataType == "landmarks")
					OutMeta.DataType = EFacepipeData::Landmarks;
				else if (DataType == "transforms")
					OutMeta.DataType = EFacepipeData::Transforms;
				else
					OutMeta.DataType = EFacepipeData::INVALID;
			}

			const json& time = Message["time"];
			{
				OutMeta.Time = time.is_number_float()? time.get<double>() : 0.0;
			}

			return true;
		}
		catch (std::exception e)
		{
		}

		return false;
	}

	bool GetBlendshapes(const MetaData& MessageMeta, const nlohmann::json& Message, std::vector<std::string>& OutNames, std::vector<float>& OutValues)
	{
		if (MessageMeta.DataType == EFacepipeData::Blendshapes)
		{
			try {
				const json& data = Message["data"];
				OutNames = data["names"].get<std::vector<std::string>>();
				OutValues = data["values"].get<std::vector<float>>();
				return true;
			}
			catch (std::exception e) 
			{
			}
		}

		return false;
	}

	bool GetLandmarks(const MetaData& MessageMeta, const nlohmann::json& Message, std::vector<float>& OutValues)
	{
		if (MessageMeta.DataType == EFacepipeData::Landmarks)
		{
			try {
				const json& data = Message["data"];
				OutValues = data["values"].get<std::vector<float>>();
				return true;
			}
			catch (std::exception e)
			{
			}
		}

		return false;
	}

	bool GetTransforms(const MetaData& MessageMeta, const nlohmann::json& Message, std::vector<float>& OutValues)
	{
		if (MessageMeta.DataType == EFacepipeData::Transforms)
		{
			try {
				const json& data = Message["data"];
				OutValues = data["values"].get<std::vector<float>>();
				return true;
			}
			catch (std::exception e) 
			{
			}
		}

		return false;
	}
}
