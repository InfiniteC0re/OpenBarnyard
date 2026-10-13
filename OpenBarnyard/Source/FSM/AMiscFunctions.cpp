#include "pch.h"
#include "AMiscFunctions.h"
#include "Helpers/APlayerProgress.h"
#include "MiniGame/AGopherShopInterface.h"
#include "Helpers/AHighScoreManager.h"
#include "HUD/AMenuNavHudElement.h"
#include "AMethodPointer.h"
#include "ARandom.h"
#include "Player/APlayerManager.h"
#include "Quests/AQuestManager.h"
#include "Player/APlayerSet.h"
#include "Units/APlayerControlComponent.h"
#include "Units/AUnit.h"
#include "Player/APlayerProfileManager.h"
#include "Helpers/AGameLoader.h"
#include "HUD/AHudManager.h"
#include "HUD/ASimPlayerPickUpHudElement.h"
#include "Units/AUnitType.h"
#include "UnitMovement/Steering/ASteer.h"
#include "Sound/AVOStream.h"
#include "Sound/ASoundManager.h"
#include "SoundBank/ui.h"
#include "ACheatActivator.h"
#include "GUI/AGUI2.h"
#include "GUI/AGUI2MessageBoxState.h"
#include "GameInterface/AGameStateController.h"
#include "Locale/ALocaleManager.h"

#include <Toshi/TSystem.h>
#include "FX/AParticleEffectLocatorManager.h"
#include "FX/AParticleEffectManager.h"
#include <Toshi/TScheduler.h>
#include <Plugins/PPropertyParser/PPropertyValue.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TPSTRING8_DECLARE( base_table );
TPSTRING8_DECLARE( table_bessydaisy );
TPSTRING8_DECLARE( table1 );
TPSTRING8_DECLARE( table2 );
TPSTRING8_DECLARE( table3 );
TPSTRING8_DECLARE( booth1 );
TPSTRING8_DECLARE( booth2 );
TPSTRING8_DECLARE( booth3 );
TPSTRING8_DECLARE( booth4 );
TPSTRING8_DECLARE( booth5 );
TPSTRING8_DECLARE( booth6 );
TPSTRING8_DECLARE( booth7 );
TPSTRING8_DECLARE( booth8 );
TPSTRING8_DECLARE( bn_cows_milk );
TPSTRING8_DECLARE( bn_honey );
TPSTRING8_DECLARE( bn_smoothie );
TPSTRING8_DECLARE( bn_draft_milk );
TPSTRING8_DECLARE( bn_goat_milk );
TPSTRING8_DECLARE( bn_stud_molasses );
TPSTRING8_DECLARE( waiter_start );

// NOTE: GetClass 00602ba0
// NOTE: GetClass 00602ba0, CreateObject 00602d00, CreateObjectInPlace 00602d70
TDEFINE_CLASS_INIT( AMiscFunctions );

// DAT_0079b020
AFunctionTable AMiscFunctions::ms_oFunctionTable( &TGetClass( AMiscFunctions ), TNULL );

// DAT_0079b080, the HUD of the state ShowChampionChart hides
AGameState::HUDParams AMiscFunctions::ms_oSavedHUDParams;

// FSM fallback: the natives are called on the singleton
// $Barnyard: FUNCTION 00602bb0
static TObject* __stdcall AMiscFunctionsFunctionFallback( AFunctionTable* a_pTable, const AFunctionCaller* a_pCaller )
{
	return AMiscFunctions::GetSingletonSafe();
}

