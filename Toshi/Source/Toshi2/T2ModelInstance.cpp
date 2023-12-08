#include "ToshiPCH.h"
#include "T2ModelInstance.h"

namespace Toshi {

	void T2ModelInstance::Create(TModelPtr* a_pModelPtr)
	{
		m_eFlags |= 0b00001000;
		m_pModelRef = a_pModelPtr;
		m_pModelInstance = a_pModelPtr->GetModel()->CreateInstance();
		TTODO("Initialise some unknown values");
	}

	void T2ModelInstance::Delete()
	{
		if (this)
		{
			if (m_pModelInstance)
			{
				m_pModelInstance->Delete();
				m_pModelInstance = TNULL;
				m_eFlags &= ~0b00001000;
			}

			delete this;
		}
	}

	void T2ModelInstance::Update(TFLOAT a_fDeltaTime)
	{
		m_pModelInstance->Update(a_fDeltaTime);
	}

}
