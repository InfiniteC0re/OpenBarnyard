#pragma once
#include <Toshi/TObject.h>
#include <Toshi/TFreeList.h>
#include <Toshi/TRefCounted.h>
#include <Toshi/TDList.h>
#include <Toshi/TPString8.h>

namespace Toshi
{
class TVector2;
class TVector3;
class TVector4;
class TQuaternion;
} // namespace Toshi

class PProperty;
class PPropertyName;
class PPropertyValue;

class PProperties
    : public Toshi::TObject
    , public Toshi::TRefCounted
{
public:
	TDECLARE_CLASS( PProperties, Toshi::TObject );
	TDECLARE_FREELIST_ALLOCATOR( PProperties );

	struct UINT32
	{
		TDECLARE_CLASS_NO_PARENT( UINT32 );
	};

	struct INT
	{
		TDECLARE_CLASS_NO_PARENT( INT );
	};

	struct FLOAT
	{
		TDECLARE_CLASS_NO_PARENT( FLOAT );
	};

	struct BOOL
	{
		TDECLARE_CLASS_NO_PARENT( BOOL );
	};

	struct TPString8
	{
		TDECLARE_CLASS_NO_PARENT( TPString8 );
	};

	struct String16
	{
		TDECLARE_CLASS_NO_PARENT( String16 );
	};

	struct String16A
	{
		TDECLARE_CLASS_NO_PARENT( String16A );
	};

	struct LocaleID
	{
		TDECLARE_CLASS_NO_PARENT( LocaleID );
	};

	struct PPropertyName
	{
		TDECLARE_CLASS_NO_PARENT( PPropertyName );
	};

	struct PPropertyValueArray
	{
		TDECLARE_CLASS_NO_PARENT( PPropertyValueArray );
	};

	// NOTE: retail registers this TClass as "TManagedPtr" (0x007ceffc). Values of a type that
	// IsA TManagedPtr hold a pointer to a ref counted block (refcount at +0x0).
	struct TManagedPtr
	{
		TDECLARE_CLASS_NO_PARENT( TManagedPtr );
	};

public:
	PProperties();
	~PProperties();

	// Deletes all the properties
	void Clear();

	// Adds a_pProperty to the end of the list (the count isn't changed, as in the original)
	void AddPropertyNoCount( PProperty* a_pProperty );

	// Adds a new property to the end of the list, even if one with the same name exists
	void AddProperty( const ::PPropertyName& a_rcName, const PPropertyValue& a_rcValue );
	void AddProperty( const Toshi::TPString8& a_rcName, const PPropertyValue& a_rcValue );
	void AddProperty( const Toshi::TPString8& a_rcName, const Toshi::TPString8& a_rcSubName, const PPropertyValue& a_rcValue );

	// Adds a new property with a comment to the end of the list (a_iUnused is ignored, the wrappers pass -1)
	// NOTE: retail has two copies of this code with the same behaviour, the names are unknown
	void AddCommentedProperty( const ::PPropertyName& a_rcName, const PPropertyValue& a_rcValue, const Toshi::TPString8& a_rcComment, TINT a_iUnused );
	void AddCommentedProperty( const Toshi::TPString8& a_rcName, const PPropertyValue& a_rcValue, const Toshi::TPString8& a_rcComment );
	void AddCommentedProperty( const Toshi::TPString8& a_rcName, const Toshi::TPString8& a_rcSubName, const PPropertyValue& a_rcValue, const Toshi::TPString8& a_rcComment );

	void AddProperty( const ::PPropertyName& a_rcName, const PPropertyValue& a_rcValue, const Toshi::TPString8& a_rcComment );
	void AddProperty( const Toshi::TPString8& a_rcName, const PPropertyValue& a_rcValue, const Toshi::TPString8& a_rcComment );
	void AddProperty( const Toshi::TPString8& a_rcName, const Toshi::TPString8& a_rcSubName, const PPropertyValue& a_rcValue, const Toshi::TPString8& a_rcComment );

	// Returns value of the first property with the name or TNULL if not found
	const PPropertyValue* GetProperty( const ::PPropertyName& a_rcName ) const;
	const PPropertyValue* GetProperty( const Toshi::TPString8& a_rcName ) const;
	const PPropertyValue* GetProperty( const Toshi::TPString8& a_rcName, const Toshi::TPString8& a_rcSubName ) const;
	const PPropertyValue* GetProperty( const Toshi::TPString8& a_rcName, TINT a_iIndex ) const;

	// Searches for a property with the name (sub name is ignored) starting at a_rIt, returns it and moves a_rIt to the next property
	const PProperty* FindProperty( const Toshi::TPString8& a_rcName, Toshi::TDList<PProperty>::Iterator& a_rIt ) const;

	// Changes the value of the property with the same name or adds a new one
	void PutProperty( const ::PPropertyName& a_rcName, const PPropertyValue& a_rcValue );
	void PutProperty( const Toshi::TPString8& a_rcName, const PPropertyValue& a_rcValue );
	void PutProperty( const Toshi::TPString8& a_rcName, const Toshi::TPString8& a_rcSubName, const PPropertyValue& a_rcValue );

	// Sorts the properties by their names (unused in retail)
	void Sort();

	Toshi::TDList<PProperty>& GetProperties() { return m_llProperties; }

	//-----------------------------------------------------------------------------
	// NOTE: everything in this block is unreferenced in retail (the library was built
	// without function-level linking). Names from JPOG's PPropertyParser.dll where it
	// has them. Retail has no asserts in the GetRequired* functions, they're the same
	// code as the GetOptional* ones.
	//-----------------------------------------------------------------------------
	PProperties( PProperties* a_pParent );

	// Return the value of the property if it's of type a_pType, TNULL otherwise
	const PPropertyValue* GetOptionalProperty( const Toshi::TPString8& a_rcName, const Toshi::TClass* a_pType ) const;
	const PPropertyValue* GetOptionalProperty( const Toshi::TPString8& a_rcName, const Toshi::TPString8& a_rcSubName, const Toshi::TClass* a_pType ) const;
	const PPropertyValue* GetOptionalProperty( const Toshi::TPString8& a_rcName, TINT a_iIndex, const Toshi::TClass* a_pType ) const;
	const PPropertyValue* GetRequiredProperty( const Toshi::TPString8& a_rcName, const Toshi::TClass* a_pType ) const;
	const PPropertyValue* GetRequiredProperty( const Toshi::TPString8& a_rcName, const Toshi::TPString8& a_rcSubName, const Toshi::TClass* a_pType ) const;
	const PPropertyValue* GetRequiredProperty( const Toshi::TPString8& a_rcName, TINT a_iIndex, const Toshi::TClass* a_pType ) const;

	// Like FindProperty, but returns the PProperties held by the found property (TNULL if it holds something else)
	PProperties* FindProperties( const Toshi::TPString8& a_rcName, Toshi::TDList<PProperty>::Iterator& a_rIt ) const;

	// Returns the PProperties held by the property a_rcName or TNULL
	PProperties* GetOptionalProperties( const Toshi::TPString8& a_rcName ) const;

	// Convert the value of the property a_rcName to a_rOut, return TFALSE if it doesn't exist or can't be converted
	TBOOL GetOptionalPropertyValue( TBOOL& a_rOut, const Toshi::TPString8& a_rcName ) const;
	TBOOL GetOptionalPropertyValue( TINT& a_rOut, const Toshi::TPString8& a_rcName ) const;
	TBOOL GetOptionalPropertyValue( TUINT32& a_rOut, const Toshi::TPString8& a_rcName ) const;
	TBOOL GetOptionalPropertyValue( TFLOAT& a_rOut, const Toshi::TPString8& a_rcName ) const;
	TBOOL GetOptionalPropertyValue( const TWCHAR*& a_rOut, const Toshi::TPString8& a_rcName ) const;
	TBOOL GetOptionalPropertyValue( Toshi::TPString8& a_rOut, const Toshi::TPString8& a_rcName ) const;
	TBOOL GetOptionalPropertyValue( Toshi::TVector2& a_rOut, const Toshi::TPString8& a_rcName ) const;
	TBOOL GetOptionalPropertyValue( Toshi::TVector3& a_rOut, const Toshi::TPString8& a_rcName ) const;
	TBOOL GetOptionalPropertyValue( Toshi::TVector4& a_rOut, const Toshi::TPString8& a_rcName ) const;
	TBOOL GetOptionalPropertyValue( Toshi::TQuaternion& a_rOut, const Toshi::TPString8& a_rcName ) const;
	TBOOL GetRequiredPropertyValue( TBOOL& a_rOut, const Toshi::TPString8& a_rcName ) const;
	TBOOL GetRequiredPropertyValue( TINT& a_rOut, const Toshi::TPString8& a_rcName ) const;
	TBOOL GetRequiredPropertyValue( TUINT32& a_rOut, const Toshi::TPString8& a_rcName ) const;
	TBOOL GetRequiredPropertyValue( TFLOAT& a_rOut, const Toshi::TPString8& a_rcName ) const;
	TBOOL GetRequiredPropertyValue( const TWCHAR*& a_rOut, const Toshi::TPString8& a_rcName ) const;
	TBOOL GetRequiredPropertyValue( Toshi::TPString8& a_rOut, const Toshi::TPString8& a_rcName ) const;
	TBOOL GetRequiredPropertyValue( Toshi::TVector2& a_rOut, const Toshi::TPString8& a_rcName ) const;
	TBOOL GetRequiredPropertyValue( Toshi::TVector3& a_rOut, const Toshi::TPString8& a_rcName ) const;
	TBOOL GetRequiredPropertyValue( Toshi::TVector4& a_rOut, const Toshi::TPString8& a_rcName ) const;
	TBOOL GetRequiredPropertyValue( Toshi::TQuaternion& a_rOut, const Toshi::TPString8& a_rcName ) const;

	PProperties* GetParentProperties() const { return m_pParent; }

private:
	PProperties*             m_pParent;        // 0x08, TNULL unless created with the unreferenced PProperties( PProperties* )
	Toshi::TDList<PProperty> m_llProperties;   // 0x0c
	TINT                     m_iNumProperties; // 0x14
};
