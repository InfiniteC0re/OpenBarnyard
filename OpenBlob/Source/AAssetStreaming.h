#pragma once
#include <Toshi/File/TFileStream.h>
#include <Toshi/Utils/TSingleton.h>

class AAssetStreaming : public Toshi::TSingleton<AAssetStreaming>
{
public:
	AAssetStreaming()
	{
		// Create thread and run it
		m_FileStream.Create(0, Toshi::TThread::THREAD_PRIORITY_ABOVE_NORMAL, 0);
		TTODO("The whole function");
	}

	Toshi::TFileStream m_FileStream; // 0x0C
};