#include "pch.h"
#include "AParticleEffectLocatorManager.h"
#include "AParticleEffectManager.h"
#include "ARandom.h"
#include "Cameras/ACameraManager.h"
#include "Terrain/ATerrainInterface.h"

#include <Toshi/TString8.h>
#include <File/TTRB.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TDEFINE_CLASS( AParticleEffectLocatorManager );

TPSTRING8_DECLARE( EffectName );
TPSTRING8_DECLARE( LocatorName );
TPSTRING8_DECLARE( Type );
TPSTRING8_DECLARE( Constant );
TPSTRING8_DECLARE( Uniform );
TPSTRING8_DECLARE( Random );
TPSTRING8_DECLARE( EmitTime );
TPSTRING8_DECLARE( EmitTimePlusMinus );
TPSTRING8_DECLARE( EmitDistance );

static constexpr const TCHAR* s_szLocatorListsPath = "data\\Particles\\LocatorLists";

// $Barnyard: FUNCTION 00650170
void AParticleEffectLocator::Stop()
{
	if ( uiFlags & FLAGS_PLAYING )
	{
		pHandle->GetInstance()->Stop();
		uiFlags &= ~FLAGS_PLAYING;
		pHandle = TNULL;
	}
}

// $Barnyard: FUNCTION 006501a0
void AParticleEffectLocator::ResetTimer()
{
	switch ( uiFlags & FLAGS_TYPEMASK )
	{
		case FLAGS_UNIFORM:
			fTimer = fEmitTime;
			break;
		case FLAGS_RANDOM:
			fTimer = ARandom::GetSingleton()->m_oRandom.GetFloatMinMax( -fEmitTimePlusMinus, fEmitTimePlusMinus ) + fEmitTime;
			break;
	}
}

// $Barnyard: FUNCTION 006501e0
void AParticleEffectLocator::Play()
{
	// NOTE: the rotation quaternion is passed as the direction vector, as in the original
	pHandle = AParticleEffectManager::GetSingleton()->PlayEffect( iEffect, vecPosition, *TREINTERPRETCAST( const TVector4*, &quatRotation ) );

	if ( pHandle )
		uiFlags |= FLAGS_PLAYING;
}

// $Barnyard: FUNCTION 00650210
void AParticleEffectLocator::Update( TFLOAT a_fDeltaTime )
{
	TVector4 vecCameraPos = ACameraManager::GetSingleton()->GetCurrentCamera()->GetMatrix().GetTranslation();
	TBOOL    bInRange     = TVector4::DistanceSq( vecCameraPos, vecPosition ) < fEmitDistance;

	if ( uiFlags & FLAGS_CONSTANT )
	{
		if ( bInRange && ( uiFlags & FLAGS_ENABLED ) )
		{
			if ( !( uiFlags & FLAGS_PLAYING ) )
				Play();
		}
		else if ( uiFlags & FLAGS_PLAYING )
		{
			pHandle->GetInstance()->Stop();
			uiFlags &= ~FLAGS_PLAYING;
			pHandle = TNULL;
		}
	}
	else if ( bInRange && ( uiFlags & FLAGS_ENABLED ) )
	{
		fTimer -= a_fDeltaTime;

		if ( fTimer < 0.0f )
		{
			ResetTimer();
			AParticleEffectManager::GetSingleton()->PlayEffect( iEffect, vecPosition, *TREINTERPRETCAST( const TVector4*, &quatRotation ) );
		}
	}
}

// $Barnyard: FUNCTION 006502f0
TBOOL AParticleEffectLocator::Setup( const PBProperties* a_pProperties )
{
	TPString8 strEffectName;
	a_pProperties->GetOptionalPropertyValue( strEffectName, TPS8( EffectName ) );
	iEffect = AParticleEffectManager::FindEffect( strEffectName.GetString() );

	TPString8 strLocatorName;
	a_pProperties->GetOptionalPropertyValue( strLocatorName, TPS8( LocatorName ) );
	ATerrainInterface::GetSingleton()->GetLocatorPosAndRot( strLocatorName.GetString(), vecPosition, quatRotation );

	TPString8 strType;
	a_pProperties->GetOptionalPropertyValue( strType, TPS8( Type ) );

	if ( strType == TPS8( Constant ) )
	{
		uiFlags |= FLAGS_CONSTANT;
	}
	else if ( strType == TPS8( Uniform ) )
	{
		uiFlags |= FLAGS_UNIFORM;
		a_pProperties->GetOptionalFloatOrIntValue( fEmitTime, TPS8( EmitTime ) );
		ResetTimer();
	}
	else if ( strType == TPS8( Random ) )
	{
		uiFlags |= FLAGS_RANDOM;
		a_pProperties->GetOptionalFloatOrIntValue( fEmitTime, TPS8( EmitTime ) );
		a_pProperties->GetOptionalFloatOrIntValue( fEmitTimePlusMinus, TPS8( EmitTimePlusMinus ) );
		ResetTimer();
	}

	a_pProperties->GetOptionalFloatOrIntValue( fEmitDistance, TPS8( EmitDistance ) );
	fEmitDistance = fEmitDistance * fEmitDistance;

	return TTRUE;
}

