#pragma once
#include "AInteractionActivity.h"

//-----------------------------------------------------------------------------
// Switches the camera helper to "type": "nohelpercameras",
// "aconversationcamerahelper", "afollowcamerahelper" or "simcamera".
//-----------------------------------------------------------------------------
class ACameraActivity
    : public AInteractionActivity
{
public:
	TDECLARE_CLASS( ACameraActivity, AInteractionActivity );

public:
	ACameraActivity();
	~ACameraActivity();

	virtual TBOOL  Create( const PBProperties* a_pProperties ) OVERRIDE; // +0x0c
	virtual RESULT Update( AInteractionSequence* a_pSequence ) OVERRIDE; // +0x1c

private:
	Toshi::TPString8 m_strType; // 0x08, "type"
};

TSTATICASSERT( sizeof( ACameraActivity ) == 0xc );
