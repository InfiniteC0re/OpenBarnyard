#pragma once
#include <Toshi/TObject.h>
#include <Toshi/TSingleton.h>
#include <Toshi/TPString8.h>

class AUnit;
class AFunctionTable;

// Maps script names to unit ids (FSM "RetrieveByName" / "RetrieveByType").
class AUnitNameManager
    : public Toshi::TObject
    , public Toshi::TSingleton<AUnitNameManager>
{
public:
	TDECLARE_CLASS( AUnitNameManager, Toshi::TObject );

	static constexpr TINT NUM_BUCKETS = 17;

	struct Entry
	{
		~Entry();

		Toshi::TPString8 strName; // 0x00
		TINT             iUnitId; // 0x04
		Entry*           pNext;   // 0x08
	};

	// Chained hash map TPString8 -> unit id.
	// NOTE: a template instance in retail (its iterator is shared with other maps).
	struct UnitNameMap
	{
		TBOOL Find( const Toshi::TPString8& a_rcName, TINT& a_rOutId ) const;
		void  Insert( const Toshi::TPString8& a_rcName, const TINT& a_rcId );
		TBOOL Remove( const Toshi::TPString8& a_rcName );
		void  Clear();

		TINT    iCount;      // 0x00
		TINT    iNumBuckets; // 0x04
		Entry** ppBuckets;   // 0x08
	};

public:
	AUnitNameManager();
	~AUnitNameManager();

	virtual AFunctionTable* GetFunctionTable(); // +0x0c

	// FSM natives
	AUnit* RetrieveByName( Toshi::TPString8 a_strName );
	AUnit* RetrieveByType( Toshi::TPString8 a_strTypeName );

	TINT             GetUnitId( Toshi::TPString8 a_strName );
	Toshi::TPString8 GetUnitName( TINT a_iUnitId );
	void             SetUnitName( Toshi::TPString8 a_strName, TINT a_iUnitId );
	void             RemoveName( Toshi::TPString8 a_strName );
	void             Clear();

public:
	static AFunctionTable ms_oFunctionTable;

private:
	UnitNameMap m_oMap; // 0x04
};
