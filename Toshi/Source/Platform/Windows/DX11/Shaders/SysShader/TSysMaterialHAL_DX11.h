#pragma once
#include "Toshi/Shaders/SysShader/TSysMaterial.h"

namespace Toshi {

	class TSysMaterialHAL :
		public TGenericClassDerived<TSysMaterialHAL, TSysMaterial, "TSysMaterialHAL", TMAKEVERSION(1, 0), TFALSE>
	{
	public:
		TSysMaterialHAL()
		{
			m_Unk = 0;
		}

	public:
		inline static int s_eSysMaterialLazyState = 1;
		inline static TVector4 s_AlphaTest;

	private:
		int m_Unk;
	};

}