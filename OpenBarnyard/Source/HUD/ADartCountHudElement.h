#pragma once
#include "AHudElement.h"

#include <GUI/T2GUIRectangle.h>
#include <GUI/T2GUITextureSection.h>

//-----------------------------------------------------------------------------
// Purpose: Column of the darts a player of the Darts minigame has left
// (ten "Dart_On"/"Dart_Off" icons).
// TClass 00781fa0, vtable 0071bf90, size 0x564.
//-----------------------------------------------------------------------------
class ADartCountHudElement
    : public AHudElement
{
public:
	TDECLARE_CLASS( ADartCountHudElement, AHudElement );

	static constexpr TINT MAX_DARTS = 10;

public:
	ADartCountHudElement();
	~ADartCountHudElement();

	//-----------------------------------------------------------------------------
	// AHudElement
	//-----------------------------------------------------------------------------
	virtual TBOOL OnAttach() OVERRIDE;
	virtual void  Update( TFLOAT a_fDeltaTime ) OVERRIDE;

	void SetOnTexture( const TCHAR* a_szSection );
	void SetOffTexture( const TCHAR* a_szSection );

	// Sets the anchor and the pivot of the column and puts it at the left or right edge
	void SetAttachment( Toshi::T2GUIATTACHMENT a_eAnchor, Toshi::T2GUIATTACHMENT a_ePivot, TFLOAT a_fY );

	void SetColour( TFLOAT a_fRed, TFLOAT a_fGreen, TFLOAT a_fBlue );
	void SetAlpha( TFLOAT a_fAlpha );

	// Fades the alpha to a_fAlpha in a_fDuration seconds
	void FadeTo( TFLOAT a_fAlpha, TFLOAT a_fDuration );

	// Mirrors the icons
	void FlipU();

	// Scales the column to a_fPercent percent of the height of the parent
	void SetScale( TFLOAT a_fPercent );

	// Shows a_iCount darts as on, the rest as off
	void SetCount( TINT a_iCount );

	void SetParent( Toshi::T2GUIElement* a_pParent ) { m_pParent = a_pParent; }

private:
	Toshi::T2GUIRectangle       m_oColumn;             // 0x014
	Toshi::T2GUIRectangle       m_aDarts[ MAX_DARTS ]; // 0x08c
	Toshi::T2GUIElement*        m_pParent;             // 0x53c
	Toshi::T2GUITextureSection* m_pOnSection;          // 0x540
	Toshi::T2GUITextureSection* m_pOffSection;         // 0x544
	TBOOL                       m_bFlipped;            // 0x548
	TINT                        m_bFading;             // 0x54c
	TFLOAT                      m_fAlpha;              // 0x550
	TFLOAT                      m_fFadeFrom;           // 0x554
	TFLOAT                      m_fFadeTo;             // 0x558
	TFLOAT                      m_fFadeTime;           // 0x55c
	TFLOAT                      m_fFadeDuration;       // 0x560
};

TSTATICASSERT( sizeof( ADartCountHudElement ) == 0x564 );
