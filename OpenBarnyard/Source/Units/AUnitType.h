#pragma once
#include <Toshi/TNodeList.h>
#include <Toshi/TDList.h>
#include <Toshi/TPString8.h>
#include <Toshi/TClass.h>

#include <Plugins/PPropertyParser/PBProperties.h>

#include "AUnitStaticData.h"

class AUnit;

// A registered unit blueprint: the class to spawn, its pooled name, steer name
// and any static data blocks. Lives in AUnitFactory's type list.
class AUnitType : public Toshi::TNodeList<AUnitType>::TNode
{
public:
	friend class AUnitFactory;

public:
	AUnitType();
	~AUnitType();

	TBOOL CreateFromProperties( const PBProperties* a_pProperties );

	// Renumbers the live units of this type through AUnit::SetUnitCountIdentifier (1-based).
	void ReindexInstances();

	// Localised name, or an empty string when the type has none.
	const TWCHAR* GetScreenName() const;

	// Static data block of exactly a_pClass (no IsA).
	AUnitStaticData* FindStaticData( Toshi::TClass* a_pClass );

	const Toshi::TPString8& GetName() const { return m_strName; }

	// $Barnyard: FUNCTION 0040f630
	const TCHAR*                    GetNameString() const { return m_strName.GetString(); }
	TINT                            GetScreenNameId() const { return m_iScreenNameId; }
	void                            SetScreenNameId( TINT a_iScreenNameId ) { m_iScreenNameId = a_iScreenNameId; }
	const Toshi::TPString8&         GetSteerName() const { return m_strSteer; }
	Toshi::TClass*                  GetUnitClass() const { return m_pClass; }
	TINT                            GetInstanceCount() const { return m_iInstanceCount; }
	Toshi::TDList<AUnitStaticData>& GetStaticData() { return m_llStaticData; }

private:
	void LoadStaticData( const PBProperties* a_pProperties );

private:
	Toshi::TPString8               m_strName;        // 0x0c
	TINT                           m_iScreenNameId;  // 0x10, localised name id (-1 = none)
	Toshi::TPString8               m_strSteer;       // 0x14
	Toshi::TClass*                 m_pClass;         // 0x18
	TINT                           m_iInstanceCount; // 0x1c
	Toshi::TDList<AUnitStaticData> m_llStaticData;   // 0x20
	TINT                           m_iUnk28;         // 0x28, zeroed in ctor
};
