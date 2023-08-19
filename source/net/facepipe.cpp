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
		case '{':
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

	bool ParseJSON(const std::string& Message, nlohmann::json& OutJSON)
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

			const json& source = Message["source"];
			{
				if (!source.is_string())
					return false;

				OutMeta.Source = source.get<std::string>();
			}

			const json& time = Message["time"];
			{
				if (!time.is_number())
					return false;

				OutMeta.Time = time.is_number_float()? time.get<double>() : 0.0;
			}

			OutMeta.DataType = EFacepipeData::INVALID;
			if (Message.contains("data"))
			{
				const json& data = Message["data"];
				if (data.contains("type"))
				{
					const json& DataType = data["type"].get<std::string>();
					if (DataType == "blendshapes")
						OutMeta.DataType = EFacepipeData::Blendshapes;
					else if (DataType == "landmarks2d")
						OutMeta.DataType = EFacepipeData::Landmarks2D;
					else if (DataType == "landmarks3d")
						OutMeta.DataType = EFacepipeData::Landmarks3D;
					else if (DataType == "mesh")
						OutMeta.DataType = EFacepipeData::Mesh;
					else if (DataType == "transforms")
						OutMeta.DataType = EFacepipeData::Transforms;
				}
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

	bool GetLandmarks(const MetaData& MessageMeta, const nlohmann::json& Message, std::vector<float>& OutValues, int& ImageWidth, int& ImageHeight)
	{
		if (MessageMeta.DataType == EFacepipeData::Landmarks2D || MessageMeta.DataType == EFacepipeData::Landmarks3D)
		{
			try {
				const json& data = Message["data"];
				OutValues = data["values"].get<std::vector<float>>();

				ImageWidth = 0;
				ImageHeight = 0;
				if (data.contains("image"))
				{
					const json& image = data["image"];
					if (image.is_array() && image.size() >= 2)
					{
						ImageWidth = image[0].get<int>();
						ImageHeight = image[1].get<int>();
					}
				}

				OutValues = data["values"].get<std::vector<float>>();
				return true;
			}
			catch (std::exception e)
			{
			}
		}

		return false;
	}

	bool GetTransforms(const MetaData& MessageMeta, const nlohmann::json& Message, std::vector<FacePipe::Transform>& OutTransforms)
	{
		if (MessageMeta.DataType == EFacepipeData::Transforms)
		{
			try {
				const json& data = Message["data"];
				if (!data.contains("values"))
					return false;

				OutTransforms.resize(0);
				for (const json& transform : data["values"])
				{
					if (transform.contains("name") && transform.contains("matrix"))
					{
						FacePipe::Transform NewTransform;
						NewTransform.Name = transform["name"].get<std::string>();
						NewTransform.Matrix = transform["matrix"].get<std::vector<float>>();

						OutTransforms.push_back(NewTransform);
					}
				}
				
				return true;
			}
			catch (std::exception e) 
			{
			}
		}

		return false;
	}
}
