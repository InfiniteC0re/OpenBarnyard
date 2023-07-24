#pragma once
#include "Toshi2/T2RedBlackTree.h"
#include "Toshi2/T2Map.h"
#include "Toshi2/T2Vector.h"

class AInputMap
{
public:
	typedef uint32_t INPUTCONTEXT;
	enum INPUTCONTEXT_ : INPUTCONTEXT
	{
		INPUTCONTEXT_UNK,
		INPUTCONTEXT_UNK2,
		INPUTCONTEXT_UNK3,
		INPUTCONTEXT_UNK4,
		INPUTCONTEXT_UNK5,
		INPUTCONTEXT_UNK6,
		INPUTCONTEXT_UNK7,
		INPUTCONTEXT_UNK8,
		INPUTCONTEXT_UNK9,
		INPUTCONTEXT_UNK10,
		INPUTCONTEXT_UNK11,
		INPUTCONTEXT_UNK12,
		INPUTCONTEXT_UNK13,
	};

	typedef uint32_t ActionButton;
	enum ActionButton_ : ActionButton {
		ACTIONBUTTON_0,
		ACTIONBUTTON_1,
		ACTIONBUTTON_2,
		ACTIONBUTTON_3,
		ACTIONBUTTON_4,
		ACTIONBUTTON_5,
		ACTIONBUTTON_6,
		ACTIONBUTTON_7,
		ACTIONBUTTON_8,
		ACTIONBUTTON_9,
		ACTIONBUTTON_10,
		ACTIONBUTTON_11,
		ACTIONBUTTON_12,
		ACTIONBUTTON_13,
		ACTIONBUTTON_14,
		ACTIONBUTTON_15,
		ACTIONBUTTON_16,
		ACTIONBUTTON_17,
		ACTIONBUTTON_18,
		ACTIONBUTTON_19,
		ACTIONBUTTON_20,
		ACTIONBUTTON_21,
		ACTIONBUTTON_22,
		ACTIONBUTTON_23,
		ACTIONBUTTON_24,
		ACTIONBUTTON_25,
		ACTIONBUTTON_26,
		ACTIONBUTTON_27,
		ACTIONBUTTON_28,
		ACTIONBUTTON_29,
		ACTIONBUTTON_30,
		ACTIONBUTTON_31,
		ACTIONBUTTON_32,
		ACTIONBUTTON_33,
		ACTIONBUTTON_34,
		ACTIONBUTTON_35,
		ACTIONBUTTON_36,
		ACTIONBUTTON_37,
		ACTIONBUTTON_38,
		ACTIONBUTTON_39,
		ACTIONBUTTON_40,
		ACTIONBUTTON_41,
		ACTIONBUTTON_42,
		ACTIONBUTTON_43,
		ACTIONBUTTON_44,
		ACTIONBUTTON_45,
		ACTIONBUTTON_46,
		ACTIONBUTTON_47
	};

	typedef uint32_t INPUTBUTTON;
	enum INPUTBUTTON_ : INPUTBUTTON
	{
		INPUTBUTTON_SPACE = 35,
	};

	// The first element of BindVector is count of keybinds
	using BindVector = Toshi::T2Vector<int, 4>;

	using ButtonMapComparator = Toshi::TComparator<ActionButton>;
	using NamePadMap = Toshi::T2Map<int, int>;
	using ButtonMap = Toshi::T2Map<ActionButton, BindVector, ButtonMapComparator>;

public:
	AInputMap()
	{
		InitialiseButtonMap();
		NamePadDoodads();
	}

	void InitialiseButtonMap();
	void NamePadDoodads();
	
	ButtonMap* GetButtonMap(INPUTCONTEXT m_eInputContext);

private:
	ButtonMap m_ActionButtonMap1;   // 0x4
	ButtonMap m_ActionButtonMap2;   // 0x20
	ButtonMap m_ActionButtonMap3;   // 0x3C
	ButtonMap m_ActionButtonMap4;   // 0x58
	ButtonMap m_ActionButtonMap5;   // 0x74
	ButtonMap m_ActionButtonMap6;   // 0x90
	ButtonMap m_ActionButtonMap7;   // 0xAC
	ButtonMap m_ActionButtonMap8;   // 0xC8
	ButtonMap m_ActionButtonMap9;   // 0xE4
	ButtonMap m_ActionButtonMap10;  // 0x100
	NamePadMap m_NamePadDoodadsMap; // 0x11C
};
