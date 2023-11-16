#pragma once
#include <Toshi/File/TTRB.h>
#include <Toshi/Strings/TPString8.h>
#include <Toshi/Input/TInputInterface.h>
#include <Toshi2/T2Map.h>

#include <Plugins/PPropertyParser/PProperties.h>

using ACommandCode = TINT;

TOBJECT(AInputMap, Toshi::TObject, TFALSE)
{
public:
	AInputMap(const Toshi::TPString8& a_rName) :
		m_Name(a_rName)
	{
		
	}

	void LoadFromProperties(const PProperties& a_rProperties);

private:
	Toshi::TPString8 m_Name;
	Toshi::T2Map<Toshi::TInputDevice::Doodad, ACommandCode> m_oDoodadToCommandMap;
};
