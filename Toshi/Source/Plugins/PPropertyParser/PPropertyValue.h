#pragma once
#include "Toshi/TClass.h"
#include "Toshi/TPString8.h"
#include "Toshi/TRefCounted.h"
#include "PProperties.h"

class PPropertyName;
class PPropertyValueArray;

namespace Toshi
{
class TVector2;
class TVector3;
class TVector4;
class TQuaternion;
} // namespace Toshi

class PPropertyValue
{
public:
	inline static const Toshi::TClass* TYPE_INT        = &TGetClass( PProperties::INT );
	inline static const Toshi::TClass* TYPE_UINT32     = &TGetClass( PProperties::UINT32 );
	inline static const Toshi::TClass* TYPE_FLOAT      = &TGetClass( PProperties::FLOAT );
	inline static const Toshi::TClass* TYPE_BOOL       = &TGetClass( PProperties::BOOL );
	inline static const Toshi::TClass* TYPE_TPSTRING8  = &TGetClass( PProperties::TPString8 );
	inline static const Toshi::TClass* TYPE_STRING16   = &TGetClass( PProperties::String16 );
	inline static const Toshi::TClass* TYPE_STRING16A  = &TGetClass( PProperties::String16A );
	inline static const Toshi::TClass* TYPE_LOCALEID   = &TGetClass( PProperties::LocaleID );
	inline static const Toshi::TClass* TYPE_PROPS      = &TGetClass( PProperties );
	inline static const Toshi::TClass* TYPE_PROPNAME   = &TGetClass( PProperties::PPropertyName );
	inline static const Toshi::TClass* TYPE_ARRAY      = &TGetClass( PProperties::PPropertyValueArray );
	inline static const Toshi::TClass* TYPE_MANAGEDPTR = &TGetClass( PProperties::TManagedPtr );
	inline static const Toshi::TClass* TYPE_UNDEF      = TNULL;

public:
	PPropertyValue();
	PPropertyValue( TINT a_iValue );
	PPropertyValue( TUINT32 a_uiValue );
	PPropertyValue( TFLOAT a_fValue );
	PPropertyValue( TBOOL a_bValue );
	PPropertyValue( TWCHAR* a_wszValue, TBOOL a_bOwned );
	PPropertyValue( PProperties* a_pProperties );
	PPropertyValue( const PPropertyName& a_rcName );
	PPropertyValue( const Toshi::TPString8& a_rcValue );
	PPropertyValue( PPropertyValueArray* a_pArray );
	PPropertyValue( Toshi::TObject* a_pObject );
	PPropertyValue( const PPropertyValue& a_rcOther );
	~PPropertyValue();

	//-----------------------------------------------------------------------------
	// NOTE: everything in this block is unreferenced in retail. The PPropertyParser
	// library was built without function-level linking, so its unused overloads were
	// kept in the exe. Names from JPOG's PPropertyParser.dll where it has them,
	// otherwise inferred from the code and the address order.
	//-----------------------------------------------------------------------------
	PPropertyValue( TINT8 a_iValue );
	PPropertyValue( TUINT8 a_uiValue );
	PPropertyValue( const Toshi::TVector2& a_rcValue );
	PPropertyValue( const Toshi::TVector3& a_rcValue );
	PPropertyValue( const Toshi::TVector4& a_rcValue );

	PPropertyValue& operator=( TINT8 a_iValue );
	PPropertyValue& operator=( TUINT8 a_uiValue );
	PPropertyValue& operator=( const TWCHAR* a_wszValue );
	PPropertyValue& operator=( PProperties* a_pProperties );
	PPropertyValue& operator=( const PPropertyName& a_rcName );
	PPropertyValue& operator=( PPropertyValueArray* a_pArray );

