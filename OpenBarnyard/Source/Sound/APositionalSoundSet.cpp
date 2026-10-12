#include "pch.h"
#include "APositionalSoundSet.h"
#include "ASoundManager.h"
#include "Helpers/AGameTimeManager.h"

#include <Plugins/PPropertyParser/PBProperties.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: inlined in retail (see APositionalSoundSet::Create)
APositionalSoundSet::APositionalSoundSet()
{
	m_iNumSounds    = 0;
	m_pSounds       = TNULL;
	m_bLoaded       = TFALSE;
	m_bPaused       = TFALSE;
	m_iLastDayPhase = 5;
}

// $Barnyard: FUNCTION 005d3820
APositionalSoundSet::~APositionalSoundSet()
{
	Unload();
}

// $Barnyard: FUNCTION 005d3350
void APositionalSoundSet::Create()
{
	new APositionalSoundSet();
}

// $Barnyard: FUNCTION 005d38f0
void APositionalSoundSet::Destroy()
{
	if ( IsSingletonCreated() )
		delete GetSingleton();
}

// $Barnyard: FUNCTION 005d3920
void APositionalSoundSet::LoadFromProperty( const PBPropertyValue* a_pFiles )
{
	const PBPropertyValueArray* pFiles = a_pFiles->GetArray();

	for ( TUINT i = 0; i < pFiles->GetSize(); i++ )
	{
		TPString8 strFile = pFiles->GetValue( i )->GetTPString8();

		if ( Load( strFile.GetString() ) == TTRUE )
			return;
	}
}

// $Barnyard: FUNCTION 005d37b0
void APositionalSoundSet::UnloadFromProperty( const PBPropertyValue* a_pFiles )
{
	Unload();
}

// $Barnyard: FUNCTION 005d3870
TBOOL APositionalSoundSet::Load( const TCHAR* a_szFileName )
{
	Unload();
	m_bPaused = TFALSE;

	if ( TTRB::Load( a_szFileName, 0 ) != TTRB::ERROR_OK )
	{
		Unload();
		return TFALSE;
	}

	TINT* pNumSounds = CastSymbol<TINT>( "numsounds" );

	if ( !pNumSounds )
	{
		Unload();
		return TFALSE;
	}

	m_iNumSounds = *pNumSounds;
	m_pSounds    = CastSymbol<Sound>( "sounds" );

	if ( !m_pSounds )
	{
		Unload();
		return TFALSE;
	}

	m_iLastDayPhase = 5;
	m_bLoaded       = TTRUE;
	return TTRUE;
}

// $Barnyard: FUNCTION 005d33e0
void APositionalSoundSet::Unload()
{
	ASoundManager* pSoundManager = ASoundManager::GetSingleton();

	for ( TINT i = 0; i < m_iNumSounds; i++ )
	{
		pSoundManager->StopCue( m_pSounds[ i ].iCue );
		m_pSounds[ i ].iCue = -1;
	}

	m_pSounds    = TNULL;
	m_iNumSounds = 0;
	TTRB::Close();
	m_bLoaded = TFALSE;
}

// $Barnyard: FUNCTION 005d3440
void APositionalSoundSet::StopSounds()
{
	if ( m_pSounds )
	{
		ASoundManager* pSoundManager = ASoundManager::GetSingleton();

		for ( TINT i = 0; i < m_iNumSounds; i++ )
		{
			pSoundManager->StopCue( m_pSounds[ i ].iCue );
			m_pSounds[ i ].iCue = -1;
		}
	}

	m_iLastDayPhase = 5;
}

// $Barnyard: FUNCTION 005d34c0
void APositionalSoundSet::Update( TFLOAT a_fDeltaTime )
{
	if ( !m_bLoaded || m_bPaused == TTRUE )
		return;

	TINT           iDayPhase     = AGameTimeManager::GetSingleton()->GetCurrentDayPhase();
	ASoundManager* pSoundManager = ASoundManager::GetSingleton();
	TVector4       vecCameraPos  = pSoundManager->GetCameraData().Position;
	TUINT          uiPhaseMask   = 1 << iDayPhase;

	if ( iDayPhase == m_iLastDayPhase )
	{
		for ( TINT i = 0; i < m_iNumSounds; i++ )
		{
			Sound*   pSound = &m_pSounds[ i ];
			TVector4 vecToCamera( vecCameraPos.x - pSound->vecPosition.x, vecCameraPos.y - pSound->vecPosition.y, vecCameraPos.z - pSound->vecPosition.z, vecCameraPos.w );

			if ( !( pSound->uiFlags & uiPhaseMask ) )
				continue;

			if ( pSound->iCue == -1 )
			{
				// Fade in once the camera is close
				if ( vecToCamera.MagnitudeSq() < MAX_DISTANCE_SQ )
				{
					pSound->iCue = pSoundManager->PlayCueAtPositionEx( pSound->iSoundId, pSound->vecPosition, 0.0f, TFALSE, 0.0f, -1 );
					pSoundManager->FadeCue( pSound->iCue, 2.0f, pSound->fVolume, 3.0f );
				}
			}
			else if ( vecToCamera.MagnitudeSq() >= MAX_DISTANCE_SQ )
			{
				// Fade out once the camera is far
				pSoundManager->FadeOutCue( pSound->iCue, 2.0f, TTRUE, 3.0f );
				pSound->iCue = -1;
			}
		}
	}
	else
	{
		// The day phase has changed
		for ( TINT i = 0; i < m_iNumSounds; i++ )
		{
			Sound*   pSound = &m_pSounds[ i ];
			TVector4 vecToCamera( vecCameraPos.x - pSound->vecPosition.x, vecCameraPos.y - pSound->vecPosition.y, vecCameraPos.z - pSound->vecPosition.z, vecCameraPos.w );

			if ( pSound->uiFlags & uiPhaseMask )
			{
				if ( pSound->iCue == -1 && vecToCamera.MagnitudeSq() < MAX_DISTANCE_SQ )
				{
					if ( pSound->uiFlags & Sound::FLAGS_NO_FADE_IN )
					{
						pSound->iCue = pSoundManager->PlayCueAtPositionEx( pSound->iSoundId, pSound->vecPosition, pSound->fVolume, TFALSE, 0.0f, -1 );
					}
					else
					{
						pSound->iCue = pSoundManager->PlayCueAtPositionEx( pSound->iSoundId, pSound->vecPosition, 0.0f, TFALSE, 0.0f, -1 );
						pSoundManager->FadeCue( pSound->iCue, 5.0f, pSound->fVolume, 3.0f );
					}
				}
			}
			else if ( pSound->iCue != -1 )
			{
				// Isn't played in this day phase
				if ( vecToCamera.MagnitudeSq() >= MAX_DISTANCE_SQ )
					pSoundManager->FadeOutCue( pSound->iCue, 2.0f, TTRUE, 3.0f );
				else
					pSoundManager->FadeOutCue( pSound->iCue, 5.0f, TTRUE, 3.0f );

				pSound->iCue = -1;
			}
		}
	}

	m_iLastDayPhase = iDayPhase;
}

// $Barnyard: FUNCTION 005d37c0
void APositionalSoundSet::SetPaused( TBOOL a_bPaused )
{
	ASoundManager* pSoundManager = ASoundManager::GetSingleton();

	for ( TINT i = 0; i < m_iNumSounds; i++ )
		pSoundManager->PauseCue( m_pSounds[ i ].iCue, a_bPaused );

	m_bPaused = a_bPaused;
}
