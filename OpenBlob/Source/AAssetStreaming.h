#pragma once
#include <Toshi/File/TFileStream.h>
#include <Toshi/Utils/TSingleton.h>

class AAssetStreaming : public Toshi::TSingleton<AAssetStreaming>
{
public:
	AAssetStreaming()
	{
		TTODO("The whole function");
	}

	Toshi::TFileStream m_FileStream; // 0x0C
};