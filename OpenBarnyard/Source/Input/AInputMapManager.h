#pragma once
#include "AInputMap.h"

#include <Toshi/Input/TInputInterface.h>
#include <Toshi/Strings/TPString8.h>
#include <Toshi2/T2Map.h>

class AInputMapManager :
	public Toshi::TGenericClassDerived<AInputMapManager, Toshi::TObject, "AInputMapManager", TMAKEVERSION(1, 0), TTRUE>,
	public Toshi::TSingleton<AInputMapManager>
{
public:
	using ActionId = TUINT32;

	struct Button
	{
		Toshi::TPString8 Name;
		TINT32 uiCode;
		TINT32 uiFlags;
	};

public:
	AInputMapManager();

	TBOOL ReadControlsData();

private:
	void InitMouseDoodads();
	void InitKeyboardDoodads();
	void InitGamepadDoodads();
	void InitButtonMap();

	void BindDoodad(Toshi::TInputDevice::Doodad a_iDoodad, const Toshi::TPString8& a_ButtonName, ActionId a_uiAction);

private:
	TINT m_iNumInputMaps;
	AInputMap* m_apInputMaps[32];
	// ...
	Toshi::T2Map<Toshi::TInputDevice::Doodad, Toshi::TPString8> m_oDoodadToNameMap;
	Toshi::T2Map<Toshi::TInputDevice::Doodad, ActionId> m_oKeyMap;
	Toshi::T2Map<TINT32, Button> m_oButtonMap;
};
