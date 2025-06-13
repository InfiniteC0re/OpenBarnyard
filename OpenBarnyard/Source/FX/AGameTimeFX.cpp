#include "pch.h"
#include "AGameTimeFX.h"
#include "Terrain/ATerrainInterface.h"

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

// $Barnyard: FUNCTION 00544e70
void AGameTimeFX::Apply()
{
	TIMPLEMENT();

	AWorldShaderHAL*  pWorldShader     = TSTATICCAST( AWorldShaderHAL, AWorldShader::GetSingleton() );
	TRenderInterface* pRenderInterface = TRenderInterface::GetSingleton();
	TViewport*        pViewport        = ARenderer::GetSingleton()->GetMainViewport();

	// Update viewport
	TVector4 vecViewportColor;
	vecViewportColor.Lerp4( m_vecShadowColor, m_vecAmbientColor, m_fShadowAmbientProgress );
	pViewport->SetBackgroundColor( TUINT8( vecViewportColor.x * 33.0f ), TUINT8( vecViewportColor.y * 89.0f ), TUINT8( vecViewportColor.z * 63.0f ), 0U );

	// TODO: Some other values and ASky...

	// Update fog
	TRenderContext* pRenderContext = pViewport->GetRenderContext();
	pRenderContext->SetFogColor( m_vecFogColor );
	pRenderContext->SetFogDistance( m_fFogStart, m_fFogEnd );

	// Update light color
	TVector4 vecLightColor = m_vecAmbientColor - m_vecShadowColor;
	vecLightColor.w        = m_vecAmbientColor.w;
	vecLightColor.Multiply( m_fLightIntensity );

	// Update ambient light color
	TVector4 vecAmbientColor;
	vecAmbientColor.Multiply( m_vecShadowColor, m_fDarkeningFactor );

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

	pRenderContext->SetAmbientColour( vecAmbientColor );
	pRenderInterface->SetLightColourMatrix( matLightColor );
	pRenderInterface->SetLightDirectionMatrix( matLightDir );

	// Something with AWeatherManager...

	if ( ATerrainInterface::IsSingletonCreated() )
		ATerrainInterface::GetSingleton()->m_fUnused3 = field24_0x12c;
}

// $Barnyard: FUNCTION 00544240
void AGameTimeFX::Interpolate( const AGameTimeFX& a_rcFrom, const AGameTimeFX& a_rcTo, TFLOAT a_fProgress )
{
	TASSERT( &a_rcFrom != &a_rcTo );

	m_vecLightDir.Lerp4( a_rcFrom.m_vecLightDir, a_rcTo.m_vecLightDir, a_fProgress );
	m_vecLightDir.Normalise();

	field1_0x10.Lerp4( a_rcFrom.field1_0x10, a_rcTo.field1_0x10, a_fProgress );
	field1_0x10.Normalise();

	m_vecShadowColor.Lerp4( a_rcFrom.m_vecShadowColor, a_rcTo.m_vecShadowColor, a_fProgress );
	m_vecAmbientColor.Lerp4( a_rcFrom.m_vecAmbientColor, a_rcTo.m_vecAmbientColor, a_fProgress );
	m_vecFogColor.Lerp4( a_rcFrom.m_vecFogColor, a_rcTo.m_vecFogColor, a_fProgress );
	field5_0x50.Lerp4( a_rcFrom.field5_0x50, a_rcTo.field5_0x50, a_fProgress );
	field6_0x60.Lerp4( a_rcFrom.field6_0x60, a_rcTo.field6_0x60, a_fProgress );
	field7_0x70.Lerp4( a_rcFrom.field7_0x70, a_rcTo.field7_0x70, a_fProgress );
	field8_0x80.Lerp4( a_rcFrom.field8_0x80, a_rcTo.field8_0x80, a_fProgress );

	field19_0x118   = TMath::LERP( a_rcFrom.field19_0x118, a_rcTo.field19_0x118, a_fProgress );
	field17_0x110   = TMath::LERP( a_rcFrom.field17_0x110, a_rcTo.field17_0x110, a_fProgress );
	field27_0x138   = TMath::LERP( a_rcFrom.field27_0x138, a_rcTo.field27_0x138, a_fProgress );
	field28_0x13c   = TMath::LERP( a_rcFrom.field28_0x13c, a_rcTo.field28_0x13c, a_fProgress );
	m_fFogStart     = TMath::LERP( a_rcFrom.m_fFogStart, a_rcTo.m_fFogStart, a_fProgress );
	m_fFogEnd       = TMath::LERP( a_rcFrom.m_fFogEnd, a_rcTo.m_fFogEnd, a_fProgress );
	field26_0x134   = TMath::LERP( a_rcFrom.field26_0x134, a_rcTo.field26_0x134, a_fProgress );
	m_fStarsOpacity = TMath::LERP( a_rcFrom.m_fStarsOpacity, a_rcTo.m_fStarsOpacity, a_fProgress );
	field30_0x144   = TMath::LERP( a_rcFrom.field30_0x144, a_rcTo.field30_0x144, a_fProgress );

	field11_0xb0.Lerp4( a_rcFrom.field11_0xb0, a_rcTo.field11_0xb0, a_fProgress );
	field9_0x90.Lerp4( a_rcFrom.field9_0x90, a_rcTo.field9_0x90, a_fProgress );
	field12_0xc0.Lerp4( a_rcFrom.field12_0xc0, a_rcTo.field12_0xc0, a_fProgress );
	field10_0xa0.Lerp4( a_rcFrom.field10_0xa0, a_rcTo.field10_0xa0, a_fProgress );
	field15_0xf0.Lerp4( a_rcFrom.field15_0xf0, a_rcTo.field15_0xf0, a_fProgress );
	field13_0xd0.Lerp4( a_rcFrom.field13_0xd0, a_rcTo.field13_0xd0, a_fProgress );
	field16_0x100.Lerp4( a_rcFrom.field16_0x100, a_rcTo.field16_0x100, a_fProgress );
	field14_0xe0.Lerp4( a_rcFrom.field14_0xe0, a_rcTo.field14_0xe0, a_fProgress );

	field24_0x12c      = TMath::LERP( a_rcFrom.field24_0x12c, a_rcTo.field24_0x12c, a_fProgress );
	field25_0x130      = TMath::LERP( a_rcFrom.field25_0x130, a_rcTo.field25_0x130, a_fProgress );
	m_fDarkeningFactor = TMath::LERP( a_rcFrom.m_fDarkeningFactor, a_rcTo.m_fDarkeningFactor, a_fProgress );
	m_fLightIntensity  = TMath::LERP( a_rcFrom.m_fLightIntensity, a_rcTo.m_fLightIntensity, a_fProgress );
	field36_0x15c      = TMath::LERP( a_rcFrom.field36_0x15c, a_rcTo.field36_0x15c, a_fProgress );
	field37_0x160      = TMath::LERP( a_rcFrom.field37_0x160, a_rcTo.field37_0x160, a_fProgress );
	field38_0x164      = TMath::LERP( a_rcFrom.field38_0x164, a_rcTo.field38_0x164, a_fProgress );
	field39_0x168      = TMath::LERP( a_rcFrom.field39_0x168, a_rcTo.field39_0x168, a_fProgress );
	field40_0x16c      = TMath::LERP( a_rcFrom.field40_0x16c, a_rcTo.field40_0x16c, a_fProgress );
	field41_0x170      = TMath::LERP( a_rcFrom.field41_0x170, a_rcTo.field41_0x170, a_fProgress );
	field42_0x174      = TMath::LERP( a_rcFrom.field42_0x174, a_rcTo.field42_0x174, a_fProgress );
}
