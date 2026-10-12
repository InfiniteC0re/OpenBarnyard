#pragma once
#include "Toshi/TPString8.h"

//-----------------------------------------------------------------------------
// Name of a property with an optional sub name (TClass tag: PProperties::PPropertyName).
//-----------------------------------------------------------------------------
class PPropertyName
{
public:
	PPropertyName();
	PPropertyName( const Toshi::TPString8& a_rcName );
	PPropertyName( const Toshi::TPString8& a_rcName, const Toshi::TPString8& a_rcSubName );
	PPropertyName( const PPropertyName& a_rcOther );
	~PPropertyName();

	PPropertyName& operator=( const PPropertyName& a_rcOther );
	TBOOL          operator==( const PPropertyName& a_rcOther ) const;
	TBOOL          operator!=( const PPropertyName& a_rcOther ) const;
	TBOOL          operator<( const PPropertyName& a_rcOther ) const;
	TBOOL          operator>( const PPropertyName& a_rcOther ) const;

	// Returns "name" or "name[subname]"
	Toshi::TPString8 GetString() const;

protected:
	// Splits "name[subname]" into the name and the sub name
	void Split( const Toshi::TPString8& a_rcString );

public:
	const Toshi::TPString8& GetName() const { return m_strName; }
	const Toshi::TPString8& GetSubName() const { return m_strSubName; }

private:
	Toshi::TPString8 m_strName;    // 0x0
	Toshi::TPString8 m_strSubName; // 0x4
};

// Appends the name and the sub name in brackets ("name[subname]")
Toshi::TString8 operator+( const Toshi::TString8& a_rcString, const PPropertyName& a_rcName );
