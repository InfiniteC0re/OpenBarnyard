#include "pch.h"
#include "AInputMapManager.h"
#include "Memory/AMemory.h"

TOSHI_NAMESPACE_USING

AInputMapManager::AInputMapManager() :
	m_pActiveInputMap(TNULL),
	m_iNumPushedInputMaps(0),
	m_oDoodadToNameMap(AMemory::GetAllocator(AMemory::POOL_Misc)),
	m_oKeyMap(AMemory::GetAllocator(AMemory::POOL_Misc)),
	m_oCommandMap(AMemory::GetAllocator(AMemory::POOL_Misc)),
	m_UnkMap(AMemory::GetAllocator(AMemory::POOL_Misc))
{
	InitMouseDoodads();
	InitKeyboardDoodads();
	InitGamepadDoodads();
	InitCommandMap();
}

TBOOL AInputMapManager::ReadControlsData()
{
	TTRB trb;
	auto eLoadResult = trb.Load(TString8::Format("Data/Options/%s.trb", "Controls_Win"));

	if (eLoadResult != TTRB::ERROR_OK)
	{
		return TFALSE;
	}

	auto properties = PProperties::LoadFromTRB(trb);
	
	while (m_InputMaps.Size() != 0)
	{
		auto pInputMap = m_InputMaps.Back();
		m_InputMaps.PopBack();

		if (pInputMap)
		{
			delete pInputMap;
		}
	}

	for (auto it = properties->Begin(); it != properties->End(); it++)
	{
		auto pInputMap = new AInputMap(it->GetName().GetString());

		pInputMap->LoadFromProperties(*it->GetValue()->GetProperties());
		m_InputMaps.PushBack(pInputMap);
	}

	return TTRUE;
}

void AInputMapManager::PushInputMap(AInputMap* a_pInputMap)
{
	TASSERT(m_iNumPushedInputMaps < MAX_NUM_INPUT_MAPS);
	m_iNumPushedInputMaps++;

	if (m_iNumPushedInputMaps < 32)
	{
		m_apPushedInputMaps[m_iNumPushedInputMaps - 1] = a_pInputMap;
	}

	m_pActiveInputMap = a_pInputMap;
}

AInputMap* AInputMapManager::PopInputMap()
{
	TASSERT(m_iNumPushedInputMaps > 0);
	m_iNumPushedInputMaps = Toshi::TMath::Max(m_iNumPushedInputMaps - 1, 0);

	m_pActiveInputMap = (m_iNumPushedInputMaps != 0) ? m_apPushedInputMaps[m_iNumPushedInputMaps - 1] : TNULL;
	return m_apPushedInputMaps[m_iNumPushedInputMaps - 1];
}

void AInputMapManager::GetEventCommands(const Toshi::TInputInterface::InputEvent* a_pEvent, AInputCommandArray& a_rCommandArray)
{
	a_rCommandArray.Clear();

	if (m_pActiveInputMap)
	{
		auto pCommandMap = &m_pActiveInputMap->GetCommandMap();
		auto pCommandIndexRes = pCommandMap->FindNode(a_pEvent->GetDoodad());

		while (pCommandIndexRes != pCommandMap->End())
		{
			auto pCommandRes = m_oCommandMap.FindNode(pCommandIndexRes->GetValue()->GetSecond());
			TBOOL bCommandValid;

			if (pCommandRes != m_oCommandMap.End())
			{
				bCommandValid = TTRUE;
				auto command = pCommandRes->GetValue()->GetSecond();

				if (!command.IsEventTypeAllowed(a_pEvent->GetEventType()))
				{
					bCommandValid = TFALSE;
				}
			}
			else
			{
				bCommandValid = TFALSE;
			}

			if (bCommandValid)
			{
				a_rCommandArray.AddCommand(pCommandIndexRes->GetValue()->GetSecond());
			}

			// Look if there is any other commands binded to this doodad
			pCommandIndexRes = pCommandMap->FindNextNode(pCommandIndexRes, a_pEvent->GetDoodad());
		}
	}
}

AInputCommand AInputMapManager::GetCommandCode(const Toshi::TPString8& a_rCommandName)
{
	for (auto it = m_oCommandMap.Begin(); it != m_oCommandMap.End(); it++)
	{
		if (it->GetSecond().Name == a_rCommandName)
		{
			return it->GetFirst();
		}
	}

	return AInputCommand_Unknown;
}

