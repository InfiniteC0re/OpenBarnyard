#pragma once
#include "Memory/AMemory.h"

#include <File/TTRB.h>
#include <Toshi/TPString8.h>
#include <Input/TInputInterface.h>
#include <Toshi/T2Map.h>

#include <Plugins/PPropertyParser/PBProperties.h>

enum AInputCommand
{
	AInputCommand_Unknown = -1,
	AInputCommand_Empty = 0,
	AInputCommand_OK = 1,
	AInputCommand_Cancel = 2,
	AInputCommand_Retry = 3,
	AInputCommand_Back = 4,
	AInputCommand_Close = 5,
	AInputCommand_Quit = 6,
	AInputCommand_Start = 7,
	AInputCommand_Select = 8,
	AInputCommand_Yes = 10,
	AInputCommand_No = 11,
	AInputCommand_Restart = 12,
	AInputCommand_Leave = 13,
	AInputCommand_Save = 14,
	AInputCommand_Move = 15,
	AInputCommand_Up = 16,
	AInputCommand_Down = 17,
	AInputCommand_LeftWithUp = 20,
	AInputCommand_RightWithUp = 21,
	AInputCommand_Left = 18,
	AInputCommand_Right = 19,
	AInputCommand_Forward = 22,
	AInputCommand_ForwardWithUp = 23,
	AInputCommand_SuperSpeed_Forward_Debug = 24,
	AInputCommand_SuperSpeed = 25,
	AInputCommand_PanLeft = 28,
	AInputCommand_PanRight = 29,
	AInputCommand_Reload = 68,
	AInputCommand_Dodge = 69,
	AInputCommand_Backward = 26,
	AInputCommand_BackwardWithUp = 27,
	AInputCommand_ZoomIn = 30,
	AInputCommand_ZoomOut = 31,
	AInputCommand_Look = 32,
	AInputCommand_LookUp = 33,
	AInputCommand_LookDown = 34,
	AInputCommand_LookLeft = 35,
	AInputCommand_LookRight = 36,
	AInputCommand_LookUpWithUp = 37,
	AInputCommand_LookDownWithUp = 38,
	AInputCommand_LookLeftWithUp = 39,
	AInputCommand_LookRightWithUp = 40,
	AInputCommand_Context1 = 41,
	AInputCommand_Context2 = 42,
	AInputCommand_Context3 = 43,
	AInputCommand_Context4 = 44,
	AInputCommand_Menu = 51,
	AInputCommand_DebugMenu = 52,
	AInputCommand_Interact = 53,
	AInputCommand_Use = 54,
	AInputCommand_Jump = 57,
	AInputCommand_Shoot = 55,
	AInputCommand_ShootAlt = 56,
	AInputCommand_Switch = 67,
	AInputCommand_SideStep = 66,
	AInputCommand_Speed = 58,
	AInputCommand_Brake = 59,
	AInputCommand_Accelerator = 60,
	AInputCommand_Bell = 61,
	AInputCommand_Trick1 = 62,
	AInputCommand_Trick2 = 63,
	AInputCommand_Trick3 = 64,
	AInputCommand_Trick4 = 65,
	AInputCommand_SelectUp = 45,
	AInputCommand_SelectDown = 46,
	AInputCommand_SelectLeft = 47,
	AInputCommand_SelectRight = 48,
	AInputCommand_SelectPrevious = 49,
	AInputCommand_SelectNext = 50,
	AInputCommand_ElevateCameraCW = 71,
	AInputCommand_ElevateCameraCCW = 70,
	AInputCommand_RotateCameraCW = 72,
	AInputCommand_RotateCamera = 73,
	AInputCommand_RotateCameraCCW = 74,
	AInputCommand_Inventory = 75,
	AInputCommand_Icons = 76,
	AInputCommand_ZLock = 78,
	AInputCommand_Quad = 79,
	AInputCommand_Horn = 80,
	AInputCommand_CUSTOM_COMMAND = 77,
};

class AInputMap : public Toshi::TObject
{
public:
	TDECLARE_CLASS(AInputMap, Toshi::TObject);

public:
	AInputMap(const Toshi::TPString8& a_rName) :
		m_Name(a_rName),
		m_oDoodadToCommandMap(AMemory::GetAllocator(AMemory::POOL_Misc))
	{
		
	}

	const Toshi::TPString8& GetName() const { return m_Name; }

	void LoadFromProperties(const PBProperties& a_rProperties);
	Toshi::T2Map<Toshi::TInputDevice::Doodad, AInputCommand>& GetCommandMap() { return m_oDoodadToCommandMap; }

private:
	Toshi::TPString8 m_Name;
	Toshi::T2Map<Toshi::TInputDevice::Doodad, AInputCommand> m_oDoodadToCommandMap;
};