	TBOOL operator==( TINT a_iValue ) const;
	TBOOL operator==( TUINT32 a_uiValue ) const;
	TBOOL operator==( TINT8 a_iValue ) const;
	TBOOL operator==( TUINT8 a_uiValue ) const;
	TBOOL operator==( TFLOAT a_fValue ) const;
	TBOOL operator==( TBOOL a_bValue ) const;
	TBOOL operator==( const TWCHAR* a_wszValue ) const;
	TBOOL operator==( const PPropertyName& a_rcName ) const;
	TBOOL operator==( const PProperties* a_pProperties ) const;
	TBOOL operator==( const PPropertyValueArray* a_pArray ) const;
	TBOOL operator==( const Toshi::TPString8& a_rcValue ) const;
	TBOOL operator==( const TCHAR* a_szValue ) const;

	// Returns TFALSE if the value is undefined
	TBOOL IsDefined() const;

	// Returns TTRUE if the value holds nothing: an empty string, no properties, an empty array...
	// Numbers and booleans are never empty.
	TBOOL IsEmpty() const;

	TBOOL GetValue( TINT8& a_rOut ) const;
	TBOOL GetValue( TUINT8& a_rOut ) const;

	// Returns the name of a_pType or "TNULL"
	static const TCHAR* ToString( const Toshi::TClass* a_pType );

	// NOTE: same code as GetString16() const
	TWCHAR* GetString16();

	TINT                 GetLocaleID() const;
	const PPropertyName& GetPropertyName() const;
	PPropertyValueArray* GetArray() const;

	void SetString16( TWCHAR* a_wszValue );
	void SetString16A( TWCHAR* a_wszValue );
	void SetLocaleID( TINT a_iLocaleID );
	void SetProperties( PProperties* a_pProperties );
	void SetPropertyName( const PPropertyName& a_rcName );

	// NOTE: same code as SetTObject( const TClass*, TObject* ), the name is inferred
	void SetManagedPtr( const Toshi::TClass* a_pType, void* a_pData );

	PPropertyValue& operator=( const PPropertyValue& a_rcOther );
	PPropertyValue& operator=( TINT a_iValue );
	PPropertyValue& operator=( TUINT32 a_uiValue );
	PPropertyValue& operator=( TFLOAT a_fValue );
	PPropertyValue& operator=( const Toshi::TPString8& a_rcValue );
	PPropertyValue& operator=( const Toshi::TVector2& a_rcValue );
	PPropertyValue& operator=( const Toshi::TVector3& a_rcValue );
	PPropertyValue& operator=( const Toshi::TVector4& a_rcValue );

	// NOTE: not in retail, keeps bools from being promoted to operator=( TINT )
	template <typename T>
	    requires std::is_same_v<T, TBOOL>
	PPropertyValue& operator=( T a_bValue )
	{
		SetBoolean( a_bValue );
		return *this;
	}

	TBOOL operator==( const PPropertyValue& a_rcOther ) const;
	TBOOL operator!=( const PPropertyValue& a_rcOther ) const;
	TBOOL operator<=( const PPropertyValue& a_rcOther ) const;
	TBOOL operator<( const PPropertyValue& a_rcOther ) const;
	TBOOL operator>=( const PPropertyValue& a_rcOther ) const;
	TBOOL operator>( const PPropertyValue& a_rcOther ) const;
	TBOOL operator==( const Toshi::TObject* a_pObject ) const;

	TBOOL CanBeType( const Toshi::TClass* a_pType ) const;

	// $Barnyard: FUNCTION 006af430
	TBOOL IsType( const Toshi::TClass* a_pType ) const { return m_pType == a_pType; }

	// Convert the value to the type of a_rOut, return TFALSE if it can't be converted (see CanBeType)
	TBOOL GetValue( TINT& a_rOut ) const;
	TBOOL GetValue( TUINT32& a_rOut ) const;
	TBOOL GetValue( TFLOAT& a_rOut ) const;
	TBOOL GetValue( TBOOL& a_rOut ) const;
	TBOOL GetValue( const TWCHAR*& a_rOut ) const;
	TBOOL GetValue( Toshi::TPString8& a_rOut ) const;

	// Returns a printable representation of the value
	Toshi::TString8 ToString() const;