// $Barnyard: FUNCTION 00605360
void AMiscFunctions::InitialiseClass()
{
	ms_oFunctionTable.Register( reinterpret_cast<void*>( &AMiscFunctionsFunctionFallback ) );

	ms_oFunctionTable.AddFunction( AMethodPointer1Args<AMiscFunctions, TPString8>::Create( "DebugPrint", &AMiscFunctions::DebugPrint ) );
	ms_oFunctionTable.AddFunction( AMethodPointer2Args<AMiscFunctions, TBOOL, TPString8>::Create( "DebugAssert", &AMiscFunctions::DebugAssert ) );
	ms_oFunctionTable.AddFunction( AMethodPointer1Args<AMiscFunctions, TPString8>::Create( "DebugAssertFail", &AMiscFunctions::DebugPrint ) );
	ms_oFunctionTable.AddFunction( AMethodPointer2Args<AMiscFunctions, AUnit*, TPString8>::Create( "AddComment", &AMiscFunctions::AddComment ) );
	ms_oFunctionTable.AddFunction( AMethodPointer1ArgsRT<TBOOL, AMiscFunctions, TBOOL>::Create( "Not", &AMiscFunctions::Not ) );
	ms_oFunctionTable.AddFunction( AMethodPointer2ArgsRT<TBOOL, AMiscFunctions, TBOOL, TBOOL>::Create( "Or", &AMiscFunctions::Or ) );
	ms_oFunctionTable.AddFunction( AMethodPointer2ArgsRT<TBOOL, AMiscFunctions, TBOOL, TBOOL>::Create( "And", &AMiscFunctions::And ) );
	ms_oFunctionTable.AddFunction( AMethodPointer2ArgsRT<TBOOL, AMiscFunctions, TBOOL, TBOOL>::Create( "Xor", &AMiscFunctions::Xor ) );
	ms_oFunctionTable.AddFunction( AMethodPointer2ArgsRT<TFLOAT, AMiscFunctions, TFLOAT, TFLOAT>::Create( "Subtract", &AMiscFunctions::Subtract ) );
	ms_oFunctionTable.AddFunction( AMethodPointer2ArgsRT<TFLOAT, AMiscFunctions, TFLOAT, TFLOAT>::Create( "Add", &AMiscFunctions::Add ) );
	ms_oFunctionTable.AddFunction( AMethodPointer2ArgsRT<TFLOAT, AMiscFunctions, TFLOAT, TFLOAT>::Create( "Multiply", &AMiscFunctions::Multiply ) );
	ms_oFunctionTable.AddFunction( AMethodPointer2Args<AMiscFunctions, TPString8, const PPropertyValue&>::Create( "DebugPrintValue", &AMiscFunctions::DebugPrintValue ) );
	ms_oFunctionTable.AddFunction( AMethodPointer2ArgsRT<TFLOAT, AMiscFunctions, TFLOAT, TFLOAT>::Create( "GetRandom", &AMiscFunctions::GetRandom ) );
	ms_oFunctionTable.AddFunction( AMethodPointer2ArgsRT<TINT, AMiscFunctions, TINT, TINT>::Create( "GetRandomInt", &AMiscFunctions::GetRandomInt ) );
	ms_oFunctionTable.AddFunction( AMethodPointer0ArgsRT<TFLOAT, AMiscFunctions>::Create( "GetCurTimeDelta", &AMiscFunctions::GetCurTimeDelta ) );
	ms_oFunctionTable.AddFunction( AMethodPointer2Args<AMiscFunctions, TINT, TINT>::Create( "GameMessage", &AMiscFunctions::GameMessage ) );
	ms_oFunctionTable.AddFunction( AMethodPointer0ArgsRT<TPString8, AMiscFunctions>::Create( "GetRandomWaiterGoal", &AMiscFunctions::GetRandomWaiterGoal ) );
	ms_oFunctionTable.AddFunction( AMethodPointer0ArgsRT<TPString8, AMiscFunctions>::Create( "GetRandomBartenderGoal", &AMiscFunctions::GetRandomBartenderGoal ) );
	ms_oFunctionTable.AddFunction( AMethodPointer1ArgsRT<TBOOL, AMiscFunctions, TINT>::Create( "PlayerHasBought", &AMiscFunctions::PlayerHasBought ) );
	ms_oFunctionTable.AddFunction( AMethodPointer1Args<AMiscFunctions, TBOOL>::Create( "PlayerCanGetOnBike", &AMiscFunctions::PlayerCanGetOnBike ) );
	ms_oFunctionTable.AddFunction( AMethodPointer1Args<AMiscFunctions, TBOOL>::Create( "EnableSuperSquirtParticles", &AMiscFunctions::EnableSuperSquirtParticles ) );
	ms_oFunctionTable.AddFunction( AMethodPointer1Args<AMiscFunctions, TBOOL>::Create( "EnablePlayerSquirt", &AMiscFunctions::EnablePlayerSquirt ) );
	ms_oFunctionTable.AddFunction( AMethodPointer2Args<AMiscFunctions, AUnit*, TPString8>::Create( "PlayRandomVO", &AMiscFunctions::PlayRandomVO ) );
	ms_oFunctionTable.AddFunction( AMethodPointer0ArgsRT<TBOOL, AMiscFunctions>::Create( "IsVOPlaying", &AMiscFunctions::IsVOPlaying ) );
	ms_oFunctionTable.AddFunction( AMethodPointer2ArgsRT<TBOOL, AMiscFunctions, AUnit*, TFLOAT>::Create( "PlayerNearUnit", &AMiscFunctions::PlayerNearUnit ) );
	ms_oFunctionTable.AddFunction( AMethodPointer0Args<AMiscFunctions>::Create( "ShowChampionChart", &AMiscFunctions::ShowChampionChart ) );
	ms_oFunctionTable.AddFunction( AMethodPointer0Args<AMiscFunctions>::Create( "HideChampionChart", &AMiscFunctions::HideChampionChart ) );
	ms_oFunctionTable.AddFunction( AMethodPointer0Args<AMiscFunctions>::Create( "SaveFinishedBarnDetail", &AMiscFunctions::SaveFinishedBarnDetail ) );
	ms_oFunctionTable.AddFunction( AMethodPointer0Args<AMiscFunctions>::Create( "DeinitBarnDetailQuest", &AMiscFunctions::DeinitBarnDetailQuest ) );
	ms_oFunctionTable.AddFunction( AMethodPointer1Args<AMiscFunctions, TPString8>::Create( "EnableInstructionScreen", &AMiscFunctions::EnableInstructionScreen ) );
	ms_oFunctionTable.AddFunction( AMethodPointer0ArgsRT<TINT, AMiscFunctions>::Create( "GetCurAbsQuestTier", &AMiscFunctions::GetCurAbsQuestTier ) );
	ms_oFunctionTable.AddFunction( AMethodPointer0Args<AMiscFunctions>::Create( "ClearSquirtedAnimals", &AMiscFunctions::ClearSquirtedAnimals ) );
}

