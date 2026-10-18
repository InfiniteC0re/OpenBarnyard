#pragma once

#if !defined( TOSHI_FINAL ) && defined( TRENDERINTERFACE_DX8 )
#  define ADEBUGOVERLAY // the overlay is compiled (development builds)
#  include <Toshi/TSingleton.h>
#  include <Render/TDebugText.h>
#  include <Math/TVector4.h>

class ACamera;

//-----------------------------------------------------------------------------
// Purpose: development text overlay (not in Final builds), toggled from the
// debug menu (ADebugMenuState) or started on with "-debugoverlay".
// The systems print their own lines every frame with ADEBUGLINE (the player's
// steer, the game time, the cameras, the current state...), the overlay sorts
// them by group, draws them at the top left with the frame time at the bottom
// and labels the units close to the camera.
//-----------------------------------------------------------------------------
class ADebugOverlay
    : public Toshi::TSingleton<ADebugOverlay>
{
public:
	// The groups of lines, from the top of the screen
	enum LINE : TUINT8
	{
		LINE_INPUT,         // the movement input of the player
		LINE_PLAYER,        // the player (ground time, milk)
		LINE_ANIMATION,     // the animations of the player
		LINE_STEER,         // the steer of the player
		LINE_FOLLOW_CAMERA, // the camera helpers
		LINE_GAME_TIME,     // AGameTimeManager
		LINE_STATE,         // the current game state
		LINE_CAMERA,        // the current camera
		LINE_NUMOF
	};

	static constexpr TUINT32 ANIMATION_COLOUR  = 0xffff6000;
	static constexpr TUINT32 STEER_COLOUR      = 0xff00ffff;
	static constexpr TUINT32 INFO_COLOUR       = 0xffffffff;
	static constexpr TUINT32 UNIT_COLOUR       = 0xffffffff;
	static constexpr TINT    LINE_HEIGHT       = 11;    // same as the log of TDebugD3DText
	static constexpr TINT    MAX_LINES         = 64;
	static constexpr TINT    MAX_LINE_LENGTH   = 192;
	static constexpr TFLOAT  MAX_UNIT_DISTANCE = 40.0f; // units further from the camera aren't labelled
	static constexpr TFLOAT  UNIT_LABEL_HEIGHT = 2.0f;  // height of the labels above the unit's position

public:
	ADebugOverlay();
	~ADebugOverlay();

	// Draws the overlay, call it in the scene, after the GUI is rendered
	void Render( TFLOAT a_fDeltaTime );

	void  SetEnabled( TBOOL a_bEnabled );
	TBOOL IsEnabled() const { return m_bEnabled; }

	// TRUE while the lines are shown (ADEBUGLINE checks it before formatting)
	static TBOOL IsPrinting() { return IsSingletonCreated() && GetSingleton()->m_bEnabled; }

	// Adds a line of the group a_eLine for the next frame (identical lines of a frame are added once)
	void PrintLine( LINE a_eLine, TUINT32 a_uiColour, const TCHAR* a_szFormat, ... );

	// Text at the screen position of a_rcPosition this frame, also drawn when the overlay is off (AWorldDebugDraw)
	void PrintWorldText( ACamera* a_pCamera, const Toshi::TVector4& a_rcPosition, const TCHAR* a_szText, TUINT32 a_uiColour );

private:
	Toshi::TDebugText* GetDebugText();

	// Adds the labels of the units close to the camera
	void RenderUnitLabels( ACamera* a_pCamera );

private:
	struct Line
	{
		LINE    eLine;
		TUINT32 uiColour;
		TCHAR   szText[ MAX_LINE_LENGTH ];
	};

	Toshi::TDebugText* m_pDebugText;
	TBOOL              m_bEnabled;
	TINT               m_iNumLines;              // lines printed since the last frame
	TINT               m_iNumShownLines;         // lines of the last frame that wasn't paused
	Line               m_aLines[ MAX_LINES ];
	Line               m_aShownLines[ MAX_LINES ];
};

// Prints a line of the group a_eLine on the debug overlay (only formatted while it's on)
#  define ADEBUGLINE( LINE, COLOUR, ... )                                                     \
	  do                                                                                       \
	  {                                                                                        \
		  if ( ADebugOverlay::IsPrinting() )                                                   \
			  ADebugOverlay::GetSingleton()->PrintLine( ADebugOverlay::LINE, COLOUR, __VA_ARGS__ ); \
	  } while ( 0 )

#else
#  define ADEBUGLINE( LINE, COLOUR, ... ) \
	  do                                  \
	  {                                   \
	  } while ( 0 )
#endif // !TOSHI_FINAL && TRENDERINTERFACE_DX8
