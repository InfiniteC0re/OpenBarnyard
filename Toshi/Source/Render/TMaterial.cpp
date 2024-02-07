#include "ToshiPCH.h"
#include "TMaterial.h"

namespace Toshi {

	TDEFINE_CLASS_NORUNTIME(TMaterial);

	TMaterial::~TMaterial()
	{

	}

	void TMaterial::OnDestroy()
	{
		TASSERT(TTRUE == IsCreated());
		m_Flags &= ~Flags_Created;
	}

	TBOOL TMaterial::Create()
	{
		TASSERT(TFALSE == IsCreated());
		m_Flags |= Flags_Created;
		return TTRUE;
	}

	void TMaterial::PreRender()
	{

	}

	void TMaterial::PostRender()
	{

	}

}