// $Barnyard: FUNCTION 00602bc0
void AMiscFunctions::DeinitialiseClass()
{
	ms_oFunctionTable.Unregister();
}

// $Barnyard: FUNCTION 00602bd0
AMiscFunctions::AMiscFunctions()
{
}

// $Barnyard: FUNCTION 00602c00
// $Barnyard: FUNCTION 00602dc0
AMiscFunctions::~AMiscFunctions()
{
}

// $Barnyard: FUNCTION 00604820
void AMiscFunctions::DebugPrint( TPString8 a_strText )
{
	// NOTE: compiled out in retail, printed here to follow the FSMs
	TINFO( "FSM: %s\n", a_strText.GetString() );
}

// $Barnyard: FUNCTION 00604ea0
void AMiscFunctions::DebugAssert( TBOOL a_bCondition, TPString8 a_strText )
{
	// NOTE: compiled out in retail
	TASSERT( a_bCondition, a_strText.GetString() );
}

// $Barnyard: FUNCTION 00604840
void AMiscFunctions::AddComment( AUnit* a_pUnit, TPString8 a_strText )
{
	// Compiled out in retail
}

// $Barnyard: FUNCTION 00602c20
TBOOL AMiscFunctions::Not( TBOOL a_bValue )
{
	return !a_bValue;
}

// $Barnyard: FUNCTION 00602c30
TBOOL AMiscFunctions::Or( TBOOL a_bValue1, TBOOL a_bValue2 )
{
	return a_bValue1 || a_bValue2;
}

// $Barnyard: FUNCTION 00602c50
TBOOL AMiscFunctions::And( TBOOL a_bValue1, TBOOL a_bValue2 )
{
	return a_bValue1 && a_bValue2;
}

// $Barnyard: FUNCTION 00602c70
TBOOL AMiscFunctions::Xor( TBOOL a_bValue1, TBOOL a_bValue2 )
{
	return a_bValue1 != a_bValue2 && ( a_bValue1 || a_bValue2 );
}

// $Barnyard: FUNCTION 00602ca0
TFLOAT AMiscFunctions::Subtract( TFLOAT a_fValue1, TFLOAT a_fValue2 )
{
	return a_fValue1 - a_fValue2;
}

