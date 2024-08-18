#include "ToshiPCH.h"
#include "T2NamedPipeServer_Win.h"

#include "Toshi/T2String.h"

#include <Windows.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_START

T2NamedPipeServerThread::T2NamedPipeServerThread( T2NamedPipeServer* a_pServer ) :
    m_pServer( a_pServer )
{
}

void T2NamedPipeServerThread::Main()
{
	while ( TTRUE )
	{
		TBOOL bHasClient = TTRUE;

		if ( ConnectNamedPipe( m_pServer->m_hPipe, NULL ) == FALSE )
		{
			auto iErr = GetLastError();

			if ( iErr == ERROR_NO_DATA )
			{
				DisconnectNamedPipe( m_pServer->m_hPipe );
				TTRACE( "A client has disconnected from the '%s' named pipe\n", m_pServer->m_szName );
				m_pServer->m_bHasClient = TFALSE;
				bHasClient              = TFALSE;
			}
			else if ( iErr != ERROR_PIPE_CONNECTED )
			{
				TASSERT( TFALSE, "Failed ConnectNamedPipe" );
				bHasClient = TFALSE;
			}
		}

		if ( bHasClient )
		{
			if ( m_pServer->m_bHasClient == TFALSE )
			{
				TTRACE( "A client has connected to the '%s' named pipe\n", m_pServer->m_szName );
				m_pServer->m_bHasClient = TTRUE;
			}

			if ( m_pServer->m_fnUpdateStream )
			{
				m_pServer->m_fnUpdateStream( m_pServer->m_pMemory, m_pServer->m_uiMemorySize, m_pServer->m_pUserData );
			}

			if ( m_pServer->m_pMemory && m_pServer->m_uiMemorySize > 0 )
			{
				WriteFile(
				    m_pServer->m_hPipe,
				    m_pServer->m_pMemory,
				    m_pServer->m_uiMemorySize,
				    NULL,
				    NULL );
			}
		}

		ThreadSleep( m_pServer->m_uiSendInterval );
	}
}

T2NamedPipeServer::T2NamedPipeServer() :
    m_pThread( TNULL ),
    m_pMemory( TNULL ),
    m_fnUpdateStream( TNULL ),
    m_uiMemorySize( 0 ),
    m_hPipe( INVALID_HANDLE_VALUE ),
    m_bHasClient( TFALSE ),
    m_bStarted( TFALSE ),
    m_pUserData( TNULL )
{
}

T2NamedPipeServer::~T2NamedPipeServer()
{
	Stop();
}

TBOOL T2NamedPipeServer::Start( const TCHAR* a_szName, TUINT a_uiSendInterval, TUINT a_uiOutBufferSize, TUINT a_uiInBufferSize )
{
	TASSERT( TNULL != a_szName );
	TASSERT( TFALSE == m_bStarted );

	m_uiSendInterval = a_uiSendInterval;
	m_szName         = a_szName;

	T2FormatString256 pipeName;
	pipeName.Format( "\\\\.\\pipe\\%s", a_szName );

	m_hPipe = CreateNamedPipeA(
	    pipeName.Get(),
	    PIPE_ACCESS_OUTBOUND,
	    PIPE_TYPE_MESSAGE,
	    1,
	    a_uiOutBufferSize,
	    a_uiInBufferSize,
	    0,
	    NULL );

	TASSERT( INVALID_HANDLE_VALUE != m_hPipe );

	m_pThread  = new T2NamedPipeServerThread( this );
	m_bStarted = m_pThread->Create( 0, TThread::THREAD_PRIORITY_LOWEST, 0 );

	TTRACE( "Started named pipe '%s'\n", m_szName );

	TASSERT( TTRUE == m_bStarted );
	return m_bStarted;
}

void T2NamedPipeServer::Stop()
{
	if ( m_bStarted && m_pThread )
	{
		m_pThread->Destroy();

		delete m_pThread;
		m_pThread  = TNULL;
		m_bStarted = TFALSE;
	}
}

void T2NamedPipeServer::SetMemoryStreamUpdateCallback( UpdateStreamCallback_t a_fnUpdateCallback )
{
	m_fnUpdateStream = a_fnUpdateCallback;
}

void T2NamedPipeServer::SetMemoryStream( void* a_pMemory, TUINT a_uiSize )
{
	m_pMemory      = a_pMemory;
	m_uiMemorySize = a_uiSize;
}

TOSHI_NAMESPACE_END
