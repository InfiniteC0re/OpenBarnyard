#include "ToshiPCH.h"
#include "TSysShader.h"

namespace Toshi {

	const char* TSysShader::GetShaderPrefix()
	{
		return "";
	}

	TBOOL TSysShader::IsLightScattering() const
	{
		return TFALSE;
	}

	TBOOL TSysShader::SetLightScattering(TBOOL scattering) const
	{
		return TFALSE;
	}

}