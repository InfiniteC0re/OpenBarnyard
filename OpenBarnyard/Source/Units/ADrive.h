#pragma once
#include <Toshi/TObject.h>
#include <Toshi/TPString8.h>
#include <Plugins/PPropertyParser/PBProperties.h>

class AUnit;
class ADriveFunction;
struct AInteractionActivityData;

//-----------------------------------------------------------------------------
// One "drive" block of an ADriveGroup: something a unit may want to do. Its
// value mixes the "need" and the "avail" functions (weighted by "needfactor"
// and "availfactor") and is then passed through the "filter" function. The
// name of the drive is the FSM that AUnitBehaviour runs when it wins, except
// for the "suggestion", "command" and "schedule" drives.
//
// TClass 0x00783ee8 (version 0.1, size 0x1c, not creatable), vtable 0x007247f0.
//-----------------------------------------------------------------------------
class ADrive
    : public Toshi::TObject
{
public:
	TDECLARE_CLASS( ADrive, Toshi::TObject );

	// NOTE: Returned by value from Evaluate.
	struct Rating
	{
		ADrive* pDrive; // 0x00
		TFLOAT  fValue; // 0x04
	};

public:
	ADrive();
	~ADrive();

	// Reads the "drive" block, the functions reserve their unit data in a_pDataSize.
	TBOOL Create( const PBProperties* a_pProperties, TINT* a_pDataSize );

	// Lets the functions initialise their data of a new unit.
	void InitData( AUnit* a_pUnit, AInteractionActivityData* a_pData );

	// Computes how much the unit wants this drive.
	Rating Evaluate( AUnit* a_pUnit, AInteractionActivityData* a_pData );

	const Toshi::TPString8& GetName() const { return m_strName; }

private:
	Toshi::TPString8 m_strName;      // 0x04, "name"
	TFLOAT           m_fNeedFactor;  // 0x08, "needfactor"
	TFLOAT           m_fAvailFactor; // 0x0c, "availfactor"
	ADriveFunction*  m_pNeed;        // 0x10, "need"
	ADriveFunction*  m_pAvail;       // 0x14, "avail"
	ADriveFunction*  m_pFilter;      // 0x18, "filter"
};

TSTATICASSERT( sizeof( ADrive ) == 0x1c );
