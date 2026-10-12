#pragma once
#include <Math/TVector4.h>

class AModelInstance;
class AInteriorGoal;
class AInteriorNetwork;
class ATerrainLocatorList;

//-----------------------------------------------------------------------------
// Purpose: the spinning milk bottles ("data/models/milk_bottle_01") that fill
// up the milk of the player's launcher. They are placed at the
// "powerup_full_milk" locators or goals and come back a while after they were
// picked up. Used by the sim (ASimState + 0xff50) and by some mini games.
// NOTE: not a TClass in retail, the name is made up.
//-----------------------------------------------------------------------------
class AMilkBottlePickups
{
public:
	static constexpr TINT MAX_BOTTLES = 40;

	struct Bottle
	{
		Bottle( const Toshi::TVector4& a_rcPosition, AInteriorGoal* a_pGoal );

		// A collected bottle is hidden for a while
		void SetCollected( TBOOL a_bCollected );
		void Update( TFLOAT a_fDeltaTime );

		TBOOL           bCollected; // 0x00
		Toshi::TVector4 vPosition;  // 0x04
		TFLOAT          fOpacity;   // 0x14, fades in after the bottle comes back
		TFLOAT          fUnk18;     // 0x18
		AInteriorGoal*  pGoal;      // 0x1c, the position is taken from the goal once it's known
		TFLOAT          fTime;      // 0x20, rotation of the bottle or, while collected, seconds before it comes back
		TFLOAT          fGoalTime;  // 0x24
	};

public:
	AMilkBottlePickups();
	~AMilkBottlePickups();

	// Places a bottle at a_rcPosition. Returns TNULL if there's no room for it
	Bottle* AddBottle( const Toshi::TVector4& a_rcPosition, AInteriorGoal* a_pGoal );

	// Places the bottles at the "powerup_full_milk" goals of a_pNetwork and loads the model
	TINT LoadFromNetwork( AInteriorNetwork* a_pNetwork );

	// Places the bottles at the "powerup_full_milk" locators of a_pLocators and loads the model
	TINT LoadFromLocators( ATerrainLocatorList* a_pLocators );

	// Deletes the bottles
	void Clear();

	// Picks up the bottles the player walks into (if a_bCanPickUp is set)
	void Update( TFLOAT a_fDeltaTime, TBOOL a_bCanPickUp );

	void Render();

	void SetAllCollected( TBOOL a_bCollected );

	// Returns the closest bottle that isn't collected within a_rDistance, a_rDistance is set to its distance
	Bottle* FindClosestBottle( const Toshi::TVector4& a_rcPosition, TFLOAT& a_rDistance );

	void SetVisible( TBOOL a_bVisible ) { m_bVisible = a_bVisible; }

private:
	void SetupModel();

public:
	TINT            m_iNumBottles;              // 0x00
	Bottle*         m_apBottles[ MAX_BOTTLES ]; // 0x04
	AModelInstance* m_pModel;                   // 0xa4
	TBOOL           m_bVisible;                 // 0xa8
};

TSTATICASSERT( sizeof( AMilkBottlePickups::Bottle ) == 0x28 );
TSTATICASSERT( sizeof( AMilkBottlePickups ) == 0xac );
