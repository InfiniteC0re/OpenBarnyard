#include "pch.h"
#include "AGameTimeFX.h"
#include "Helpers/AInstanceLightingHelper.h"
#include "Terrain/ATerrainInterface.h"
#include "Render/ASkyDome.h"
#include "Render/ARainRenderer.h"
#include "Render/ASunFlareEffect.h"

#ifdef TOSHI_SKU_WINDOWS

#  include "Platform/DX8/AWorldShader/AWorldShader_DX8.h"
#  include "Render/ARenderer.h"

#endif // TOSHI_SKU_WINDOWS

#include <Render/TRenderInterface.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $Barnyard: FUNCTION 00544660
AGameTimeFX::AGameTimeFX()
    : m_fShadowAmbientProgress( 1.0f )
    , m_fDarkeningFactor( 1.0f )
    , m_fLightIntensity( 1.0f )
    , field19_0x118( 1.0f )
    , field32_0x14c( 0.0f )
    , field8_0x80( 1.0f, 1.0f, 1.0f, 1.0f )
    , field7_0x70( 1.0f, 1.0f, 1.0f, 1.0f )
    , m_fFogStart( 50.0f )
    , m_fFogEnd( 150.0f )
{
}

AGameTimeFX::~AGameTimeFX()
{
}

// NOTE: Compiler generated in retail (used when copying a setting to force it, e.g. "BarnNight").
// It stores 0x124 and 0x14c twice, which doesn't change anything.
// $Barnyard: FUNCTION 00492d30
AGameTimeFX& AGameTimeFX::operator=( const AGameTimeFX& a_rcOther )
{
	m_vecLightDir            = a_rcOther.m_vecLightDir;
	m_vecSunFlareDir         = a_rcOther.m_vecSunFlareDir;
	m_vecShadowColor         = a_rcOther.m_vecShadowColor;
	m_vecAmbientColor        = a_rcOther.m_vecAmbientColor;
	m_vecFogColor            = a_rcOther.m_vecFogColor;
	m_vecSkyTint             = a_rcOther.m_vecSkyTint;
	m_vecSkySunDir           = a_rcOther.m_vecSkySunDir;
	field7_0x70              = a_rcOther.field7_0x70;
	field8_0x80              = a_rcOther.field8_0x80;
	m_oSkyColorMatrix        = a_rcOther.m_oSkyColorMatrix;
	m_oSkyLightMatrix        = a_rcOther.m_oSkyLightMatrix;
	m_fSkyBlend              = a_rcOther.m_fSkyBlend;
	m_szName                 = a_rcOther.m_szName;
	field19_0x118            = a_rcOther.field19_0x118;
	m_fFogStart              = a_rcOther.m_fFogStart;
	m_fFogEnd                = a_rcOther.m_fFogEnd;
	m_iSkyIndex              = a_rcOther.m_iSkyIndex;
	m_fShadowAmbientProgress = a_rcOther.m_fShadowAmbientProgress;
	field24_0x12c            = a_rcOther.field24_0x12c;
	field25_0x130            = a_rcOther.field25_0x130;
	field26_0x134            = a_rcOther.field26_0x134;
	m_fSkySunMin             = a_rcOther.m_fSkySunMin;
	m_fSkySunMax             = a_rcOther.m_fSkySunMax;
	m_fStarsOpacity          = a_rcOther.m_fStarsOpacity;
	m_fSkyRotationSpeed      = a_rcOther.m_fSkyRotationSpeed;
	field31_0x148            = a_rcOther.field31_0x148;
	field32_0x14c            = a_rcOther.field32_0x14c;
	m_fDarkeningFactor       = a_rcOther.m_fDarkeningFactor;
	m_fLightIntensity        = a_rcOther.m_fLightIntensity;
	m_fBlendStartFraction    = a_rcOther.m_fBlendStartFraction;
	field36_0x15c            = a_rcOther.field36_0x15c;
	field37_0x160            = a_rcOther.field37_0x160;
	m_fRainIntensity         = a_rcOther.m_fRainIntensity;
	m_fRainFallSpeed         = a_rcOther.m_fRainFallSpeed;
	m_fRainWindStrength      = a_rcOther.m_fRainWindStrength;
	m_fRainDropWidth         = a_rcOther.m_fRainDropWidth;
	m_fRainDropLength        = a_rcOther.m_fRainDropLength;

	return *this;
}

