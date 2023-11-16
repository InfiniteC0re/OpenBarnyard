#pragma once
#include "AInputMap.h"

#include <Toshi/Input/TInputInterface.h>
#include <Toshi/Strings/TPString8.h>
#include <Toshi2/T2Vector.h>
#include <Toshi2/T2Map.h>

class AInputMapManager :
	public Toshi::TGenericClassDerived<AInputMapManager, Toshi::TObject, "AInputMapManager", TMAKEVERSION(1, 0), TTRUE>,
	public Toshi::TSingleton<AInputMapManager>
{
public:
	using ActionId = TUINT32;

	struct Command
	{
		Toshi::TPString8 Name;
		TINT32 uiCode;
		TINT32 uiFlags;
	};

public:
	AInputMapManager();

	TBOOL ReadControlsData();

	ACommandCode GetCommandCode(const Toshi::TPString8& a_rCommandName);
	Toshi::TInputDevice::Doodad GetDoodadFromKey(const Toshi::TPString8& a_rKey);

private:
	void InitMouseDoodads();
	void InitKeyboardDoodads();
	void InitGamepadDoodads();
	void InitCommandMap();

	void BindDoodad(Toshi::TInputDevice::Doodad a_iDoodad, const Toshi::TPString8& a_ButtonName, ActionId a_uiAction);

private:
	Toshi::T2Vector<AInputMap*, 32> m_InputMaps;
	// ...
	Toshi::T2Map<Toshi::TInputDevice::Doodad, Toshi::TPString8> m_oDoodadToNameMap;
	Toshi::T2Map<Toshi::TInputDevice::Doodad, ActionId> m_oKeyMap;
	Toshi::T2Map<ACommandCode, Command> m_oCommandMap;
	Toshi::T2RedBlackTree<void*> m_UnkMap;
};
