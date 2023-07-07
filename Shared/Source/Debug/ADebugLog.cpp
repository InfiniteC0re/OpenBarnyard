#include "pch.h"
#include "ADebugLog.h"

ADebugLog::ADebugLog(const char* a_szBaseName, const char* a_szExtension, int a_iBufferCapacity)
{
	m_iBufUsedSize = 0;
	m_iBufSize = a_iBufferCapacity;

	if (m_iBufSize <= 0)
		m_iBufSize = 16384;

	TASSERT(m_iBufSize > DEBUGLOG_FMTSTR_BUFLEN);

	auto iBaseNameLen = Toshi::TStringManager::String8Length(a_szBaseName);
	TASSERT(iBaseNameLen <= 8);

	m_szBaseName = TREINTERPRETCAST(char*, TMemalign(16, iBaseNameLen + 1));
	Toshi::TStringManager::String8Copy(m_szBaseName, a_szBaseName);

	auto iSecondaryNameLen = Toshi::TStringManager::String8Length(a_szExtension);
	m_szExtension = TREINTERPRETCAST(char*, TMemalign(16, iSecondaryNameLen + 1));
	Toshi::TStringManager::String8Copy(m_szExtension, a_szExtension);

	m_pBuffer = TREINTERPRETCAST(char*, TMemalign(4, m_iBufSize));
}

ADebugLog::~ADebugLog()
{
	Flush();
	TFree(m_szExtension);
	TFree(m_szBaseName);
	TFree(m_pBuffer);
}

void ADebugLog::CreateFile()
{
	char szBuffer[256];
	Toshi::TStringManager::String8Format(szBuffer, sizeof(szBuffer), "%s.%s", m_szBaseName, m_szExtension);
	
	auto pFile = Toshi::TFile::Create(
		szBuffer,
		Toshi::TFile::FileMode_CreateNew | Toshi::TFile::FileMode_Write | Toshi::TFile::FileMode_NoBuffer
	);

	TASSERT(pFile != TNULL);
	if (pFile) pFile->Destroy();
}

void ADebugLog::Write(const char* a_szFormat, ...)
{
	static char s_FmtStr[DEBUGLOG_FMTSTR_BUFLEN];
	
	va_list args;
	va_start(args, a_szFormat);
	Toshi::T2String8::FormatV(s_FmtStr, DEBUGLOG_FMTSTR_BUFLEN, a_szFormat, &args);
	s_FmtStr[DEBUGLOG_FMTSTR_BUFLEN - 1] = '\0';
	va_end(args);

	auto iFmtStrSize = int(Toshi::TStringManager::String8Length(s_FmtStr));

	if (m_iBufSize - m_iBufUsedSize < iFmtStrSize)
		Flush();

	Toshi::TUtil::MemCopy(m_pBuffer + m_iBufUsedSize, s_FmtStr, iFmtStrSize);
	m_iBufUsedSize += iFmtStrSize;

	if (m_iBufSize <= m_iBufUsedSize)
		Flush();
}

void ADebugLog::Flush()
{
	if (m_iBufUsedSize > 0)
	{
		char szBuffer[256];
		Toshi::TStringManager::String8Format(szBuffer, sizeof(szBuffer), "%s.%s", m_szBaseName, m_szExtension);

		int iWriteSize = m_iBufUsedSize;
		m_iBufUsedSize = 0;

		auto pFile = Toshi::TFile::Create(
			szBuffer,
			Toshi::TFile::FileMode_CreateNew | Toshi::TFile::FileMode_Write | Toshi::TFile::FileMode_NoBuffer
		);

		TASSERT(pFile != TNULL);
		
		if (pFile)
		{
			pFile->Write(m_pBuffer, iWriteSize);
			pFile->Destroy();
		}
	}
}
