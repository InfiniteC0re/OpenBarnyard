#pragma once
#include "AUnitStaticData.h"

#include <Toshi/TPString8.h>

// Static data of a unit type with models that can be attached to it
// ("AModelAttachComponentData" block, "Models" array of animatable object type names).
// Creates an AModelAttachComponent for every unit of the type.
//
// TClass 0x0078a210 (size 0x1c, runtime creatable), vtable 0x00728928.
class AModelAttachComponentData : public AUnitStaticData
{
public:
	TDECLARE_CLASS( AModelAttachComponentData, AUnitStaticData );

public:
	AModelAttachComponentData();
	~AModelAttachComponentData();

	virtual TBOOL Create( const PBProperties* a_pProperties ) OVERRIDE; // +0x0c
	virtual void  OnUnitCreate( AUnit* a_pUnit ) OVERRIDE;              // +0x10

	TINT                    GetNumModels() const { return m_iNumModels; }
	const Toshi::TPString8& GetModel( TINT a_iIndex ) const { return m_pModels[ a_iIndex ]; }

private:
	// NOTE: TArray<TPString8>::Resize (0041b880), see AModelSwapComponentData::ResizeModels
	void ResizeModels( TINT a_iSize );

private:
	TINT              m_iGrowSize;       // 0x0c, unused
	TINT              m_iNumModels;      // 0x10
	TINT              m_iNumAllocModels; // 0x14
	Toshi::TPString8* m_pModels;         // 0x18, "Models"
};

TSTATICASSERT( sizeof( AModelAttachComponentData ) == 0x1c );
