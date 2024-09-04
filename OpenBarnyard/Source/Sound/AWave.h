#pragma once

TDECLARE_POINTER_HANDLE( AWaveSampleHandle );

struct AWave
{
	constexpr AWave() :
	    iFlags( 0 ),
	    iFrequency( 22050 ),
	    fLength( 0.0f ),
	    pSampleHandle( TNULL ),
	    iIndex( -1 )
	{}

	constexpr AWave( TINT a_iFlags, TINT a_iFrequency, TFLOAT a_fLength, AWaveSampleHandle a_pSampleHandle, TINT a_iIndex ) :
	    iFlags( a_iFlags ),
	    iFrequency( a_iFrequency ),
	    fLength( a_fLength ),
	    pSampleHandle( a_pSampleHandle ),
	    iIndex( a_iIndex )
	{}

	TINT              iFlags;
	TINT              iFrequency;
	TFLOAT            fLength;
	AWaveSampleHandle pSampleHandle;
	TINT              iIndex;
};
