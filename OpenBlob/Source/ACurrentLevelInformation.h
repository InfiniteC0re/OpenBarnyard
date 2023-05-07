#pragma once
#include <Toshi/File/TTRB.h>
#include "AAssetStreaming.h"
#include "AAssetStreamingJobs.h"

class ACurrentLevelInformation
{
	using DataBlock = int;
	static constexpr int DataBlockCount = 24;

	static constexpr std::pair<int, const char*> sm_dataBlockFilenames[DataBlockCount] = {
		{1, "Common/CameraShakeCommon.trb"},
		{1, "Blob_FX/FXParticles.trb"},
		{1, "Entities/BlobProperties"},
		{1, "Sound/SoundBreakPoints,trb"}
	};

	ACurrentLevelInformation(char* a_szLevelName) : m_szLevelName(a_szLevelName)
	{
		Toshi::TUtil::MemClear(m_datablocks, sizeof(m_datablocks));
		Toshi::TUtil::MemClear(m_streamingJobs, sizeof(m_streamingJobs));
	}

	void Create(DataBlock a_type);
	Toshi::TTRB* GetDataBlock(DataBlock a_type);
	void Stream();

	char* m_szLevelName;
	ATRBLoaderJob* m_streamingJobs[DataBlockCount];
	Toshi::TTRB* m_datablocks[DataBlockCount];
};

