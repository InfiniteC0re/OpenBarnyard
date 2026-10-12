#pragma once
#ifdef BARNYARD_LATEST

#  include "AGUI2XUIElement.h"
#  include "AGUI2XUITimeline.h"

//-----------------------------------------------------------------------------
// RTTI: AGUI2XUICanvas : AGUI2XUIElement
//-----------------------------------------------------------------------------
class AGUI2XUICanvas
    : public AGUI2XUIElement
{
public:
	TDECLARE_CLASS( AGUI2XUICanvas, AGUI2XUIElement );

public:
	AGUI2XUICanvas();
	~AGUI2XUICanvas();

	//-----------------------------------------------------------------------------
	// Toshi::T2GUIElement
	//-----------------------------------------------------------------------------
	virtual void Tick( TFLOAT a_fDeltaTime ) override;

	//-----------------------------------------------------------------------------
	// AGUI2XUIElement
	//-----------------------------------------------------------------------------
	virtual TBOOL Create( AGUI2XUIResource& a_rResource, XURXUIElementData* a_pData, TBOOL a_bCreateChildren ) override;
	virtual void  PlayAnimation( const TWCHAR* a_wszStartFrame, const TWCHAR* a_wszEndFrame ) override;
	virtual void  StopAnimation() override;

	//-----------------------------------------------------------------------------
	// Own virtuals
	//-----------------------------------------------------------------------------
	virtual void AddXUIChild( AGUI2XUIElement* a_pElement ); // +0x8C

private:
	AGUI2XUITimelinePlayer  m_oTimelinePlayer;  // 0x98
	AGUI2XUITimelineControl m_oTimelineControl; // 0xC4
};

#endif // BARNYARD_LATEST
