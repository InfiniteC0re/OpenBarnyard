#pragma once
#include <GUI/T2GUIElement.h>
#include <GUI/T2GUIRectangle.h>
#include <GUI/T2GUITextureSection.h>

//-----------------------------------------------------------------------------
// A row of up to 5 chicken wings (the Chicken Launch lives). A lost wing plays
// a short burning animation (Wings_Hot1..4 -> Wing_Burnt), a regained one pops
// in with a spinning flare.
// Plain T2GUIElement (no TClass), size 0x568.
// vtable 00739d78: +0x00 dtor 00639000, +0x04 Tick 006392d0.
// Name from the Wii build (AGUI2Feathers).
// Used by AChickenFireMinigameState.
//-----------------------------------------------------------------------------
class AGUI2Feathers
    : public Toshi::T2GUIElement
{
public:
	static constexpr TINT MAX_NUM_FEATHERS = 5;

public:
	AGUI2Feathers();
	~AGUI2Feathers();

	//-----------------------------------------------------------------------------
	// Toshi::T2GUIElement
	//-----------------------------------------------------------------------------
	virtual void Tick( TFLOAT a_fDeltaTime ) override;

	// Creates a_iNumFeathers wings (all burnt until SetNumFeathers is called)
	TBOOL Create( TINT a_iNumFeathers );

	// Unlinks the wings and frees the texture section list
	void Destroy();

	// Sets the number of intact wings, animating the lost and the regained ones
	void SetNumFeathers( TINT a_iNumFeathers );

private:
	Toshi::T2GUIRectangle        m_aWings[ MAX_NUM_FEATHERS ];          // 0x54
	TINT                         m_iNumWings;                           // 0x2ac
	TINT                         m_iNumFeathers;                        // 0x2b0
	TINT                         m_iNumTextures;                        // 0x2b4
	Toshi::T2GUITextureSection** m_ppTextures;                          // 0x2b8
	Toshi::T2GUIRectangle        m_aFlares[ MAX_NUM_FEATHERS ];         // 0x2bc
	TFLOAT                       m_aTimers[ MAX_NUM_FEATHERS ];         // 0x514
	TFLOAT                       m_aFlareScales[ MAX_NUM_FEATHERS ];    // 0x528
	TFLOAT                       m_aFlareRotations[ MAX_NUM_FEATHERS ]; // 0x53c
	TINT                         m_aTextureIndices[ MAX_NUM_FEATHERS ]; // 0x550
	TUINT8                       m_uiBurningMask;                       // 0x564
};

TSTATICASSERT( sizeof( AGUI2Feathers ) == 0x568 );
