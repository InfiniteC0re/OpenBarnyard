#include "pch.h"
#include "AWorldShader.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TDEFINE_CLASS_NORUNTIME( AWorldShader );

// $Barnyard: FUNCTION 005f5e10
AWorldShader::AWorldShader()
{
	m_fUnk2 = 0.5f;
	m_pUnk3 = TNULL;
}

// $Barnyard: FUNCTION 005f5e40
AWorldShader::~AWorldShader()
{
}

void AWorldShader::EnableRenderEnvMap( TBOOL a_bEnable )
{
}

TBOOL AWorldShader::IsHighEndSkinning()
{
	return TTRUE;
}

void AWorldShader::EnableHighEndSkinning( TBOOL a_bEnable )
{
}

TBOOL AWorldShader::IsCapableHighEndSkinning()
{
	return TTRUE;
}

TBOOL AWorldShader::IsLightScattering()
{
	return TFALSE;
}

void AWorldShader::SetLightScattering( TBOOL a_bEnable )
{
}

TBOOL AWorldShader::IsAlphaBlendMaterial()
{
	return TFALSE;
}

void AWorldShader::SetAlphaBlendMaterial( TBOOL a_bIsAlphaBlendMaterial )
{
}
