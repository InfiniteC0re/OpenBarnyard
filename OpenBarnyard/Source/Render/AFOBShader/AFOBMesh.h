#pragma once
#include <Render/TMesh.h>

//-----------------------------------------------------------------------------
// NOTE: stripped from PC retail, only the TClass object is left (see AFOBShader.h).
// Layout is unknown apart from the size (0x2C).
//-----------------------------------------------------------------------------
class MODAPI AFOBMesh : public Toshi::TMesh
{
public:
	TDECLARE_CLASS( AFOBMesh, Toshi::TMesh );

private:
	TUINT8 PADDING[ 0x2C - sizeof( Toshi::TMesh ) ];
};
