#include "pch.h"
#include "AInputMap.h"
#include "AInputMapManager.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TDEFINE_CLASS_NORUNTIME(AInputMap);

void AInputMap::LoadFromProperties(const PBProperties& a_rProperties)
{
	m_oDoodadToCommandMap.Clear();
	
	auto pInputManager = AInputMapManager::GetSingleton();

	for (auto it = a_rProperties.Begin(); it != a_rProperties.End(); it++)
	{
		auto eCommandCode = pInputManager->GetCommandCode(it->GetName().GetString());
		auto pArray = it->GetValue()->GetArray();

		for (TUINT i = 0; i < pArray->GetSize(); i++)
		{
			auto pValue = pArray->GetValue(i);
			auto iDoodad = pInputManager->GetDoodadFromKey(pValue->GetTPString8());

			m_oDoodadToCommandMap.Insert(iDoodad, eCommandCode);
		}
	}
}