// $Barnyard: FUNCTION 00602cb0
TFLOAT AMiscFunctions::Add( TFLOAT a_fValue1, TFLOAT a_fValue2 )
{
	return a_fValue1 + a_fValue2;
}

// $Barnyard: FUNCTION 00602cc0
TFLOAT AMiscFunctions::Multiply( TFLOAT a_fValue1, TFLOAT a_fValue2 )
{
	return a_fValue1 * a_fValue2;
}

// $Barnyard: FUNCTION 006047d0
void AMiscFunctions::DebugPrintValue( TPString8 a_strText, const PPropertyValue& a_rcValue )
{
	// Compiled out in retail
}

// $Barnyard: FUNCTION 00602de0
TFLOAT AMiscFunctions::GetRandom( TFLOAT a_fMin, TFLOAT a_fMax )
{
	return ARandom::GetSingleton()->m_oRandom.GetFloatMinMax( a_fMin, a_fMax );
}

// $Barnyard: FUNCTION 00602e40
TINT AMiscFunctions::GetRandomInt( TINT a_iMin, TINT a_iMax )
{
	return a_iMin + ARandom::GetSingleton()->m_oRandom.GetInt( a_iMax - a_iMin );
}

// $Barnyard: FUNCTION 00602cd0
TFLOAT AMiscFunctions::GetCurTimeDelta()
{
	return g_oSystemManager.GetScheduler()->GetCurrentDeltaTime();
}

// $Barnyard: FUNCTION 00602e90
void AMiscFunctions::GameMessage( TINT a_iTitle, TINT a_iText )
{
	T2GUIElement* pRootElement = AGUI2::GetRootElement();
	TFLOAT        fWidth       = pRootElement->GetWidth() * 0.4f;
	TFLOAT        fHeight      = pRootElement->GetHeight() * 0.5f;

	AGUI2MessageBoxState* pMessageBox = new AGUI2MessageBoxState();
	pMessageBox->Create( fWidth, fHeight, "rekord26", "rekord26", "Border_Top_Left", "Border_Horizontal", "Border_Top_Right", "Border_Vertical", "Border_Middle", "Border_Vertical", "Border_Bottom_Left", "Border_Horizontal", "Border_Bottom_Right", TFALSE );
	pMessageBox->GetTextBox()->SetScale( 0.8f );
	pMessageBox->SetText( ALocaleManager::Interface()->GetString( a_iTitle ) );
	pMessageBox->AddButton( ALocaleManager::Interface()->GetString( a_iText ) );
	pMessageBox->SetPadding( 60.0f, 10.0f );
	pMessageBox->SetAutoSize( TTRUE );
	pMessageBox->SetTextColour( 0xffffefb4 );
	pMessageBox->SetButtonScales( 0.8f, 0.8f );
	pMessageBox->SetDialogColour( 0xffffffff );
	pMessageBox->SetButtonColours( 0xffffffff, 0xff74482b );
	pMessageBox->SetTextShadowEnabled( TTRUE );
	pMessageBox->SetTextShadowColour( 0xa0000000 );
	pMessageBox->SetTextShadowAlpha( 0.5f );
	pMessageBox->SetTextShadowOffset( 5.0f, 5.0f );
	pMessageBox->SetPadding( 64.0f, 64.0f );

	ASoundManager::GetSingleton()->PlayCue( soundbank::UI_CONFIRM );
	AGameStateController::GetSingleton()->PushState( pMessageBox );
}

