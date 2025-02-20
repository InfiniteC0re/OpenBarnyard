#include "pch.h"
#include "AGrassShader.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TDEFINE_CLASS_NORUNTIME( AGrassShader );

// $Barnyard: FUNCTION 005f7a40
AGrassShader::AGrassShader()
    : m_Unk2( 0.5f )
    , m_Unk3( 0 )
{
}

// $Barnyard: FUNCTION 005f7a70
// $Barnyard: FUNCTION 005f7bc0
AGrassShader::~AGrassShader()
{
}

void AGrassShader::EnableRenderEnvMap( TBOOL a_bEnable )
{
}

TBOOL AGrassShader::IsHighEndSkinning()
{
	return TTRUE;
}

void AGrassShader::EnableHighEndSkinning( TBOOL a_bEnable )
{
}

TBOOL AGrassShader::IsCapableHighEndSkinning()
{
	return TTRUE;
}

TBOOL AGrassShader::IsLightScattering()
{
	return TFALSE;
}

void AGrassShader::SetLightScattering( TBOOL a_bEnable )
{
}

TBOOL AGrassShader::IsAlphaBlendMaterial()
{
	return TFALSE;
}

void AGrassShader::SetAlphaBlendMaterial( TBOOL a_bIsAlphaBlendMaterial )
{
}
