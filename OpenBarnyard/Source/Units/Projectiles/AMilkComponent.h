#pragma once
#include "Units/AUnitStaticData.h"

// Milk squirt settings of a unit type ("AMilkComponent" block). Despite the name
// it's static data, AUnitPlayer::Create copies it into its milk squirt launcher.
// Values that aren't in the properties stay -1.
class AMilkComponent : public AUnitStaticData
{
public:
	TDECLARE_CLASS( AMilkComponent, AUnitStaticData );

public:
	AMilkComponent();
	~AMilkComponent();

	virtual TBOOL Create( const PBProperties* a_pProperties ) OVERRIDE; // +0x0c

	TFLOAT GetFireAccuracy() const { return m_fFireAccuracy; }
	TFLOAT GetFireDelay() const { return m_fFireDelay; }       // NOTE: folded with other float getters of a field at +0x10 (0058ac00)
	TFLOAT GetRechargeRate() const { return m_fRechargeRate; } // NOTE: folded with other float getters of a field at +0x14 (0058ac50)
	TFLOAT GetAvoidFireAngle() const { return m_fAvoidFireAngle; }
	TFLOAT GetFireSpeed() const { return m_fFireSpeed; } // NOTE: folded with other float getters of a field at +0x1c (0058acf0)
	TFLOAT GetGrossOutShot() const { return m_fGrossOutShot; }
	TFLOAT GetVerticalAngle() const { return m_fVerticalAngle; } // NOTE: folded with other float getters of a field at +0x24 (00570320)
	TINT   GetMaxAmmo() const { return m_iMaxAmmo; }             // NOTE: folded with other getters of a field at +0x28 (0066b130)

private:
	TFLOAT m_fFireAccuracy;   // 0x0c, "milkfireaccuracy", converted to radians
	TFLOAT m_fFireDelay;      // 0x10, "milkfiredelay", AProjectileLauncher::SetFireInterval
	TFLOAT m_fRechargeRate;   // 0x14, "milkrechargerate", ALaunchAmmo::SetRechargeRate
	TFLOAT m_fAvoidFireAngle; // 0x18, "milkavoidfireangle"
	TFLOAT m_fFireSpeed;      // 0x1c, "milkfirespeed", AProjectileLauncher::SetLaunchSpeed
	TFLOAT m_fGrossOutShot;   // 0x20, "milkgrossoutshot"
	TFLOAT m_fVerticalAngle;  // 0x24, "milkverticalangle", converted to radians, AProjectileLauncher::SetVerticalAngle
	TINT   m_iMaxAmmo;        // 0x28, "milkmaxammo", ALaunchAmmo::SetMaxAmmo
};

TSTATICASSERT( sizeof( AMilkComponent ) == 0x2c );
