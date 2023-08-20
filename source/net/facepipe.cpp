#include "facepipe.h"
#include <type_traits>

/*
* Protocol layout
* 
* type|protocol|source|scene,camera,subject|time|content
* 
* e.g.
* 
* a|facepipe|mediapipe|0,0,0|42.3312|...
* 
* First byte of datagram.message[0] is the datagram type (see ToType)
*	e.g. 'a' = ascii
* 
* Protocol is a name so that the same socket could be used for other things.
*	default facepipe
* 
* Source is the application generating the packet, in facepipe it signifies the tracking source.
*	mediapipe, arkit, nvidia, etc
* 
* Scene,Camera,Subject are integers. These exist to allow multicamera setups to track the same subject.
*	0 is the default value
*	scene and camera is most likely to be 0 at all times while there can be multiple subjects
* 
* Time is a 64 bit floating point value in seconds - either from epoch or application start (does not matter as long as it ticks at normal rate)
* 
* Content is where the packet specific data begins. In facepipe the first word is the type followed by the data:
*	Landmarks2D: l2d|0.1,0.2,0.3,0.4,...
*	Landmarks3D: l3d|0.1,0.2,0.3,0.4,0.5,0.6,...
*	Blendshapes: bs|mouthShrugUpper=0.5|eyeSquint_R=0.2
*	Matrices:	 mat44|face=0.1,0.2,0.3,0.4,0.5,0.6,0.7,0.8,0.9,1.0,1.1,1.2,1.3,1.4,1.5|eyeL=...|eyeR=...|jaw=...
*/

namespace FacePipe
{
	bool NextSubstring(const std::string_view& StrView, char Delimiter, size_t& b, size_t& e)
	{
		if (e + 1 >= StrView.size())
			return false;

		// Check if we are standing on a delimiter - if we are we start over on the next character and scan forward
		if (StrView[e] == Delimiter)
		{
			++e;
			b = e;
		}

		while (e < StrView.size())
		{
			if (StrView[e] == Delimiter)
			{
				break;
			}

			++e;
		}

		return true; // end of vector or delimiter
	}

	double ParseDouble(const std::string_view& StringView) noexcept
	{
		std::string s(StringView);
		char* pEnd = nullptr;
		double v = std::strtod(s.c_str(), &pEnd);
		return (*pEnd)? 0.0f : v;
	}

	float ParseFloat(const std::string_view& StringView) noexcept
	{
		return (float) ParseDouble(StringView);
	}

	int ParseInt(const std::string_view& StringView) noexcept
	{
		std::string s(StringView);
		char* pEnd = nullptr;
		int v = (int) std::strtol(s.c_str(), &pEnd, 10); // base 10
		return (*pEnd)? 0 : v;
	}

	template<typename T>
	std::vector<T> ParseArray(const std::string_view& StrView)
	{
		std::vector<T> values;
		size_t b = 0;
		size_t e = 0;
		while (NextSubstring(StrView, ',', b, e))
		{
			if constexpr (std::is_same<T, float>())
				values.push_back(ParseFloat(std::string_view(StrView.begin()+b, StrView.begin()+e)));
			else if constexpr (std::is_same<T, double>())
				values.push_back(ParseDouble(std::string_view(StrView.begin()+b, StrView.begin()+e)));
			else if constexpr (std::is_same<T, int>())
				values.push_back(ParseInt(std::string_view(StrView.begin()+b, StrView.begin()+e)));
		}

		return values;
	}

	std::vector<float> ParseFloatArray(const std::string_view& StrView)
	{
		std::vector<float> values;
		size_t b = 0;
		size_t e = 0;
		while (NextSubstring(StrView, ',', b, e))
		{
			values.push_back(ParseFloat(std::string_view(StrView.begin() + b, StrView.begin() + e)));
		}

		return values;
	}

