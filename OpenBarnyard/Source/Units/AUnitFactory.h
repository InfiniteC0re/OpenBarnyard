#pragma once
#include <Toshi/TObject.h>
#include <Toshi/TNodeList.h>
#include <Toshi/TPString8.h>

class AUnit;
class AUnitType;
class PBProperties;

// Registry of unit blueprints keyed by pooled lowercase name.
class AUnitFactory : public Toshi::TObject
{
public:
	TDECLARE_CLASS( AUnitFactory, Toshi::TObject );

public:
	AUnitFactory();
	~AUnitFactory();

	// Registers every entry of the "Main" PBProperties block of a_szFilePath.
	TBOOL LoadUnitTypes( const TCHAR* a_szFilePath );

	// NOTE: compiled to a folded RETURNS_TRUE stub in the retail build (called with
	// "Data/Units" right after LoadUnitTypes); the name is a guess.
	TBOOL LoadUnitLibraries( const TCHAR* a_szDirectory ) { return TTRUE; }

	AUnitType* RegisterType( const TCHAR* a_szName, const PBProperties* a_pProperties );
	AUnitType* Find( const Toshi::TPString8& a_rcName );

	// Spawns a unit of a_pType's class and binds it to the type.
	AUnit* CreateInstance( AUnitType* a_pType );

	// Called when an instance of a_pUnit's type is released back.
	void ReleaseInstance( AUnit* a_pUnit );

private:
	Toshi::TNodeList<AUnitType> m_llTypes; // 0x04
};
