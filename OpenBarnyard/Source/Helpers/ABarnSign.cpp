#include "pch.h"
#include "ABarnSign.h"
#include "Animation/AAnimatableObjectManager.h"
#include "Animation/AModelRepos.h"
#include "Terrain/ATerrainInterface.h"
#include "ASimAnimModelHelper.h"
#include "AWeatherManager.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $Barnyard: FUNCTION 0054c720
ABarnSign::ABarnSign()
    : m_pModelInstance( TNULL )
    , m_fPhase( 0.0f )
    , m_fSwingAngle( 0.0f )
    , m_fSwingSpeed( 1.5f )
    , m_fSwingAmount( 0.1f )
{
}

ABarnSign::~ABarnSign()
{
}

// NOTE: Inlined into AWindmillHelper::OnDestroy (0054d6d0); retail only destroys the model there, AWindmillHelper::PopBarnSign resets the rest
void ABarnSign::Destroy()
{
	g_pModelRepos->DestroyModelInstance( m_pModelInstance );

	m_pModelInstance = TNULL;
	m_fPhase         = 0.0f;
	m_fSwingAngle    = 0.0f;
	m_fSwingSpeed    = 1.5f;
	m_fSwingAmount   = 0.1f;
}

// $Barnyard: FUNCTION 0054d6f0
void ABarnSign::SpawnAtLocator( const TCHAR* a_pszLocatorName )
{
	AAnimatableObjectType* pModelType = AAnimatableObjectManager::GetSingleton()->FindType( TPString8( "barnsign" ) );
	TASSERT( TNULL != pModelType );

	m_pModelInstance = pModelType->Instantiate();

	// Get locator transform
	TMatrix44 matLocator;
	ATerrainInterface::GetSingleton()->GetLocatorTransform( a_pszLocatorName, matLocator );

	// Update model instance
	m_pModelInstance->SetTransform( matLocator );
	m_pModelInstance->SetSkeletonUpdating( TTRUE );
}

// $Barnyard: FUNCTION 0054d160
void ABarnSign::Update( TFLOAT a_fDeltaTime, TBOOL a_bDistanceCulling )
{
	if ( !m_pModelInstance->IsVisible() )
		return;

	if ( a_bDistanceCulling && !ASimAnimModelHelper::IsInAnimationRange( m_pModelInstance ) )
		return;

	if ( AWeatherManager::GetSingleton()->IsHeavyRain() )
	{
		// NOTE: TMath::IncrMaxf, adds and clamps to the max
		if ( m_fSwingSpeed < 4.5f )
			m_fSwingSpeed = TMath::Min( m_fSwingSpeed + a_fDeltaTime * 4.5f, 4.5f );

		if ( m_fSwingAmount < 0.3f )
			m_fSwingAmount = TMath::Min( m_fSwingAmount + a_fDeltaTime * 0.3f, 0.3f );
	}
	else
	{
		// NOTE: TMath::DecrMinf, subtracts and clamps to the min
		if ( m_fSwingSpeed > 1.5f )
			m_fSwingSpeed = TMath::Max( m_fSwingSpeed - a_fDeltaTime * 1.5f, 1.5f );

		if ( m_fSwingAmount > 0.1f )
			m_fSwingAmount = TMath::Max( m_fSwingAmount - a_fDeltaTime * 0.1f, 0.1f );
	}

	m_fPhase += a_fDeltaTime * m_fSwingSpeed;

	if ( m_fPhase > TMath::TWO_PI )
		m_fPhase -= TMath::TWO_PI;

	TFLOAT fSwingAngle = TMath::Sin( m_fPhase ) * m_fSwingAmount;
	TFLOAT fDelta      = m_fSwingAngle - fSwingAngle;
	m_fSwingAngle      = fSwingAngle;

	TMatrix44 matTransform;
	m_pModelInstance->GetLocalTransform( matTransform );
	matTransform.RotateY( fDelta );
	m_pModelInstance->SetTransform( matTransform );
}