Toshi::TInputDevice::Doodad AInputMapManager::GetDoodadFromKey(const Toshi::TPString8& a_rKey)
{
	for (auto it = m_oDoodadToNameMap.Begin(); it != m_oDoodadToNameMap.End(); it++)
	{
		if (it->GetSecond() == a_rKey)
		{
			return it->GetFirst();
		}
	}

	return -1;
}

void AInputMapManager::InitMouseDoodads()
{
	BindDoodad(0x30001, "Button1", 0x582);
	BindDoodad(0x30002, "Button2", 0x583);
	BindDoodad(0x30003, "Button3", 0x584);
	BindDoodad(0x30004, "Button4", 0x585);
	BindDoodad(0x30005, "Button5", 0x586);
	BindDoodad(0x30006, "Button6", 0x587);
	BindDoodad(0x30007, "Button7", 0x588);
	BindDoodad(0x30008, "Button8", 0x589);
	BindDoodad(0x30009, "Wheel Fwd", 0x58A);
	BindDoodad(0x3000A, "Wheel Back", 0x58B);
	BindDoodad(0x3000B, "Cursor Axis", 0x58C);
	BindDoodad(0x3000C, "Wheel Axis", 0x58D);
}

void AInputMapManager::InitKeyboardDoodads()
{
	BindDoodad(0x20001, "Virtual Stick", 0x506);
	BindDoodad(0x2002e, "Left Shift", 0x530);
	BindDoodad(0x2003a, "Right Shift", 0x53c);
	BindDoodad(0x20021, "Left Control", 0x523);
	BindDoodad(0x2006b, "Right Control", 0x56d);
	BindDoodad(0x2003c, "Left Alt", 0x53e);
	BindDoodad(0x2006f, "Right Alt", 0x571);
	BindDoodad(0x2007b, "Left Windows", 0x57d);
	BindDoodad(0x2007c, "Right Windows", 0x57e);
	BindDoodad(0x20005, "Escape", 0x507);
	BindDoodad(0x20006, "1", 0x508);
	BindDoodad(0x20007, "2", 0x509);
	BindDoodad(0x20008, "3", 0x50a);
	BindDoodad(0x20009, "4", 0x50b);
	BindDoodad(0x2000a, "5", 0x50c);
	BindDoodad(0x2000b, "6", 0x50d);
	BindDoodad(0x2000c, "7", 0x50e);
	BindDoodad(0x2000d, "8", 0x50f);
	BindDoodad(0x2000e, "9", 0x510);
	BindDoodad(0x2000f, "0", 0x511);
	BindDoodad(0x20010, "Minus", 0x512);
	BindDoodad(0x20011, "Equals", 0x513);
	BindDoodad(0x20012, "Backspace", 0x514);
	BindDoodad(0x20013, "Tab", 0x515);
	BindDoodad(0x20014, "Q", 0x516);
	BindDoodad(0x20015, "W", 0x517);
	BindDoodad(0x20016, "E", 0x518);
	BindDoodad(0x20017, "R", 0x519);
	BindDoodad(0x20018, "T", 0x51a);
	BindDoodad(0x20019, "Y", 0x51b);
	BindDoodad(0x2001a, "U", 0x51c);
	BindDoodad(0x2001b, "I", 0x51d);
	BindDoodad(0x2001c, "O", 0x51e);
	BindDoodad(0x2001d, "P", 0x51f);
	BindDoodad(0x2001e, "Left Bracket", 0x520);
	BindDoodad(0x2001f, "Right Bracket", 0x521);
	BindDoodad(0x20020, "Return", 0x522);
	BindDoodad(0x20022, "A", 0x524);
	BindDoodad(0x20023, "S", 0x525);
	BindDoodad(0x20024, "D", 0x526);
	BindDoodad(0x20025, "F", 0x527);
	BindDoodad(0x20026, "G", 0x528);
	BindDoodad(0x20027, "H", 0x529);
	BindDoodad(0x20028, "J", 0x52a);
	BindDoodad(0x20029, "K", 0x52b);
	BindDoodad(0x2002a, "L", 0x52c);
	BindDoodad(0x2002b, "Semicolon", 0x52d);
	BindDoodad(0x2002c, "Apostrophe", 0x52e);
	BindDoodad(0x2002d, "Grave", 0x52f);
	BindDoodad(0x2002f, "Backslash", 0x531);
	BindDoodad(0x20030, "Z", 0x532);
	BindDoodad(0x20031, "X", 0x533);
	BindDoodad(0x20032, "C", 0x534);
	BindDoodad(0x20033, "V", 0x535);
	BindDoodad(0x20034, "B", 0x536);
	BindDoodad(0x20035, "N", 0x537);
	BindDoodad(0x20036, "M", 0x538);
	BindDoodad(0x20037, "Comma", 0x539);
	BindDoodad(0x20038, "Period", 0x53a);
	BindDoodad(0x20039, "Slash", 0x53b);
	BindDoodad(0x2003b, "Multiply", 0x53d);
	BindDoodad(0x2003d, "Space", 0x53f);
	BindDoodad(0x2003e, "Capslock", 0x540);
	BindDoodad(0x2003f, "F1", 0x541);
	BindDoodad(0x20040, "F2", 0x542);
	BindDoodad(0x20041, "F3", 0x543);
	BindDoodad(0x20042, "F4", 0x544);
	BindDoodad(0x20043, "F5", 0x545);
	BindDoodad(0x20044, "F6", 0x546);
	BindDoodad(0x20045, "F7", 0x547);
	BindDoodad(0x20046, "F8", 0x548);
	BindDoodad(0x20047, "F9", 0x549);
	BindDoodad(0x20048, "F10", 0x54a);
	BindDoodad(0x20049, "NumLock", 0x54b);
	BindDoodad(0x2004a, "ScrollLock", 0x54c);
	BindDoodad(0x2004b, "Numpad 7", 0x54d);
	BindDoodad(0x2004c, "Numpad 8", 0x54e);
	BindDoodad(0x2004d, "Numpad 9", 0x54f);
	BindDoodad(0x2004e, "Subtract", 0x550);
	BindDoodad(0x2004f, "Numpad 4", 0x551);
	BindDoodad(0x20050, "Numpad 5", 0x552);
	BindDoodad(0x20051, "Numpad 6", 0x553);
	BindDoodad(0x20052, "Add", 0x554);
	BindDoodad(0x20053, "Numpad 1", 0x555);
	BindDoodad(0x20054, "Numpad 2", 0x556);
	BindDoodad(0x20055, "Numpad 3", 0x557);
	BindDoodad(0x20056, "Numpad 0", 0x558);
	BindDoodad(0x20057, "Decimal", 0x559);
	BindDoodad(0x20058, "F11", 0x55a);
	BindDoodad(0x20059, "F12", 0x55b);
	BindDoodad(0x2005a, "F13", 0x55c);
	BindDoodad(0x2005b, "F14", 0x55d);
	BindDoodad(0x2005c, "F15", 0x55e);
	BindDoodad(0x2005d, "Kana", 0x55f);
	BindDoodad(0x2005e, "Convert", 0x560);
	BindDoodad(0x2005f, "No Convert", 0x561);
	BindDoodad(0x20060, "Yen", 0x562);
	BindDoodad(0x20061, "Numpad Equals", 0x563);
	BindDoodad(0x20062, "Circumflex", 0x564);
	BindDoodad(0x20063, "At", 0x565);
	BindDoodad(0x20064, "Colon", 0x566);
	BindDoodad(0x20065, "Underline", 0x567);
	BindDoodad(0x20066, "Kanji", 0x568);
	BindDoodad(0x20067, "Stop", 0x569);
	BindDoodad(0x20068, "Ax", 0x56a);
	BindDoodad(0x20069, "Unlabeled", 0x56b);
	BindDoodad(0x2006a, "Numpad Enter", 0x56c);
	BindDoodad(0x2006c, "Numpad Comma", 0x56e);
	BindDoodad(0x2006d, "Divide", 0x56f);
	BindDoodad(0x2006e, "SysRq", 0x570);
	BindDoodad(0x20071, "Pause", 0x572);
	BindDoodad(0x20070, "Home", 0x573);
	BindDoodad(0x20072, "Up", 0x574);
	BindDoodad(0x20073, "Prior", 0x575);
	BindDoodad(0x20074, "Left", 0x576);
	BindDoodad(0x20075, "Right", 0x577);
	BindDoodad(0x20076, "End", 0x578);
	BindDoodad(0x20077, "Down", 0x579);
	BindDoodad(0x20078, "Next", 0x57a);
	BindDoodad(0x20079, "Insert", 0x57b);
	BindDoodad(0x2007a, "Delete", 0x57c);
	BindDoodad(0x2007d, "Apps", 0x57f);
	BindDoodad(0x2007e, "Power", 0x580);
	BindDoodad(0x2007f, "Sleep", 0x581);
}

