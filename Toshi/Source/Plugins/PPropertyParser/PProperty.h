#pragma once
#include "PProperties.h"
#include "PPropertyName.h"
#include "PPropertyValue.h"

#include <Toshi/TFreeList.h>

//-----------------------------------------------------------------------------
// Purpose: A named value of PProperties.
//-----------------------------------------------------------------------------
class PProperty
    : public Toshi::TDList<PProperty>::TNode
{
public:
	TDECLARE_FREELIST_ALLOCATOR( PProperty );

public:
	PProperty( const PPropertyName& a_rcName, const PPropertyValue& a_rcValue );
	~PProperty();

	// NOTE: always inlined in retail, the out of line copy is unreferenced
	// $Barnyard: FUNCTION 006b29b0
	PProperty( const PPropertyName& a_rcName, const PPropertyValue& a_rcValue, const Toshi::TPString8& a_rcComment )
	    : m_oName( a_rcName )
	    , m_oValue( a_rcValue )
	    , m_strComment( a_rcComment )
	{}

	const PPropertyName&  GetName() const { return m_oName; }
	const PPropertyValue& GetValue() const { return m_oValue; }

	const Toshi::TPString8& GetComment() const { return m_strComment; }

	void SetValue( const PPropertyValue& a_rcValue ) { m_oValue = a_rcValue; }

private:
	PPropertyName    m_oName;      // 0x08
	PPropertyValue   m_oValue;     // 0x10
	Toshi::TPString8 m_strComment; // 0x18
};
