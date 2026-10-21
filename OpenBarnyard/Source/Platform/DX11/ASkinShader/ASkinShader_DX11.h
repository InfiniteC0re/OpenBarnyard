#pragma once
#include "Render/ASkinShader/ASkinShader.h"

#include <Render/TOrderTable.h>

#include <d3d11.h>

class MODAPI ASkinShaderHAL : public ASkinShader
{
public:
	TDECLARE_CLASS( ASkinShaderHAL, ASkinShader );

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

	static constexpr TUINT NUM_ORDER_TABLES = 3;
	static constexpr TUINT MAX_BONES        = 28;

	enum CONSTANT_ : TINT
	{
		CONSTANT_MVP             = 0,
		CONSTANT_BONEINDEXSCALE  = 4,
		CONSTANT_BONES           = 5,
		CONSTANT_LIGHTDIRECTION  = 89,
		CONSTANT_AMBIENTCOLOUR   = 90,
		CONSTANT_LIGHTCOLOUR     = 91,
		CONSTANT_LIGHTINGRANGE   = 93,
		CONSTANT_UPVECTOR        = 94,
		CONSTANT_ALPHATEST       = 96,
		CONSTANT_FOG             = 97,
		CONSTANT_FOGCOLOUR       = 98,
		CONSTANT_LIGHTINGLERP    = 99,
		CONSTANT_STAGES          = 100,
	};

public:
	ASkinShaderHAL();
	~ASkinShaderHAL();

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
	// ASkinShader
	//-----------------------------------------------------------------------------
	virtual void           EnableRenderEnvMap( TBOOL a_bEnable ) override;
	virtual TBOOL          IsHighEndSkinning() override;
	virtual void           EnableHighEndSkinning( TBOOL a_bEnable ) override;
	virtual TBOOL          IsCapableHighEndSkinning() override;
	virtual TBOOL          IsLightScattering() override;
	virtual void           SetLightScattering( TBOOL a_bEnable ) override;
	virtual TBOOL          IsAlphaBlendMaterial() override;
	virtual void           SetAlphaBlendMaterial( TBOOL a_bIsAlphaBlendMaterial ) override;
	virtual ASkinMaterial* CreateMaterial( const TCHAR* a_szName ) override;
	virtual ASkinMesh*     CreateMesh( const TCHAR* a_szName ) override;
	virtual TINT           AddLight( const Toshi::TVector3& a_rPosition, TFLOAT a_fIntensity ) override;
	virtual void           SetLight( TINT a_iIndex, const Toshi::TVector3& a_rPosition, TFLOAT a_fIntensity ) override;
	virtual void           RemoveLight( TINT a_iIndex ) override;

	//-----------------------------------------------------------------------------
	// Own methods
	//-----------------------------------------------------------------------------
	virtual TBOOL IsEnableRenderEnvMap();
	virtual void  SetSomeColour( TUINT a_uiR, TUINT a_uiG, TUINT a_uiB, TUINT a_uiA );
	virtual TINT  SetUnknown1( TINT a_Unknown, TUINT8 a_fAlpha );
	virtual void  SetUnknown2( TINT a_Unknown );

	void SetAlphaRef( TINT a_iAlphaRef );

	Toshi::TOrderTable* GetOrderTable( TUINT a_uiIndex )
	{
		TASSERT( a_uiIndex < NUM_ORDER_TABLES );
		return &m_aOrderTables[ a_uiIndex ];
	}

private:
	void SetStates();

public:
	inline static RENDERSTATE      sm_eRenderStateFlags = RENDERSTATE_ALPHABLEND;
	inline static Toshi::TMatrix44 sm_oWorldViewMatrix;

private:
	ID3D11VertexShader* m_pVertexShader;
	ID3D11VertexShader* m_pVertexShaderHD;
	ID3D11PixelShader*  m_pPixelShader;
	ID3D11PixelShader*  m_pPixelShaderHD;
	ID3D11InputLayout*  m_pInputLayout;
	TINT                m_iAlphaRef;
	TBOOL               m_bRenderEnvMap;
	Toshi::TOrderTable  m_aOrderTables[ NUM_ORDER_TABLES ];
	TBOOL               m_bHighEndSkinning;
	TBOOL               m_bLightScattering;
	TBOOL               m_bIsAlphaBlendMaterial;
};
