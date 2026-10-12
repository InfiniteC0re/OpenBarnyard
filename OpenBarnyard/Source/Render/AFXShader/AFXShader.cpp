#include "pch.h"
#include "AFXShader.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $Barnyard: FUNCTION 005f97d0
TDEFINE_CLASS_NORUNTIME( AFXShader );

// $Barnyard: FUNCTION 005f99b0
AFXShader::AFXShader()
    : m_SomeColour( 0 ), m_Unk2( 128 ), m_Unk3( 1.0f ), m_Unk4( 0 ), m_vUnk5( 1.0f, 1.0f, 1.0f, 1.0f )
{
}

// $Barnyard: FUNCTION 005f99f0
// $Barnyard: FUNCTION 005f9a70
AFXShader::~AFXShader()
{
}

// DOES_NOTHING
void AFXShader::EnableRenderEnvMap( TBOOL a_bEnable )
{
}

// RETURNS_TRUE
TBOOL AFXShader::IsHighEndSkinning()
{
	return TTRUE;
}

// DOES_NOTHING
void AFXShader::EnableHighEndSkinning( TBOOL a_bEnable )
{
}

// RETURNS_TRUE
TBOOL AFXShader::IsCapableHighEndSkinning()
{
	return TTRUE;
}

// RETURNS_FALSE
TBOOL AFXShader::IsLightScattering()
{
	return TFALSE;
}

// DOES_NOTHING
void AFXShader::SetLightScattering( TBOOL a_bEnable )
{
}

// RETURNS_FALSE
TBOOL AFXShader::IsAlphaBlendMaterial()
{
	return TFALSE;
}

// DOES_NOTHING
void AFXShader::SetAlphaBlendMaterial( TBOOL a_bIsAlphaBlendMaterial )
{
}

// DOES_NOTHING
void AFXShader::EnableClouds( TBOOL a_bEnabled )
{
}

// RETURNS_FALSE
TBOOL AFXShader::IsEnabledClouds()
{
	return TFALSE;
}

// RETURNS_FALSE
TBOOL AFXShader::IsCapablePointLights()
{
	return TFALSE;
}

// DOES_NOTHING
void AFXShader::SetPointLights( TBOOL a_bEnabled )
{
}

// RETURNS_FALSE
TBOOL AFXShader::IsPointLightsEnabled()
{
	return TFALSE;
}