// $Barnyard: FUNCTION 00604860
TPString8 AMiscFunctions::GetRandomWaiterGoal()
{
	// 0x00778ae0
	static const TPString8* s_aTables[] = {
		&TPS8( base_table ),
		&TPS8( table_bessydaisy ),
	};

	// 0x00778ae8, unlocked by the items 955, 956 and 957 of the gopher shop
	static const TPString8* s_aShopTables[] = {
		&TPS8( table1 ),
		&TPS8( table2 ),
		&TPS8( table3 ),
	};

	// 0x00778af4, the first four are unlocked by the item 966 of the gopher shop, the others by 965
	static const TPString8* s_aBooths[] = {
		&TPS8( booth8 ),
		&TPS8( booth7 ),
		&TPS8( booth6 ),
		&TPS8( booth5 ),
		&TPS8( booth1 ),
		&TPS8( booth2 ),
		&TPS8( booth3 ),
		&TPS8( booth4 ),
	};

	TINT  iNumBooths2 = AGopherShopInterface::GetNumBought( 965 );
	TINT  iNumBooths1 = AGopherShopInterface::GetNumBought( 966 );
	TUINT uiTables    = AGopherShopInterface::GetNumBought( 955 ) | 2 * ( AGopherShopInterface::GetNumBought( 956 ) | 2 * AGopherShopInterface::GetNumBought( 957 ) );

	TRandom& rRandom = ARandom::GetSingleton()->m_oRandom;

	while ( TTRUE )
	{
		TFLOAT fRandom;

		while ( TTRUE )
		{
			fRandom = TFLOAT( rRandom.RandRaw() >> 1 ) * ( 3.0f / 2147483648.0f );

			if ( fRandom <= 2.0f )
				break;

			// A booth
			if ( TFLOAT( rRandom.RandRaw() >> 1 ) * ( 2.0f / 2147483648.0f ) > 1.0f && iNumBooths1 > 0 )
				return *s_aBooths[ rRandom.GetInt( 4 ) ];

			if ( iNumBooths2 > 0 )
				return *s_aBooths[ rRandom.GetInt( 4 ) + 4 ];
		}

		if ( fRandom <= 1.0f )
			break;

		if ( TFLOAT( rRandom.RandRaw() >> 1 ) * ( 2.0f / 2147483648.0f ) <= 1.0f )
		{
			// A table bought from the shop
			if ( uiTables > 0 )
			{
				TINT iTable = rRandom.GetInt( 3 );

#ifdef BARNYARD_COMMUNITY_PATCH
				// NOTE: retail tests ( 1 << iTable ) != 0 which is always true, any table can be picked once one is bought
				if ( uiTables & ( 1 << iTable ) )
#else
				if ( ( 1 << iTable ) != 0 )
#endif
					return *s_aShopTables[ iTable ];
			}
		}
		else
		{
			// Bessy and Daisy's table is only there from the 17th tier
			TINT iTable = rRandom.GetInt( 2 );

			if ( iTable != 1 || AQuestManager::GetSingleton()->GetCurAbsQuestTier() >= 17 )
				return *s_aTables[ iTable ];
		}
	}

	return TPS8( waiter_start );
}

// $Barnyard: FUNCTION 00604ac0
TPString8 AMiscFunctions::GetRandomBartenderGoal()
{
	// 0x007367b8, the drinks of the gopher shop
	static const TINT s_aDrinkItems[] = { 924, 932, 931, 927, 925, 926 };

	// 0x00778b14
	static const TPString8* s_aDrinks[] = {
		&TPS8( bn_cows_milk ),
		&TPS8( bn_honey ),
		&TPS8( bn_smoothie ),
		&TPS8( bn_draft_milk ),
		&TPS8( bn_goat_milk ),
		&TPS8( bn_stud_molasses ),
	};

	// The cow's milk (the first one) is always there
	TINT iNumDrinks = 1;

	for ( TUINT i = 1; i < 6; i++ )
		iNumDrinks += AGopherShopInterface::GetNumBought( s_aDrinkItems[ i ] );

	if ( iNumDrinks == 1 )
		return *s_aDrinks[ 0 ];

	TINT iDrink;

	do
		iDrink = ARandom::GetSingleton()->m_oRandom.GetInt( 6 );
	while ( AGopherShopInterface::GetNumBought( s_aDrinkItems[ iDrink ] ) <= 0 );

	return *s_aDrinks[ iDrink ];
}

// $Barnyard: FUNCTION 00602ce0
TBOOL AMiscFunctions::PlayerHasBought( TINT a_iItem )
{
	return AGopherShopInterface::GetNumBought( a_iItem ) > 0;
}

// $Barnyard: FUNCTION 00603060
void AMiscFunctions::PlayerCanGetOnBike( TBOOL a_bCan )
{
	ACheatActivator::SetRequirementSatisfied( ACheatActivator::GetSingleton()->GetRequirement( ACheatActivator::REQUIREMENT_SECONDARY_HAS_BIKE_KEY ), a_bCan );
}

