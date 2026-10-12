#include "pch.h"
#include "ASysShader.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TDEFINE_CLASS_NORUNTIME( ASysShader );

// $Barnyard: FUNCTION 005f0360
ASysShader::ASysShader()
{
}

// $Barnyard: FUNCTION 005f0380
// $Barnyard: FUNCTION 005f0490
ASysShader::~ASysShader()
{
}

TBOOL ASysShader::IsLightScattering()
{
	return TFALSE;
}

void ASysShader::SetLightScattering( TBOOL a_bEnabled )
{
}
