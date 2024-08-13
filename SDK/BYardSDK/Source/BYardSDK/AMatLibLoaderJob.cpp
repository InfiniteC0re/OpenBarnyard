#include "pch.h"
#include "AMatLibLoaderJob.h"

void AMatLibLoaderJob::BeginJob()
{
}

TBOOL AMatLibLoaderJob::RunJob()
{
	return TFALSE;
}

TBOOL AMatLibLoaderJob::CancelJob()
{
	return TTRUE;
}

void AMatLibLoaderJob::InitJob( const char* a_szFileName, Toshi::TTRB* a_pTRB, AMaterialLibrary*& a_rpOutMatLib, Toshi::TMemory::MemBlock* a_pMemBlock )
{
}