void AInputMapManager::InitGamepadDoodads()
{
	BindDoodad(0x10001, "DPadUp", 0x58e);
	BindDoodad(0x10002, "DPadRight", 0x58f);
	BindDoodad(0x10003, "DPadDown", 0x590);
	BindDoodad(0x10004, "DPadLeft", 0x591);
	BindDoodad(0x10005, "Select", 0x592);
	BindDoodad(0x10006, "Start", 0x593);
	BindDoodad(0x10007, "Left Stick Click", 0x594);
	BindDoodad(0x10008, "Right Stick Click", 0x595);
	BindDoodad(0x10009, "Context1", 0x596);
	BindDoodad(0x1000a, "Context2", 0x597);
	BindDoodad(0x1000b, "Context3", 0x598);
	BindDoodad(0x1000c, "Context4", 0x599);
	BindDoodad(0x1000d, "L1", 0x59a);
	BindDoodad(0x1000e, "L2", 0x59b);
	BindDoodad(0x1000f, "R1", 0x59c);
	BindDoodad(0x10010, "R2", 0x59d);
	BindDoodad(0x10011, "Left Stick Up", 0x59e);
	BindDoodad(0x10012, "Left Stick Right", 0x59f);
	BindDoodad(0x10013, "Left Stick Down", 0x5a0);
	BindDoodad(0x10014, "Left Stick Left", 0x5a1);
	BindDoodad(0x10015, "Right Stick Up", 0x5a2);
	BindDoodad(0x10016, "Right Stick Right", 0x5a3);
	BindDoodad(0x10017, "Right Stick Down", 0x5a4);
	BindDoodad(0x10018, "Right Stick Left", 0x5a5);
	BindDoodad(0x1001b, "Virtual DPad Stick", 0x5a8);
	BindDoodad(0x10019, "Left Stick", 0x5a6);
	BindDoodad(0x1001a, "Right Stick", 0x5a7);
}

