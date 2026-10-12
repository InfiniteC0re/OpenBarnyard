#include "pch.h"
#include "ASplashManager.h"
#include "ARandom.h"
#include "Animation/AModelRepos.h"

#include <Math/TMatrix44.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $Barnyard: FUNCTION 00653190
ASplashManager::Splash::Splash()
    : m_fScale( -1.0f ), m_fTime( -1.0f )
{
}

// $Barnyard: FUNCTION 006531b0
ASplashManager::Splash::~Splash()
{
	Destroy();
}

// $Barnyard: FUNCTION 00653150
void ASplashManager::Splash::Destroy()
{
	if ( m_pModelInstance )
	{
		AModelInstance* pModelInstance = m_pModelInstance;
		m_pModelInstance               = TNULL;

		g_pModelRepos->DestroyModelInstance( pModelInstance );
	}
}

// $Barnyard: FUNCTION 00653240
void ASplashManager::Splash::Spawn( const TVector4& a_rcPosition, const TVector4& a_rcDirection, TFLOAT a_fScale )
{
	m_fScale = a_fScale;
	m_fTime  = 0.6f;

	TMatrix44 matTransform        = TMatrix44::IDENTITY;
	matTransform.GetTranslation() = a_rcPosition;

	TFLOAT fAngle = ( ARandom::GetSingleton()->m_oRandom.RandRaw() >> 1 ) * TMath::TWO_PI * 4.6566129e-10f;

	TVector4 vUp = TVector4::VEC_POSZ;
	vUp.AsVector3().RotateY( fAngle );
	matTransform.LookAtDirection( a_rcDirection, vUp );

	m_pModelInstance->SetTransform( matTransform );
	m_pModelInstance->SetSkeletonUpdating( TTRUE );
}

// $Barnyard: FUNCTION 00653340
void ASplashManager::Splash::Update( TFLOAT a_fDeltaTime )
{
	if ( m_fTime > 0.0f )
	{
		m_fTime -= a_fDeltaTime;

		if ( m_fTime > 0.0f )
		{
			TFLOAT fOpacity = m_fTime * 1.6666666f;
			m_pModelInstance->SetOpacity( fOpacity );

			TFLOAT fProgress = 1.0f - fOpacity;

			TVector4 vScale;
			vScale.x = fProgress;
			vScale.y = fProgress;

			if ( m_eType != SPLASHTYPE_MILK )
			{
				TFLOAT fValue = fProgress * 1.4f - 0.69999999f;
				vScale.z      = TMath::Max( 0.0001f, fValue * fValue * -1.0f + 0.49000001f );
			}
			else
			{
				TFLOAT fValue = fProgress + fProgress - 1.0f;
				vScale.z      = TMath::Max( 0.0001f, fValue * fValue * -1.0f + 1.0f );
			}

			vScale.w = 1.0f;
			vScale.Multiply( m_fScale );
			m_pModelInstance->SetScale( vScale );
		}
		else
		{
			m_pModelInstance->SetSkeletonUpdating( TFALSE );
		}
	}
}

// $Barnyard: FUNCTION 00653480
void ASplashManager::Splash::Create( SPLASHTYPE a_eType )
{
	m_eType = a_eType;

	if ( a_eType == SPLASHTYPE_MILK )
	{
		m_pModelInstance = g_pModelRepos->InstantiateNewModel( TPS8D( "data/models/milksplash.trb" ), TNULL );
	}
	else if ( a_eType == SPLASHTYPE_WATER )
	{
		m_pModelInstance = g_pModelRepos->InstantiateNewModel( TPS8D( "data/models/watersplash.trb" ), TNULL );
		m_pModelInstance->SetUnlit();
	}
	else
	{
		m_pModelInstance = g_pModelRepos->InstantiateNewModel( TPS8D( "data/models/milksplash.trb" ), TNULL );
	}

	m_pModelInstance->SetSkeletonUpdating( TFALSE );
	m_pModelInstance->SetTransform( TVector4::VEC_ZERO, TVector4::VEC_NEGZ );
}

// $Barnyard: FUNCTION 00653890
ASplashManager::ASplashManager()
{
	// NOTE: retail passes a global TTRB pointer (0x0077dd64) that is never set
	g_pModelRepos->LoadModel( TPS8D( "Data/Models/milksplash.trb" ), TNULL );
	g_pModelRepos->LoadModel( TPS8D( "Data/Models/watersplash.trb" ), TNULL );
	g_pModelRepos->LoadModel( TPS8D( "Data/Models/tomatosplash.trb" ), TNULL );

	for ( TINT i = 0; i < SPLASHTYPE_NUMOF; i++ )
	{
		for ( TINT j = 0; j < MAX_NUM_SPLASHES; j++ )
			m_aSplashes[ i ][ j ].Create( SPLASHTYPE( i ) );
	}
}

// $Barnyard: FUNCTION 00653730
// $Barnyard: FUNCTION 006539f0
ASplashManager::~ASplashManager()
{
	for ( TINT i = 0; i < SPLASHTYPE_NUMOF; i++ )
	{
		for ( TINT j = 0; j < MAX_NUM_SPLASHES; j++ )
			m_aSplashes[ i ][ j ].Destroy();
	}

	g_pModelRepos->UnloadModel( TPS8D( "Data/Models/milksplash.trb" ), TTRUE );
	g_pModelRepos->UnloadModel( TPS8D( "Data/Models/watersplash.trb" ), TTRUE );
	g_pModelRepos->UnloadModel( TPS8D( "Data/Models/tomatosplash.trb" ), TTRUE );
}

// $Barnyard: FUNCTION 00653680
void ASplashManager::Update( TFLOAT a_fDeltaTime )
{
	for ( TINT i = 0; i < SPLASHTYPE_NUMOF; i++ )
	{
		for ( TINT j = 0; j < MAX_NUM_SPLASHES; j++ )
			m_aSplashes[ i ][ j ].Update( a_fDeltaTime );
	}
}

// $Barnyard: FUNCTION 006536b0
ASplashManager::Splash* ASplashManager::GetFreeSplash( SPLASHTYPE a_eType )
{
	for ( TINT i = 0; i < MAX_NUM_SPLASHES; i++ )
	{
		if ( m_aSplashes[ a_eType ][ i ].m_fTime <= 0.0f )
			return &m_aSplashes[ a_eType ][ i ];
	}

	return TNULL;
}

// $Barnyard: FUNCTION 00653700
void ASplashManager::Spawn( const TVector4& a_rcPosition, const TVector4& a_rcDirection, TFLOAT a_fScale, SPLASHTYPE a_eType )
{
	Splash* pSplash = GetFreeSplash( a_eType );

	if ( pSplash )
		pSplash->Spawn( a_rcPosition, a_rcDirection, a_fScale );
}
