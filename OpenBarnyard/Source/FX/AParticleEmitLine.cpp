#include "pch.h"
#include "AParticleEmitLine.h"
#include "AParticleSystem.h"

#include <Math/TMathInline.h>
#include <Math/TMatrix44.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

static constexpr TFLOAT POINT_SCALE     = 0.2f;  // size of the points
static constexpr TFLOAT POINT_SPACING   = 0.2f;  // distance between the points
static constexpr TFLOAT POINTS_PER_UNIT = 5.0f;  // 1.0f / POINT_SPACING
static constexpr TFLOAT LINE_HEIGHT     = 0.03f; // the points are lifted a bit above a_rcFrom

// Fixed colour init data of an emitter (type 18, see AParticleInit::Resolve)
struct AParticleEmitLineColour
{
	AParticleFunctionData oHeader;  // 0x0
	TUINT32               uiColour; // 0xc
};

// NOTE: inlined in the original
void AParticleEmitLine::SetEffectColour( AParticleEffect* a_pEffect, TUINT32 a_uiColour )
{
	AParticleInit* pInit                                                 = a_pEffect->pEmitters[ 0 ].pSettings->apInits[ AParticleEmitterSettings::INIT_COLOUR ];
	TREINTERPRETCAST( AParticleEmitLineColour*, pInit->pData )->uiColour = a_uiColour;
}

// $Barnyard: FUNCTION 00651610
void AParticleEmitLine::Create( const TCHAR* a_szEffectName )
{
	TINT iEffect = AParticleEffectManager::FindEffect( a_szEffectName );

	for ( TINT i = 0; i < MAX_POINTS; i++ )
	{
		m_aPoints[ i ].pEffect = AParticleEffectManager::CloneEffect( iEffect );
		SetEffectColour( m_aPoints[ i ].pEffect, 0 );

		// NOTE: retail doesn't check for TNULL (no free effect instance or handle)
		m_aPoints[ i ].pHandle = AParticleEffectManager::GetSingleton()->SpawnEffect( m_aPoints[ i ].pEffect, TVector4::VEC_ZERO, TVector4::VEC_NEGY );
		m_aPoints[ i ].pHandle->GetInstance()->GetTransform().Scale( POINT_SCALE, POINT_SCALE, POINT_SCALE );
	}
}

// $Barnyard: FUNCTION 00651690
void AParticleEmitLine::Destroy()
{
	for ( TINT i = 0; i < MAX_POINTS; i++ )
	{
		m_aPoints[ i ].pHandle->GetInstance()->Kill();

		AParticleEffect* pEffect = m_aPoints[ i ].pEffect;

		if ( pEffect )
		{
			pEffect->DeleteClone();
			TFree( pEffect );
		}
	}
}

// $Barnyard: FUNCTION 006513e0
void AParticleEmitLine::Reset()
{
	m_iNumPoints = 0;

	for ( TINT i = 0; i < MAX_POINTS; i++ )
		SetEffectColour( m_aPoints[ i ].pEffect, 0 );
}

