#pragma once
#include "AInputMap.h"

#include <Input/TInputInterface.h>
#include <Toshi/TPString8.h>
#include <Toshi/T2Vector.h>
#include <Toshi/T2Map.h>

struct AInputCommandArray
{
	static constexpr TUINT MAX_NUM_COMMANDS = 40;

	void Clear()
	{
		for (TINT i = 0; i < iNumCommands; i++)
		{
			aCommands[i] = AInputCommand_Empty;
		}

		iNumCommands = 0;
	}

	void AddCommand(AInputCommand a_eCommand)
	{
		if (iNumCommands < MAX_NUM_COMMANDS)
		{
			aCommands[iNumCommands++] = a_eCommand;
		}
	}

	TINT iNumCommands = 0;
	AInputCommand aCommands[MAX_NUM_COMMANDS];
};

class AInputMapManager :
	public Toshi::TObject,
	public Toshi::TSingleton<AInputMapManager>
{
public:
	TDECLARE_CLASS(Toshi::TObject);

	using ActionId = TUINT32;

	enum InputEvent : TUINT32
	{
		InputEvent_None = 0,
		InputEvent_GoneDown = BITFLAG(Toshi::TInputInterface::EVENT_TYPE_GONE_DOWN),
		InputEvent_GoneUp = BITFLAG(Toshi::TInputInterface::EVENT_TYPE_GONE_UP),
		InputEvent_Repeat = BITFLAG(Toshi::TInputInterface::EVENT_TYPE_REPEAT),
		InputEvent_Unknown = BITFLAG(Toshi::TInputInterface::EVENT_TYPE_UNKNOWN),
		InputEvent_Moved = BITFLAG(Toshi::TInputInterface::EVENT_TYPE_MOVED),
	};

	struct Command
	{
		Toshi::TPString8 Name;
		TINT32 uiCode;
		TINT32 uiEventType;

		TBOOL IsEventTypeAllowed(Toshi::TInputInterface::EVENT_TYPE a_eEventType)
		{
			return HASFLAG(uiEventType & (1 << (a_eEventType & 0x1F)));
		}
	};

	static constexpr TUINT MAX_NUM_INPUT_MAPS = 32;

public:
	AInputMapManager();

	TBOOL ReadControlsData();

	void PushInputMap(AInputMap* a_pInputMap);
	AInputMap* PopInputMap();

	AInputMap* FindMap(const Toshi::TPString8& a_rMapName);
	TBOOL SetMap(const Toshi::TPString8& a_rMapName);

	void GetEventCommands(const Toshi::TInputInterface::InputEvent* a_pEvent, AInputCommandArray& a_rCommandArray);

	AInputCommand GetCommandCode(const Toshi::TPString8& a_rCommandName);
	Toshi::TInputDevice::Doodad GetDoodadFromKey(const Toshi::TPString8& a_rKey);

private:
	void InitMouseDoodads();
	void InitKeyboardDoodads();
	void InitGamepadDoodads();
	void InitCommandMap();

	void BindDoodad(Toshi::TInputDevice::Doodad a_iDoodad, const Toshi::TPString8& a_ButtonName, ActionId a_uiAction);

private:
	Toshi::T2Vector<AInputMap*, MAX_NUM_INPUT_MAPS> m_InputMaps;
	AInputMap* m_pActiveInputMap;
	Toshi::T2Map<Toshi::TInputDevice::Doodad, Toshi::TPString8> m_oDoodadToNameMap;
	Toshi::T2Map<Toshi::TInputDevice::Doodad, ActionId> m_oKeyMap;
	Toshi::T2Map<AInputCommand, Command> m_oCommandMap;
	Toshi::T2RedBlackTree<void*> m_UnkMap;
	TINT m_iNumPushedInputMaps;
	// ...
	AInputMap* m_apPushedInputMaps[MAX_NUM_INPUT_MAPS];
};
