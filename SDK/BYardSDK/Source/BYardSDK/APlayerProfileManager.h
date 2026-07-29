#pragma once
#include "THookedSingleton.h"

class APlayerProfileManager
    : public THookedSingleton<APlayerProfileManager, 0x007b493c>
{
public:
	TINT GetMainGender() const { return m_iGender; }
	TINT GetMainCostume() const { return m_iSkin; }

	void ResetMainCharacter() { m_iGender = 0; }

private:
	TUINT8 m_aUnk1[ 0x10 ];  // 0x00
	TINT   m_iGender;        // 0x10 (0 male, 1 female)
	TUINT8 m_aUnk2[ 0x34 ];  // 0x14
	TINT   m_iSkin;          // 0x48
};
