#pragma once

TOSHI_NAMESPACE_START

class TOSHI_API TBitArray
{
public:
	// Constructors/destructor
	constexpr TBitArray() :
	    m_pBuffer( TNULL ),
	    m_uiInitialisedWith( 0 ),
	    m_iSize( 0 )
	{}

	TBitArray( TINT a_iNumBits, TINT a_iInitialiseWith );

	~TBitArray();

	// Allocates memory for the specified number of bits
	void Create( TINT a_iNumBits, TINT a_iInitialiseWith );

	// Frees the buffer
	void Destroy();

	// Resets all the bits to their initial state
	void Reset();

	// Sets a bit at a_iIndex to a_iValue
	void SetBit( TINT a_iIndex, TINT a_iValue );

	// Sets a bit at a_iIndex to 1
	void SetBit( TINT a_iIndex );

	// Toggles a bit at a_iIndex
	void ToggleBit( TINT a_iIndex );

	// Sets a bit at a_iIndex to 0
	void ClearBit( TINT a_iIndex );

	// Returns 1 if a bit at a_iIndex is set or zero if it's not set
	TINT GetBit( TINT a_iIndex ) const;

	// Returns index of the first set (1) bit
	TINT FindFirstSetBit();

	// Returns index of the first clear (0) bit
	TINT FindFirstClearBit();

	// Returns number of bytes allocated for the buffer
	TINT GetNumAllocatedBytes() const;

private:
	TUINT32* m_pBuffer;
	TINT     m_iSize;
	TUINT    m_uiInitialisedWith;
};

TOSHI_NAMESPACE_END
