#include "pch.h"
#include "AKeyframeLibStreamJob.h"

#include <Toshi/Render/TRenderInterface.h>

AKeyframeLibStreamJob::AKeyframeLibStreamJob() :
	TFileStreamJob(TNULL)
{
	m_pLibrary = TNULL;
	m_szName[0] = '\0';
}

void AKeyframeLibStreamJob::Process()
{
	
}

void AKeyframeLibStreamJob::Init(const char* a_szFileName)
{
	
}
