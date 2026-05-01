#pragma once

#include "UnitMovement/PathfindingNetworks/AInteriorArea.h"
#include "UnitMovement/PathfindingNetworks/AInteriorGoal.h"

#include <File/TTRB.h>
#include <Math/TVector4.h>
#include <Toshi/TPString8.h>
#include <Toshi/T2Map.h>
#include <Toshi/T2Vector.h>
#include <Toshi/T2SList.h>

class AInteriorNetwork
	: public Toshi::TObject
{
public:
	TDECLARE_CLASS( AInteriorNetwork, Toshi::TObject );

	struct INetwork
	{
		TINT            m_iNumAreas;
		TINT            m_iNumGoals;
		TINT            m_iNumSpecialAreas;
		TINT            m_iNumNormalAreas;
		AInteriorArea*  m_pAreas;
		AInteriorGoal*  m_pGoals;
		TINT*           m_piSpecialAreas;
		void**          m_ppSpecialAreaData;
		TINT            m_iUnk1;
		TINT            m_iUnk2;
		TINT            m_iUnk3;
		TINT            m_iUnk4;
	};

	struct LookupList
	{
		TINT  iCount;
		TINT* pIndices;

		operator TINT*() { return pIndices; }
	};

public:
	AInteriorNetwork();
	~AInteriorNetwork();

	TBOOL LoadFromFile( const Toshi::TPString8& a_rcNetworkName, const TCHAR* a_szFilePath );
	void  ResolvePointHeight( TINT a_iAreaIndex, Toshi::TVector4& a_rPoint, TUINT a_uiFlags );

	TINT           FindAreaIndex( const Toshi::TPString8& a_rcName ) const;
	TINT           FindContainingArea( const Toshi::TVector4& a_rcPoint ) const;
	AInteriorGoal* FindGoal( const Toshi::TPString8& a_rcName ) const;
	AInteriorGoal* FindGoalInArea( const Toshi::TPString8& a_rcName, TINT a_iAreaIndex = -1 ) const;
	TBOOL          HasGoal( const Toshi::TPString8& a_rcName ) const;
	AInteriorGoal* FindRandomGoal( const Toshi::TPString8& a_rcName ) const;
	AInteriorGoal* FindRandomVisibleGoal( const Toshi::TPString8& a_rcName ) const;
	AInteriorGoal* FindFarthestGoal( const Toshi::TPString8& a_rcName, const Toshi::TVector4& a_rcPosition ) const;

	TBOOL BuildPathToArea( TINT a_iStartArea, TINT a_iTargetArea, void* a_pPath );
	TBOOL BuildPathToAnyArea( TINT a_iStartArea, const TINT* a_pTargetAreas, TINT a_iNumTargetAreas, void* a_pPath, TINT* a_pOutArea );
	
	const Toshi::TPString8& GetName() const { return m_strName; }

private:
	void BuildAreaNameLookup();
	void BuildGoalNameLookup();

	void ClearAreaNameLookup();
	void ClearGoalNameLookup();

public:
	Toshi::TTRB*      m_pTRB;
	INetwork*         m_pINetwork;
	Toshi::TPString8  m_strName;
	TBOOL             m_bSetHeightFromEnv;

	Toshi::T2Vector<Toshi::T2SList<Toshi::T2SListNodeWrapper<void*>>, 70> m_vecArray;

	void* m_pUpdateGoalInterfaces;
	TINT  m_Unk1;

	// ...
	TBOOL m_Unk2;

	Toshi::T2Map<Toshi::TPString8, LookupList, Toshi::TPString8::Comparator>* m_pAreaNameLookup;
	Toshi::T2Map<Toshi::TPString8, LookupList, Toshi::TPString8::Comparator>* m_pGoalNameLookup;
};
