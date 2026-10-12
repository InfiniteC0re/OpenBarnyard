#include "pch.h"
#include "ALineShader.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 005f5250
TDEFINE_CLASS_NORUNTIME( ALineMaterial );

// NOTE: GetClass 005f5260
TDEFINE_CLASS_NORUNTIME( ALineMesh );

TDEFINE_CLASS_NORUNTIME( ALineShader );

// $Barnyard: FUNCTION 005f52e0
ALineMaterial::ALineMaterial()
{
}

// $Barnyard: FUNCTION 005f5300
// $Barnyard: FUNCTION 005f5370
ALineMaterial::~ALineMaterial()
{
}

// $Barnyard: FUNCTION 005f5310
ALineMesh::ALineMesh()
{
}

// $Barnyard: FUNCTION 005f5330
ALineShader::ALineShader()
{
}

// $Barnyard: FUNCTION 005f5350
// $Barnyard: FUNCTION 005f5390
ALineShader::~ALineShader()
{
}

// $Barnyard: FUNCTION 005f5270
TBOOL ALineShader::Create()
{
	return TShader::Create();
}

// $Barnyard: FUNCTION 005f5280
void ALineShader::AddLines( const TVector3* a_pStarts, const TVector3* a_pEnds, const TUINT32* a_pStartColours, const TUINT32* a_pEndColours, TINT a_iNumLines )
{
	for ( TINT i = 0; i < a_iNumLines; i++ )
	{
		AddLineDirect( a_pStarts[ i ], a_pEnds[ i ], a_pStartColours[ i ], a_pEndColours[ i ] );
	}
}
