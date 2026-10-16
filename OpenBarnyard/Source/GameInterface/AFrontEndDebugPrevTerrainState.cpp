#include "pch.h"
#include "AFrontEndDebugPrevTerrainState.h"
#include "GUI/AGUI2.h"
#include "Helpers/AInstanceManager.h"

#include <GUI/T2GUIFontManager.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

#ifdef BARNYARD_COMMUNITY_PATCH
TDEFINE_CLASS( AFrontEndDebugPrevTerrainState );
#else
// NOTE: CreateObject returns TNULL in all builds (the constructor is stripped)
TDEFINE_CLASS_NORUNTIME( AFrontEndDebugPrevTerrainState );
#endif

TPSTRING8_DECLARE( DialogInputMap );

// NOTE: constructor stripped from all builds, reconstructed from the PS2 virtuals.
// The destructor frees both arrays and OnActivate fills them, so they're allocated here.
AFrontEndDebugPrevTerrainState::AFrontEndDebugPrevTerrainState()
    : m_pButtons( new AGUI2Button[ NUM_TERRAINS ] )
    , m_pTerrainNames( new const TWCHAR*[ NUM_TERRAINS ] )
    , m_iUnk150( 0 )
    , m_fVisibilityLOD0Distance( 0.0f )
    , m_fVisibilityBlendEnd( 0.0f )
    , m_fVisibilityBlendStart( 0.0f )
{
}

// $BarnyardPS2: FUNCTION 0010c968
AFrontEndDebugPrevTerrainState::~AFrontEndDebugPrevTerrainState()
{
	if ( m_pButtons )
		delete[] m_pButtons;

	if ( m_pTerrainNames )
		delete[] m_pTerrainNames;
}

// $BarnyardPS2: FUNCTION 0010d0f8
TBOOL AFrontEndDebugPrevTerrainState::ProcessCommand( AInputCommand a_eInputCommand, const TInputInterface::InputEvent* a_pInputEvent, TBOOL& a_rStopEvents )
{
	if ( a_eInputCommand == AInputCommand_Quit || a_eInputCommand == AInputCommand_Cancel )
	{
		Remove();
		return TTRUE;
	}

	if ( a_eInputCommand != AInputCommand_Start && m_oMenu.ProcessInputCommand( a_eInputCommand, a_pInputEvent ) )
	{
		// NOTE: the original also builds a string temporary from the name of the focused terrain (debug output compiled out)

		// Scrolls the list to the focused button
		AGUI2Button* pFocused = TSTATICCAST( AGUI2Button, m_oMenu.GetFocusedMenuItem() );
		m_oMenu.SetTransform( 0.0f, TFLOAT( pFocused->GetId() ) * -23.0f - 50.0f );

		return TTRUE;
	}

	return AGameState::ProcessCommand( a_eInputCommand, a_pInputEvent, a_rStopEvents );
}

// $BarnyardPS2: FUNCTION 0010d0d8
TBOOL AFrontEndDebugPrevTerrainState::OnUpdate( TFLOAT a_fDeltaTime )
{
	return AGameState::OnUpdate( a_fDeltaTime );
}

// $BarnyardPS2: FUNCTION 0010ca28
void AFrontEndDebugPrevTerrainState::OnInsertion()
{
	SetInputMap( TPS8( DialogInputMap ) );

#ifdef BARNYARD_COMMUNITY_PATCH
	// The front end has no instance manager on PC
	if ( AInstanceManager::IsSingletonCreated() )
#endif
		AInstanceManager::GetSingleton()->GetVisibilityDistances( m_fVisibilityLOD0Distance, m_fVisibilityBlendEnd, m_fVisibilityBlendStart );

	AGameState::OnInsertion();
	m_HUDParams.SetFlags( AHudElement::MASK_MENU_NAV );
}

// $BarnyardPS2: FUNCTION 0010ca80
void AFrontEndDebugPrevTerrainState::OnRemoval()
{
	AGameState::OnRemoval();
}

