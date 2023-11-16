#pragma once
#include "Memory/AMemory.h"

#include <Toshi/File/TTRB.h>
#include <Toshi/Strings/TPString8.h>
#include <Toshi/Input/TInputInterface.h>
#include <Toshi2/T2Map.h>

#include <Plugins/PPropertyParser/PProperties.h>

using AInputCommand = TINT;

TOBJECT(AInputMap, Toshi::TObject, TFALSE)
{
public:
	AInputMap(const Toshi::TPString8& a_rName) :
		m_Name(a_rName),
		m_oDoodadToCommandMap(AMemory::GetAllocator(AMemory::POOL_Misc))
	{
		
	}

	void LoadFromProperties(const PProperties& a_rProperties);
	Toshi::T2Map<Toshi::TInputDevice::Doodad, AInputCommand>& GetCommandMap() { return m_oDoodadToCommandMap; }

private:
	Toshi::TPString8 m_Name;
	Toshi::T2Map<Toshi::TInputDevice::Doodad, AInputCommand> m_oDoodadToCommandMap;
};
