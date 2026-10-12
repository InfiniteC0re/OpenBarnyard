#pragma once
#include "AVisibleUnit.h"

//-----------------------------------------------------------------------------
// A unit with a plain (non-animated) model taken straight from data\models\,
// placed at the steer's position (e.g. "ben_grave", "milkweed", "funkgrenade").
//
// TClass 0077e818 (parent AVisibleUnit, size 0xa0), vtables 00718720,
// 00718714 (AGoalCollisionUpdateInterface).
//-----------------------------------------------------------------------------
class AStaticUnit
    : public AVisibleUnit
{
public:
	TDECLARE_CLASS( AStaticUnit, AVisibleUnit );

public:
	// NOTE: the retail ctor only exists inlined into the TClass create functions
	AStaticUnit();
	~AStaticUnit();

	//-----------------------------------------------------------------------------
	// AUnit
	//-----------------------------------------------------------------------------
	virtual AFunctionTable* GetFunctionTable() override;                      // +0x0c
	virtual void            SetSize( TFLOAT a_fSize ) override;               // +0x28
	virtual TBOOL           Create( AUnitTransform* a_pTransform ) override;  // +0x58
	virtual void            OnDestroy() override;                             // +0x60
	virtual TBOOL           Think( TFLOAT a_fDeltaTime ) override;            // +0x6c
	virtual void            UpdateComponents( TFLOAT a_fDeltaTime ) override; // +0x74

	//-----------------------------------------------------------------------------
	// AVisibleUnit
	//-----------------------------------------------------------------------------
	virtual void            SetVisible( TBOOL a_bVisible ) override; // +0x90
	virtual TBOOL           IsVisible() override;                    // +0x94
	virtual void            UpdateBoundingSphere() override;         // +0xa4
	virtual AModelInstance* GetModelInstance() override;             // +0xa8

	// Replaces the model with data\models\<name> of the AUnitModelStaticData
	// entry a_iModelIndex (random entry when negative).
	void CreateModelInstance( TINT a_iModelIndex );

	void SetModelYOffset( TFLOAT a_fOffset ) { m_fModelYOffset = a_fOffset; }

public:
	static AFunctionTable ms_oFunctionTable;

private:
	AModelInstanceRef m_pModelInstance; // 0x94
	TINT              m_iModelIndex;    // 0x98, not initialised by the ctor
	TFLOAT            m_fModelYOffset;  // 0x9c, added to the steer's height; 0 from the ctor, set by ALadderBeansGame (the height of the milkweeds)
};

TSTATICASSERT( sizeof( AStaticUnit ) == 0xa0 );
