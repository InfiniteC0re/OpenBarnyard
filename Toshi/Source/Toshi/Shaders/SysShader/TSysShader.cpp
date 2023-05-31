#include "ToshiPCH.h"
#include "TSysShader.h"

namespace Toshi {

	const char* TSysShader::GetShaderPrefix()
	{
		return "";
	}

	bool TSysShader::IsLightScattering() const
	{
		return false;
	}

	bool TSysShader::SetLightScattering(bool scattering) const
	{
		return false;
	}

}