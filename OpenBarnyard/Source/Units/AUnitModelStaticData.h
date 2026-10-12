#pragma once
#include "AUnitStaticData.h"

#include <Toshi/TPString8.h>

// Models of a unit type, read from its "Models" (array) or "Model" (string) property.
// AAnimatedUnit picks one of them by index when creating its animatable object.
//
// TClass 0x0077ea3c (size 0x1c, alignment 4), vtable 0x00718cb8.
class AUnitModelStaticData : public AUnitStaticData
{
public:
	TDECLARE_CLASS( AUnitModelStaticData, AUnitStaticData );

	// Resizable array of model names
	struct ModelNameArray
	{
		ModelNameArray()
		    : iUnk( 0 ), iSize( 0 ), iCapacity( 0 ), pData( TNULL )
		{}

		~ModelNameArray() { delete[] pData; }

		// Reallocates the buffer keeping the first min( a_iCapacity, iSize ) names. 0 frees everything.
		void SetCapacity( TINT a_iCapacity );

		TINT              iUnk;      // 0x0
		TINT              iSize;     // 0x4
		TINT              iCapacity; // 0x8
		Toshi::TPString8* pData;     // 0xc
	};

public:
	AUnitModelStaticData();
	~AUnitModelStaticData();

	//-----------------------------------------------------------------------------
	// AUnitStaticData
	//-----------------------------------------------------------------------------
	virtual TBOOL Create( const PBProperties* a_pProperties ) override; // +0x0c

	TINT                    GetNumModels() const { return m_oModelNames.iSize; }
	const Toshi::TPString8& GetModelName( TINT a_iIndex ) const { return m_oModelNames.pData[ a_iIndex ]; }

private:
	ModelNameArray m_oModelNames; // 0x0c
};

TSTATICASSERT( sizeof( AUnitModelStaticData ) == 0x1c );
