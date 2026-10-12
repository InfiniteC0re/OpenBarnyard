#pragma once
#include "AUnitStaticData.h"

#include <Math/TVector4.h>
#include <Toshi/TPString8.h>

class AUnit;

// Object behaviour of a unit type (things that can be carried, thrown, used...),
// read from its "AObjectTraits" properties.
class AObjectTraits : public AUnitStaticData
{
public:
	TDECLARE_CLASS( AObjectTraits, AUnitStaticData );

	using FLAGS = TUINT8;
	enum FLAGS_ : FLAGS
	{
		FLAGS_NONE        = 0,
		FLAGS_HEAVY       = BITFLAG( 0 ), // "Heavy"
		FLAGS_DETECTOR    = BITFLAG( 1 ), // "Detector"
		FLAGS_AUTOPICKUP  = BITFLAG( 2 ), // "AutoPickup"
		FLAGS_OBSTRUCTION = BITFLAG( 3 ), // "Obstruction", AVisibleUnit::Create gives the unit an AKinematicsObstructionUnit
	};

	// Interaction started when something is done with the object, created from the
	// When* sub-blocks of the traits.
	// NOTE: the JP Wii build's RTTI names them AUsageInterface (Trigger), AThrowUsage, AUseUsage, AAreaUsage,
	// AEquipUsage and ABreakUsage (vtables 80502d54..80502e04). Their code is the same as on PC.
	class Trigger
	{
	public:
		virtual ~Trigger();                                                         // +0x00
		virtual TBOOL Create( const PBProperties* a_pProperties ) { return TTRUE; } // +0x04, RETURNS_TRUE 00413920
		virtual TBOOL Execute( AUnit* a_pUser, AUnit* a_pObject ) = 0;              // +0x08
	};

	// "WhenThrow", vtable 0x00718ba0
	class ThrowTrigger : public Trigger
	{
	public:
		virtual TBOOL Create( const PBProperties* a_pProperties ) OVERRIDE;
		virtual TBOOL Execute( AUnit* a_pUser, AUnit* a_pObject ) OVERRIDE;

	public:
		TFLOAT m_fStrength       = 20.0f; // 0x04
		TFLOAT m_fUpDirection    = 0.0f;  // 0x08
		TFLOAT m_fBaseThrowSpeed = 15.0f; // 0x0c
		TBOOL  m_bCanSpin        = TTRUE; // 0x10
		TBOOL  m_bDestroy        = TTRUE; // 0x11
	};

	// "WhenUse", vtable 0x00718bac
	class UseTrigger : public Trigger
	{
	public:
		using FLAGS = TUINT;
		enum FLAGS_ : FLAGS
		{
			FLAGS_NONE = 0,
			FLAGS_DROP = BITFLAG( 0 ), // "Drop"
		};

		virtual TBOOL Create( const PBProperties* a_pProperties ) OVERRIDE;
		virtual TBOOL Execute( AUnit* a_pUser, AUnit* a_pObject ) OVERRIDE;

	public:
		FLAGS m_eFlags = FLAGS_NONE; // 0x04
	};

	// "WhenArea", vtable 0x00718bc4
	class AreaTrigger : public Trigger
	{
	public:
		// $Barnyard: FUNCTION 0041ac00
		// $Barnyard: FUNCTION 0041abe0
		~AreaTrigger() {}

		virtual TBOOL Create( const PBProperties* a_pProperties ) OVERRIDE;
		virtual TBOOL Execute( AUnit* a_pUser, AUnit* a_pObject ) OVERRIDE;

	public:
		Toshi::TPString8 m_strAreaName; // 0x04
	};

	// "WhenEquip", vtable 0x00718b94
	class EquipTrigger : public Trigger
	{
	public:
		virtual TBOOL Execute( AUnit* a_pUser, AUnit* a_pObject ) OVERRIDE;

		// Assigns the "ObjectUnEquip" interaction
		static TBOOL UnEquip( AUnit* a_pUser, AUnit* a_pObject );
	};

	// "WhenBreak", vtable 0x00718bb8
	class BreakTrigger : public Trigger
	{
	public:
		using FLAGS = TUINT;
		enum FLAGS_ : FLAGS
		{
			FLAGS_NONE              = 0,
			FLAGS_BREAK_WHEN_THROWN = BITFLAG( 0 ), // "BreakWhenThrown"
		};

		virtual TBOOL Create( const PBProperties* a_pProperties ) OVERRIDE;
		virtual TBOOL Execute( AUnit* a_pUser, AUnit* a_pObject ) OVERRIDE;

	public:
		FLAGS m_eFlags = FLAGS_NONE; // 0x04
	};

public:
	AObjectTraits();
	~AObjectTraits();

	//-----------------------------------------------------------------------------
	// AUnitStaticData
	//-----------------------------------------------------------------------------
	virtual TBOOL Create( const PBProperties* a_pProperties ) OVERRIDE; // +0x0c

	FLAGS GetFlags() const { return m_eFlags; }
	TBOOL IsHeavy() const { return m_eFlags & FLAGS_HEAVY; }
	TBOOL IsDetector() const { return m_eFlags & FLAGS_DETECTOR; }
	TBOOL IsAutoPickup() const { return m_eFlags & FLAGS_AUTOPICKUP; }
	TBOOL IsObstruction() const { return m_eFlags & FLAGS_OBSTRUCTION; }

public:
	FLAGS            m_eFlags;                 // 0x0c
	ThrowTrigger*    m_pWhenThrow;             // 0x10
	UseTrigger*      m_pWhenUse;               // 0x14
	AreaTrigger*     m_pWhenArea;              // 0x18
	EquipTrigger*    m_pWhenEquip;             // 0x1c
	BreakTrigger*    m_pWhenBreak;             // 0x20
	Toshi::TVector4  m_vecHoldRotate;          // 0x24
	Toshi::TVector4  m_vecHoldOffset;          // 0x34
	Toshi::TVector4  m_vecItemOffset;          // 0x44
	Toshi::TVector4  m_vecItemFacing;          // 0x54
	Toshi::TVector4  m_vecItemOffsetHud;       // 0x64
	Toshi::TPString8 m_strInventoryForUse;     // 0x74
	Toshi::TPString8 m_strInventoryForStore;   // 0x78
	TFLOAT           m_fHoldScale;             // 0x7c
	TFLOAT           m_fItemScale;             // 0x80
	TFLOAT           m_fRestitution;           // 0x84
	TFLOAT           m_fTangentialRestitution; // 0x88
	TINT             m_iMaxCarry;              // 0x8c
};

TSTATICASSERT( sizeof( AObjectTraits ) == 0x90 );
