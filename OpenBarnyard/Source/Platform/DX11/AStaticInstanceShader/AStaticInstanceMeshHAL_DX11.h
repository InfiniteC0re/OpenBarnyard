#pragma once
#include "Render/AStaticInstanceShader/AStaticInstanceMesh.h"

class MODAPI AStaticInstanceMeshHAL
    : public AStaticInstanceMesh
{
public:
	TDECLARE_CLASS( AStaticInstanceMeshHAL, AStaticInstanceMesh );

public:
	AStaticInstanceMeshHAL();
	~AStaticInstanceMeshHAL();

	//-----------------------------------------------------------------------------
	// Toshi::TMesh
	//-----------------------------------------------------------------------------
	virtual TBOOL Render() OVERRIDE; // +0x18

	//-----------------------------------------------------------------------------
	// Own methods
	//-----------------------------------------------------------------------------
	virtual void SetUnknown( void* a_pUnk ); // +0x30

private:
	void* m_pUnk24; // 0x24
};

TSTATICASSERT( sizeof( AStaticInstanceMeshHAL ) == 0x28 );
