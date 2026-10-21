#pragma once
#include "Render/AStaticInstanceShader/AStaticInstanceShader.h"

#include <Render/TOrderTable.h>

class MODAPI AStaticInstanceShaderHAL
    : public AStaticInstanceShader
{
public:
	TDECLARE_CLASS( AStaticInstanceShaderHAL, AStaticInstanceShader );

public:
	AStaticInstanceShaderHAL();
	~AStaticInstanceShaderHAL();

	//-----------------------------------------------------------------------------
	// Toshi::TShader
	//-----------------------------------------------------------------------------
	virtual void  Flush() OVERRIDE;
	virtual void  StartFlush() OVERRIDE {} // DOES_NOTHING
	virtual void  EndFlush() OVERRIDE {}   // DOES_NOTHING
	virtual TBOOL Create() OVERRIDE;
	virtual TBOOL Validate() OVERRIDE;
	virtual void  Invalidate() OVERRIDE;
	virtual TBOOL TryInvalidate() OVERRIDE;
	virtual TBOOL TryValidate() OVERRIDE;
	virtual void  Render( Toshi::TRenderPacket* a_pRenderPacket ) OVERRIDE {} // DOES_NOTHING

	//-----------------------------------------------------------------------------
	// AStaticInstanceShader
	//-----------------------------------------------------------------------------
	virtual AStaticInstanceMaterial* CreateMaterial( const TCHAR* a_szName ) OVERRIDE;
	virtual AStaticInstanceMesh*     CreateMesh( const TCHAR* a_szName ) OVERRIDE;

private:
	Toshi::TOrderTable m_oOrderTable; // 0x0c
};