	bool ParseHeader(const std::vector<char>& Message, MetaData& OutMeta, std::string_view& OutContent)
	{
		// a|protocol|source|scene,camera,subject|time|content

		OutMeta = MetaData();

		if (Message.size() <= 2) // a| - first two characters must exist for us to do anything with this
			return false;

		// First byte is the type
		EDatagramType Type = EDatagramType::Invalid;
		switch (Message[0])
		{
		case 'a': { Type = EDatagramType::ASCII; break; }
		case 'b': { Type = EDatagramType::Bytes; break; }
		case 's': { Type = EDatagramType::String; break; }
		case 'w': { Type = EDatagramType::WString; break; }
		case 'e': { Type = EDatagramType::Encoded; break; }
		default: { break; }
		}

		if (Type != EDatagramType::ASCII) // we don't support anything else at the moment
			return false;
		
		// we start on the first '|'
		size_t end = Message.size();
		size_t b = 0; // substring begin (always first character to parse)
		size_t e = 1; // substring end	 (always next | or end of message)

		int index = 0;
		const std::string_view MessageView(Message.data(), Message.size());
		while (NextSubstring(MessageView, '|', b, e))
		{
			const std::string_view substr(Message.begin() + b, Message.begin() + e);

			switch (++index)
			{
				case 1: 
				{ 
					if (substr != "facepipe")
					{
						return false;
					}
					break;
				}
				case 2: 
				{ 
					OutMeta.Source = substr; 
					break;
				}
				case 3: 
				{
					std::vector<int> channels = ParseArray<int>(substr);
					if (channels.size() != 3)
					{
						return false;
					}
					OutMeta.Scene = channels[0];
					OutMeta.Camera = channels[1];
					OutMeta.Subject = channels[2];
					break;
				}
				case 4:
				{
					OutMeta.Time = ParseDouble(substr);
					break;
				}
				case 5:
				{
					if (substr == "l2d")
						OutMeta.DataType = EFacepipeData::Landmarks2D;
					else if (substr == "l3d")
						OutMeta.DataType = EFacepipeData::Landmarks3D;
					else if (substr == "bs")
						OutMeta.DataType = EFacepipeData::Blendshapes;
					else if (substr == "mat44")
						OutMeta.DataType = EFacepipeData::Matrices4x4;
					break;
				}
				case 6:
				{
					OutContent = std::string_view(Message.begin() + b, Message.end());
					return true;
				}
			}
		}

		return false; // only when we reach case 6 are we successful
	}

	bool GetBlendshapes(const MetaData& MessageMeta, const std::string_view Content, std::map<std::string, float>& OutBlendshapes)
	{
		if (MessageMeta.DataType != EFacepipeData::Blendshapes)
			return false;

		size_t b = 0;
		size_t e = 0;
		while (NextSubstring(Content, '|', b, e))
		{
			const std::string_view BS(Content.begin() + b, Content.begin() + e);

			size_t b1=0;
			size_t e1=0;
			if (NextSubstring(BS, '=', b1, e1))
			{
				std::string Name(BS.begin() + b1, BS.begin() + e1);

				if (NextSubstring(BS, '=', b1, e1))
				{
					OutBlendshapes[Name] = ParseFloat(std::string_view(BS.begin() + b1, BS.begin() + e1));
				}
			}
		}
		
		return true;
	}

	bool GetLandmarks(const MetaData& MessageMeta, const std::string_view Content, std::vector<float>& OutValues, int& ImageWidth, int& ImageHeight)
	{
		if (MessageMeta.DataType != EFacepipeData::Landmarks2D && MessageMeta.DataType != EFacepipeData::Landmarks3D)
			return false;

		size_t b = 0;
		size_t e = 0;
		if (!NextSubstring(Content, '|', b, e))
			return false;

		std::vector<int> ImageDimensions = ParseArray<int>(std::string_view(Content.begin()+b, Content.begin()+e));
		if (ImageDimensions.size() != 2)
			return false;
		ImageWidth = ImageDimensions[0];
		ImageHeight = ImageDimensions[1];

		if (!NextSubstring(Content, '|', b, e))
			return false;

		OutValues = ParseArray<float>(std::string_view(Content.begin()+b, Content.begin()+e));

		return true;
	}

	bool GetTransforms(const MetaData& MessageMeta, const std::string_view Content, std::map<std::string, std::vector<float>>& OutMatrices)
	{
		if (MessageMeta.DataType != EFacepipeData::Matrices4x4)
			return false;

		size_t b = 0;
		size_t e = 0;
		while (NextSubstring(Content, '|', b, e))
		{
			const std::string_view Mat4x4(Content.begin() + b, Content.begin() + e);

			size_t b1 = 0;
			size_t e1 = 0;
			if (NextSubstring(Mat4x4, '=', b1, e1))
			{
				std::string Name(Mat4x4.begin() + b1, Mat4x4.begin() + e1);

				if (NextSubstring(Mat4x4, '=', b1, e1))
				{
					OutMatrices[Name] = ParseFloatArray(std::string_view(Mat4x4.begin() + b1, Mat4x4.begin() + e1));
				}
			}
		}

		return true;
	}
}