// $BarnyardPS2: FUNCTION 0010caa0
void AFrontEndDebugPrevTerrainState::OnActivate()
{
	T2GUIFont* pFont = T2GUIFontManager::FindFont( "Rekord18" );

	m_pTerrainNames[ 0 ]  = L"Sim";
	m_pTerrainNames[ 1 ]  = L"GateCrash";
	m_pTerrainNames[ 2 ]  = L"Barn";
	m_pTerrainNames[ 3 ]  = L"Golf1";
	m_pTerrainNames[ 4 ]  = L"Golf2";
	m_pTerrainNames[ 5 ]  = L"Golf3";
	m_pTerrainNames[ 6 ]  = L"Golf4";
	m_pTerrainNames[ 7 ]  = L"Golf5";
	m_pTerrainNames[ 8 ]  = L"Golf6";
	m_pTerrainNames[ 9 ]  = L"Golf7";
	m_pTerrainNames[ 10 ] = L"Golf8";
	m_pTerrainNames[ 11 ] = L"Golf9";
	m_pTerrainNames[ 12 ] = L"Golf10";
	m_pTerrainNames[ 13 ] = L"Golf11";
	m_pTerrainNames[ 14 ] = L"Golf12";
	m_pTerrainNames[ 15 ] = L"Golf13";
	m_pTerrainNames[ 16 ] = L"Golf14";
	m_pTerrainNames[ 17 ] = L"Golf15";
	m_pTerrainNames[ 18 ] = L"Golf16";
	m_pTerrainNames[ 19 ] = L"Golf17";
	m_pTerrainNames[ 20 ] = L"Golf18";
	m_pTerrainNames[ 21 ] = L"PeckRange";
	m_pTerrainNames[ 22 ] = L"FrontEnd";
	m_pTerrainNames[ 23 ] = L"Statue";
	m_pTerrainNames[ 24 ] = L"BeadyStatues";
	m_pTerrainNames[ 25 ] = L"Woods";
	m_pTerrainNames[ 26 ] = L"Barn_Day";
	m_pTerrainNames[ 27 ] = L"GopherShop";
	m_pTerrainNames[ 28 ] = L"CowTipping";
	m_pTerrainNames[ 29 ] = L"VeggieDefender";
	m_pTerrainNames[ 30 ] = L"PoolRoom";
	m_pTerrainNames[ 31 ] = L"PoolBoat";
	m_pTerrainNames[ 32 ] = L"WhackARacc";
	m_pTerrainNames[ 33 ] = L"Tapping";
	m_pTerrainNames[ 34 ] = L"CarChase";
	m_pTerrainNames[ 35 ] = L"Cooking";
	m_pTerrainNames[ 36 ] = L"Cocktail";
	m_pTerrainNames[ 37 ] = L"ChasingChicks";
	m_pTerrainNames[ 38 ] = L"SquirterBensHill";
	m_pTerrainNames[ 39 ] = L"SquirterBeadyFarm";
	m_pTerrainNames[ 40 ] = L"ChickenCoopInterior";
	m_pTerrainNames[ 41 ] = L"FarmHouseInterior";
	m_pTerrainNames[ 42 ] = L"Darts";
	m_pTerrainNames[ 43 ] = L"SSDarts";
	m_pTerrainNames[ 44 ] = L"Test1";
	m_pTerrainNames[ 45 ] = L"SS01";
	m_pTerrainNames[ 46 ] = L"SS02";
	m_pTerrainNames[ 47 ] = L"SS03";
	m_pTerrainNames[ 48 ] = L"SS04";
	m_pTerrainNames[ 49 ] = L"SS05";
	m_pTerrainNames[ 50 ] = L"SS06";
	m_pTerrainNames[ 51 ] = L"SS07a";
	m_pTerrainNames[ 52 ] = L"SS07b";
	m_pTerrainNames[ 53 ] = L"SS08";
	m_pTerrainNames[ 54 ] = L"SS09";
	m_pTerrainNames[ 55 ] = L"SS10";
	m_pTerrainNames[ 56 ] = L"SS11";

	for ( TINT i = 0; i < NUM_TERRAINS; i++ )
	{
		AGUI2Button& rButton = m_pButtons[ i ];
		rButton.Create( pFont, 200.0f, m_pTerrainNames[ i ], 2.5f );

		TFLOAT fWidth, fHeight;
		rButton.GetDimensions( fWidth, fHeight );

		rButton.SetFocusedColour( 0xffa2ebfb );
		rButton.SetEnabledColour( 0xff556cd9 );
		rButton.SetFocusedScale( 1.0f );
		rButton.SetUseHighlight( TTRUE );
		rButton.SetHighlightImage( "grad2" );
		rButton.OnFocusLost();
		rButton.SetId( i );
		rButton.SetTransform( 0.0f, TFLOAT( i ) * 1.25f * fHeight );

		m_oMenu.AddMenuItem( rButton );
	}

	AGUI2::GetRootElement()->AddChildTail( m_oMenu );
	m_oMenu.SetFocusAt( m_pButtons[ 0 ] );
	m_oMenu.SetTransform( 0.0f, -50.0f );
	m_oMenu.SetFocus( TTRUE );
	m_oMenu.Show();
	m_oMenu.SetItemActivationCallback( MenuItemActivationCallback );
	m_oMenu.SetCallbackUserData( this );

	AGameState::OnActivate();
}

// $BarnyardPS2: FUNCTION 0010d058
void AFrontEndDebugPrevTerrainState::OnDeactivate()
{
	for ( TINT i = 0; i < NUM_TERRAINS; i++ )
		m_pButtons[ i ].RemoveSelf();

	m_oMenu.RemoveSelf();
	AGameState::OnDeactivate();
}

// $BarnyardPS2: FUNCTION 0010d200
void AFrontEndDebugPrevTerrainState::MenuItemActivationCallback( void* a_pUserData, AGUI2MenuItem* a_pItem )
{
	TSTATICCAST( AFrontEndDebugPrevTerrainState, a_pUserData )->OnTerrainChosen();
}

// $BarnyardPS2: FUNCTION 0010d220
void AFrontEndDebugPrevTerrainState::OnTerrainChosen()
{
	// NOTE: this is all the original does, nothing loads the chosen terrain
	m_oMenu.Hide();
}
