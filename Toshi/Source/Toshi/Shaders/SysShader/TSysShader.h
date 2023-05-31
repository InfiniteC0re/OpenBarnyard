#pragma once
#include "Toshi/Core/TObject.h"
#include "Toshi/Render/TShader.h"

namespace Toshi {

	class TSysShader :
		public TGenericClassDerived<TSysShader, TShader, "TSysShader", TMAKEVERSION(1, 0), TFALSE>,
		public TSingleton<TSysShader>
	{
	public:
		virtual const char* GetShaderPrefix() override;

		virtual bool IsLightScattering() const;
		virtual bool SetLightScattering(bool scattering) const;
	};

}