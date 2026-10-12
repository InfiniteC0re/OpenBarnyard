#pragma once
#include "Render/AWorldShader/AWorldShader.h"

#include <Toshi/TNodeList.h>
#include <Render/TOrderTable.h>

#include "Platform/DX8/ASkinShader/ASkinLowEnd_DX8.h"

class AWorldShaderHAL : public AWorldShader
{
public:
	TDECLARE_CLASS( AWorldShaderHAL, AWorldShader );

	// NOTE: same node type as ASkinShaderHAL, the destructor (005f6a10, releases
	// the D3D vertex buffer) and DeleteAll (005f7530) are folded in retail
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

	static constexpr TUINT NUM_ORDER_TABLES = 9;

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

	// Returns TTRUE if alpha blend material is enabled
	virtual TBOOL IsAlphaBlendMaterial() override;

	// Enabled alpha blend material
	virtual void SetAlphaBlendMaterial( TBOOL a_bIsAlphaBlendMaterial ) override;

	// Creates AWorldMaterial and returns it
	virtual AWorldMaterial* CreateMaterial( const TCHAR* a_szName ) override;

	// Creates AWorldMesh and returns it
	virtual AWorldMesh* CreateMesh( const TCHAR* a_szName ) override;

	// Returns TTRUE if shaders are supported
	virtual TBOOL IsHighEndMode();

	// Enables high end mode (compiled shader) if they are supported by hardware
	virtual void SetHighEndMode( TBOOL a_bEnable );

	// Returns TTRUE if shaders are supported by hardware
	virtual TBOOL IsCapableShaders();

	// Returns TTRUE if rendering of env map is enabled
	virtual TBOOL IsRenderEnvMapEnabled();

	// Probably used in debug mode but is stripped out in release
	virtual void* CreateUnknown( void*, void*, void*, void* );

	void SetColours( const Toshi::TVector4& a_rShadowColour, const Toshi::TVector4 a_rAmbientColour );

	void CheckCPUSupportsSSE();
	void CheckCPUSupportsSSE2();

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
	void SetupLowEndMode();

private:
	Toshi::TNodeList<AUnknown> m_SomeList;
	DWORD                      m_hVertexShader;
	TINT*                      m_pUnk2;
	TUINT                      m_iAlphaRef;
	TBOOL                      m_bRenderEnvMap;
	Toshi::TOrderTable         m_aOrderTables[ NUM_ORDER_TABLES ];
	TBOOL                      m_bIsHighEndMode;
	TBOOL                      m_bAlphaBlendMaterial;
	TBOOL                      m_bCPUSupportsSSE;
	TBOOL                      m_bCPUSupportsSSE2;
	Toshi::TVector4            m_ShadowColour;
	Toshi::TVector4            m_AmbientColour;
};
