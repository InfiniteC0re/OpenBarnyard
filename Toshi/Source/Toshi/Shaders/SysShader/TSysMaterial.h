#pragma once
#include "Toshi/Core/TObject.h"
#include "Toshi/Render/TMaterial.h"

namespace Toshi {

	class TSysMaterial :
		public TGenericClassDerived<TSysMaterial, TMaterial, "TSysMaterial", TMAKEVERSION(1, 0), TFALSE>
	{
	public:
		TSysMaterial()
		{
			m_Unk = 0;
		}

	private:
		int m_Unk;
	};

}