#pragma once
#include "AUnitStaticData.h"

#include <Toshi/TPString8.h>

// Static data of a unit type whose model can be swapped for another animatable object
// ("AModelSwapComponentData" block, "Models" array of animatable object type names).
// Creates an AModelSwapComponent for every unit of the type.
//
// TClass 0x0078a240 (version 0.1, size 0x1c, runtime creatable), vtable 0x007289c8.
class AModelSwapComponentData : public AUnitStaticData
{
public:
	TDECLARE_CLASS( AModelSwapComponentData, AUnitStaticData );

public:
	AModelSwapComponentData();
	~AModelSwapComponentData();

	virtual TBOOL Create( const PBProperties* a_pProperties ) OVERRIDE; // +0x0c
	virtual void  OnUnitCreate( AUnit* a_pUnit ) OVERRIDE;              // +0x10

	TINT                    GetNumModels() const { return m_iNumModels; }
	const Toshi::TPString8& GetModel( TINT a_iIndex ) const { return m_pModels[ a_iIndex ]; }

private:
	// NOTE: retail uses a TArray<TPString8> instantiation whose Resize (0041b880) allocates
	// with new[]; TArray in this repo doesn't construct its elements, so it's reproduced here.
	void ResizeModels( TINT a_iSize );

private:
	TINT              m_iGrowSize;       // 0x0c, unused
	TINT              m_iNumModels;      // 0x10
	TINT              m_iNumAllocModels; // 0x14
	Toshi::TPString8* m_pModels;         // 0x18, "Models"
};

TSTATICASSERT( sizeof( AModelSwapComponentData ) == 0x1c );
