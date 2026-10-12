#pragma once
#ifdef BARNYARD_LATEST

#  include "AGUI2XUIElement.h"
#  include "AGUI2XUITimeline.h"

//-----------------------------------------------------------------------------
// RTTI: AGUI2XUIVisual : AGUI2XUIElement
//-----------------------------------------------------------------------------
class AGUI2XUIVisual
    : public AGUI2XUIElement
{
public:
	TDECLARE_CLASS( AGUI2XUIVisual, AGUI2XUIElement );

public:
	AGUI2XUIVisual();
	~AGUI2XUIVisual();

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
	virtual void  OnPress( TBOOL a_bEnded ) override;

	// Sets the text of the text presenters and the image of the image presenters among the children
	void SetPresenterData( const TWCHAR* a_wszText, const TWCHAR* a_wszImagePath );

private:
	AGUI2XUITimelineControl m_oTimelineControl; // 0x98
	AGUI2XUITimelinePlayer  m_oTimelinePlayer;  // 0xB4
};

#endif // BARNYARD_LATEST
