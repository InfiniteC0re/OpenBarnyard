#pragma once
#include "Render/AGrassShader/AGrassShader.h"

#include <Render/TOrderTable.h>

class AGrassShaderHAL
    : public AGrassShader
{
public:
	TDECLARE_CLASS( AGrassShaderHAL, AGrassShader );

public:
	AGrassShaderHAL();
	~AGrassShaderHAL();

	//-----------------------------------------------------------------------------
	// Toshi::TShader
	//-----------------------------------------------------------------------------
	virtual void  Flush() OVERRIDE;
	virtual void  StartFlush() OVERRIDE;
	virtual void  EndFlush() OVERRIDE;
	virtual TBOOL Create() OVERRIDE;
	virtual TBOOL Validate() OVERRIDE;
	virtual void  Invalidate() OVERRIDE;
	virtual TBOOL TryInvalidate() OVERRIDE;
	virtual TBOOL TryValidate() OVERRIDE;
	virtual void  Render( Toshi::TRenderPacket* a_pRenderPacket ) OVERRIDE;

	//-----------------------------------------------------------------------------
	// AGrassShader
	//-----------------------------------------------------------------------------
	virtual AGrassMaterial* CreateMaterial( const TCHAR* a_szName ) OVERRIDE;
	virtual AGrassMesh*     CreateMesh( const TCHAR* a_szName ) OVERRIDE;

private:
	void UpdateAnimation();

private:
	Toshi::TOrderTable m_oOrderTable;
	DWORD              m_hVertexShader;
};
