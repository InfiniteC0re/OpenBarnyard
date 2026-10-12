#pragma once
#include <Toshi/TObject.h>
#include <Toshi/TPString8.h>

class AUnit;

//-----------------------------------------------------------------------------
// A gift held by an AGiftComponent (see AGift_DeprecatedActivity): the name of
// the interaction the receiver gets and the time it was given.
// TClass 0x0078c1a4 (version 2.0, size 0xc, runtime creatable), vtable 0x0072d180.
//-----------------------------------------------------------------------------
class AGift : public Toshi::TObject
{
public:
	TDECLARE_CLASS( AGift, Toshi::TObject );

public:
	AGift();
	AGift( const Toshi::TPString8& a_rcInteraction );
	~AGift();

	// Seconds since the gift was given
	TFLOAT GetAge();

	// Restarts the age of the gift
	void ResetTime();

	// Assigns the interaction of the gift to a_pUnit unless it already has one
	TBOOL AssignInteraction( AUnit* a_pUnit, TFLOAT a_fUnused );

	const Toshi::TPString8& GetInteraction() const { return m_strInteraction; }

private:
	Toshi::TPString8 m_strInteraction; // 0x04
	TFLOAT           m_fTime;          // 0x08, system time the gift was given
};

TSTATICASSERT( sizeof( AGift ) == 0xc );
