#pragma once
#include "PPropertyValue.h"

#include <Toshi/TRefCounted.h>
#include <Math/TMathInline.h>

//-----------------------------------------------------------------------------
// Ref-counted array of PPropertyValue (TClass tag: PProperties::PPropertyValueArray).
// Also used as the argument list passed to AFunction::Call.
//-----------------------------------------------------------------------------
class PPropertyValueArray
    : public Toshi::TRefCounted
{
public:
	// NOTE: inlined everywhere, retail zeroes the whole array (grow size included)
	PPropertyValueArray()
	    : m_iGrowSize( 0 )
	    , m_iNumValues( 0 )
	    , m_iNumAllocValues( 0 )
	    , m_pValues( TNULL )
	{}

	PPropertyValueArray( TINT a_iSize );
	PPropertyValueArray( const PPropertyValue* a_pValues, TINT a_iNumValues );
	~PPropertyValueArray();

	// Checks if the array consists of exactly N numbers
	TBOOL IsTVector2() const;
	TBOOL IsTVector3() const;
	TBOOL IsTVector4() const;
	TBOOL IsTQuaternion() const;

	// Fill a_rOut if the array consists of numbers (see the cpp for accepted sizes)
	TBOOL GetTVector2( Toshi::TVector2& a_rOut ) const;
	TBOOL GetTVector3( Toshi::TVector3& a_rOut ) const;
	TBOOL GetTVector4( Toshi::TVector4& a_rOut ) const;
	TBOOL GetTQuaternion( Toshi::TQuaternion& a_rOut ) const;

	// Checks if a_iCount values (or any number if -1) can be converted to a_pType
	// NOTE: inlined in retail, the out of line copy is unreferenced
	// $Barnyard: FUNCTION 006afb30
	TBOOL CanBeTypeArray( const Toshi::TClass* a_pType, TINT a_iCount ) const
	{
		if ( a_iCount != -1 && m_iNumValues != a_iCount )
			return TFALSE;

		for ( TINT i = 0; i < m_iNumValues; i++ )
		{
			if ( !m_pValues[ i ].CanBeType( a_pType ) )
				return TFALSE;
		}

		return TTRUE;
	}

	// Changes the capacity of the array, the values that fit are kept
	void Resize( TINT a_iNewSize );

	// NOTE: inlined in retail (see PBPropertyReader::ToValueArray)
	// $Barnyard: FUNCTION 00596a90
	PPropertyValue* Push( const PPropertyValue& a_rcValue )
	{
		if ( m_iNumAllocValues < m_iNumValues + 1 )
			Resize( Toshi::TMath::Max( m_iGrowSize + m_iNumAllocValues, m_iNumValues + 1 ) );

		m_iNumValues++;
		m_pValues[ m_iNumValues - 1 ] = a_rcValue;

		return &m_pValues[ m_iNumValues - 1 ];
	}

	// NOTE: inlined TArray<PPropertyValue>::SetSize, the capacity only grows
	void SetSize( TINT a_iSize )
	{
		if ( a_iSize > m_iNumAllocValues )
			Resize( a_iSize );

		m_iNumValues = a_iSize;
	}

	// NOTE: inlined TArray<PPropertyValue>::Clear, keeps the allocated values
	void Clear()
	{
		if ( m_iNumAllocValues < 0 )
			Resize( 0 );

		m_iNumValues = 0;
	}

	// Releases the array once it's not referenced anymore (used by Toshi::TRef)
	void Delete() { delete this; }

	TINT GetSize() const { return m_iNumValues; }

	PPropertyValue*       GetValue( TINT a_iIndex ) { return &m_pValues[ a_iIndex ]; }
	const PPropertyValue* GetValue( TINT a_iIndex ) const { return &m_pValues[ a_iIndex ]; }

private:
	// NOTE: retail inherits these from a non-POD TArray<PPropertyValue> base at +0x4 (ctor and Resize,
	// reproduced by PPropertyValueArray::PPropertyValueArray and PPropertyValueArray::Resize). Toshi::TArray only handles POD types, so they live here.
	TINT            m_iGrowSize;       // 0x4
	TINT            m_iNumValues;      // 0x8
	TINT            m_iNumAllocValues; // 0xC
	PPropertyValue* m_pValues;         // 0x10 (allocated with new[])
};
