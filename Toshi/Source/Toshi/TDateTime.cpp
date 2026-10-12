#include "ToshiPCH.h"
#include "TDateTime.h"

#include <time.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

// $Barnyard: FUNCTION 006bba20
void TDateTime::Pack( TUINT32& a_rOutDate, TUINT32& a_rOutTime ) const
{
	a_rOutDate = uiDay | ( ( uiMonth | ( TUINT32( uiYear ) << 8 ) ) << 8 );
	a_rOutTime = ( uiSecond | ( ( ( TUINT32( uiHour ) << 8 ) | uiMinute ) << 8 ) ) << 8;
}

// $Barnyard: FUNCTION 006bba60
void TDateTime::UnPack( const TUINT32& a_rcDate, const TUINT32& a_rcTime )
{
	uiYear   = TUINT16( a_rcDate >> 16 );
	uiMonth  = TUINT8( a_rcDate >> 8 );
	uiDay    = TUINT8( a_rcDate );
	uiHour   = TUINT8( a_rcTime >> 24 );
	uiMinute = TUINT8( a_rcTime >> 16 );
	uiSecond = TUINT8( a_rcTime >> 8 );
}

// $Barnyard: FUNCTION 006bbbb0
TBOOL TDateTime::GetLocalTime( TDateTime& a_rOutDateTime )
{
	__time32_t iTime;
	_time32( &iTime );

	tm* pTime = _localtime32( &iTime );

	a_rOutDateTime.uiHour   = TUINT8( pTime->tm_hour );
	a_rOutDateTime.uiMinute = TUINT8( pTime->tm_min );
	a_rOutDateTime.uiSecond = TUINT8( pTime->tm_sec );
	a_rOutDateTime.uiDay    = TUINT8( pTime->tm_mday );
	a_rOutDateTime.uiMonth  = TUINT8( pTime->tm_mon );
	a_rOutDateTime.uiYear   = TUINT16( pTime->tm_year + 1900 );

	return TTRUE;
}

TOSHI_NAMESPACE_END
