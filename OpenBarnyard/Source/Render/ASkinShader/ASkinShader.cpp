#include "pch.h"
#include "ASkinShader.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TDEFINE_CLASS_NORUNTIME( ASkinShader );

ASkinShader::ASkinShader() :
    m_SomeColour( 0 ),
    m_Unk2( 128 ),
    m_Unk3( 1.0f ),
    m_Unk4( 0 )
{
}

ASkinShader::~ASkinShader()
{
}

void ASkinShader::EnableRenderEnvMap( TBOOL a_bEnable )
{
}

TBOOL ASkinShader::IsHighEndSkinning()
{
	return TTRUE;
}

void ASkinShader::EnableHighEndSkinning( TBOOL a_bEnable )
{
}

TBOOL ASkinShader::IsCapableHighEndSkinning()
{
	return TTRUE;
}

TBOOL ASkinShader::IsLightScattering()
{
	return TFALSE;
}

void ASkinShader::SetLightScattering( TBOOL a_bEnable )
{
}

TBOOL ASkinShader::IsAlphaBlendMaterial()
{
	return TFALSE;
}

void ASkinShader::SetAlphaBlendMaterial( TBOOL a_bIsAlphaBlendMaterial )
{
}

void ASkinShader::EnableClouds( TBOOL a_bEnabled )
{
}

TBOOL ASkinShader::IsEnabledClouds()
{
	return TFALSE;
}

TBOOL ASkinShader::IsCapablePointLights()
{
	return TFALSE;
}

void ASkinShader::SetPointLights( TBOOL a_bEnabled )
{
}

TBOOL ASkinShader::IsPointLightsEnabled()
{
	return TFALSE;
}
