#pragma once
#include <Toshi/TObject.h>
#include <Toshi/TSingleton.h>
#include <Toshi/TPString8.h>
#include <Toshi/T2Map.h>

#include <Plugins/PPropertyParser/PBProperties.h>

//-----------------------------------------------------------------------------
// Up to 32 named flags loaded from a Data/SocialGroups file. The flags are
// split into groups (every block of the file but "maskgroup") and the
// "maskgroup" blocks name masks made of several flags.
// NOTE: Vtable 0072d20c.
//-----------------------------------------------------------------------------
class AUnitFlagSet
{
public:
	static constexpr TINT MAX_NUM_FLAGS  = 32;
	static constexpr TINT MAX_NUM_GROUPS = 6;

	struct Group
	{
		TINT  iFirstFlag; // 0x00
		TINT  iEndFlag;   // 0x04, one past the last flag of the group
		TUINT uiMask;     // 0x08, every flag of the group
	};

	using MaskMap = Toshi::T2Map<Toshi::TPString8, TUINT, Toshi::TPString8::Comparator>;

public:
	AUnitFlagSet();
	virtual ~AUnitFlagSet();

	// Loads the groups and the masks from a properties TRB file.
	TBOOL Load( const TCHAR* a_szFileName );

	// Index of the flag, -1 if there is none of that name.
	TINT GetFlagIndex( const Toshi::TPString8& a_rcName ) const;

	// Looks up the mask of the "maskgroup" called a_rcName.
	TBOOL GetMask( TUINT& a_ruiOutMask, const Toshi::TPString8& a_rcName );

	// TRUE if no other flag of a_iFlag's group is set in a_ruiFlags.
	TBOOL CanSetFlag( const TUINT& a_ruiFlags, TINT a_iFlag ) const;

	// Sets every flag of a_iFlag's group in a_ruiFlags.
	void SetGroupFlags( TUINT& a_ruiFlags, TINT a_iFlag ) const;

private:
	void AddGroup( const PBPropertyValueArray* a_pFlags );
	void AddMaskGroup( const PBProperties* a_pProperties );

private:
	TINT             m_iNumFlags;                   // 0x04
	Toshi::TPString8 m_aFlagNames[ MAX_NUM_FLAGS ]; // 0x08
	TINT             m_iNumGroups;                  // 0x88
	Group            m_aGroups[ MAX_NUM_GROUPS ];   // 0x8c
	MaskMap          m_oMasks;                      // 0xd4
};

TSTATICASSERT( sizeof( AUnitFlagSet ) == 0xec );

//-----------------------------------------------------------------------------
// The social flag sets units are described with (see AUnitProfileComponent).
//-----------------------------------------------------------------------------
class AUnitFlagSetsManager
    : public Toshi::TObject
    , public Toshi::TSingleton<AUnitFlagSetsManager>
{
public:
	TDECLARE_CLASS( AUnitFlagSetsManager, Toshi::TObject );

	using FLAGSET = TINT;
	enum FLAGSET_ : FLAGSET
	{
		FLAGSET_RELATIONSHIPS,
		FLAGSET_INTERESTS,
		FLAGSET_REPUTATIONS,
		FLAGSET_NUMOF
	};

public:
	AUnitFlagSetsManager();
	~AUnitFlagSetsManager();

	AUnitFlagSet* GetFlagSet( FLAGSET a_eFlagSet ) { return &m_aFlagSets[ a_eFlagSet ]; }

private:
	AUnitFlagSet m_aFlagSets[ FLAGSET_NUMOF ]; // 0x04
};

TSTATICASSERT( sizeof( AUnitFlagSetsManager ) == 0x2c8 );
