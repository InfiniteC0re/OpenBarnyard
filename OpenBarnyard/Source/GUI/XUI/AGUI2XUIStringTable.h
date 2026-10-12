#pragma once
#ifdef BARNYARD_LATEST

#  include <File/TTRB.h>

//-----------------------------------------------------------------------------
// Localised strings of a XUI scene (Data/XUI/StringTables/<lang>/<name>_xus.trb).
// NOTE: there's no RTTI for this class, the name follows TXUIStringTable of newer TOSHI.
//-----------------------------------------------------------------------------
class AGUI2XUIStringTable
{
public:
	struct Entry
	{
		TWCHAR* wszString; // 0x0
		TWCHAR* wszId;     // 0x4 path of the element the string belongs to
	};

public:
	AGUI2XUIStringTable();
	~AGUI2XUIStringTable();

	TBOOL         Load( const TCHAR* a_szFileName, TBOOL a_bIsTRB, Toshi::TTRB* a_pTRB );
	const TWCHAR* Lookup( const TWCHAR* a_wszId );

private:
	TBOOL ReadHeader( TUINT8* a_pData );
	TBOOL ReadStrings( TUINT8* a_pData );

private:
	TUINT32 m_uiMagic;      // 0x00
	TUINT8  m_ucUnk1;       // 0x04
	TUINT8  m_ucUnk2;       // 0x05
	TUINT32 m_uiSize;       // 0x08
	TUINT16 m_usNumStrings; // 0x0C
	Entry*  m_pEntries;     // 0x10
};

#endif // BARNYARD_LATEST
