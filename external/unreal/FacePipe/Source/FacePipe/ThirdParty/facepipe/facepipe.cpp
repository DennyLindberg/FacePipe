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
	double VectorView::ParseDouble(const std::vector<char>& Message) noexcept
	{
		std::string s(Message.begin() + b, Message.begin() + e);
		char* pEnd = nullptr;
		double v = std::strtod(s.c_str(), &pEnd);
		return (*pEnd) ? 0.0f : v;
	}

	float VectorView::ParseFloat(const std::vector<char>& Message) noexcept
	{
		return (float)ParseDouble(Message);
	}

	int VectorView::ParseInt(const std::vector<char>& Message) noexcept
	{
		std::string s(Message.begin() + b, Message.begin() + e);
		char* pEnd = nullptr;
		int v = (int)std::strtol(s.c_str(), &pEnd, 10); // base 10
		return (*pEnd) ? 0 : v;
	}

	std::vector<float> VectorView::ParseFloatArray(const std::vector<char>& Message)
	{
		std::vector<float> values;
		VectorView SubView(b);
		while (SubView.NextSubstring(Message, ',', e))
		{
			values.push_back(SubView.ParseFloat(Message));
		}

		return values;
	}

	bool VectorView::NextSubstring(const std::vector<char>& Message, char Delimiter, size_t End)
	{
		if (End > Message.size())
			End = Message.size();

		if (e + 1 >= End)
			return false;

		// Check if we are standing on a delimiter - if we are we start over on the next character and scan forward
		if (Message[e] == Delimiter)
		{
			++e;
			b = e;
		}

		while (e < End)
		{
			if (Message[e] == Delimiter)
			{
				break;
			}

			++e;
		}

		return true; // end of vector or delimiter
	}
}

namespace FacePipe
{
	bool ParseHeader(const std::vector<char>& Message, MessageInfo& OutInfo)
	{
		// a|protocol|source|scene,camera,subject|time|content

		OutInfo = MessageInfo();

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
		
		VectorView HeaderView(0,1);
		int index = 0;
		while (HeaderView.NextSubstring(Message, '|', Message.size()))
		{
			switch (++index)
			{
				case 1: 
				{ 
					if (HeaderView.String(Message) != "facepipe")
					{
						return false;
					}
					break;
				}
				case 2: 
				{ 
					OutInfo.Source = HeaderView.String(Message); 
					break;
				}
				case 3: 
				{
					std::vector<int> channels = HeaderView.ParseArray<int>(Message);
					if (channels.size() != 3)
					{
						return false;
					}
					OutInfo.Scene = channels[0];
					OutInfo.Camera = channels[1];
					OutInfo.Subject = channels[2];
					break;
				}
				case 4:
				{
					OutInfo.Time = HeaderView.ParseDouble(Message);
					break;
				}
				case 5:
				{
					std::string type = HeaderView.String(Message);
					if (type == "l2d")
						OutInfo.DataType = EFacepipeData::Landmarks2D;
					else if (type == "l3d")
						OutInfo.DataType = EFacepipeData::Landmarks3D;
					else if (type == "bs")
						OutInfo.DataType = EFacepipeData::Blendshapes;
					else if (type == "mat44")
						OutInfo.DataType = EFacepipeData::Matrices4x4;
					break;
				}
				case 6:
				{
					OutInfo.ContentView = VectorView(HeaderView.b, Message.size());
					return true;
				}
			}
		}

		return false; // only when we reach case 6 are we successful
	}

	bool GetBlendshapes(const std::vector<char>& Message, const MessageInfo& Info, std::map<std::string, float>& OutBlendshapes)
	{
		if (Info.DataType != EFacepipeData::Blendshapes)
			return false;

		VectorView BSView(Info.ContentView.b);
		while (BSView.NextSubstring(Message, '|', Info.ContentView.e))
		{
			VectorView TupleView(BSView.b);
			if (TupleView.NextSubstring(Message, '=', BSView.e))
			{
				std::string Name = TupleView.String(Message);

				if (TupleView.NextSubstring(Message, '=', BSView.e))
				{
					OutBlendshapes[Name] = TupleView.ParseFloat(Message);
				}
			}
		}
		
		return true;
	}

	bool GetLandmarks(const std::vector<char>& Message, const MessageInfo& Info, std::vector<float>& OutValues, int& ImageWidth, int& ImageHeight)
	{
		if (Info.DataType != EFacepipeData::Landmarks2D && Info.DataType != EFacepipeData::Landmarks3D)
			return false;

		VectorView LandmarkView(Info.ContentView.b);
		if (!LandmarkView.NextSubstring(Message, '|', Info.ContentView.e))
			return false;

		std::vector<int> ImageDimensions = LandmarkView.ParseArray<int>(Message);
		if (ImageDimensions.size() != 2)
			return false;
		ImageWidth = ImageDimensions[0];
		ImageHeight = ImageDimensions[1];

		if (!LandmarkView.NextSubstring(Message, '|', Info.ContentView.e))
			return false;

		OutValues = LandmarkView.ParseArray<float>(Message);

		return true;
	}

	bool GetMatrices(const std::vector<char>& Message, const MessageInfo& Info, std::map<std::string, std::vector<float>>& OutMatrices)
	{
		if (Info.DataType != EFacepipeData::Matrices4x4)
			return false;

		VectorView MatView(Info.ContentView.b);
		while (MatView.NextSubstring(Message, '|', Info.ContentView.e))
		{
			VectorView TupleView(MatView.b);
			if (TupleView.NextSubstring(Message, '=', MatView.e))
			{
				std::string Name = TupleView.String(Message);

				if (TupleView.NextSubstring(Message, '=', MatView.e))
				{
					OutMatrices[Name] = TupleView.ParseFloatArray(Message);
				}
			}
		}

		return true;
	}
}
