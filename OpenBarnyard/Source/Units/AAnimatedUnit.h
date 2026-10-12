#pragma once
#include "AVisibleUnit.h"
#include "Animation/AAnimatableObject.h"

#include <Toshi/TPString8.h>
#include <Math/TVector4.h>

// A visible unit whose visual is an AAnimatableObject (created from the unit
// type's model list in Create). It also lets other animated units be attached
// to bones of its model (AddUnitToParent / AttachUnit / DetachUnit).
//
// TClass 0x0077e65c (version 1.1, size 0x98, alignment 1, not runtime creatable),
// vtable 0x00717f30, secondary (AGoalCollisionUpdateInterface) vtable 0x00717f20.
class AAnimatedUnit : public AVisibleUnit
{
public:
	TDECLARE_CLASS( AAnimatedUnit, AVisibleUnit );

public:
	AAnimatedUnit();
	~AAnimatedUnit();

	//-----------------------------------------------------------------------------
	// AUnit
	//-----------------------------------------------------------------------------
	virtual AFunctionTable* GetFunctionTable() override;                      // +0x0c
	virtual void            SetSize( TFLOAT a_fSize ) override;               // +0x28
	virtual TBOOL           Create( AUnitTransform* a_pTransform ) override;  // +0x58
	virtual void            OnDestroy() override;                             // +0x60
	virtual void            OnUpdate( TFLOAT a_fDeltaTime ) override {}       // +0x70, DOES_NOTHING 006c6060
	virtual void            UpdateComponents( TFLOAT a_fDeltaTime ) override; // +0x74

	//-----------------------------------------------------------------------------
	// AVisibleUnit
	//-----------------------------------------------------------------------------
	virtual void            SetVisible( TBOOL a_bVisible ) override;  // +0x90
	virtual TBOOL           IsVisible() override;                     // +0x94
	virtual void            SetOpacity( TFLOAT a_fOpacity ) override; // +0xa0
	virtual void            UpdateBoundingSphere() override;          // +0xa4
	virtual AModelInstance* GetModelInstance() override;              // +0xa8

	//-----------------------------------------------------------------------------
	// Own virtuals
	//-----------------------------------------------------------------------------
	// Attaches this unit to a_rcParentBone of a_pParent (FSM "AddUnitToParent").
	virtual void   AddUnitToParent( AUnit* a_pParent, const Toshi::TPString8& a_rcParentBone );                                                                                            // +0xb4
	virtual void   AttachUnitWithOffset( AUnit* a_pUnit, const Toshi::TPString8& a_rcParentBone, const Toshi::TPString8& a_rcUnitBone, const Toshi::TVector4& a_rcOffset, TFLOAT a_fUnk ); // +0xb8
	virtual void   AttachUnitToBone( AUnit* a_pUnit, const Toshi::TPString8& a_rcParentBone, const Toshi::TPString8& a_rcUnitBone );                                                       // +0xbc
	virtual void   AttachUnit( AUnit* a_pUnit, const Toshi::TPString8& a_rcParentBone );                                                                                                   // +0xc0
	virtual AUnit* GetAttachedUnitByBoneId( TINT a_iBone, TINT a_iIndex );                                                                                                                 // +0xc4, a_iIndex < 0: first one owned by a unit
	virtual AUnit* GetAttachedUnit( const Toshi::TPString8& a_rcBone, TINT a_iIndex );                                                                                                     // +0xc8, a_iIndex < 0: first one owned by a unit
	virtual void   DetachUnit( AUnit* a_pUnit, TBOOL a_bReactivate, TBOOL a_bKeepUpright );                                                                                                // +0xcc
	virtual void   RemoveFromParentAttachedUnit();                                                                                                                                         // +0xd0, FSM "RemoveFromParentAttachedUnit"

	// FSM "IsAttachedToParent"
	TBOOL IsAttachedToParent( AUnit* a_pParent );

	// Detaches (and stops skeleton updates of) every object attached to a_rcBone.
	void DetachObjectsAtBone( const Toshi::TPString8& a_rcBone );

	TBOOL ApplyMaterial( TINT a_iSkin, TBOOL a_bSkinLighting );
	TINT  GetNumSkins();

	AAnimatableObjectType* GetAnimatableObjectType();

	// Replaces the animatable object (only when the unit already has one).
	void ReplaceAnimatableObject( AAnimatableObject* a_pAnimatableObject );

	// $Barnyard: FUNCTION 00415b30
	AAnimatableObject* GetAnimatableObject() const { return m_pAnimatableObject; }

public:
	static AFunctionTable ms_oFunctionTable;

	// Name of the a_iModelIndex'th model of the unit type (AUnitModelStaticData)
	Toshi::TPString8 GetModelName( TINT a_iModelIndex );

private:
	// Creates m_pAnimatableObject from the a_iModelIndex'th model of the unit type
	TBOOL CreateAnimatableObject( TINT a_iModelIndex );

protected:
	AAnimatableObject* m_pAnimatableObject; // 0x94
};

TSTATICASSERT( sizeof( AAnimatedUnit ) == 0x98 );
