#pragma once
#include <Toshi/File/TFile.h>

class ADebugLog
{
public:
	static constexpr size_t DEBUGLOG_FMTSTR_BUFLEN = 0x400;

public:
	ADebugLog(const char* a_szBaseName, const char* a_szExtension, int a_iBufferCapacity);
	~ADebugLog();

	void CreateFile();
	void Write(const char* a_szFormat, ...);

	void Flush();

private:
	char* m_szBaseName;
	char* m_szExtension;
	char* m_pBuffer;
	int m_iBufSize;
	int m_iBufUsedSize;
};