void AInputMapManager::InitCommandMap()
{
#define ADD_COMMAND(NAME, UNK, EVENT_TYPE) m_oCommandMap.Insert(AInputCommand_##NAME, { #NAME, UNK, EVENT_TYPE })
#define ADD_COMMAND_EX(COMMAND, NAME, UNK, EVENT_TYPE) m_oCommandMap.Insert(COMMAND, { NAME, UNK, EVENT_TYPE })

	ADD_COMMAND(Unknown, -1, 0);
	ADD_COMMAND_EX(AInputCommand_Empty, Toshi::TPString8(), -1, 0);
	ADD_COMMAND(OK, 0x4b9, InputEvent_GoneDown | InputEvent_Repeat | InputEvent_Unknown | InputEvent_Moved);
	ADD_COMMAND(Cancel, 0x4ba, InputEvent_GoneDown | InputEvent_Repeat | InputEvent_Unknown | InputEvent_Moved);
	ADD_COMMAND(Retry, 0x4bb, InputEvent_GoneDown | InputEvent_Repeat | InputEvent_Unknown | InputEvent_Moved);
	ADD_COMMAND(Back, 0x4bc, InputEvent_GoneDown | InputEvent_Repeat | InputEvent_Unknown | InputEvent_Moved);
	ADD_COMMAND(Close, 0x4bd, InputEvent_GoneDown | InputEvent_Repeat | InputEvent_Unknown | InputEvent_Moved);
	ADD_COMMAND(Quit, 0x4be, InputEvent_GoneDown | InputEvent_Repeat | InputEvent_Unknown | InputEvent_Moved);
	ADD_COMMAND(Start, 0x4bf, InputEvent_GoneDown | InputEvent_Repeat | InputEvent_Unknown | InputEvent_Moved);
	ADD_COMMAND(Select, 0x4c0, InputEvent_GoneDown | InputEvent_Repeat | InputEvent_Unknown | InputEvent_Moved);
	ADD_COMMAND(Yes, 0x4c2, InputEvent_GoneDown | InputEvent_Repeat | InputEvent_Unknown | InputEvent_Moved);
	ADD_COMMAND(No, 0x4c3, InputEvent_GoneDown | InputEvent_Repeat | InputEvent_Unknown | InputEvent_Moved);
	ADD_COMMAND(Restart, 0x4c4, InputEvent_GoneDown | InputEvent_Repeat | InputEvent_Unknown | InputEvent_Moved);
	ADD_COMMAND(Leave, 0x4c5, InputEvent_GoneDown | InputEvent_Repeat | InputEvent_Unknown | InputEvent_Moved);
	ADD_COMMAND(Save, 0x4c6, InputEvent_GoneDown | InputEvent_Repeat | InputEvent_Unknown | InputEvent_Moved);
	ADD_COMMAND(Move, 0x4c9, InputEvent_GoneDown | InputEvent_Repeat | InputEvent_Unknown | InputEvent_Moved);
	ADD_COMMAND(Up, 0x4c7, InputEvent_GoneDown | InputEvent_Repeat | InputEvent_Unknown | InputEvent_Moved);
	ADD_COMMAND(Down, 0x4c8, InputEvent_GoneDown | InputEvent_Repeat | InputEvent_Unknown | InputEvent_Moved);
	ADD_COMMAND(LeftWithUp, 0x4eb, InputEvent_GoneDown | InputEvent_GoneUp | InputEvent_Repeat | InputEvent_Unknown | InputEvent_Moved);
	ADD_COMMAND(RightWithUp, 0x4ea, InputEvent_GoneDown | InputEvent_GoneUp | InputEvent_Repeat | InputEvent_Unknown | InputEvent_Moved);
	ADD_COMMAND(Left, 0x4ca, InputEvent_GoneDown | InputEvent_Repeat | InputEvent_Unknown | InputEvent_Moved);
	ADD_COMMAND(Right, 0x4cb, InputEvent_GoneDown | InputEvent_Repeat | InputEvent_Unknown | InputEvent_Moved);
	ADD_COMMAND(Forward, 0x4cc, InputEvent_GoneDown | InputEvent_Repeat | InputEvent_Unknown | InputEvent_Moved);
	ADD_COMMAND(ForwardWithUp, 0x4ec, InputEvent_GoneDown | InputEvent_GoneUp | InputEvent_Repeat | InputEvent_Unknown | InputEvent_Moved);
	ADD_COMMAND(SuperSpeed_Forward_Debug, 0x4f6, InputEvent_GoneDown | InputEvent_GoneUp | InputEvent_Repeat | InputEvent_Unknown | InputEvent_Moved);
	ADD_COMMAND(SuperSpeed, 0x4f7, InputEvent_GoneDown | InputEvent_GoneUp | InputEvent_Repeat | InputEvent_Unknown | InputEvent_Moved);
	ADD_COMMAND(PanLeft, 0x4fe, InputEvent_GoneDown | InputEvent_GoneUp | InputEvent_Repeat | InputEvent_Unknown | InputEvent_Moved);
	ADD_COMMAND(PanRight, 0x4ff, InputEvent_GoneDown | InputEvent_GoneUp | InputEvent_Repeat | InputEvent_Unknown | InputEvent_Moved);
	ADD_COMMAND(Reload, 0x4f4, InputEvent_GoneDown | InputEvent_Repeat | InputEvent_Unknown | InputEvent_Moved);
	ADD_COMMAND(Dodge, 0x4f5, InputEvent_GoneDown | InputEvent_GoneUp | InputEvent_Repeat | InputEvent_Unknown | InputEvent_Moved);
	ADD_COMMAND(Backward, 0x4cd, InputEvent_GoneDown | InputEvent_Repeat | InputEvent_Unknown | InputEvent_Moved);
	ADD_COMMAND(BackwardWithUp, 0x4ed, InputEvent_GoneDown | InputEvent_GoneUp | InputEvent_Repeat | InputEvent_Unknown | InputEvent_Moved);
	ADD_COMMAND(ZoomIn, 0x4d3, InputEvent_GoneDown | InputEvent_Repeat | InputEvent_Unknown | InputEvent_Moved);
	ADD_COMMAND(ZoomOut, 0x4d4, InputEvent_GoneDown | InputEvent_Repeat | InputEvent_Unknown | InputEvent_Moved);
	ADD_COMMAND(Look, 0x4d5, InputEvent_GoneDown | InputEvent_Repeat | InputEvent_Unknown | InputEvent_Moved);
	ADD_COMMAND(LookUp, 0x4d6, InputEvent_GoneDown | InputEvent_Repeat | InputEvent_Unknown | InputEvent_Moved);
	ADD_COMMAND(LookDown, 0x4d7, InputEvent_GoneDown | InputEvent_Repeat | InputEvent_Unknown | InputEvent_Moved);
	ADD_COMMAND(LookLeft, 0x4d8, InputEvent_GoneDown | InputEvent_Repeat | InputEvent_Unknown | InputEvent_Moved);
	ADD_COMMAND(LookRight, 0x4d9, InputEvent_GoneDown | InputEvent_Repeat | InputEvent_Unknown | InputEvent_Moved);
	ADD_COMMAND(LookUpWithUp, 0x4b7, InputEvent_GoneDown | InputEvent_GoneUp | InputEvent_Repeat | InputEvent_Unknown | InputEvent_Moved);
	ADD_COMMAND(LookDownWithUp, 0x4b8, InputEvent_GoneDown | InputEvent_GoneUp | InputEvent_Repeat | InputEvent_Unknown | InputEvent_Moved);
	ADD_COMMAND(LookLeftWithUp, 0x4b5, InputEvent_GoneDown | InputEvent_GoneUp | InputEvent_Repeat | InputEvent_Unknown | InputEvent_Moved);
	ADD_COMMAND(LookRightWithUp, 0x4b6, InputEvent_GoneDown | InputEvent_GoneUp | InputEvent_Repeat | InputEvent_Unknown | InputEvent_Moved);
	ADD_COMMAND(Context1, 0x4da, InputEvent_GoneDown | InputEvent_Repeat | InputEvent_Unknown | InputEvent_Moved);
	ADD_COMMAND(Context2, 0x4db, InputEvent_GoneDown | InputEvent_Repeat | InputEvent_Unknown | InputEvent_Moved);
	ADD_COMMAND(Context3, 0x4dc, InputEvent_GoneDown | InputEvent_Repeat | InputEvent_Unknown | InputEvent_Moved);
	ADD_COMMAND(Context4, 0x4dd, InputEvent_GoneDown | InputEvent_Repeat | InputEvent_Unknown | InputEvent_Moved);
	ADD_COMMAND(Menu, 0x4e4, InputEvent_GoneDown | InputEvent_Repeat | InputEvent_Unknown | InputEvent_Moved);
	ADD_COMMAND(DebugMenu, 0x4e5, InputEvent_GoneDown | InputEvent_Repeat | InputEvent_Unknown | InputEvent_Moved);
	ADD_COMMAND(Interact, 0x4e6, InputEvent_GoneDown | InputEvent_Repeat | InputEvent_Unknown | InputEvent_Moved);
	ADD_COMMAND(Use, 0x4f2, InputEvent_GoneDown | InputEvent_Repeat | InputEvent_Unknown | InputEvent_Moved);
	ADD_COMMAND(Jump, 0x4f0, InputEvent_GoneDown | InputEvent_GoneUp | InputEvent_Repeat | InputEvent_Unknown | InputEvent_Moved);
	ADD_COMMAND(Shoot, 0x4e7, InputEvent_GoneDown | InputEvent_GoneUp | InputEvent_Repeat | InputEvent_Unknown | InputEvent_Moved);
	ADD_COMMAND(ZLock, 0x500, InputEvent_GoneDown | InputEvent_GoneUp | InputEvent_Repeat | InputEvent_Unknown | InputEvent_Moved);
	ADD_COMMAND(ShootAlt, 0x4e8, InputEvent_GoneDown | InputEvent_GoneUp | InputEvent_Repeat | InputEvent_Unknown | InputEvent_Moved);
	ADD_COMMAND(Switch, 0x4f9, InputEvent_GoneDown | InputEvent_GoneUp | InputEvent_Repeat | InputEvent_Unknown | InputEvent_Moved);
	ADD_COMMAND(SideStep, 0x4f8, InputEvent_GoneDown | InputEvent_GoneUp | InputEvent_Repeat | InputEvent_Unknown | InputEvent_Moved);
	ADD_COMMAND(Speed, 0x4ef, InputEvent_GoneDown | InputEvent_Repeat | InputEvent_Unknown | InputEvent_Moved);
	ADD_COMMAND(Brake, 0x4ee, InputEvent_GoneDown | InputEvent_Repeat | InputEvent_Unknown | InputEvent_Moved);
	ADD_COMMAND(Accelerator, 0x4e9, InputEvent_GoneDown | InputEvent_Repeat | InputEvent_Unknown | InputEvent_Moved);
	ADD_COMMAND(Bell, 0x4f3, InputEvent_GoneDown | InputEvent_Repeat | InputEvent_Unknown | InputEvent_Moved);
	ADD_COMMAND(Trick1, 0x4fa, InputEvent_GoneDown | InputEvent_GoneUp | InputEvent_Repeat | InputEvent_Unknown | InputEvent_Moved);
	ADD_COMMAND(Trick2, 0x4fb, InputEvent_GoneDown | InputEvent_GoneUp | InputEvent_Repeat | InputEvent_Unknown | InputEvent_Moved);
	ADD_COMMAND(Trick3, 0x4fc, InputEvent_GoneDown | InputEvent_GoneUp | InputEvent_Repeat | InputEvent_Unknown | InputEvent_Moved);
	ADD_COMMAND(Trick4, 0x4fd, InputEvent_GoneDown | InputEvent_GoneUp | InputEvent_Repeat | InputEvent_Unknown | InputEvent_Moved);
	ADD_COMMAND(SelectUp, 0x4de, InputEvent_GoneDown | InputEvent_Repeat | InputEvent_Unknown | InputEvent_Moved);
	ADD_COMMAND(SelectDown, 0x4df, InputEvent_GoneDown | InputEvent_Repeat | InputEvent_Unknown | InputEvent_Moved);
	ADD_COMMAND(SelectLeft, 0x4e0, InputEvent_GoneDown | InputEvent_Repeat | InputEvent_Unknown | InputEvent_Moved);
	ADD_COMMAND(SelectRight, 0x4e1, InputEvent_GoneDown | InputEvent_Repeat | InputEvent_Unknown | InputEvent_Moved);
	ADD_COMMAND(SelectPrevious, 0x4e2, InputEvent_GoneDown | InputEvent_Repeat | InputEvent_Unknown | InputEvent_Moved);
	ADD_COMMAND(SelectNext, 0x4e3, InputEvent_GoneDown | InputEvent_Repeat | InputEvent_Unknown | InputEvent_Moved);
	ADD_COMMAND(ElevateCameraCW, 0x4ce, InputEvent_GoneDown | InputEvent_Repeat | InputEvent_Unknown | InputEvent_Moved);
	ADD_COMMAND(ElevateCameraCCW, 0x4cf, InputEvent_GoneDown | InputEvent_Repeat | InputEvent_Unknown | InputEvent_Moved);
	ADD_COMMAND(RotateCameraCW, 0x4d0, InputEvent_GoneDown | InputEvent_Repeat | InputEvent_Unknown | InputEvent_Moved);
	ADD_COMMAND(RotateCamera, 0x4d1, InputEvent_GoneDown | InputEvent_Repeat | InputEvent_Unknown | InputEvent_Moved);
	ADD_COMMAND(RotateCameraCCW, 0x4d2, InputEvent_GoneDown | InputEvent_Repeat | InputEvent_Unknown | InputEvent_Moved);
	ADD_COMMAND(Inventory, 0x4f1, InputEvent_GoneDown | InputEvent_Repeat | InputEvent_Unknown | InputEvent_Moved);
	ADD_COMMAND(Icons, 0x501, InputEvent_GoneDown | InputEvent_GoneUp | InputEvent_Repeat | InputEvent_Unknown | InputEvent_Moved);
	ADD_COMMAND(ZLock, 0x500, InputEvent_GoneDown | InputEvent_GoneUp | InputEvent_Repeat | InputEvent_Unknown | InputEvent_Moved);
	ADD_COMMAND(Quad, -1, InputEvent_GoneDown | InputEvent_Repeat | InputEvent_Unknown | InputEvent_Moved);
	ADD_COMMAND(Horn, 0x4b4, InputEvent_GoneDown | InputEvent_GoneUp | InputEvent_Repeat | InputEvent_Unknown | InputEvent_Moved);
	ADD_COMMAND_EX(AInputCommand_CUSTOM_COMMAND, "CUSTOM COMMAND", -1, 0);
}

void AInputMapManager::BindDoodad(Toshi::TInputDevice::Doodad a_iDoodad, const Toshi::TPString8& a_ButtonName, ActionId a_uiAction)
{
	m_oDoodadToNameMap.Insert(a_iDoodad, a_ButtonName);
	m_oKeyMap.Insert(a_iDoodad, a_uiAction);
}
