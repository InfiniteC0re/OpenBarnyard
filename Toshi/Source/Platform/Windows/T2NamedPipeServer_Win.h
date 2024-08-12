#pragma once
#include "Toshi/Defines.h"
#include "TThread_Win.h"

namespace Toshi
{

class T2NamedPipeServer;

class T2NamedPipeServerThread : public TThread
{
public:
    T2NamedPipeServerThread( T2NamedPipeServer* a_pServer );

    virtual void Main() override;

private:
    T2NamedPipeServer* m_pServer;
};

//-----------------------------------------------------------------------------
// Use this class to establish communication between two different processes.
// Note: Supports only one connection instance.
//-----------------------------------------------------------------------------
class T2NamedPipeServer
{
public:
    friend T2NamedPipeServerThread;

    using UpdateStreamCallback_t = void ( * )( void*& a_rMemoryStream, TUINT& a_rDataSize, void* a_pUserData );

public:
    T2NamedPipeServer();
    ~T2NamedPipeServer();

    TBOOL Start( const TCHAR* a_szName, TUINT a_uiSendInterval, TUINT a_uiOutBufferSize = 1, TUINT a_uiInBufferSize = 64 * 1024 );

    void Stop();

    void SetMemoryStreamUpdateCallback( UpdateStreamCallback_t a_fnUpdateCallback );
    void SetMemoryStream( void* a_pMemory, TUINT a_uiSize );

    void SetUserData( void* a_pUserData ) { m_pUserData = a_pUserData; }

    TBOOL HasConnectedClient() const { return m_bHasClient; }

private:
    T2NamedPipeServerThread* m_pThread;
    void*                    m_pUserData;
    const TCHAR*             m_szName;
    UpdateStreamCallback_t   m_fnUpdateStream;
    TUINT                    m_uiSendInterval;
    void*                    m_pMemory;
    TUINT                    m_uiMemorySize;
    HANDLE                   m_hPipe;
    TBOOL                    m_bHasClient;
    TBOOL                    m_bStarted;
};

} // namespace Toshi