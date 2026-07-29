#pragma once

class ASaveSlot
{
public:
	TBOOL         IsOccupied() const { return m_uiOccupied == 1; }
	const TWCHAR* GetName() const { return TREINTERPRETCAST( const TWCHAR*, m_wcsName ); }

	TINT GetGender() const { return *TREINTERPRETCAST( const TINT*, m_aGender ); }
	TINT GetSkinIndex() const { return *TREINTERPRETCAST( const TINT*, m_aSkin ); }

	TUINT8  GetDay() const { return m_uiDay; }
	TUINT8  GetMonth() const { return m_uiMonth; } // 0-based
	TUINT16 GetYear() const { return TUINT16( m_aYear[ 0 ] | ( m_aYear[ 1 ] << 8 ) ); }
	TUINT8  GetHour() const { return m_uiHour; } // 24-hour
	TUINT8  GetMinute() const { return m_uiMinute; }

private:
	TUINT8 m_uiOccupied;      // 0x00
	TUINT8 m_aUnk1[ 0x6818 ]; // 0x01
	TUINT8 m_aGender[ 4 ];    // 0x6819
	TUINT8 m_aSkin[ 4 ];      // 0x681d
	TUINT8 m_wcsName[ 64 ];   // 0x6821
	TUINT8 m_aUnk2[ 0x6907 ]; // 0x6861
	TUINT8 m_uiDay;           // 0xd168
	TUINT8 m_uiMonth;         // 0xd169
	TUINT8 m_aYear[ 2 ];      // 0xd16a
	TUINT8 m_aUnk3[ 2 ];      // 0xd16c (0xd16d = seconds)
	TUINT8 m_uiMinute;        // 0xd16e
	TUINT8 m_uiHour;          // 0xd16f
};