	const Toshi::TClass*    GetType() const;
	Toshi::TObject*         GetTObject() const;
	TBOOL                   GetBoolean() const;
	TINT                    GetInteger() const;
	TUINT32                 GetUINT32() const;
	TFLOAT                  GetFloat() const;
	const TWCHAR*           GetString16() const;
	PProperties*            GetProperties() const;
	Toshi::TPString8&       GetTPString8();
	const Toshi::TPString8& GetTPString8() const;

	// Fill a_rOut from an array of numbers, return TFALSE if the value isn't such an array
	TBOOL GetTVector2( Toshi::TVector2& a_rOut ) const;
	TBOOL GetTVector3( Toshi::TVector3& a_rOut ) const;
	TBOOL GetTVector4( Toshi::TVector4& a_rOut ) const;
	TBOOL GetTQuaternion( Toshi::TQuaternion& a_rOut ) const;

	// Returns the raw storage of the value as an int (no type check)
	// $Barnyard: FUNCTION 006af680
	TINT& GetIntegerRef() { return m_uValue.iValue; }

	// NOTE: 006af680..006af6b0 are four unreferenced copies of the same accessor placed right after
	// GetInteger/GetUINT32/GetFloat/GetBoolean, the names are inferred from that order
	// $Barnyard: FUNCTION 006af690
	TUINT32& GetUINT32Ref() { return m_uValue.uiValue; }
	// $Barnyard: FUNCTION 006af6a0
	TFLOAT& GetFloatRef() { return m_uValue.fValue; }
	// $Barnyard: FUNCTION 006af6b0
	TBOOL& GetBooleanRef() { return m_uValue.bValue; }

	void SetUndef();
	void SetTObject( Toshi::TObject* a_pObject );
	void SetBoolean( TBOOL a_bValue );
	void SetInteger( TINT a_iValue );
	void SetUINT32( TUINT32 a_uiValue );
	void SetFloat( TFLOAT a_fValue );
	void SetTPString8( const Toshi::TPString8& a_rValue );
	void SetArray( PPropertyValueArray* a_pArray );

	// Changes the type to a_pType (a TObject class) and keeps a_pObject
	void SetTObject( const Toshi::TClass* a_pType, Toshi::TObject* a_pObject );

protected:
	TBOOL ChangeType( const Toshi::TClass* a_pType );
	void  Assign( const PPropertyValue& a_rcOther );

	// Releases the value of the current type (shared by ChangeType and the destructor)
	void ReleaseValue();

	// NOTE: unreferenced in retail (names from JPOG's PPropertyParser.dll)
	// $Barnyard: FUNCTION 006af780
	Toshi::TRef<PProperties>& GetPropertiesMP() { return *TREINTERPRETCAST( Toshi::TRef<PProperties>*, &m_uValue ); }
	// $Barnyard: FUNCTION 006af790
	Toshi::TRef<PPropertyValueArray>& GetPropArrayMP() { return *TREINTERPRETCAST( Toshi::TRef<PPropertyValueArray>*, &m_uValue ); }

	static TBOOL IsManagedPtr( const Toshi::TClass* a_pType ) { return a_pType != TNULL && a_pType->IsA( TYPE_MANAGEDPTR ); }

private:
	const Toshi::TClass* m_pType; // 0x0

	union
	{
		TINT                 iValue;
		TUINT32              uiValue;
		TFLOAT               fValue;
		TBOOL                bValue;
		Toshi::TObject*      pObject;
		void*                pData;
		TWCHAR*              wszValue;
		PProperties*         pProperties;
		PPropertyName*       pName;
		PPropertyValueArray* pArray;
		// TManagedPtr types: ref counted block (refcount at +0x0)
		Toshi::TRefCounted* pManaged;
		// NOTE: TYPE_TPSTRING8 keeps a Toshi::TPString8 (one TPooledString8*) in place.
		Toshi::TPooledString8* pPooledString;
	} m_uValue; // 0x4
};