// NOTE: retail binds the static function (00603090) directly
void AMiscFunctions::EnableSuperSquirtParticles( TBOOL a_bEnable )
{
	SetSuperSquirtParticlesEnabled( a_bEnable );
}

// $Barnyard: FUNCTION 00603090
void AMiscFunctions::SetSuperSquirtParticlesEnabled( TBOOL a_bEnable )
{
	AParticleEffectLocatorManager* pLocatorManager = AParticleEffectLocatorManager::GetSingleton();
	TINT                           iEffect         = AParticleEffectManager::FindEffect( "supersquirt" );

	for ( TINT i = 0; i < pLocatorManager->GetNumLocators(); i++ )
	{
		AParticleEffectLocator* pLocator = pLocatorManager->GetLocator( i );

		if ( pLocator->iEffect == iEffect )
		{
			if ( a_bEnable )
				pLocator->uiFlags |= AParticleEffectLocator::FLAGS_ENABLED;
			else
				pLocator->uiFlags &= ~AParticleEffectLocator::FLAGS_ENABLED;
		}
	}
}

// $Barnyard: FUNCTION 006030e0
void AMiscFunctions::EnablePlayerSquirt( TBOOL a_bEnable )
{
	AUnit*                   pPlayer        = APlayerManager::GetSingleton()->GetPlayerSet( 0 )->GetHumanPlayerUnit( 0 );
	APlayerControlComponent* pPlayerControl = TSTATICCAST( APlayerControlComponent, pPlayer->FindComponent( &TGetClass( APlayerControlComponent ) ) );

	if ( pPlayerControl )
	{
		// Restored once the current quest sequence is over
		AQuestManager::SetSavedLauncherInput( a_bEnable );

		pPlayerControl->SetLauncherInput( a_bEnable );
		pPlayerControl->SetUnk32( a_bEnable );
	}
}

// $Barnyard: FUNCTION 00604b50
void AMiscFunctions::PlayRandomVO( AUnit* a_pUnit, TPString8 a_strName )
{
	AVOStream::GetSingleton()->PlaySet( a_pUnit->GetType()->GetName().GetString(), a_strName.GetString(), *a_pUnit->GetSteer()->GetPosition() );
}

// $Barnyard: FUNCTION 00603120
TBOOL AMiscFunctions::IsVOPlaying()
{
	return AVOStream::GetSingleton()->IsPlayRequested() || AVOStream::GetSingleton()->IsPlaying();
}

// $Barnyard: FUNCTION 00603150
TBOOL AMiscFunctions::PlayerNearUnit( AUnit* a_pUnit, TFLOAT a_fDistanceSq )
{
	ASteer* pPlayerSteer = APlayerManager::GetSingleton()->GetPlayerUnit( 0 )->GetSteer();

	const TVector4* pUnitPosition   = a_pUnit->GetSteer()->GetPosition();
	const TVector4* pPlayerPosition = pPlayerSteer->GetPosition();

	return TVector4::DistanceSq( *pPlayerPosition, *pUnitPosition ) < a_fDistanceSq;
}

// $Barnyard: FUNCTION 006031a0
void AMiscFunctions::ShowChampionChart()
{
	AHighScoreManager::GetSingleton()->ShowChampionChart();

	AMenuNavHudElement* pMenuNav = TSTATICCAST( AMenuNavHudElement, AHudManager::GetSingleton()->FindElement( &TGetClass( AMenuNavHudElement ) ) );

	// Hides the HUD of the current state while the chart is shown
	AGameState::HUDParams& rHUDParams = AGameStateController::GetSingleton()->GetCurrentState()->GetHUDParams();
	ms_oSavedHUDParams                = rHUDParams;
	rHUDParams.SetFlags( AHudElement::MASK_MENU_NAV );

	pMenuNav->SetTexts( -1, -1, 1776 );
	AHudManager::GetSingleton()->OnUpdate( 0.0f );
}

// $Barnyard: FUNCTION 00603210
void AMiscFunctions::HideChampionChart()
{
	AHighScoreManager::GetSingleton()->HideChart();

	AMenuNavHudElement* pMenuNav = TSTATICCAST( AMenuNavHudElement, AHudManager::GetSingleton()->FindElement( &TGetClass( AMenuNavHudElement ) ) );

	AGameStateController::GetSingleton()->GetCurrentState()->GetHUDParams().SetFlags( ms_oSavedHUDParams.m_uiFlags );

	pMenuNav->SetDefaultTexts();
	AHudManager::GetSingleton()->OnUpdate( 0.0f );
}

