#pragma once
#include "AFunctionTable.h"
#include "GameInterface/AGameState.h"

#include <Toshi/TClass.h>
#include <Toshi/TSingleton.h>
#include <Toshi/TPString8.h>

class AUnit;
class PPropertyValue;

//-----------------------------------------------------------------------------
// Natives of the FSM scripts that don't belong to an object ("amiscfunctions::").
// Created by AFSMManager (vtable 0x007367d0).
//-----------------------------------------------------------------------------
class AMiscFunctions
    : public Toshi::TObject
    , public Toshi::TSingleton<AMiscFunctions>
{
public:
	TDECLARE_CLASS( AMiscFunctions, Toshi::TObject );

	// Script natives of the class, for the FSM
	// $Barnyard: FUNCTION 00602bf0
	virtual AFunctionTable* GetFunctionTable() { return &ms_oFunctionTable; } // +0x0c

public:
	AMiscFunctions();
	~AMiscFunctions();

	// FSM natives
	void             DebugPrint( Toshi::TPString8 a_strText );
	void             DebugAssert( TBOOL a_bCondition, Toshi::TPString8 a_strText );
	void             AddComment( AUnit* a_pUnit, Toshi::TPString8 a_strText );
	TBOOL            Not( TBOOL a_bValue );
	TBOOL            Or( TBOOL a_bValue1, TBOOL a_bValue2 );
	TBOOL            And( TBOOL a_bValue1, TBOOL a_bValue2 );
	TBOOL            Xor( TBOOL a_bValue1, TBOOL a_bValue2 );
	TFLOAT           Subtract( TFLOAT a_fValue1, TFLOAT a_fValue2 );
	TFLOAT           Add( TFLOAT a_fValue1, TFLOAT a_fValue2 );
	TFLOAT           Multiply( TFLOAT a_fValue1, TFLOAT a_fValue2 );
	void             DebugPrintValue( Toshi::TPString8 a_strText, const PPropertyValue& a_rcValue );
	TFLOAT           GetRandom( TFLOAT a_fMin, TFLOAT a_fMax );
	TINT             GetRandomInt( TINT a_iMin, TINT a_iMax );
	TFLOAT           GetCurTimeDelta();
	void             GameMessage( TINT a_iTitle, TINT a_iText );
	Toshi::TPString8 GetRandomWaiterGoal();
	Toshi::TPString8 GetRandomBartenderGoal();
	TBOOL            PlayerHasBought( TINT a_iItem );
	void             PlayerCanGetOnBike( TBOOL a_bCan );
	void             EnableSuperSquirtParticles( TBOOL a_bEnable );

	// Enables or disables the "supersquirt" particle effect locators
	static void SetSuperSquirtParticlesEnabled( TBOOL a_bEnable );
	static void ShowMessageBox( TINT a_iTitle, TINT a_iText );

	void  EnablePlayerSquirt( TBOOL a_bEnable );
	void  PlayRandomVO( AUnit* a_pUnit, Toshi::TPString8 a_strName );
	TBOOL IsVOPlaying();
	TBOOL PlayerNearUnit( AUnit* a_pUnit, TFLOAT a_fDistanceSq );
	void  ShowChampionChart();
	void  HideChampionChart();
	void  SaveFinishedBarnDetail();
	void  DeinitBarnDetailQuest();
	void  EnableInstructionScreen( Toshi::TPString8 a_strName );
	TINT  GetCurAbsQuestTier();
	void  ClearSquirtedAnimals();

public:
	static AFunctionTable ms_oFunctionTable;

private:
	static AGameState::HUDParams ms_oSavedHUDParams;
};
