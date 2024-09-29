#include "pch.h"
#include "ATextureUpdater.h"

#include <BYardSDK/THookedSystemManager.h>

#include <Toshi/TScheduler.h>

TOSHI_NAMESPACE_USING

void ATextureUpdateWatcher::Main()
{
	m_Overlapped.hEvent = CreateEvent( NULL, FALSE, 0, NULL );

	ReadDirectoryChangesW(
	    m_hFile, m_ChangeBuf, 1024, TRUE,
	    FILE_NOTIFY_CHANGE_FILE_NAME |
	        FILE_NOTIFY_CHANGE_DIR_NAME |
	        FILE_NOTIFY_CHANGE_LAST_WRITE,
	    NULL, &m_Overlapped, NULL
	);

	while ( TTRUE )
	{
		DWORD result = WaitForSingleObject( m_Overlapped.hEvent, 0 );

		if ( m_bDestroyed )
		{
			CloseHandle( m_hFile );
			Exit( this );
			return;
		}

		if ( result == WAIT_OBJECT_0 )
		{
			DWORD bytes_transferred;
			GetOverlappedResult( m_hFile, &m_Overlapped, &bytes_transferred, FALSE );
			FILE_NOTIFY_INFORMATION* event = (FILE_NOTIFY_INFORMATION*)m_ChangeBuf;

			if ( g_bAutoReload )
			{
				TMutexLock lock( g_LoadMutex );

				while ( TTRUE )
				{
					DWORD name_len = event->FileNameLength / sizeof( wchar_t );

					static char sFileName[ MAX_PATH ];
					TStringManager::StringUnicodeToChar( sFileName, event->FileName, name_len );
					TBOOL bIsDDS = T2String8::CompareNoCase( sFileName + name_len - 4, ".dds" ) == 0;

					if ( bIsDDS )
					{
						g_LastDumpTimer.Update();

						if ( g_LastDumpTimer.GetDelta() > 1.0f )
						{
							switch ( event->Action )
							{
								case FILE_ACTION_ADDED:
								case FILE_ACTION_MODIFIED:
								case FILE_ACTION_REMOVED:
								{
									sFileName[ name_len - 3 ] = 't';
									sFileName[ name_len - 2 ] = 'g';
									sFileName[ name_len - 1 ] = 'a';

									ThreadSleep( 50 );
									ATextureUpdater::ReloadTexture( sFileName );
								}
							}
						}
					}

					// Are there more events to handle?
					if ( event->NextEntryOffset )
					{
						*( (TUINT8**)&event ) += event->NextEntryOffset;
					}
					else
					{
						break;
					}
				}
			}

			// Queue the next event
			ReadDirectoryChangesW(
			    m_hFile, m_ChangeBuf, 1024, TRUE,
			    FILE_NOTIFY_CHANGE_FILE_NAME |
			        FILE_NOTIFY_CHANGE_DIR_NAME |
			        FILE_NOTIFY_CHANGE_LAST_WRITE,
			    NULL, &m_Overlapped, NULL
			);
		}
	}
}

void ATextureUpdateWatcher::Init( const char* a_szPath )
{
	m_szPath = a_szPath;

	m_hFile = CreateFileA(
	    m_szPath,
	    FILE_LIST_DIRECTORY,
	    FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
	    NULL,
	    OPEN_EXISTING,
	    FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
	    NULL
	);
}

void ATextureUpdateWatcher::Destroy()
{
	m_bDestroyed = TTRUE;
}
