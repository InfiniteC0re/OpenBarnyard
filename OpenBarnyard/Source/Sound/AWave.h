#pragma once

TDECLARE_POINTER_HANDLE( AWaveSampleHandle );

struct AWave
{
	constexpr AWave() :
		uiFlags( 0 ),
		uiFrequency( 22050 ),
		fLength( 0.0f ),
		pSampleHandle( TNULL ),
		iIndex( -1 )
	{}

	constexpr AWave( TUINT a_uiFlags, TUINT a_uiFrequency, TFLOAT a_fLength, AWaveSampleHandle a_pSampleHandle, TINT a_iIndex ) :
		uiFlags( a_uiFlags ),
		uiFrequency( a_uiFrequency ),
		fLength( a_fLength ),
		pSampleHandle( a_pSampleHandle ),
		iIndex( a_iIndex )
	{}

	TUINT			  uiFlags;
	TUINT			  uiFrequency;
	TFLOAT			  fLength;
	AWaveSampleHandle pSampleHandle;
	TINT			  iIndex;
};
