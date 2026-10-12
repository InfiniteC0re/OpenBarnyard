#pragma once
#include "ASimPersistData.h"
#include "FSM/AFunctionTable.h"
#include "GameInterface/AMessagePopupState.h"

#include <Toshi/TObject.h>
#include <Toshi/TEvent.h>

class AUnit;
class AInteriorArea;
class ACollisionObjectMesh;

//-----------------------------------------------------------------------------
// The ladder beans of the sim: 8 milkweed patches ("mweedNN" special areas of the
// default network). The player plants a milkweed_seed in a patch (area interaction
// "milkweed", which calls the FSM native EnableMilkWeedForCurrentArea), then every
// squirt of milk grows it a stage, up to a ladder the player can climb. Growing
// ladders unlocks rewards (APlayerProgress::UnlockLadderBeansFlags). Their state
// lives in ASimPersistData. Created by ASimState::OnInsertion (ASimState::m_pLadderBeansGame).
//
// TClass 0x00783b58 (parent TObject), vtable 0x00723ae0, size 0x60.
//-----------------------------------------------------------------------------
class ALadderBeansGame
    : public Toshi::TObject
{
public:
	TDECLARE_CLASS( ALadderBeansGame, Toshi::TObject );

	using PopupListener = Toshi::TListener<AMessagePopupState, AMessagePopupState::ResponseEvent, ALadderBeansGame>;

public:
	ALadderBeansGame();
	~ALadderBeansGame();

	virtual AFunctionTable* GetFunctionTable(); // +0x0c

	// Spawns the milkweed units in their patches and sets the patches up from the saved state
	TBOOL Create();

	// Destroys the milkweed units and their collision objects
	void Destroy();

	// Moves the growing milkweeds towards the height of their stage. Returns TTRUE once nothing grows anymore
	TBOOL Update( TFLOAT a_fDeltaTime );

	// Fades the milkweeds out with the distance to the camera
	void Render();

	// Whether a milkweed is planted in the patch of ladder a_uiIndex
	TBOOL IsPlanted( TUINT a_uiIndex ) const;

	// Returns the index of the ladder whose milkweed is a_pUnit, -1 if it's not a milkweed of the game
	TINT FindMilkweed( AUnit* a_pUnit ) const;

	// Plants a milkweed in the patch of ladder a_uiIndex and takes a milkweed_seed from the player
	void PlantMilkweed( TUINT a_uiIndex );

	// FSM: plants a milkweed in the patch the player stands in
	void EnableMilkweedForCurrentArea();

	// The milk has hit the milkweed of ladder a_uiIndex: it grows a stage (unless it's still growing).
	// Growing the last stage of a ladder can unlock a reward, shown in a popup.
	void OnMilkweedSquirted( TUINT a_uiIndex );

	AUnit* GetMilkweed( TUINT a_uiIndex ) const { return m_apMilkweeds[ a_uiIndex ]; }

	// Index of the ladder whose patch is the area a_szAreaName ("mweedNN"), -1 if none
	static TINT FindLadder( const TCHAR* a_szAreaName );

	// The NN of the area name "mweedNN" of ladder a_uiIndex, -1 if a_uiIndex is out of range
	static TINT GetLadderId( TUINT a_uiIndex );

	// Position of the fully grown milkweed of the patch a_pArea (the player faces it while planting)
	static TBOOL GetMilkweedPosition( AInteriorArea* a_pArea, Toshi::TVector4& a_rOutPosition );

public:
	static AFunctionTable ms_oFunctionTable;

private:
	// Places the milkweed of the patch a_pArea at its current height
	void UpdateMilkweed( AInteriorArea* a_pArea );

	// A patch stays a special area (the planting spot) until its ladder is fully grown, then it's walkable (climbable)
	static void SetPatchGrown( const TCHAR* a_szAreaName, TBOOL a_bGrown );

	static TBOOL __stdcall OnRewardPopupResponse( ALadderBeansGame* a_pGame, AMessagePopupState* a_pPopup, AMessagePopupState::ResponseEvent* a_pEvent );

private:
	TBOOL                            m_bChanged;                                                // 0x04, some milkweed is growing
	ASimPersistData::LadderBeansSet* m_pLadders;                                                // 0x08, ASimPersistData::GetLadderBeans
	AUnit*                           m_apMilkweeds[ ASimPersistData::NUM_LADDER_BEANS ];        // 0x0c
	ACollisionObjectMesh*            m_apCollisionObjects[ ASimPersistData::NUM_LADDER_BEANS ]; // 0x2c
	PopupListener                    m_oPopupListener;                                          // 0x4c
};

TSTATICASSERT( sizeof( ALadderBeansGame ) == 0x60 );
