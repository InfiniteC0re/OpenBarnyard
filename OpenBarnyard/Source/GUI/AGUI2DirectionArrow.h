#pragma once
#include "AGUI2ModelRenderer.h"

#include <GUI/T2GUIElement.h>
#include <Math/TVector4.h>

//-----------------------------------------------------------------------------
// Purpose: A 3D arrow (the car_arrow_green model) rendered on the HUD that points
// in a world direction relative to the current camera, used by Joy Ride to show
// where the track goes.
// vtable 0x00739c98. NOTE: the name is invented.
//-----------------------------------------------------------------------------
class AGUI2DirectionArrow
    : public Toshi::T2GUIElement
{
public:
	AGUI2DirectionArrow();
	~AGUI2DirectionArrow();

	//-----------------------------------------------------------------------------
	// Toshi::T2GUIElement
	//-----------------------------------------------------------------------------
	virtual void Tick( TFLOAT a_fDeltaTime ) OVERRIDE; // +0x04

	//-----------------------------------------------------------------------------
	// Own methods
	//-----------------------------------------------------------------------------

	// Shows or hides the arrow and its model renderer
	virtual void SetArrowVisible( TBOOL a_bVisible ); // +0x48

	// $Barnyard: FUNCTION 00638830
	void ShowArrow() { SetArrowVisible( TTRUE ); }

	// $Barnyard: FUNCTION 00638840
	void HideArrow() { SetArrowVisible( TFALSE ); }

	// Creates the model renderer (a_fWidth x a_fHeight) and the arrow model
	void Create( TFLOAT a_fWidth, TFLOAT a_fHeight );

	// Points the arrow from a_rcFrom to a_rcTo (on the XZ plane, relative to the camera direction)
	void PointAt( const Toshi::TVector4& a_rcFrom, const Toshi::TVector4& a_rcTo );

	AGUI2ModelRenderer& GetModelRenderer() { return m_oModelRenderer; }

private:
	AGUI2ModelRenderer m_oModelRenderer; // 0x54
	Toshi::TVector4    m_vDirection;     // 0xd4, direction of the arrow model
};

TSTATICASSERT( sizeof( AGUI2DirectionArrow ) == 0xe4 );
