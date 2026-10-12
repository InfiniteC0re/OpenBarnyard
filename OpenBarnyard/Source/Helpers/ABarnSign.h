#pragma once
#include "Animation/AModel.h"

class ABarnSign
{
public:
	ABarnSign();
	~ABarnSign();

	void SpawnAtLocator( const TCHAR* a_pszLocatorName );
	void Destroy();

	// Swings the sign, harder during heavy rain
	void Update( TFLOAT a_fDeltaTime, TBOOL a_bDistanceCulling );

	AModelInstance* GetModelInstance() const { return m_pModelInstance; }

private:
	AModelInstance* m_pModelInstance; // 0x00
	TFLOAT          m_fPhase;         // 0x04
	TFLOAT          m_fSwingAngle;    // 0x08, the angle applied last frame
	TFLOAT          m_fSwingSpeed;    // 0x0C
	TFLOAT          m_fSwingAmount;   // 0x10
};
