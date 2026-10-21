#pragma once
#include "Render/AGrassShader/AGrassShader.h"

#include <Render/TOrderTable.h>

#include <d3d11.h>

class MODAPI AGrassShaderHAL
    : public AGrassShader
{
public:
	TDECLARE_CLASS( AGrassShaderHAL, AGrassShader );

	enum CONSTANT_ : TINT
	{
		CONSTANT_MVP        = 0,
		CONSTANT_AMBIENT    = 4,
		CONSTANT_SHADOW     = 5,
		CONSTANT_ONE        = 6,
		CONSTANT_LAYER      = 7,
		CONSTANT_ANIMATION  = 8,
		CONSTANT_ALPHATEST  = 9,
		CONSTANT_FOG        = 10,
		CONSTANT_FOGCOLOUR  = 11,
		CONSTANT_STAGES     = 12,
	};

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
	Toshi::TOrderTable  m_oOrderTable;
	ID3D11VertexShader* m_pVertexShader;
	ID3D11PixelShader*  m_pPixelShader;
	ID3D11InputLayout*  m_pInputLayout;
};
