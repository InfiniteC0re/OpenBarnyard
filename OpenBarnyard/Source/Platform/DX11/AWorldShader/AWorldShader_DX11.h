#pragma once
#include "Render/AWorldShader/AWorldShader.h"

#include <Render/TOrderTable.h>

#include <d3d11.h>

class MODAPI AWorldShaderHAL : public AWorldShader
{
public:
	TDECLARE_CLASS( AWorldShaderHAL, AWorldShader );

	// Render states set by the last packet, the material's PreRender sets RENDERSTATE_ALPHABLEND to force them again
	using RENDERSTATE = TUINT;
	enum RENDERSTATE_ : RENDERSTATE
	{
		RENDERSTATE_NONE       = 0,
		RENDERSTATE_UNK0       = BITFLAG( 0 ),
		RENDERSTATE_UNK1       = BITFLAG( 1 ),
		RENDERSTATE_UNK3       = BITFLAG( 3 ),
		RENDERSTATE_UNK4       = BITFLAG( 4 ),
		RENDERSTATE_ALPHAREF   = RENDERSTATE_UNK3 | RENDERSTATE_UNK4,                                       // the alpha reference is scaled by the packet alpha
		RENDERSTATE_ALPHABLEND = RENDERSTATE_UNK0 | RENDERSTATE_UNK1 | RENDERSTATE_UNK3 | RENDERSTATE_UNK4, // alpha blending is on
	};

	static constexpr TUINT NUM_ORDER_TABLES = 9;

	enum CONSTANT_ : TINT
	{
		CONSTANT_MVP          = 0,
		CONSTANT_AMBIENT      = 4,
		CONSTANT_SHADOW       = 5,
		CONSTANT_ONE          = 6,
		CONSTANT_UVOFFSETS    = 7,
		CONSTANT_ALPHAWEIGHTS = 8,
		CONSTANT_ALPHATEST    = 9,
		CONSTANT_FOG          = 10,
		CONSTANT_FOGCOLOUR    = 11,
		CONSTANT_STAGES       = 12,
	};

public:
	AWorldShaderHAL();
	~AWorldShaderHAL();

	//-----------------------------------------------------------------------------
	// Toshi::TShader
	//-----------------------------------------------------------------------------
	virtual void  Flush() override;
	virtual void  StartFlush() override;
	virtual void  EndFlush() override;
	virtual TBOOL Create() override;
	virtual TBOOL Validate() override;
	virtual void  Invalidate() override;
	virtual TBOOL TryInvalidate() override;
	virtual TBOOL TryValidate() override;
	virtual void  Render( Toshi::TRenderPacket* a_pRenderPacket ) override;

	//-----------------------------------------------------------------------------
	// AWorldShader
	//-----------------------------------------------------------------------------
	virtual void EnableRenderEnvMap( TBOOL a_bEnable ) override;

	//-----------------------------------------------------------------------------
	// Own methods
	//-----------------------------------------------------------------------------

	virtual TBOOL           IsAlphaBlendMaterial() override;
	virtual void            SetAlphaBlendMaterial( TBOOL a_bIsAlphaBlendMaterial ) override;
	virtual AWorldMaterial* CreateMaterial( const TCHAR* a_szName ) override;
	virtual AWorldMesh*     CreateMesh( const TCHAR* a_szName ) override;
	virtual TBOOL           IsHighEndMode();
	virtual void            SetHighEndMode( TBOOL a_bEnable );
	virtual TBOOL           IsCapableShaders();
	virtual TBOOL           IsRenderEnvMapEnabled();
	virtual void*           CreateUnknown( void*, void*, void*, void* );

	void SetColours( const Toshi::TVector4& a_rShadowColour, const Toshi::TVector4 a_rAmbientColour );

	TUINT GetAlphaRef() const { return m_iAlphaRef; }
	void  SetAlphaRef( TUINT val ) { m_iAlphaRef = val; }

	const Toshi::TVector4& GetShadowColour() const { return m_ShadowColour; }
	const Toshi::TVector4& GetAmbientColour() const { return m_AmbientColour; }

	Toshi::TOrderTable* GetOrderTable( TUINT a_uiIndex )
	{
		TASSERT( a_uiIndex < NUM_ORDER_TABLES );
		return &m_aOrderTables[ a_uiIndex ];
	}

	inline static RENDERSTATE s_RenderStateFlags = RENDERSTATE_ALPHABLEND;

private:
	ID3D11VertexShader* m_pVertexShader;
	ID3D11PixelShader*  m_pPixelShader;
	ID3D11InputLayout*  m_pInputLayout;
	TUINT               m_iAlphaRef;
	TBOOL               m_bRenderEnvMap;
	Toshi::TOrderTable  m_aOrderTables[ NUM_ORDER_TABLES ];
	TBOOL               m_bIsHighEndMode;
	TBOOL               m_bAlphaBlendMaterial;
	Toshi::TVector4     m_ShadowColour;
	Toshi::TVector4     m_AmbientColour;
};