// $Barnyard: FUNCTION 00603260
void AMiscFunctions::SaveFinishedBarnDetail()
{
	APlayerProgress::GetSingleton()->SetCurrentSlotBarnDetail();
}

// $Barnyard: FUNCTION 00603430
void AMiscFunctions::DeinitBarnDetailQuest()
{
	// The detail costs 500 pick ups unless the player has bought it, the counter doesn't show it
	ASimPlayerPickUpHudElement* pPickUpHud = AHudManager::GetSingleton()->FindElement<ASimPlayerPickUpHudElement>();

	if ( pPickUpHud )
		pPickUpHud->m_bUnk1 = TFALSE;

	APlayerSimProfile* pProfile = APlayerProfileManager::GetSingleton()->GetMainProfile();

	// NOTE: the original calls AGopherShopInterface::HasBought (the shop purchases), which is the same as PlayerHasBought
	if ( pProfile->GetDisplayedPickups() >= 500 && !PlayerHasBought( 923 ) )
		pProfile->AddPickups( -500 );

	if ( pPickUpHud )
		pPickUpHud->m_bUnk1 = TTRUE;
}

// $Barnyard: FUNCTION 00604bf0
void AMiscFunctions::EnableInstructionScreen( TPString8 a_strName )
{
	// NOTE: despite the name, it unlocks the sim loading screen picture called a_strName
	AGameLoader::UnlockSimLoadingPicture( a_strName );
}

// $Barnyard: FUNCTION 00602e60
TINT AMiscFunctions::GetCurAbsQuestTier()
{
	if ( AQuestManager::IsSingletonCreated() )
		return AQuestManager::GetSingleton()->GetCurAbsQuestTier();

	return -1;
}

// $Barnyard: FUNCTION 00603490
void AMiscFunctions::ClearSquirtedAnimals()
{
	APlayerSimProfile* pProfile = APlayerProfileManager::GetSingleton()->GetMainProfile();

	if ( pProfile )
		pProfile->ClearSquirtedUnits();
}

// Shows a message box with a single button (a bigger variant of GameMessage)
// $Barnyard: FUNCTION 00603270
void AMiscFunctions::ShowMessageBox( TINT a_iTitle, TINT a_iText )
{
	T2GUIElement* pRootElement = AGUI2::GetRootElement();
	TFLOAT        fWidth       = pRootElement->GetWidth() * 0.5f;
	TFLOAT        fHeight      = pRootElement->GetHeight() * 0.6f;

	AGUI2MessageBoxState* pMessageBox = new AGUI2MessageBoxState();
	pMessageBox->Create( fWidth, fHeight, "rekord26", "rekord26", "Border_Top_Left", "Border_Horizontal", "Border_Top_Right", "Border_Vertical", "Border_Middle", "Border_Vertical", "Border_Bottom_Left", "Border_Horizontal", "Border_Bottom_Right", TTRUE );
	pMessageBox->GetTextBox()->SetScale( 0.7f );
	pMessageBox->SetText( ALocaleManager::Interface()->GetString( a_iTitle ) );
	pMessageBox->AddButton( ALocaleManager::Interface()->GetString( a_iText ) );
	pMessageBox->SetAutoSize( TTRUE );
	pMessageBox->SetSize( 500.0f, 400.0f );
	pMessageBox->SetTextColour( 0xffffefb4 );
	pMessageBox->SetButtonScales( 0.7f, 0.7f );
	pMessageBox->SetDialogColour( 0xffffffff );
	pMessageBox->SetButtonColours( 0xffffffff, 0xff74482b );
	pMessageBox->SetTextShadowEnabled( TTRUE );
	pMessageBox->SetTextShadowColour( 0xa0000000 );
	pMessageBox->SetTextShadowAlpha( 0.5f );
	pMessageBox->SetTextShadowOffset( 5.0f, 5.0f );
	pMessageBox->SetPadding( 64.0f, 64.0f );

	ASoundManager::GetSingleton()->PlayCue( soundbank::UI_CONFIRM );
	AGameStateController::GetSingleton()->PushState( pMessageBox );
}
