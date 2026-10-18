#pragma once
#include <Toshi/TPString8.h>

class PPropertyValue;

TOSHI_NAMESPACE_START

class TGUIDeserialisable;

//-----------------------------------------------------------------------------
// Purpose: describes a property of a TGUIDeserialisable class: its name, the
// type of its value and how it's read and written (see TGUIPropertySpecSimple).
// Properties are either plain values, enumerations (a name of a list of names
// in the file, the index as the value) or names of classes derived from a base
// class.
//
// NOTE: the vtable pointer is after the fields in Nicktoons (GCC); Nicktoons
// has an additional string after the type (m_strUnknown), the constructors
// take it after the type (JPOG doesn't have it).
//-----------------------------------------------------------------------------
class TGUIPropertySpec
{
public:
	enum Kind : TINT
	{
		Kind_Value      = 0,
		Kind_Enumerated = 1,
		Kind_ClassName  = 2,
	};

	// Orders the specs by their names (string comparison)
	struct Comparator
	{
		TINT operator()( const TGUIPropertySpec* a_pA, const TGUIPropertySpec* a_pB ) const;
	};

public:
	TGUIPropertySpec( const TPString8& a_rcName );
	TGUIPropertySpec( const TPString8& a_rcName, const TClass* a_pType, const TPString8& a_rcUnknown, TBOOL a_bAutoInstall, const TClass* a_pSourceClass, TINT a_iSortHint );
	TGUIPropertySpec( const TPString8& a_rcName, const TClass* a_pType, const TPString8& a_rcUnknown, TBOOL a_bAutoInstall, const TClass* a_pSourceClass, TINT a_iSortHint, const TClass* a_pBaseClass );
	TGUIPropertySpec( const TPString8& a_rcName, const TClass* a_pType, const TPString8& a_rcUnknown, TBOOL a_bAutoInstall, const TClass* a_pSourceClass, TINT a_iSortHint, TINT a_iEnumCount, const TCHAR* const* a_ppEnumNames );
	TGUIPropertySpec( const TGUIPropertySpec& a_rcOther );
	virtual ~TGUIPropertySpec();

	virtual TGUIPropertySpec* Clone() const = 0;

	// Sets the property of a_pObject to a_rcValue
	virtual void set( TGUIDeserialisable* a_pObject, const PPropertyValue& a_rcValue ) const = 0;

	// Returns the property of a_pObject
	virtual PPropertyValue get( const TGUIDeserialisable* a_pObject ) const = 0;

	// Checks if a_rcValue is valid for the property of a_pObject
	virtual TBOOL validate( const TGUIDeserialisable* a_pObject, const PPropertyValue& a_rcValue ) const = 0;

	TGUIPropertySpec& operator=( const TGUIPropertySpec& a_rcOther );

	// $JPOG: FUNCTION TGuiInterface.dll+00008580
	TBOOL operator==( const TGUIPropertySpec& a_rcOther ) const { return m_strName == a_rcOther.m_strName; }

	// Returns the index of the enumeration name a_rcName or -1
	TINT GetEnumValue( const TPString8& a_rcName ) const;

	// $JPOG: FUNCTION TGuiInterface.dll+00008690
	const TPString8& GetName() const { return m_strName; }

	// $JPOG: FUNCTION TGuiInterface.dll+00008650
	void SetName( const TPString8& a_rcName ) { m_strName = a_rcName; }

	// $JPOG: FUNCTION TGuiInterface.dll+00008640
	const TClass* GetType() const { return m_pType; }

	// $JPOG: FUNCTION TGuiInterface.dll+00008630
	void SetType( const TClass* a_pType ) { m_pType = a_pType; }

	// $JPOG: FUNCTION TGuiInterface.dll+00008620
	TBOOL AllowsAutoInstall() const { return m_bAutoInstall; }

	// $JPOG: FUNCTION TGuiInterface.dll+000085a0
	const TClass* GetSourceClass() const { return m_pSourceClass; }

	// $JPOG: FUNCTION TGuiInterface.dll+000085b0
	TINT GetSortHint() const { return m_iSortHint; }

	// $JPOG: FUNCTION TGuiInterface.dll+00008600
	TBOOL IsEnumerated() const { return m_eKind == Kind_Enumerated && m_iEnumCount > 0; }

	// $JPOG: FUNCTION TGuiInterface.dll+000085f0
	TINT GetEnumCount() const { return ( m_eKind == Kind_Enumerated ) ? m_iEnumCount : 0; }

	// $JPOG: FUNCTION TGuiInterface.dll+000085e0
	const TPString8& GetEnumName( TINT a_iIndex ) const { return m_pEnumNames[ a_iIndex ]; }

	// $JPOG: FUNCTION TGuiInterface.dll+000085d0
	TBOOL IsClassName() const { return m_eKind == Kind_ClassName; }

	// $JPOG: FUNCTION TGuiInterface.dll+000085c0
	const TClass* GetBaseClass() const { return m_pBaseClass; }

	const TPString8& GetUnknown() const { return m_strUnknown; }

private:
	void CopyEnumNames( const TPString8* a_pEnumNames );
	void DeleteEnumNames();

protected:
	const TClass* m_pType;        // 0x00 type of the values (PPropertyValue::TYPE_*)
	TPString8     m_strName;      // 0x04
	TPString8     m_strUnknown;   // 0x08 NOTE: not in JPOG, the name is unknown
	TBOOL         m_bAutoInstall; // 0x0C the property is read/written by TGUIDeserialisable::Deserialise/Serialise
	const TClass* m_pSourceClass; // 0x10 class that has the property
	TINT          m_iSortHint;    // 0x14
	Kind          m_eKind;        // 0x18

	union
	{
		TINT          m_iEnumCount; // 0x1C Kind_Enumerated
		const TClass* m_pBaseClass; // 0x1C Kind_ClassName
	};

	TPString8* m_pEnumNames; // 0x20 Kind_Enumerated
};

TOSHI_NAMESPACE_END
