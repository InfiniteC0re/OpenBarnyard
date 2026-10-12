#pragma once
#include "Render/ASkinShader/ASkinShader.h"

#include <Toshi/TNodeList.h>
#include <Render/TOrderTable.h>

#include "ASkinLowEnd_DX8.h"

class ASkinShaderHAL : public ASkinShader
{
public:
	TDECLARE_CLASS( ASkinShaderHAL, ASkinShader );

	// Dynamic vertex buffer of the low end (CPU skinning) path
	using AUnknown = ASkinLowEnd::VertexBuffer;

	// Render states set by the last packet, the material's PreRender sets RENDERSTATE_ALPHABLEND to force them again
	using RENDERSTATE = TUINT;
	enum RENDERSTATE_ : RENDERSTATE
	{
		RENDERSTATE_NONE       = 0,
		RENDERSTATE_UNK0       = BITFLAG( 0 ),
		RENDERSTATE_UNK1       = BITFLAG( 1 ),
		RENDERSTATE_UNK3       = BITFLAG( 3 ),
		RENDERSTATE_UNK4       = BITFLAG( 4 ),
		RENDERSTATE_ALPHAREF   = RENDERSTATE_UNK3 | RENDERSTATE_UNK4,                                       // D3DRS_ALPHAREF is scaled by the packet alpha
		RENDERSTATE_ALPHABLEND = RENDERSTATE_UNK0 | RENDERSTATE_UNK1 | RENDERSTATE_UNK3 | RENDERSTATE_UNK4, // D3DRS_ALPHABLENDENABLE is on
	};

	static constexpr TUINT NUM_ORDER_TABLES = 3;

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

	// Low end path, called by ASkinLowEnd::Render
	void        SkinVerticesCPU( Toshi::TRenderPacket* a_pRenderPacket, const TBYTE* a_pSrcVertices, TBYTE* a_pDstVertices );
	static void TransformVerticesRigid( Toshi::TRenderPacket* a_pRenderPacket, const TBYTE* a_pSrcVertices, TBYTE* a_pDstVertices );

private:
	void RenderLowEnd( Toshi::TRenderPacket* a_pRenderPacket );
	void SetupLowEndMode();
	void ApplyGPUSpecificPatches();
	void CheckCPUSupportsSSE();
	void CheckCPUSupportsSSE2();

public:
	inline static RENDERSTATE      sm_eRenderStateFlags = RENDERSTATE_ALPHABLEND;
	inline static Toshi::TMatrix44 sm_oWorldViewMatrix;
	inline static Toshi::TMatrix44 sm_oViewModelMatrix;

private:
	// ...
	Toshi::TNodeList<AUnknown> m_SomeList;
	DWORD                      m_hUnknownPixelShader;
	DWORD                      m_hVertexShader;
	DWORD                      m_hVertexShaderHD;
	DWORD                      m_hPixelShader;
	TINT                       m_iAlphaRef;
	TBOOL                      m_bRenderEnvMap;
	Toshi::TOrderTable         m_aOrderTables[ NUM_ORDER_TABLES ];
	TBOOL                      m_bHighEndSkinning;
	TBOOL                      m_bLightScattering;
	TBOOL                      m_bIsAlphaBlendMaterial;
	TFLOAT                     m_fAMDPatch1;
	TFLOAT                     m_fAMDPatch2;
	TBOOL                      m_bCPUSupportsSSE;
	TBOOL                      m_bCPUSupportsSSE2;
	TBOOL                      m_bUnkFlag;
};
