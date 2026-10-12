#pragma once
#include <Toshi/TPString8.h>
#include <Toshi/T2Vector.h>

#include "AUnitStaticData.h"

class AInputMap;

// Static data of a unit type's control component ("AControlComponentData" block):
// which AControlComponent subclass to create, its input map and the game states
// during which the component is enabled.
class AControlComponentData : public AUnitStaticData
{
public:
	TDECLARE_CLASS( AControlComponentData, AUnitStaticData );

	friend class AControlComponentManager;

public:
	static constexpr TINT MAX_NUM_GAMESTATES = 16;

public:
	AControlComponentData();
	~AControlComponentData();

	virtual TBOOL Create( const PBProperties* a_pProperties ) OVERRIDE; // +0x0c
	virtual void  OnUnitCreate( AUnit* a_pUnit ) OVERRIDE;              // +0x10

	AInputMap* GetInputMap() const { return m_pInputMap; }

private:
	AInputMap*                                                m_pInputMap;     // 0x0c
	Toshi::TPString8                                          m_strType;       // 0x10, name of the AControlComponent subclass
	Toshi::T2Vector<const Toshi::TClass*, MAX_NUM_GAMESTATES> m_vecGameStates; // 0x14, AGameState classes that enable the component
};

TSTATICASSERT( sizeof( AControlComponentData ) == 0x58 );
