#pragma once
#include "TGUIPropertySpec.h"

#include <Toshi/TArray.h>
#include <Toshi/TPString8.h>

class PProperties;
class PPropertyName;
class PPropertyValue;

TOSHI_NAMESPACE_START

//-----------------------------------------------------------------------------
// Purpose: base of the GUI objects that are created from the property blocks
// of the definition files ("type = <class name>;" selects the class). The
// properties described by the property sets of the class (TGUIPropertySpec)
// are read and written automatically, derived classes read the others.
//
// NOTE: follows Nicktoons: no SetInterface/GetInterface virtuals (JPOG has
// them), errors are reported to the TGUIManager singleton.
//-----------------------------------------------------------------------------
class TGUIDeserialisable : public TObject
{
public:
	TDECLARE_CLASS( TGUIDeserialisable, TObject );

	using PropertySet = TArray<TGUIPropertySpec*>;

public:
	TGUIDeserialisable();
	TGUIDeserialisable( const TGUIDeserialisable& a_rcOther );
	~TGUIDeserialisable();

	// Sets the auto installed properties from a_pProperties
	virtual TBOOL Deserialise( const PProperties* a_pProperties );

	// Writes the type and the auto installed properties to a_pProperties
	virtual TBOOL Serialise( PProperties* a_pProperties );

	virtual TINT                    GetPropertyCount() const;
	virtual const TGUIPropertySpec* FindPropertyInfo( const TPString8& a_rcName ) const;
	virtual const TGUIPropertySpec* GetPropertyInfo( TINT a_iIndex ) const;
	virtual TBOOL                   ValidatePropertyInfo( const TGUIPropertySpec* a_pSpec ) const;

	// The specs of the properties of the class, in declaration order
	virtual PropertySet* GetPropertyInfoSet() const = 0;

	// The same specs ordered by name (TGUIPropertySpec::Comparator)
	virtual PropertySet* GetPropertyInfoOrderedSet() const = 0;

	TBOOL SetProperty( const TGUIPropertySpec* a_pSpec, const PPropertyValue& a_rcValue );
	TBOOL SetProperty( const TPString8& a_rcName, const PPropertyValue& a_rcValue );
	TBOOL SetProperty( const PPropertyName& a_rcName, const PPropertyValue& a_rcValue );

	PPropertyValue GetProperty( const TGUIPropertySpec* a_pSpec, TBOOL& a_rbFound ) const;
	PPropertyValue GetProperty( const TPString8& a_rcName, TBOOL& a_rbFound ) const;
	PPropertyValue GetProperty( const PPropertyName& a_rcName, TBOOL& a_rbFound ) const;

	TBOOL ValidateProperty( const TPString8& a_rcName, const PPropertyValue& a_rcValue ) const;
	TBOOL ValidateProperty( const PPropertyName& a_rcName, const PPropertyValue& a_rcValue ) const;

	// Return the property if it exists and has the right type, report an error otherwise
	const PPropertyValue* GetRequiredProperty( const PProperties* a_pProperties, const PPropertyName& a_rcName, const TClass* a_pType ) const;
	const PPropertyValue* GetRequiredProperty( const PProperties* a_pProperties, const TPString8& a_rcName, const TClass* a_pType ) const;
	const PPropertyValue* GetRequiredProperty( const PProperties* a_pProperties, const TPString8& a_rcName, const TPString8& a_rcSubName, const TClass* a_pType ) const;

	// Return the property if it exists and has the right type, TNULL if it doesn't exist (an error is reported if the type is wrong)
	const PPropertyValue* GetOptionalProperty( const PProperties* a_pProperties, const PPropertyName& a_rcName, const TClass* a_pType ) const;
	const PPropertyValue* GetOptionalProperty( const PProperties* a_pProperties, const TPString8& a_rcName, const TClass* a_pType ) const;
	const PPropertyValue* GetOptionalProperty( const PProperties* a_pProperties, const TPString8& a_rcName, const TPString8& a_rcSubName, const TClass* a_pType ) const;

	// Creates the object of the class named by the "type" property (it isn't deserialised)
	static TGUIDeserialisable* Create( const PProperties* a_pProperties );

	// Fills the property sets of a class with clones of a_rcSpecs
	// NOTE: the name is unknown, Nicktoons uses it in the static initialisers of the classes
	static void CreatePropertySets( PropertySet*& a_rpSet, PropertySet*& a_rpOrderedSet, const PropertySet& a_rcSpecs );

	// Adds a spec to the property sets (the ordered set keeps the specs with the same name in their order)
	// NOTE: the name is unknown, inlined in CreatePropertySets and in the static initialisers of the classes
	static void AddPropertySpec( PropertySet* a_pSet, PropertySet* a_pOrderedSet, TGUIPropertySpec* a_pSpec );

	// Deletes the specs and the property sets created by CreatePropertySets
	// NOTE: the name is unknown, inlined in the static deinitialisers of the classes
	static void DestroyPropertySets( PropertySet*& a_rpSet, PropertySet*& a_rpOrderedSet );

public:
	// NOTE: created by InitialiseClass (see TGUIFunction)
	static TPString8 ms_strType;
};

TOSHI_NAMESPACE_END
