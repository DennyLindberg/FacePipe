#pragma once

#include <stdint.h>
#include <string>
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

	// similar intention as std::string_view but it is actually supported...
	struct VectorView
	{
		VectorView(size_t start = 0) : b(start), e(start) {}
		VectorView(size_t start, size_t end) : b(start), e(end) {}
		size_t b = 0;
		size_t e = 0;

		inline std::string String(const std::vector<char>& Message)
		{
			return std::string(Message.begin() + b, Message.begin() + e);
		}

		double ParseDouble(const std::vector<char>& Message) noexcept;
		float ParseFloat(const std::vector<char>& Message) noexcept;
		int ParseInt(const std::vector<char>& Message) noexcept;
		std::vector<float> ParseFloatArray(const std::vector<char>& Message);

		template<typename T>
		std::vector<T> ParseArray(const std::vector<char>& Message)
		{
			std::vector<T> values;
			VectorView SubView(b);
			while (SubView.NextSubstring(Message, ',', e))
			{
				if constexpr (std::is_same<T, float>())
					values.push_back(SubView.ParseFloat(Message));
				else if constexpr (std::is_same<T, double>())
					values.push_back(SubView.ParseDouble(Message));
				else if constexpr (std::is_same<T, int>())
					values.push_back(SubView.ParseInt(Message));
			}

			return values;
		}

		bool NextSubstring(const std::vector<char>& Message, char Delimiter, size_t End);
	};

	struct MessageInfo
	{
		int Scene = 0;			// Scene of camera and subject
		int Camera = 0;			// Camera the subject was captured in
		int Subject = 0;		// The subject the data belongs to

		std::string Source = "None";						// Which application / capture method produced this data
		EFacepipeData DataType = EFacepipeData::INVALID;	// What the data contains
		double Time = 0.0;									// When the message was sent on the source side

		VectorView ContentView;								// Range in message where content should be parsed
	};

	struct Frame
	{
		MessageInfo Meta;
		std::map<std::string, float> Blendshapes;
		std::map<std::string, std::vector<float>> Matrices;
		std::vector<float> Landmarks;

		int ImageWidth = 0;
		int ImageHeight = 0;
	};
}

namespace FacePipe
{
	bool ParseHeader(const std::vector<char>& Message, MessageInfo& OutMeta);

	bool GetBlendshapes(const std::vector<char>& Message, const MessageInfo& Info, std::map<std::string, float>& OutBlendshapes);
	bool GetLandmarks(const std::vector<char>& Message, const MessageInfo& Info, std::vector<float>& OutValues, int& ImageWidth, int& ImageHeight);
	bool GetMatrices(const std::vector<char>& Message, const MessageInfo& Info, std::map<std::string, std::vector<float>>& OutMatrices);
}
