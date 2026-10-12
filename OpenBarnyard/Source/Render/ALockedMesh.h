#pragma once
#include <Toshi/TObject.h>

class ALockedMesh
    : public Toshi::TObject
{
public:
	TDECLARE_CLASS( ALockedMesh, Toshi::TObject );

public:
	ALockedMesh();
	~ALockedMesh();

private:
	TUINT32 m_aUnk[ 17 ]; // 0x04, unknown, all zeroed by the constructor
};

TSTATICASSERT( sizeof( ALockedMesh ) == 0x48 );
