#pragma once
#include "AHudElement.h"
#include "GUI/AGUI2TextBox.h"

#include <GUI/T2GUIRectangle.h>
#include <Math/TVector2.h>
#include <Math/TVector4.h>

//-----------------------------------------------------------------------------
// Purpose: Aiming HUD of the Darts minigame: the two aim bars over the board,
// the bullseye that marks the aimed point, the target lock that follows the
// dart and the score popup.
// TClass 00781fb8, vtable 0071c020, size 0x4e8.
//-----------------------------------------------------------------------------
class ADartFireHudElement
    : public AHudElement
{
public:
	TDECLARE_CLASS( ADartFireHudElement, AHudElement );

	// Renders the aim bars and the bullseye (which aren't its children) before its children
	// NOTE: retail inherits T2GUIElement at 0x14 (vtable 0071bfd8, deleting dtor thunk 00451d50), modelled as a member
	class Canvas
	    : public Toshi::T2GUIElement
	{
	public:
		virtual void Render() OVERRIDE;
	};

public:
	ADartFireHudElement();
	~ADartFireHudElement();

	//-----------------------------------------------------------------------------
	// AHudElement
	//-----------------------------------------------------------------------------
	virtual TBOOL OnAttach() OVERRIDE;
	virtual void  OnDetach() OVERRIDE;
	virtual void  Show() OVERRIDE;
	virtual void  Hide() OVERRIDE;
	virtual void  Update( TFLOAT a_fDeltaTime ) OVERRIDE {} // DOES_NOTHING

	void SetBarYVisible( TBOOL a_bVisible );
	void SetBarXVisible( TBOOL a_bVisible );
	void SetBarXLocked( TBOOL a_bLocked );
	void SetColour( TFLOAT a_fRed, TFLOAT a_fGreen, TFLOAT a_fBlue );
	void SetTargetLockVisible( TBOOL a_bVisible );
	void ResetColour();
	void SetTargetLockAlpha( TFLOAT a_fAlpha );
	void SetBullseyeVisible( TBOOL a_bVisible );

	// Moves the bullseye to the aim (-1..1)
	void SetBullseyePosition( TFLOAT a_fAimX, TFLOAT a_fAimY );

	void SetScoreAlpha( TFLOAT a_fAlpha );

	// Moves the horizontal bar to the vertical aim (-1..1)
	void SetBarY( TFLOAT a_fAimY );

	// Moves the vertical bar to the horizontal aim (-1..1)
	void SetBarX( TFLOAT a_fAimX );

	// Moves the target lock over a_rcPosition
	void SetTargetLockPosition( const Toshi::TVector4& a_rcPosition );

	// Shows "+score" and a_wszText at the aim
	void ShowScore( TFLOAT a_fAimX, TFLOAT a_fAimY, TINT a_iScore, const TWCHAR* a_wszText, TFLOAT a_fRed, TFLOAT a_fGreen, TFLOAT a_fBlue );

	// Covers the board (given by its corners) with the HUD
	void SetBoardCorners( const Toshi::TVector4& a_rcTop, const Toshi::TVector4& a_rcRight, const Toshi::TVector4& a_rcLeft, const Toshi::TVector4& a_rcBottom );

	void SetParent( Toshi::T2GUIElement* a_pParent ) { m_pParent = a_pParent; }

private:
	Canvas                m_oCanvas;        // 0x014
	Toshi::T2GUIRectangle m_oBoard;         // 0x068
	Toshi::T2GUIRectangle m_oBarY;          // 0x0e0
	Toshi::T2GUIRectangle m_oBarX;          // 0x158
	Toshi::T2GUIRectangle m_oBullseyeMask;  // 0x1d0
	Toshi::T2GUIRectangle m_oBullseye;      // 0x248
	Toshi::T2GUIRectangle m_oTargetLock;    // 0x2c0
	Toshi::T2GUIRectangle m_oScoreFrame;    // 0x338
	AGUI2TextBox          m_oScoreText;     // 0x3b0
	AGUI2TextBox          m_oUnusedText;    // 0x428
	TWCHAR                m_wszScore[ 8 ];  // 0x4a0, NOTE: formatted with a size of 16
	TWCHAR                m_wszText[ 20 ];  // 0x4b0
	Toshi::TVector2       m_vBoardPosition; // 0x4d8
	Toshi::T2GUIElement*  m_pParent;        // 0x4e0
	TFLOAT                m_fBarLength;     // 0x4e4
};

TSTATICASSERT( sizeof( ADartFireHudElement ) == 0x4e8 );
