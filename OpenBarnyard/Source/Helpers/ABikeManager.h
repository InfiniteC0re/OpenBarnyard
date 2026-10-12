#pragma once
#include "FSM/AFunctionTable.h"
#include "Render/AGlowViewport.h"
#include "Units/AUnitProxy.h"

#include <Toshi/TTask.h>
#include <Toshi/TSingleton.h>
#include <Toshi/TPString8.h>
#include <Toshi/TRefCounted.h>
#include <Math/TVector4.h>

class AUnit;
class AInteriorGoal;
class AFade;
class ASimPlayerMainHudElement;

//-----------------------------------------------------------------------------
// Purpose: The motorbikes of the farm. Spawns the bikes ("bike" units) at the
// bike racks ("bikepos" goals of the interior network) and lets the player get
// on one (StartupPlayerBike): the player unit is then replaced by a unit of the
// cow riding the bike (m_pPlayerBikeProxy, "american_m_bike"/"american_f_bike")
// which gets the ABikeSteer, the cow is kept in m_pRiderProxy and the bike unit
// in m_pBikeProxy. OnUpdate runs the ride (see STATE).
// TClass 0x00783c74 (version 1.0, size 0x610), singleton at 0x00783c70,
// vtable 0x00723f10.
//-----------------------------------------------------------------------------
class ABikeManager
    : public Toshi::TTask
    , public Toshi::TSingleton<ABikeManager>
{
public:
	TDECLARE_CLASS( ABikeManager, Toshi::TTask );

	static constexpr TINT MAX_NUM_BIKES  = 56;
	static constexpr TINT MAX_NUM_RACKS  = 35;
	static constexpr TINT NUM_BIKE_TYPES = 5;

	// The bike models the player has, see GetBikeTypeFlag
	using BIKEFLAGS = TUINT32;

	// The ride
	enum STATE : TINT
	{
		STATE_GETON_RACK, // 0, getting on a bike at a rack (bk_gton_rack_*)
		STATE_WALKTOBIKE, // 1, the player walks to the bike
		STATE_GETON,      // 2, getting on (bk_gton_*)
		STATE_RIDING,     // 3
		STATE_GETOFF,     // 4, getting off (bk_gtoff_*)
		STATE_CRASH,      // 5, the rider falls off (AFallingCowSteer)
		STATE_FALLING,    // 6
		STATE_SUNK_FADE,  // 7, the bike sank, fading out to respawn the bike
		STATE_FELL_FADE,  // 8, the rider fell into the water, fading out to respawn
		STATE_NONE,       // 9
	};

	struct Bike
	{
		TINT   iType;   // 0x00, index of m_aBikeTypes
		TINT   iRack;   // 0x04, -1 if the bike isn't at a rack
		AUnit* pUnit;   // 0x08
		TBOOL  bParked; // 0x0c, the bike stands at a rack
	};

	struct Rack
	{
		AInteriorGoal* pGoal; // 0x00, "bikeposNN"
		AUnit*         pRack; // 0x04, the "bikerack" unit
		AUnit*         pBike; // 0x08
	};

public:
	ABikeManager();
	~ABikeManager();

	//-----------------------------------------------------------------------------
	// Toshi::TTask
	//-----------------------------------------------------------------------------
	virtual TBOOL OnCreate() OVERRIDE;
	virtual TBOOL OnUpdate( TFLOAT a_fDeltaTime ) OVERRIDE;

	//-----------------------------------------------------------------------------
	// Own methods
	//-----------------------------------------------------------------------------
	virtual AFunctionTable* GetFunctionTable(); // +0x34

	// FSM: makes the player get on the closest bike
	void StartupPlayerBike();

	// TRUE if a_pBike is parked
	TBOOL IsBikeParked( AUnit* a_pBike ) const;
	void  SetBikeParked( AUnit* a_pBike, TBOOL a_bParked );

	// The bike closest to a_rcPosition (on the XZ plane) and its type
	void FindClosestBike( const Toshi::TVector4& a_rcPosition, AUnit*& a_rpBike, TINT& a_riType ) const;

	void             SetBikeTypeUnlocked( BIKEFLAGS a_uiFlag, TBOOL a_bUnlocked );
	static BIKEFLAGS GetBikeTypeFlag( TINT a_iType );

	// The best bike type the player has
	TINT GetBestBikeType() const;

	void LoadBikeAssets();
	void DestroyBikeAssets();

	// Forgets the unlocked bike types and removes the glow of the bike
	void Disable();

	// Spawns the bike rack units at the "bikeposNN" goals of the default network
	void CreateRacks();

	// Spawns a_iNumBikes bikes at the racks closest to the player
	void SpawnBikesAtRacks( TINT a_iNumBikes );

	// Spawns m_iNumBikes bikes (bike races)
	void CreateBikes();

	// Destroys the bikes
	void DestroyBikes();

	// Puts the bikes that aren't parked or are far from the player back to the racks closest to the player
	void RespawnBikes( TFLOAT a_fDeltaTime, TBOOL a_bForce );

	// The rider asked to get off (ABikeControlComponent)
	void SetWantsToGetOff( TBOOL a_bGetOff );

	// The music of the ride
	static TINT GetRideMusic();

	// Ends the ride, the rider plays a_rcAnim
	void EndRide( const Toshi::TPString8& a_rcAnim, TBOOL a_bUnused, TBOOL a_bRight );
	void EndRide( TBOOL a_bRight );

	// Releases the units of the ride
	void ReleaseUnits();

	// Destroys the unit of the ride and gives the player back the cow, a_bKeepBike lets the bike keep its physics state
	void DestroyPlayerBike( TBOOL a_bKeepBike );

	TBOOL  IsPlayerOnBike() const { return m_bPlayerOnBike; }
	TINT   GetLoadedBikeType() const { return m_iLoadedBikeType; }
	AUnit* GetRider() const { return m_pRiderProxy ? m_pRiderProxy->GetUnit() : TNULL; }
	AUnit* GetPlayerBike() const { return m_pPlayerBikeProxy ? m_pPlayerBikeProxy->GetUnit() : TNULL; }
	AUnit* GetBike() const { return m_pBikeProxy ? m_pBikeProxy->GetUnit() : TNULL; }
	TINT   GetNumBikes() const { return m_iNumBikes; }
	STATE  GetState() const { return m_eState; }

	void SetMainHudElement( ASimPlayerMainHudElement* a_pElement ) { m_pMainHudElement = a_pElement; }

	// Sorts the racks by their distance to a_rcPosition, a_piOutRacks gets the indices of the racks
	void SortRacksByDistance( const Toshi::TVector4& a_rcPosition, TINT* a_piOutRacks );

	// Replaces the player unit by the unit of the cow riding the bike
	void CreatePlayerBike();

private:
	// Makes the ride camera follow the player bike
	void AttachRideCamera( TFLOAT a_fTransitionTime );

	// Gives the camera back to the rider
	void DetachRideCamera( TFLOAT a_fTransitionTime, TBOOL a_bBehindRider );

	// Plays the milkdrip effect at the neck of a_pUnit
	void PlayMilkDrip( AUnit* a_pUnit );

	// Where the rider is put after falling into the water
	void GetRespawnPosition( const Toshi::TVector4& a_rcPosition, Toshi::TVector4& a_rOutPosition, Toshi::TVector4& a_rOutDirection );

	// Puts every bike back to the racks closest to the player
	void ParkAllBikes();

	static void BlendCameraFOV( TFLOAT a_fDuration );

public:
	static AFunctionTable ms_oFunctionTable;

private:
	Bike                       m_aBikes[ MAX_NUM_BIKES ];      // 0x020
	Rack                       m_aRacks[ MAX_NUM_RACKS ];      // 0x3a0
	TINT                       m_iNumBikes;                    // 0x544
	TINT                       m_iNumRacks;                    // 0x548
	BIKEFLAGS                  m_uiBikeTypes;                  // 0x54c
	TFLOAT                     m_fRespawnTimer;                // 0x550
	TINT                       m_iUnk554;                      // 0x554
	TCHAR                      PADDING1[ 0x20 ];               // 0x558
	AFade*                     m_pFade;                        // 0x578
	ASimPlayerMainHudElement*  m_pMainHudElement;              // 0x57c
	Toshi::TRef<AUnitProxy>    m_pPlayerBikeProxy;             // 0x580, the cow riding the bike (the player unit during the ride)
	Toshi::TRef<AUnitProxy>    m_pBikeProxy;                   // 0x584, the bike
	Toshi::TRef<AUnitProxy>    m_pRiderProxy;                  // 0x588, the cow that got on the bike
	STATE                      m_eState;                       // 0x58c
	TBOOL                      m_bWantsToGetOff;               // 0x590
	TBOOL                      m_bWalking;                     // 0x591
	TFLOAT                     m_fGetOffTime;                  // 0x594
	AUnit*                     m_pHeldUnit;                    // 0x598, the unit the rider holds on c_l_clavicle
	AGlowViewport::GlowObject* m_pGlowObject;                  // 0x59c, the headlight of the bike
	TFLOAT                     m_fSavedNear;                   // 0x5a0, near clip distance before the ride
	TBOOL                      m_bPlayerOnBike;                // 0x5a4
	Toshi::TPString8           m_aBikeTypes[ NUM_BIKE_TYPES ]; // 0x5a8, "bike1" to "bike5"
	TFLOAT                     m_fUnk5BC;                      // 0x5bc
	TFLOAT                     m_fWalkTime;                    // 0x5c0, time left to walk to the bike
	Toshi::TVector4            m_vecRackDirection;             // 0x5c4
	Toshi::TVector4            m_vecRackPosition;              // 0x5d4
	Toshi::TVector4            m_vecGetOnPosition;             // 0x5e4
	Toshi::TVector4            m_vecLastPosition;              // 0x5f4
	TINT                       m_iMilkDripEffect;              // 0x604
	TINT                       m_iLoadedBikeType;              // 0x608
	TBOOL                      m_bRestartMusic;                // 0x60c
	TBOOL                      m_bMusicStopped;                // 0x60d
};
