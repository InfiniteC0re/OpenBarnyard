#pragma once
#include "AUnitComponent.h"

#include <Toshi/TArray.h>
#include <Toshi/TPString8.h>

class AAnimatableObject;
class AModelSwapComponentData;

// Replaces a unit's model with one of the animatable objects listed in its
// AModelSwapComponentData (e.g. the player's "land"/"up" models), optionally until an
// animation of the swapped model is over.
//
// TClass 0x0078a270 (version 0.1, size 0x2c, runtime creatable), vtable 0x00728990.
class AModelSwapComponent : public AUnitComponent
{
public:
	TDECLARE_CLASS( AModelSwapComponent, AUnitComponent );

	using FLAGS = TUINT8;
	enum FLAGS_ : FLAGS
	{
		FLAGS_NONE               = 0,
		FLAGS_UNTIL_ANIM_IS_OVER = BITFLAG( 0 ), // swaps back once m_strAnimation stops playing
	};

public:
	AModelSwapComponent();
	~AModelSwapComponent();

	//-----------------------------------------------------------------------------
	// AUnitComponent
	//-----------------------------------------------------------------------------
	virtual AUnitComponentManager* GetManager() OVERRIDE;                    // +0x10
	virtual void                   UpdateAI( TFLOAT a_fDeltaTime ) OVERRIDE; // +0x2c

	// Creates an animatable object for every model of a_pData, owned by a_pUnit
	TBOOL CreateModels( AUnit* a_pUnit, AModelSwapComponentData* a_pData );

	// Shows model a_iModel (with skin a_iSkin if a_bApplySkin) instead of the unit's own one.
	// a_pAnimation is played on it; without a_bApplySkin the model is only kept while it plays.
	void SwapModel( TINT a_iModel, TINT a_iSkin, const Toshi::TPString8* a_pAnimation, TBOOL a_bApplySkin );

	AAnimatableObject* GetSwappedModel() const { return m_pSwappedModel; }

private:
	// NOTE: retail initialises the grow size to 0
	Toshi::TArray<AAnimatableObject*> m_aModels;       // 0x10
	AAnimatableObject*                m_pSwappedModel; // 0x20
	Toshi::TPString8                  m_strAnimation;  // 0x24
	FLAGS                             m_eFlags;        // 0x28
};

TSTATICASSERT( sizeof( AModelSwapComponent ) == 0x2c );
