#pragma once

TOSHI_NAMESPACE_START

// Calendar date and time of day (7 bytes)
struct TDateTime
{
	TUINT16 uiYear;   // 0x00
	TUINT8  uiMonth;  // 0x02, 0-11
	TUINT8  uiHour;   // 0x03
	TUINT8  uiMinute; // 0x04
	TUINT8  uiSecond; // 0x05
	TUINT8  uiDay;    // 0x06, 1-31

	// Packs the date into day | month << 8 | year << 16 and the time into second << 8 | minute << 16 | hour << 24
	void Pack( TUINT32& a_rOutDate, TUINT32& a_rOutTime ) const;

	// Unpacks values made by Pack
	void UnPack( const TUINT32& a_rcDate, const TUINT32& a_rcTime );

	// Fills a_rOutDateTime with the current local time
	static TBOOL GetLocalTime( TDateTime& a_rOutDateTime );
};

TOSHI_NAMESPACE_END