// $Barnyard: FUNCTION 006516d0
void AParticleEmitLine::AddLine( const TVector4& a_rcFrom, const TVector4& a_rcTo, TFLOAT a_fOffset )
{
	TVector4 vecDir;
	vecDir.x = a_rcTo.x - a_rcFrom.x;
	vecDir.y = a_rcTo.y - a_rcFrom.y;
	vecDir.z = a_rcTo.z - a_rcFrom.z;
	vecDir.w = a_rcTo.w;

	TFLOAT   fLength = vecDir.NormaliseMag();
	TVector4 vecForward( vecDir );

	TVector4 vecRight( vecDir );
	vecRight.AsVector3().RotateY( TMath::HALF_PI );

	TVector4 vecUp( TVector4::VEC_NEGY );

	// Wraps a_fOffset into 0.0f..1.0f (006b68c0)
	if ( a_fOffset > 1.0f )
		a_fOffset = 0.0f;
	else if ( a_fOffset < 0.0f )
		a_fOffset = 1.0f;

	TINT iNumPoints = TINT( fLength * POINTS_PER_UNIT + 1.5f - a_fOffset );
	iNumPoints      = TMath::Min( iNumPoints, MAX_POINTS );

	TVector4 vecPos( a_rcFrom );
	vecPos.y += LINE_HEIGHT;

	vecDir.Multiply( POINT_SPACING );

	TINT iEnd = TMath::Min( iNumPoints + m_iNumPoints, MAX_POINTS );

	TVector4 vecOffset;
	vecOffset.Multiply( vecDir, a_fOffset - 0.5f );
	vecPos.x += vecOffset.x;
	vecPos.y += vecOffset.y;
	vecPos.z += vecOffset.z;

	for ( TINT i = m_iNumPoints; i < iEnd; i++ )
	{
		TMatrix44& rTransform = m_aPoints[ i ].pHandle->GetInstance()->GetTransform();
		rTransform.Identity();
		rTransform.AsBasisVector3( BASISVECTOR_RIGHT )   = vecRight.AsVector3();
		rTransform.AsBasisVector3( BASISVECTOR_UP )      = vecForward.AsVector3();
		rTransform.AsBasisVector3( BASISVECTOR_FORWARD ) = vecUp.AsVector3();
		rTransform.GetTranslation()                      = vecPos;
		rTransform.Scale( POINT_SCALE, POINT_SCALE, POINT_SCALE );

		vecPos.x += vecDir.x;
		vecPos.y += vecDir.y;
		vecPos.z += vecDir.z;
	}

	m_iNumPoints = iEnd;
}

// $Barnyard: FUNCTION 00651460
void AParticleEmitLine::SetColours( TINT a_iFrom, TINT a_iTo, TUINT32 a_uiFromColour, TUINT32 a_uiToColour )
{
	TFLOAT fProgress = 0.0f;
	TFLOAT fStep     = 1.0f / TFLOAT( a_iTo - a_iFrom );

	TFLOAT fFromB = TFLOAT( TUINT8( a_uiFromColour ) );
	TFLOAT fFromG = TFLOAT( TUINT8( a_uiFromColour >> 8 ) );
	TFLOAT fFromR = TFLOAT( TUINT8( a_uiFromColour >> 16 ) );
	TFLOAT fFromA = TFLOAT( TUINT8( a_uiFromColour >> 24 ) );
	TFLOAT fToB   = TFLOAT( TUINT8( a_uiToColour ) );
	TFLOAT fToG   = TFLOAT( TUINT8( a_uiToColour >> 8 ) );
	TFLOAT fToR   = TFLOAT( TUINT8( a_uiToColour >> 16 ) );
	TFLOAT fToA   = TFLOAT( TUINT8( a_uiToColour >> 24 ) );

	TUINT8 uiB = TUINT8( a_uiToColour );
	TUINT8 uiG = TUINT8( a_uiToColour >> 8 );
	TUINT8 uiR = TUINT8( a_uiToColour >> 16 );
	TUINT8 uiA = TUINT8( a_uiToColour >> 24 );

	// The last point gets a_uiToColour, the colour goes towards a_uiFromColour
	for ( TINT i = a_iTo - 1; i >= a_iFrom; i-- )
	{
		fProgress += fStep;
		SetEffectColour( m_aPoints[ i ].pEffect, uiB | ( uiG << 8 ) | ( uiR << 16 ) | ( TUINT32( uiA ) << 24 ) );

		uiR = TUINT8( TINT( fToR * ( 1.0f - fProgress ) + fFromR * fProgress ) );
		uiB = TUINT8( TINT( fToB * ( 1.0f - fProgress ) + fFromB * fProgress ) );
		uiG = TUINT8( TINT( fToG * ( 1.0f - fProgress ) + fFromG * fProgress ) );
		uiA = TUINT8( TINT( fToA * ( 1.0f - fProgress ) + fFromA * fProgress ) );
	}
}
