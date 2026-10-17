#pragma once

#if !defined( TOSHI_FINAL ) && defined( TRENDERINTERFACE_DX8 )
#  include <Toshi/TSingleton.h>
#  include <Render/TDebugText.h>
#  include <Input/TInputInterface.h>

class ACamera;
class AGameState;
class AAnimatableObject;

class ADebugOverlay
    : public Toshi::TSingleton<ADebugOverlay>
{
public:
	static constexpr TUINT32 ANIMATION_COLOUR  = 0xffff6000;
	static constexpr TUINT32 STEER_COLOUR      = 0xff00ffff;
	static constexpr TUINT32 INFO_COLOUR       = 0xffffffff;
	static constexpr TUINT32 UNIT_COLOUR       = 0xffffffff;
	static constexpr TINT    LINE_HEIGHT       = 11;    // same as the log of TDebugD3DText
	static constexpr TFLOAT  MAX_UNIT_DISTANCE = 40.0f; // units further from the camera aren't labelled
	static constexpr TFLOAT  UNIT_LABEL_HEIGHT = 2.0f;  // height of the labels above the unit's position

public:
	ADebugOverlay();
	~ADebugOverlay();

	// Draws the overlay, call it in the scene, after the GUI is rendered
	void Render( TFLOAT a_fDeltaTime );

	// Toggles the overlay when the DebugMenu command of a_pGameState's input map goes down
	void ProcessInput( AGameState* a_pGameState, const Toshi::TInputInterface::InputEvent* a_pEvent );

	void  SetEnabled( TBOOL a_bEnabled );
	TBOOL IsEnabled() const { return m_bEnabled; }

private:
	// Adds the lines about the player (above the time) to the log
	void RenderPlayer( TFLOAT a_fDeltaTime );

	// Adds a line per animation of a_pObject to the log
	void RenderAnimations( AAnimatableObject* a_pObject );

	// Adds the time of the game to the log
	void RenderGameTime();

	// Adds the labels of the units close to the camera
	void RenderUnitLabels( ACamera* a_pCamera );

private:
	Toshi::TDebugText* m_pDebugText;
	TFLOAT             m_fGroundTime; // time the player has been on the ground
	TBOOL              m_bEnabled;
};

#endif // !TOSHI_FINAL && TRENDERINTERFACE_DX8