// $Barnyard: FUNCTION 00650490
AParticleEffectLocator* AParticleEffectLocatorManager::CreateLocator( const PBProperties* a_pProperties )
{
	TPString8 strLocatorName;
	a_pProperties->GetOptionalPropertyValue( strLocatorName, TPS8( LocatorName ) );

	if ( !ATerrainInterface::GetSingleton()->FindLocator( strLocatorName.GetString() ) )
		return TNULL;

	AParticleEffectLocator* pLocator = new AParticleEffectLocator();
	pLocator->Setup( a_pProperties );

	return pLocator;
}

// $Barnyard: FUNCTION 00650a00
// $Barnyard: FUNCTION 00650a90
AParticleEffectLocatorManager::AParticleEffectLocatorManager()
    : m_iNumLocators( 0 )
{
}

// $Barnyard: FUNCTION 006505c0
// $Barnyard: FUNCTION 00650860
AParticleEffectLocatorManager::~AParticleEffectLocatorManager()
{
}

// $Barnyard: FUNCTION 00650640
void AParticleEffectLocatorManager::LoadLocatorList( const TPString8& a_rcName )
{
	if ( !ATerrainInterface::IsSingletonCreated() )
	{
		// Load it once the terrain exists
		m_strPendingList = a_rcName;
		return;
	}

	TString8 strFilePath;
	strFilePath.Format( "%s\\%s.trb", s_szLocatorListsPath, a_rcName.GetString() );

	TTRB oTRB;

	if ( oTRB.Load( strFilePath ) == TTRB::ERROR_OK )
	{
		const PBProperties* pMain     = TSTATICCAST( const PBProperties, oTRB.GetSymbolAddress( "Main" ) );
		const PBProperties* pLocators = pMain->FindProperties( "ParticleEffectLocators" );

		TINT iNumProperties = pLocators->GetPropertyCount();

		for ( TINT i = 0; i < iNumProperties; i++ )
		{
			const PBPropertyValue* pValue = pLocators->GetPropertyValue( i );

			if ( pValue->GetType() == PBPropertyValue::Type::Properties )
			{
				AParticleEffectLocator* pLocator = CreateLocator( pValue->GetProperties() );

				if ( pLocator )
					m_apLocators[ m_iNumLocators++ ] = pLocator;
			}
		}

		oTRB.Close();
	}
}

// $Barnyard: FUNCTION 006507c0
void AParticleEffectLocatorManager::Clear()
{
	m_strPendingList = TPString8();

	while ( m_iNumLocators != 0 )
	{
		AParticleEffectLocator* pLocator = m_apLocators[ m_iNumLocators - 1 ];

		if ( pLocator )
		{
			pLocator->Stop();
			delete pLocator;
		}

		m_apLocators[ --m_iNumLocators ] = TNULL;
	}
}

// $Barnyard: FUNCTION 00650880
void AParticleEffectLocatorManager::OnDestroy()
{
	Clear();
	TTask::OnDestroy();
}

// $Barnyard: FUNCTION 00650890
TBOOL AParticleEffectLocatorManager::OnUpdate( TFLOAT a_fDeltaTime )
{
	if ( !m_strPendingList.IsEmpty() )
	{
		if ( ATerrainInterface::IsSingletonCreated() )
		{
			LoadLocatorList( m_strPendingList );
			m_strPendingList = TPString8();
		}
	}
	else
	{
		for ( TINT i = m_iNumLocators - 1; i >= 0; i-- )
			m_apLocators[ i ]->Update( a_fDeltaTime );
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 00650950
TBOOL AParticleEffectLocatorManager::Reset()
{
	Clear();
	return TTask::Reset();
}

// $Barnyard: FUNCTION 00650960
void AParticleEffectLocatorManager::LoadLocatorLists( const PBPropertyValue* a_pList )
{
	const PBPropertyValueArray* pArray = a_pList->GetArray();

	for ( TUINT i = 0; i < pArray->GetSize(); i++ )
		LoadLocatorList( pArray->GetValue( i )->GetTPString8() );
}
