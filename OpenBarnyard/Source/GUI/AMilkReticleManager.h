#pragma once
#include "GUI/ATargetReticle.h"
#include "Units/AUnitProxy.h"

#include <Toshi/TRefCounted.h>
#include <Math/TVector4.h>

class AUnit;
class AAimedSquirtProjectileLauncher;

// Target reticles of the milk squirt: while the player aims (AAimedSquirtProjectileLauncher),
// every milk target (unit flag 0x200) around the player gets a reticle that locks on to it,
// and the launcher is locked on to one of them (PickTarget).
// ASimState and the milk squirting minigames have one each (+0xa32c of ASimState).
// NOTE: The class has no RTTI.
class AMilkReticleManager
{
public:
	static constexpr TINT MAX_NUM_TARGETS = 25;

	// AUnit flag of the units the milk squirt can lock on to (set by the minigames)
	static constexpr TUINT32 UNIT_FLAG_MILK_TARGET = BITFLAG( 9 );

	enum STATE : TINT
	{
		STATE_WAITING,   // waits for its turn to lock on
		STATE_LOCKING,   // closes in on the target
		STATE_LOCKED,    // locked on
		STATE_RELEASING, // opens up again, the target is gone or the player has stopped aiming
		STATE_HIDDEN,
	};

	struct Target
	{
		// $Barnyard: FUNCTION 004207e0
		Target() {}

		// $Barnyard: FUNCTION 00420830
		~Target() {}

		Toshi::TRef<AUnitProxy> oUnit;      // 0x000
		ATargetReticle          oReticle;   // 0x004
		Toshi::TVector4         vPosition;  // 0x36c, c_pelvis bone or the middle of the unit
		TBOOL                   bOffScreen; // 0x37c
		TBOOL                   bFound;     // 0x37d, by the last FindTargets
		STATE                   eState;     // 0x380
		TINT                    iBone;      // 0x384, c_pelvis or c_pip_pelvis, -1 if there's none
	};

	// A target PickTarget can choose, they are sorted by fScore (highest first)
	struct Candidate
	{
		TFLOAT fDot;      // 0x0, of the camera's forward direction and the direction to the target (XZ)
		TFLOAT fDistance; // 0x4, from the camera
		TFLOAT fScore;    // 0x8
		TINT   iTarget;   // 0xc, index of m_aTargets
	};

	struct CandidateList
	{
		TINT      iNumCandidates;                 // 0x00
		Candidate aCandidates[ MAX_NUM_TARGETS ]; // 0x04

		void InsertAt( TINT a_iIndex, const Candidate& a_rcCandidate );
		void MakeRoom( TINT a_iIndex, TINT a_iCount, TBOOL a_bMove );
	};

public:
	AMilkReticleManager();
	~AMilkReticleManager();

	// Targets are searched for within a_fSearchRadius of a_pCentreUnit, the reticles are added to a_pParent (or the AGUI2 root)
	void Create( AUnit* a_pCentreUnit, TFLOAT a_fSearchRadius, Toshi::T2GUIElement* a_pParent );

	// Hides every reticle unless a_bActive is set
	void SetActive( TBOOL a_bActive );

	TINT GetNumTargets() const { return m_iNumTargets; }

	// NOTE: Cow Tipping updates the manager itself (00421760 from 004a7a40)
	friend class ACowTippingMiniGameState;

	// For the player of the sim
	void UpdateSim( TFLOAT a_fDeltaTime );

	// For the milk squirting minigames
	void UpdateMiniGame( TFLOAT a_fDeltaTime );

	// $Barnyard: FUNCTION 00420550
	void SetSearchCentre( const Toshi::TVector4& a_rcCentre ) { m_vSearchCentre = a_rcCentre; }

	void SetIgnoreScreen( TBOOL a_bIgnoreScreen ) { m_bIgnoreScreen = a_bIgnoreScreen; }
	void SetSearchInterval( TFLOAT a_fInterval ) { m_fSearchInterval = a_fInterval; }

private:
	// Searches for new targets and updates the reticles
	void Update( TFLOAT a_fDeltaTime, TBOOL a_bAiming );

	// Gives a reticle to every milk target around the centre unit
	void FindTargets();

	void AddTarget( AUnit* a_pUnit );
	void SetTarget( AUnit* a_pUnit, TINT a_iTarget );

	// Sets the target the launcher is locked on to, plays a sound when it changes
	void SetCurrentTarget( AUnit* a_pUnit );

	// Chooses the target the launcher should lock on to, a_pLauncher has to be able to hit it (with a_iWeight above 1, it's chosen less likely otherwise)
	AUnit* PickTarget( AAimedSquirtProjectileLauncher* a_pLauncher, TINT a_iWeight );

	void InsertCandidate( TINT a_iTarget, TFLOAT a_fDot, TFLOAT a_fDistance, TINT a_iWeight );

	// The next waiting target starts locking on
	void NextLockingTarget();

	// Moves the reticle to the target and advances its state
	void UpdateTargetPosition( TINT a_iTarget );

	void UpdateReticle( TINT a_iTarget, TFLOAT a_fDeltaTime );

private:
	Target          m_aTargets[ MAX_NUM_TARGETS ]; // 0x0000
	CandidateList   m_oCandidates;                 // 0x5848
	Toshi::TVector4 m_vSearchCentre;               // 0x59dc
	AUnit*          m_pCentreUnit;                 // 0x59ec
	AUnit*          m_pCurrentTarget;              // 0x59f0
	AUnit*          m_pPrevTarget;                 // 0x59f4
	TFLOAT          m_fSearchRadius;               // 0x59f8
	TFLOAT          m_fSearchInterval;             // 0x59fc
	TINT            m_iNumTargets;                 // 0x5a00
	TINT            m_iLockingTarget;              // 0x5a04, the one in STATE_LOCKING
	TINT            m_iPickIndex;                  // 0x5a08, index of m_oCandidates
	TBOOL           m_bActive;                     // 0x5a0c, the reticles are shown
	TBOOL           m_bAiming;                     // 0x5a0d
	TBOOL           m_bIgnoreScreen;               // 0x5a0e, targets out of the screen keep their reticles
	TINT            m_iTargetSound;                // 0x5a10
	TFLOAT          m_fPickTimer;                  // 0x5a14
	TFLOAT          m_fSearchTimer;                // 0x5a18
	TFLOAT          m_fForgetTimer;                // 0x5a1c, m_pPrevTarget is forgotten once the player stops aiming for a second
	TBOOL           m_bTargetPicked;               // 0x5a20, UpdateMiniGame
};

TSTATICASSERT( sizeof( AMilkReticleManager::Target ) == 0x388 );
TSTATICASSERT( sizeof( AMilkReticleManager ) == 0x5a24 );
