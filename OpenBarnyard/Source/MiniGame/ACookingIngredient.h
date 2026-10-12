#pragma once
#include <Toshi/TPString8.h>

class AObjectTraits;
class AAnimatableObject;

//-----------------------------------------------------------------------------
// Purpose: Fixed size list of the cooking data (count, then every element
// constructed). Clearing assigns default elements.
// NOTE: the name is invented.
//-----------------------------------------------------------------------------
template <class T, TINT MAX_SIZE>
class ACookingList
{
public:
	ACookingList()
	    : m_iNumItems( 0 )
	{}

	// $Barnyard: FUNCTION 0044a560
	// $Barnyard: FUNCTION 0044be80
	void Clear()
	{
		for ( TINT i = 0; i < m_iNumItems; i++ )
			m_aItems[ i ] = T();

		m_iNumItems = 0;
	}

	// $Barnyard: FUNCTION 005d6440
	ACookingList& operator=( const ACookingList& a_rcOther )
	{
		TINT i = 0;

		for ( ; i < a_rcOther.m_iNumItems; i++ )
			m_aItems[ i ] = a_rcOther.m_aItems[ i ];

		for ( ; i < m_iNumItems; i++ )
			m_aItems[ i ] = T();

		m_iNumItems = a_rcOther.m_iNumItems;
		return *this;
	}

	// Removes the last item and returns it
	T PopBack()
	{
		T item                    = m_aItems[ m_iNumItems - 1 ];
		m_aItems[ --m_iNumItems ] = T();

		return item;
	}

	TINT PushBack( const T& a_rcItem )
	{
		TINT iIndex             = m_iNumItems;
		m_aItems[ m_iNumItems ] = a_rcItem;
		m_iNumItems++;

		return iIndex;
	}

	TINT     Size() const { return m_iNumItems; }
	T&       operator[]( TINT a_iIndex ) { return m_aItems[ a_iIndex ]; }
	const T& operator[]( TINT a_iIndex ) const { return m_aItems[ a_iIndex ]; }

private:
	TINT m_iNumItems;          // 0x00
	T    m_aItems[ MAX_SIZE ]; // 0x04
};

//-----------------------------------------------------------------------------
// Purpose: An ingredient of the cooking minigame (cooking.AllIngredients),
// with the animatable object of its model.
// NOTE: the name is invented. Used by ACookingMiniGame and ARecipeDisplayState.
//-----------------------------------------------------------------------------
class ACookingIngredient
{
public:
	// $Barnyard: FUNCTION 004ffbb0
	ACookingIngredient()
	    : m_fTime( 0.0f )
	    , m_pTraits( TNULL )
	    , m_wszScreenName( TNULL )
	    , m_iCount( 0 )
	    , m_pObject( TNULL )
	    , m_fScale( 1.0f )
	    , m_bCentred( TFALSE )
	    , m_iUnk24( 3 )
	{}

	// $Barnyard: FUNCTION 004ffbe0
	~ACookingIngredient() {}

	// Creates the animatable object of the unit type a_szName
	void Create( const TCHAR* a_szName, TFLOAT a_fScale, TBOOL a_bCentred );

	// Destroys the animatable object
	void Destroy();

	// Returns a new ingredient of the same type (with its own model)
	ACookingIngredient* Clone() const;

	AObjectTraits*          GetTraits() const { return m_pTraits; }
	const Toshi::TPString8& GetName() const { return m_strName; }
	TFLOAT                  GetScale() const { return m_fScale; }
	TBOOL                   IsCentred() const { return m_bCentred; }

	// $Barnyard: FUNCTION 004ffb80
	const TWCHAR* GetScreenName() const { return m_wszScreenName; }

	// NOTE: shared getter 004ffb50 (TD3DAdapter::Mode::GetAdapter)
	AAnimatableObject* GetObject() const { return m_pObject; }

	// $Barnyard: FUNCTION 004ffb60
	TINT GetCount() const { return m_iCount; }

	// $Barnyard: FUNCTION 004ffb70
	void IncrementCount() { m_iCount++; }

	// NOTE: shared setter 0064e660
	void SetCount( TINT a_iCount ) { m_iCount = a_iCount; }

	// NOTE: shared setter 0066a1c0
	void SetScreenName( const TWCHAR* a_wszScreenName ) { m_wszScreenName = a_wszScreenName; }

	TFLOAT GetTime() const { return m_fTime; }
	void   SetTime( TFLOAT a_fTime ) { m_fTime = a_fTime; }
	void   AddTime( TFLOAT a_fTime ) { m_fTime += a_fTime; }

	void SetUnk24( TINT a_iValue ) { m_iUnk24 = a_iValue; }

private:
	TFLOAT             m_fTime;         // 0x00, spins the model while it's in the pan of ACookingMiniGame
	AObjectTraits*     m_pTraits;       // 0x04
	const TWCHAR*      m_wszScreenName; // 0x08
	TINT               m_iCount;        // 0x0c, ACookingMiniGame: how many the player has left
	AAnimatableObject* m_pObject;       // 0x10
	Toshi::TPString8   m_strName;       // 0x14, the unit type
	TFLOAT             m_fScale;        // 0x18
	TBOOL              m_bCentred;      // 0x1c, the model is centred on its bounding sphere when shown
	TINT               m_iUnk20;        // 0x20
	TINT               m_iUnk24;        // 0x24
};

TSTATICASSERT( sizeof( ACookingIngredient ) == 0x28 );
