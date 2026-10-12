#pragma once
#ifdef BARNYARD_LATEST

#  include "AGUI2XUIElement.h"
#  include "AGUI2XUITimeline.h"

//-----------------------------------------------------------------------------
// RTTI: AGUI2XUIGroup : AGUI2XUIElement
//-----------------------------------------------------------------------------
class AGUI2XUIGroup
    : public AGUI2XUIElement
{
public:
	TDECLARE_CLASS( AGUI2XUIGroup, AGUI2XUIElement );

public:
	AGUI2XUIGroup();
	~AGUI2XUIGroup();

	//-----------------------------------------------------------------------------
	// Toshi::T2GUIElement
	//-----------------------------------------------------------------------------
	virtual void Tick( TFLOAT a_fDeltaTime ) override;

	//-----------------------------------------------------------------------------
	// AGUI2XUIElement
	//-----------------------------------------------------------------------------
	virtual TBOOL Create( AGUI2XUIResource& a_rResource, XURXUIElementData* a_pData, TBOOL a_bCreateChildren ) override;
	virtual void  PlayAnimation( const TWCHAR* a_wszStartFrame, const TWCHAR* a_wszEndFrame ) override;
	virtual void  PlayTimelineRange( TFLOAT a_fStart, TFLOAT a_fEnd ) override;
	virtual void  StopAnimation() override;

private:
	AGUI2XUITimelineControl m_oTimelineControl; // 0x98
	AGUI2XUITimelinePlayer  m_oTimelinePlayer;  // 0xB4
};

#endif // BARNYARD_LATEST
