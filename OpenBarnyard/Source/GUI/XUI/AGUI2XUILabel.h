#pragma once
#ifdef BARNYARD_LATEST

#  include "AGUI2XUIControl.h"

#  include <Toshi/TString8.h>

//-----------------------------------------------------------------------------
// RTTI: AGUI2XUILabel : AGUI2XUIControl
// NOTE: none of the recovered code uses the string
//-----------------------------------------------------------------------------
class AGUI2XUILabel
    : public AGUI2XUIControl
{
public:
	TDECLARE_CLASS( AGUI2XUILabel, AGUI2XUIControl );

public:
	AGUI2XUILabel();
	~AGUI2XUILabel();

private:
	Toshi::TString8 m_strUnk; // 0xC8
};

#endif // BARNYARD_LATEST
