#pragma once
#include "AAnimatedUnit.h"

class AProjectileLauncher;

// The player character. It runs on its own AI/think cadence in AUnitManager::OnUpdate
// and owns the projectile launchers (milk squirt, aimed squirt) of its APlayerLauncherData.
//
// TClass 0x0077e5cc (version 0.1, size 0xb0, runtime creatable), vtable 0x00717cc0,
// secondary (AGoalCollisionUpdateInterface) vtable 0x00717cb4.
class AUnitPlayer : public AAnimatedUnit
{
public:
	TDECLARE_CLASS( AUnitPlayer, AAnimatedUnit );

	static constexpr TINT NUM_LAUNCHERS = 2;

public:
	AUnitPlayer();
	~AUnitPlayer();

	//-----------------------------------------------------------------------------
	// AUnit
	//-----------------------------------------------------------------------------
	virtual TBOOL SetMovement( TBOOL a_bEnabled ) override;        // +0x18
	virtual TBOOL Create( AUnitTransform* a_pTransform ) override; // +0x58
	virtual void  OnDestroy() override;                            // +0x60
	virtual void  UpdateAI( TFLOAT a_fDeltaTime ) override;        // +0x68
	virtual TBOOL Think( TFLOAT a_fDeltaTime ) override;           // +0x6c

	//-----------------------------------------------------------------------------
	// AVisibleUnit
	//-----------------------------------------------------------------------------
	virtual void SetVisible( TBOOL a_bVisible ) override; // +0x90, toggles the swapped model if there's one

	//-----------------------------------------------------------------------------
	// Own virtuals
	//-----------------------------------------------------------------------------
	virtual TFLOAT GetUnkAC();                  // +0xd4
	virtual void   SetUnkAC( TFLOAT a_fValue ); // +0xd8

	// Switches to launcher a_iIndex (-1 deactivates the current one)
	void SetActiveLauncher( TINT a_iIndex, TBOOL a_bTransferState );
	void SetActiveLauncher( AProjectileLauncher* a_pLauncher, TBOOL a_bTransferState );

	// Activates the launcher of the primary (TFALSE) or secondary (TTRUE) slot of APlayerLauncherData
	void SelectLauncher( TBOOL a_bSecondary );

	// AModelSwapComponent related, called from 005348b0 and 00534ac0 ("land" / "up")
	void SetSwappedModel( TBOOL a_bLand );

	// Sets the launcher input flags of the ASimplePlayerControlComponent (unless dying)
	void SetControlFlags( TBOOL a_bValue );

	AProjectileLauncher* GetActiveLauncher() const { return m_pActiveLauncher; }
	AProjectileLauncher* GetLauncher( TINT a_iIndex ) const { return m_apLaunchers[ a_iIndex ]; }

	// Launcher of the primary slot (inlined in retail)
	AProjectileLauncher* GetPrimaryLauncher() const
	{
		TINT iIndex = m_aiLauncherSlots[ 0 ];
		return ( iIndex > -1 && iIndex < NUM_LAUNCHERS ) ? m_apLaunchers[ iIndex ] : TNULL;
	}

	// Launcher of the secondary slot
	AProjectileLauncher* GetSecondaryLauncher() const;

private:
	AProjectileLauncher* m_apLaunchers[ NUM_LAUNCHERS ];     // 0x98
	TINT                 m_aiLauncherSlots[ NUM_LAUNCHERS ]; // 0xa0, launcher index per slot (APlayerLauncherData +0x10/+0x14)
	AProjectileLauncher* m_pActiveLauncher;                  // 0xa8
	TFLOAT               m_fUnkAC;                           // 0xac
};

TSTATICASSERT( sizeof( AUnitPlayer ) == 0xb0 );