// $Barnyard: FUNCTION 00544e70
void AGameTimeFX::Apply()
{
	AWorldShaderHAL*  pWorldShader     = TSTATICCAST( AWorldShaderHAL, AWorldShader::GetSingleton() );
	TRenderInterface* pRenderInterface = TRenderInterface::GetSingleton();
	ARenderer*        pRenderer        = ARenderer::GetSingleton();

	pWorldShader->SetColours( m_vecShadowColor, m_vecAmbientColor );

	AInstanceLightingHelper::GetSingleton()->SetShadeCoeff( field24_0x12c );

	// Update viewport
	TVector4 vecViewportColor;
	vecViewportColor.Lerp4( m_vecShadowColor, m_vecAmbientColor, m_fShadowAmbientProgress );
	pRenderer->GetMainViewport()->SetBackgroundColor( TUINT8( vecViewportColor.x * 63.0f ), TUINT8( vecViewportColor.y * 89.0f ), TUINT8( vecViewportColor.z * 33.0f ), 0U );

	ASunFlareEffect* pSunFlare = ASunFlareEffect::GetSingleton();
	pSunFlare->SetSunIntensity( field26_0x134 );
	pSunFlare->SetSunDirection( m_vecLightDir );
	pSunFlare->SetMoonIntensity( m_fStarsOpacity );
	pSunFlare->SetMoonDirection( m_vecSunFlareDir );

	// Update the sky
	ASkyDome* pSkyDome = pRenderer->GetSkyDome();
	pSkyDome->SetStarsOpacity( m_fStarsOpacity );
	pSkyDome->SetRotationSpeed( m_fSkyRotationSpeed );
	pSkyDome->SetSunDirection( m_vecSkySunDir );
	pSkyDome->SetSunMin( m_fSkySunMin );
	pSkyDome->SetSunMax( m_fSkySunMax );
	pSkyDome->SetTint( m_vecSkyTint );

	// NOTE: the original passes field32_0x14c to a DOES_NOTHING method of ASkinLightingManager here

	// Update fog
	TRenderContext* pRenderContext = pRenderer->GetMainViewport()->GetRenderContext();
	pRenderContext->SetFogDistance( m_fFogStart, m_fFogEnd );
	pRenderContext->SetFogColor( m_vecFogColor );

	// Update light color
	TVector4 vecLightColor = m_vecAmbientColor - m_vecShadowColor;
	vecLightColor.w        = m_vecAmbientColor.w;

	// Update ambient light color
	TVector4 vecAmbientColor;
	vecAmbientColor.Multiply( m_vecShadowColor, m_fDarkeningFactor );
	vecLightColor.Multiply( m_fLightIntensity );

	// Clamp the value
	TMath::Clip( vecAmbientColor.x, 0.0f, 1.0f );
	TMath::Clip( vecAmbientColor.y, 0.0f, 1.0f );
	TMath::Clip( vecAmbientColor.z, 0.0f, 1.0f );
	TMath::Clip( vecLightColor.x, 0.0f, 1.0f );
	TMath::Clip( vecLightColor.y, 0.0f, 1.0f );
	TMath::Clip( vecLightColor.z, 0.0f, 1.0f );

	// Construct light matrices
	TMatrix44 matLightColor           = TMatrix44::IDENTITY;
	matLightColor.AsBasisVector4( 0 ) = vecLightColor;

	TMatrix44 matLightDir;
	matLightDir.AsBasisVector4( 0 ) = m_vecLightDir;
	matLightDir.AsBasisVector4( 1 ) = TVector4::VEC_ZERO;
	matLightDir.AsBasisVector4( 2 ) = TVector4::VEC_ZERO;
	matLightDir.AsBasisVector4( 3 ) = TVector4::VEC_ZERO;
	matLightDir.m_f14               = 0.0f;

	pRenderContext->SetAmbientColour( vecAmbientColor );
	pRenderInterface->SetLightColourMatrix( matLightColor );
	pRenderInterface->SetLightDirectionMatrix( matLightDir );

	// Update rain
	// NOTE: the original keeps the rain renderer at ARenderer+0x80, here it's a singleton
	if ( ARainRenderer* pRainRenderer = ARainRenderer::GetSingleton() )
	{
		pRainRenderer->SetRainIntensity( m_fRainIntensity );
		pRainRenderer->SetRainSpeed( m_fRainFallSpeed, m_fRainWindStrength );
		pRainRenderer->SetRainDropSize( m_fRainDropWidth, m_fRainDropLength );
	}

	if ( ATerrainInterface::IsSingletonCreated() )
		ATerrainInterface::GetSingleton()->m_fUnused3 = field24_0x12c;
}

