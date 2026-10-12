#pragma once
#include <GUI/T2GUIElement.h>
#include <GUI/T2GUIRectangle.h>

//-----------------------------------------------------------------------------
// A spinning, pulsing and flickering flare sprite that fades in and out.
// Plain T2GUIElement (no TClass), size 0x104.
// vtable 00739dc8: +0x00 dtor 006397e0, +0x04 Tick 00639b80, +0x0c Render 00639720.
// Name from the Wii build (AGUI2Flare).
// Used by AGUI2MessagePopupFlare.
//-----------------------------------------------------------------------------
class AGUI2Flare
    : public Toshi::T2GUIElement
{
public:
	AGUI2Flare();
	~AGUI2Flare();

	//-----------------------------------------------------------------------------
	// Toshi::T2GUIElement
	//-----------------------------------------------------------------------------
	virtual void Tick( TFLOAT a_fDeltaTime ) override;
	virtual void Render() override;

	// Sets up the flare with the a_szTexture section (hidden until Start is called).
	// a_fRotation and a_fAlphaPhase are in half turns.
	void Create( const TCHAR* a_szTexture, TFLOAT a_fRotation, TFLOAT a_fRotationSpeed, TFLOAT a_fAlphaPhase, TFLOAT a_fAlphaSpeed, TFLOAT a_fScaleAmount, TFLOAT a_fScaleSpeed, TFLOAT a_fScale );

	// Sets the sprite (TNULL hides it)
	void SetTexture( const TCHAR* a_szTexture );

	// Fades the flare in (a_bFadeIn) or out at a_fFadeSpeed per second (0.0 is instant).
	// a_fDuration <= 0.0 makes the flare stay.
	void Start( TBOOL a_bFadeIn, TFLOAT a_fFadeSpeed, TFLOAT a_fDuration );

private:
	void Update( TFLOAT a_fDeltaTime );

public:
	Toshi::T2GUIRectangle m_oFlare;         // 0x54
	TUINT8                PADDING1[ 8 ];    // 0xcc
	TFLOAT                m_fRotation;      // 0xd4
	TFLOAT                m_fRotationSpeed; // 0xd8
	TFLOAT                m_fScalePhase;    // 0xdc
	TUINT8                PADDING2[ 4 ];    // 0xe0
	TFLOAT                m_fAlphaPhase;    // 0xe4
	TFLOAT                m_fAlphaSpeed;    // 0xe8
	TFLOAT                m_fFade;          // 0xec, 0.0 - 1.0
	TFLOAT                m_fDuration;      // 0xf0
	TFLOAT                m_fTime;          // 0xf4, counts down from 0.0
	TFLOAT                m_fScaleSpeed;    // 0xf8
	TFLOAT                m_fScaleAmount;   // 0xfc
	TFLOAT                m_fFadeSpeed;     // 0x100, negative while fading out
};

TSTATICASSERT( sizeof( AGUI2Flare ) == 0x104 );
