#pragma once
#include <Toshi/TSingleton.h>
#include <Toshi/T2Vector.h>
#include <Thread/T2MutexLock.h>
#include <ToshiTools/T2NamedPipeServer.h>

class ASplitsServer : public Toshi::TSingleton<ASplitsServer>
{
public:
	static constexpr TSIZE MAX_BUFFER_SIZE = 65535;

public:
	ASplitsServer();

	void StopServer();

	void StartRun();
	void EndRun( TINT a_iMilliseconds, TINT a_iSeconds, TINT a_iMinutes, TINT a_iHours );

	void Reset();

	void Resume();
	void Pause();

	void SetLoadingStart();
	void SetLoadingEnd();

	void Split( TINT a_iMilliseconds, TINT a_iSeconds, TINT a_iMinutes, TINT a_iHours );
	void SendTime( TINT a_iMilliseconds, TINT a_iSeconds, TINT a_iMinutes, TINT a_iHours );

private:
	void WriteString( const char* a_pchBuffer );

private:
	Toshi::T2NamedPipeServer   m_NamedPipe;
	Toshi::T2Mutex             m_EventsMutex;
	Toshi::T2Vector<char*, 64> m_QueuedEvents;
	Toshi::TMemory::MemBlock*  m_pMemBlock;
	TCHAR                      m_Buffer[ MAX_BUFFER_SIZE ];
	TSIZE                      m_uiBufferSize;
};