// $Barnyard: FUNCTION 00544240
void AGameTimeFX::Interpolate( const AGameTimeFX& a_rcFrom, const AGameTimeFX& a_rcTo, TFLOAT a_fProgress )
{
	TASSERT( &a_rcFrom != &a_rcTo );

	m_vecLightDir.Lerp4( a_rcFrom.m_vecLightDir, a_rcTo.m_vecLightDir, a_fProgress );
	m_vecLightDir.Normalise();

	m_vecSunFlareDir.Lerp4( a_rcFrom.m_vecSunFlareDir, a_rcTo.m_vecSunFlareDir, a_fProgress );
	m_vecSunFlareDir.Normalise();

	m_vecShadowColor.Lerp4( a_rcFrom.m_vecShadowColor, a_rcTo.m_vecShadowColor, a_fProgress );
	m_vecAmbientColor.Lerp4( a_rcFrom.m_vecAmbientColor, a_rcTo.m_vecAmbientColor, a_fProgress );
	m_vecFogColor.Lerp4( a_rcFrom.m_vecFogColor, a_rcTo.m_vecFogColor, a_fProgress );
	m_vecSkyTint.Lerp4( a_rcFrom.m_vecSkyTint, a_rcTo.m_vecSkyTint, a_fProgress );
	m_vecSkySunDir.Lerp4( a_rcFrom.m_vecSkySunDir, a_rcTo.m_vecSkySunDir, a_fProgress );
	field7_0x70.Lerp4( a_rcFrom.field7_0x70, a_rcTo.field7_0x70, a_fProgress );
	field8_0x80.Lerp4( a_rcFrom.field8_0x80, a_rcTo.field8_0x80, a_fProgress );

	field19_0x118       = TMath::LERP( a_rcFrom.field19_0x118, a_rcTo.field19_0x118, a_fProgress );
	m_fSkyBlend         = TMath::LERP( a_rcFrom.m_fSkyBlend, a_rcTo.m_fSkyBlend, a_fProgress );
	m_fSkySunMin        = TMath::LERP( a_rcFrom.m_fSkySunMin, a_rcTo.m_fSkySunMin, a_fProgress );
	m_fSkySunMax        = TMath::LERP( a_rcFrom.m_fSkySunMax, a_rcTo.m_fSkySunMax, a_fProgress );
	m_fFogStart         = TMath::LERP( a_rcFrom.m_fFogStart, a_rcTo.m_fFogStart, a_fProgress );
	m_fFogEnd           = TMath::LERP( a_rcFrom.m_fFogEnd, a_rcTo.m_fFogEnd, a_fProgress );
	field26_0x134       = TMath::LERP( a_rcFrom.field26_0x134, a_rcTo.field26_0x134, a_fProgress );
	m_fStarsOpacity     = TMath::LERP( a_rcFrom.m_fStarsOpacity, a_rcTo.m_fStarsOpacity, a_fProgress );
	m_fSkyRotationSpeed = TMath::LERP( a_rcFrom.m_fSkyRotationSpeed, a_rcTo.m_fSkyRotationSpeed, a_fProgress );

	m_oSkyColorMatrix.AsBasisVector4( 2 ).Lerp4( a_rcFrom.m_oSkyColorMatrix.AsBasisVector4( 2 ), a_rcTo.m_oSkyColorMatrix.AsBasisVector4( 2 ), a_fProgress );
	m_oSkyColorMatrix.AsBasisVector4( 0 ).Lerp4( a_rcFrom.m_oSkyColorMatrix.AsBasisVector4( 0 ), a_rcTo.m_oSkyColorMatrix.AsBasisVector4( 0 ), a_fProgress );
	m_oSkyColorMatrix.AsBasisVector4( 3 ).Lerp4( a_rcFrom.m_oSkyColorMatrix.AsBasisVector4( 3 ), a_rcTo.m_oSkyColorMatrix.AsBasisVector4( 3 ), a_fProgress );
	m_oSkyColorMatrix.AsBasisVector4( 1 ).Lerp4( a_rcFrom.m_oSkyColorMatrix.AsBasisVector4( 1 ), a_rcTo.m_oSkyColorMatrix.AsBasisVector4( 1 ), a_fProgress );
	m_oSkyLightMatrix.AsBasisVector4( 2 ).Lerp4( a_rcFrom.m_oSkyLightMatrix.AsBasisVector4( 2 ), a_rcTo.m_oSkyLightMatrix.AsBasisVector4( 2 ), a_fProgress );
	m_oSkyLightMatrix.AsBasisVector4( 0 ).Lerp4( a_rcFrom.m_oSkyLightMatrix.AsBasisVector4( 0 ), a_rcTo.m_oSkyLightMatrix.AsBasisVector4( 0 ), a_fProgress );
	m_oSkyLightMatrix.AsBasisVector4( 3 ).Lerp4( a_rcFrom.m_oSkyLightMatrix.AsBasisVector4( 3 ), a_rcTo.m_oSkyLightMatrix.AsBasisVector4( 3 ), a_fProgress );
	m_oSkyLightMatrix.AsBasisVector4( 1 ).Lerp4( a_rcFrom.m_oSkyLightMatrix.AsBasisVector4( 1 ), a_rcTo.m_oSkyLightMatrix.AsBasisVector4( 1 ), a_fProgress );

	field24_0x12c       = TMath::LERP( a_rcFrom.field24_0x12c, a_rcTo.field24_0x12c, a_fProgress );
	field25_0x130       = TMath::LERP( a_rcFrom.field25_0x130, a_rcTo.field25_0x130, a_fProgress );
	m_fDarkeningFactor  = TMath::LERP( a_rcFrom.m_fDarkeningFactor, a_rcTo.m_fDarkeningFactor, a_fProgress );
	m_fLightIntensity   = TMath::LERP( a_rcFrom.m_fLightIntensity, a_rcTo.m_fLightIntensity, a_fProgress );
	field36_0x15c       = TMath::LERP( a_rcFrom.field36_0x15c, a_rcTo.field36_0x15c, a_fProgress );
	field37_0x160       = TMath::LERP( a_rcFrom.field37_0x160, a_rcTo.field37_0x160, a_fProgress );
	m_fRainIntensity    = TMath::LERP( a_rcFrom.m_fRainIntensity, a_rcTo.m_fRainIntensity, a_fProgress );
	m_fRainFallSpeed    = TMath::LERP( a_rcFrom.m_fRainFallSpeed, a_rcTo.m_fRainFallSpeed, a_fProgress );
	m_fRainWindStrength = TMath::LERP( a_rcFrom.m_fRainWindStrength, a_rcTo.m_fRainWindStrength, a_fProgress );
	m_fRainDropWidth    = TMath::LERP( a_rcFrom.m_fRainDropWidth, a_rcTo.m_fRainDropWidth, a_fProgress );
	m_fRainDropLength   = TMath::LERP( a_rcFrom.m_fRainDropLength, a_rcTo.m_fRainDropLength, a_fProgress );
}
