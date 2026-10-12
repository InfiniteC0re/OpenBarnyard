#pragma once

TDECLARE_POINTER_HANDLE( AWaveSampleHandle );

struct AWave
{
	using FLAGS = TINT32;
	enum FLAGS_ : FLAGS
	{
		FLAGS_NONE = 0,
		FLAGS_2D   = BITFLAG( 0 ), // AWaveBank::SetWaveFlag1
		FLAGS_3D   = BITFLAG( 1 ), // AWaveBank::SetWaveFlag2, FSOUND_HW3D instead of FSOUND_HW2D
		FLAGS_UNK2 = BITFLAG( 2 ), // set with FLAGS_UNK3 while the bank is loaded
		FLAGS_UNK3 = BITFLAG( 3 ),
	};

	constexpr AWave()
	    : iFlags( 0 ), iFrequency( 22050 ), fLength( 0.0f ), pSampleHandle( TNULL ), iIndex( -1 )
	{}

	constexpr AWave( TINT a_iFlags, TINT a_iFrequency, TFLOAT a_fLength, AWaveSampleHandle a_pSampleHandle, TINT a_iIndex )
	    : iFlags( a_iFlags ), iFrequency( a_iFrequency ), fLength( a_fLength ), pSampleHandle( a_pSampleHandle ), iIndex( a_iIndex )
	{}

	FLAGS             iFlags;
	TINT              iFrequency;
	TFLOAT            fLength;
	AWaveSampleHandle pSampleHandle;
	TINT              iIndex;
};
