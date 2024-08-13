#include "pch.h"
#include "ASplitsServer.h"

#include <Toshi/T2String.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

struct MsgQueueHeader_t
{
	TUINT uiTotalSize;
	TUINT uiNumMessages;
};

static TCHAR s_szBuffer[ 8192 ];
static TUINT s_uiBufferPos = 0;

ASplitsServer::ASplitsServer() :
	m_uiBufferSize( 0 )
{
	m_NamedPipe.Start(
		"BYSpeedrunHelper",
		100,
		0,
		MAX_BUFFER_SIZE );

	m_pMemBlock = g_pMemory->CreateMemBlock(
		128 * 1024,
		"Autosplitter",
		g_pMemory->GetGlobalBlock(),
		0 );

	TASSERT( m_pMemBlock != TNULL );

	TBOOL bCreatedMutex = m_EventsMutex.Create();
	TASSERT( bCreatedMutex == TTRUE );

	m_NamedPipe.SetUserData( this );
	m_NamedPipe.SetMemoryStreamUpdateCallback( []( void*& a_rMemoryStream, TUINT& a_rDataSize, void* a_pUserData ) {
		ASplitsServer* pSplits = (ASplitsServer*)a_pUserData;
		T2MUTEX_LOCK_SCOPE( pSplits->m_EventsMutex );

		// Reset buffer
		a_rMemoryStream = TNULL;
		a_rDataSize		= 0;

		// If anything is ready to send, process and send the data
		if ( pSplits->m_QueuedEvents.Size() > 0 )
		{
			// Initialise header
			MsgQueueHeader_t* pHeader = (MsgQueueHeader_t*)pSplits->m_Buffer;
			pHeader->uiTotalSize	  = 0;
			pHeader->uiNumMessages	  = pSplits->m_QueuedEvents.Size();

			pSplits->m_uiBufferSize = sizeof( MsgQueueHeader_t );

			// Copy events to the buffer
			for ( TSIZE i = 0; i < pSplits->m_QueuedEvents.Size(); i++ )
			{
				TCHAR* pchMsg	= pSplits->m_QueuedEvents[ i ];
				TUINT  uiMsgLen = T2String8::Length( pchMsg );

				// Store message length
				*(TUINT32*)( pSplits->m_Buffer + pSplits->m_uiBufferSize ) = uiMsgLen;
				pSplits->m_uiBufferSize += sizeof( TUINT32 );

				// Store the message
				for ( TSIZE k = 0; k < uiMsgLen; k++ )
				{
					pSplits->m_Buffer[ pSplits->m_uiBufferSize++ ] = pchMsg[ k ];
				}

				pSplits->m_Buffer[ pSplits->m_uiBufferSize ] = ';';
			}

			// Reset buffer pos
			s_uiBufferPos = 0;

			pSplits->m_QueuedEvents.Clear();
			pSplits->m_Buffer[ pSplits->m_uiBufferSize++ ] = '\0';

			// Write all messages size
			pHeader->uiTotalSize = pSplits->m_uiBufferSize;

			if ( pSplits->m_uiBufferSize > 8 )
			{
				// Send values to the named pipe class
				a_rMemoryStream = pSplits->m_Buffer;
				a_rDataSize		= pSplits->m_uiBufferSize;
			}
		}
	} );
}

void ASplitsServer::StopServer()
{
	m_NamedPipe.Stop();
}

void ASplitsServer::StartRun()
{
	if ( !m_NamedPipe.HasConnectedClient() )
		return;

	WriteString( "1" );
}

void ASplitsServer::EndRun( TINT a_iMilliseconds, TINT a_iSeconds, TINT a_iMinutes, TINT a_iHours )
{
	if ( !m_NamedPipe.HasConnectedClient() )
		return;

	T2FormatString256 fmtStr;
	fmtStr.Format( "2%d_%d_%d_%d", a_iHours, a_iMinutes, a_iSeconds, a_iMilliseconds );
	WriteString( fmtStr.Get() );
}

void ASplitsServer::Reset()
{
	if ( !m_NamedPipe.HasConnectedClient() )
		return;

	WriteString( "3" );
}

void ASplitsServer::Resume()
{
	if ( !m_NamedPipe.HasConnectedClient() )
		return;

	WriteString( "4" );
}

void ASplitsServer::Pause()
{
	if ( !m_NamedPipe.HasConnectedClient() )
		return;

	WriteString( "5" );
}

void ASplitsServer::SetLoadingStart()
{
	if ( !m_NamedPipe.HasConnectedClient() )
		return;

	WriteString( "8" );
}

void ASplitsServer::SetLoadingEnd()
{
	if ( !m_NamedPipe.HasConnectedClient() )
		return;

	WriteString( "9" );
}

void ASplitsServer::Split( TINT a_iMilliseconds, TINT a_iSeconds, TINT a_iMinutes, TINT a_iHours )
{
	if ( !m_NamedPipe.HasConnectedClient() )
		return;

	T2FormatString256 fmtStr;
	fmtStr.Format( "6%d_%d_%d_%d", a_iHours, a_iMinutes, a_iSeconds, a_iMilliseconds );
	WriteString( fmtStr.Get() );
}

void ASplitsServer::SendTime( TINT a_iMilliseconds, TINT a_iSeconds, TINT a_iMinutes, TINT a_iHours )
{
	if ( !m_NamedPipe.HasConnectedClient() )
		return;

	T2FormatString256 fmtStr;
	fmtStr.Format( "7%d_%d_%d_%d", a_iHours, a_iMinutes, a_iSeconds, a_iMilliseconds );
	WriteString( fmtStr.Get() );
}

void ASplitsServer::WriteString( const TCHAR* a_pchBuffer )
{
	if ( !m_NamedPipe.HasConnectedClient() || m_QueuedEvents.Size() >= m_QueuedEvents.Capacity() || s_uiBufferPos >= TARRAYSIZE( s_szBuffer ) )
		return;

	T2MUTEX_LOCK_SCOPE( m_EventsMutex );

	TUINT uiSize = T2String8::Length( a_pchBuffer );

	T2String8::Copy( s_szBuffer + s_uiBufferPos, a_pchBuffer, uiSize );
	s_szBuffer[ s_uiBufferPos + uiSize ] = '\0';

	m_QueuedEvents.PushBack( s_szBuffer + s_uiBufferPos );
	s_uiBufferPos += uiSize + 1;
}
