#pragma once
#include <fstream>

struct SVertexPaintColorData {
	std::vector<Vector3> myColors;
	int myVertexMeshID;
};

class CBinReader
{
public:
	CBinReader();
	~CBinReader();

	static SVertexPaintColorData LoadVertexColorData(const std::string& aBinFilePath);

private:

	template<typename T>
	static size_t Read(T& aData, char* aStreamPtr, const unsigned int aCount = 1)
	{
		memcpy(&aData, aStreamPtr, sizeof(T) * aCount);
		return sizeof(T);
	}

	static std::string ReadStringAuto(char* aStreamPtr)
	{
		int length = 0;
		aStreamPtr += Read(length, aStreamPtr);

		std::string text = "";
		aStreamPtr += ReadString(text, aStreamPtr, length);

		return text;
	}

	static size_t ReadString(std::string& data, char* aStreamPtr, size_t aLength)
	{
		data.reserve(aLength + 1);
		memcpy(&data.data()[0], aStreamPtr, sizeof(char) * aLength);
		data.data()[aLength] = '\0';
		return sizeof(char) * aLength;
	}
};