#pragma once
#include "Toshi/Shaders/SysShader/TSysMaterial.h"

namespace Toshi {

	class TOrderTable;

	class TSysMaterialHAL :
		public TGenericClassDerived<TSysMaterialHAL, TSysMaterial, "TSysMaterialHAL", TMAKEVERSION(1, 0), TFALSE>
	{
	public:
		TSysMaterialHAL()
		{
			m_pOrderTable = TNULL;
		}

		virtual ~TSysMaterialHAL();

		virtual void PreRender() override;
		virtual void PostRender() override;
		virtual TBOOL Create(BlendMode eBlendMode) override;
		virtual void SetBlendMode(BlendMode eBlendMode) override;

		void SetOrderTable(TOrderTable* pOrderTable);

	public:
		inline static int s_eSysMaterialLazyState = 1;
		inline static TVector4 s_AlphaTest;

	private:
		TOrderTable* m_pOrderTable;
	};

}