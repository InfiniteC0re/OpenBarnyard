#pragma once
#include "Toshi/Core/TObject.h"
#include "Toshi/Render/TShader.h"
#include "TSysMaterial.h"
#include "TSysMesh.h"

namespace Toshi {

	class TSysShader :
		public TGenericClassDerived<TSysShader, TShader, "TSysShader", TMAKEVERSION(1, 0), TFALSE>,
		public TSingleton<TSysShader>
	{
	public:
		virtual const char* GetShaderPrefix() override;

		virtual TSysMaterial* CreateMaterial() = 0;
		virtual TSysMesh* CreateMesh(const char* name) = 0;
		virtual TBOOL IsLightScattering() const;
		virtual TBOOL SetLightScattering(TBOOL scattering) const;
	};

}