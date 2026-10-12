#pragma once
#ifdef BARNYARD_LATEST

#  include "AGUI2XUIControl.h"
#  include "AGUI2XUITimeline.h"

class AGUI2XUIVisual;

//-----------------------------------------------------------------------------
// RTTI: AGUI2XUIScene : AGUI2XUIControl
//-----------------------------------------------------------------------------
class AGUI2XUIScene
    : public AGUI2XUIControl
{
public:
	TDECLARE_CLASS( AGUI2XUIScene, AGUI2XUIControl );

public:
	AGUI2XUIScene();
	~AGUI2XUIScene();

	//-----------------------------------------------------------------------------
	// Toshi::T2GUIElement
	//-----------------------------------------------------------------------------
	virtual void Tick( TFLOAT a_fDeltaTime ) override;
	virtual void Render() override;
	virtual void SetFocus( TBOOL a_bFocused ) override;

	//-----------------------------------------------------------------------------
	// AGUI2XUIElement
	//-----------------------------------------------------------------------------
	virtual TBOOL Create( AGUI2XUIResource& a_rResource, XURXUIElementData* a_pData, TBOOL a_bCreateChildren ) override;
	virtual TBOOL ProcessInput( const Toshi::TInputInterface::InputEvent* a_pEvent ) override;
	virtual void  PlayAnimation( const TWCHAR* a_wszStartFrame, const TWCHAR* a_wszEndFrame ) override;
	virtual void  PlayTimelineRange( TFLOAT a_fStart, TFLOAT a_fEnd ) override;
	virtual void  StopAnimation() override;

	void   StopAndResetAnimation();
	TFLOAT GetNamedFrameTime( const TWCHAR* a_wszName );
	void   SetTimelineTime( TFLOAT a_fTime );

	AGUI2XUIControl* GetFocusedControl() const { return m_pFocusedControl; }

private:
	AGUI2XUITimelineControl m_oTimelineControl; // 0xC8
	TUINT32                 m_uiUnk2;           // 0xE4 never used
	AGUI2XUIElement*        m_pDefaultFocus;    // 0xE8
	const TWCHAR*           m_wszClassOverride; // 0xEC
	AGUI2XUIVisual*         m_pTransFrom;       // 0xF0
	AGUI2XUIVisual*         m_pTransTo;         // 0xF4
	AGUI2XUIVisual*         m_pTransBackFrom;   // 0xF8
	AGUI2XUIVisual*         m_pTransBackTo;     // 0xFC
	TUINT32                 m_uiUnk1;           // 0x100
	TBOOL                   m_bIgnorePresses;   // 0x104
	AGUI2XUIControl*        m_pFocusedControl;  // 0x108
	AGUI2XUITimelinePlayer  m_oTimelinePlayer;  // 0x10C
};

#endif // BARNYARD_LATEST
