#pragma once
#include "Render/AFXShader/AFXShader.h"
#include "Platform/DX8/ASkinShader/ASkinLowEnd_DX8.h"

#include <Toshi/TNodeList.h>
#include <Toshi/T2SList.h>
#include <Render/TOrderTable.h>

class MODAPI AFXShaderHAL : public AFXShader
{
public:
	TDECLARE_CLASS( AFXShaderHAL, AFXShader );

	friend class AFXMaterialHAL;
	friend class AFXMeshHAL;

	static constexpr TUINT MAX_LIGHTS = 256;

	struct Light : Toshi::T2SList<Light>::Node
	{
		TINT            iIndex;     // 0x04
		Toshi::TVector3 vPosition;  // 0x08
		TFLOAT          fIntensity; // 0x14
	};

public:
	AFXShaderHAL();
	~AFXShaderHAL();

	//-----------------------------------------------------------------------------
	// Toshi::TShader
	//-----------------------------------------------------------------------------
	virtual void  OnDestroy() override;
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
	// AFXShader
	//-----------------------------------------------------------------------------
	virtual void         EnableRenderEnvMap( TBOOL a_bEnable ) override;
	virtual TBOOL        IsHighEndSkinning() override;
	virtual void         EnableHighEndSkinning( TBOOL a_bEnable ) override;
	virtual TBOOL        IsCapableHighEndSkinning() override;
	virtual TBOOL        IsLightScattering() override;
	virtual void         SetLightScattering( TBOOL a_bEnable ) override;
	virtual TBOOL        IsAlphaBlendMaterial() override;
	virtual void         SetAlphaBlendMaterial( TBOOL a_bIsAlphaBlendMaterial ) override;
	virtual AFXMaterial* CreateMaterial( const TCHAR* a_szName ) override;
	virtual AFXMesh*     CreateMesh( const TCHAR* a_szName ) override;
	virtual TINT         AddLight( const Toshi::TVector3& a_rPosition, TFLOAT a_fIntensity ) override;
	virtual void         SetLight( TINT a_iIndex, const Toshi::TVector3& a_rPosition, TFLOAT a_fIntensity ) override;
	virtual void         RemoveLight( TINT a_iIndex ) override;
	virtual TBOOL        IsCapablePointLights() override;
	virtual void         SetPointLights( TBOOL a_bEnabled ) override;
	virtual TBOOL        IsPointLightsEnabled() override;

	//-----------------------------------------------------------------------------
	// Own methods
	//-----------------------------------------------------------------------------
	virtual TBOOL  IsCapableLightScattering();                                          // +0x7C
	virtual void   SetUnknownFloat( TFLOAT a_fValue );                                  // +0x80
	virtual TFLOAT GetUnknownFloat();                                                   // +0x84
	virtual TBOOL  IsEnableRenderEnvMap();                                              // +0x88
	virtual void   SetSomeColour( TUINT a_uiR, TUINT a_uiG, TUINT a_uiB, TUINT a_uiA ); // +0x8C
	virtual TINT   SetUnknown1( TINT a_Unknown, TUINT8 a_fAlpha );                      // +0x90
	virtual void   SetUnknown2( TINT a_Unknown );                                       // +0x94

	// Low end path, called by ASkinLowEnd::Render
	void        SkinVerticesCPU( Toshi::TRenderPacket* a_pRenderPacket, const TBYTE* a_pSrcVertices, TBYTE* a_pDstVertices );
	static void TransformVerticesRigid( Toshi::TRenderPacket* a_pRenderPacket, const TBYTE* a_pSrcVertices, TBYTE* a_pDstVertices );

	// Copied from the render context by AFXMeshHAL::Render (FLOAT_0079ae78)
	inline static Toshi::TMatrix44 sm_oWorldViewMatrix;

private:
	void RenderLowEnd( Toshi::TRenderPacket* a_pRenderPacket );
	void SetupLowEndMode();
	void ApplyGPUSpecificPatches();
	void CheckCPUSupportsSSE();
	void CheckCPUSupportsSSE2();

private:
	TUINT8                                      PADDING[ 0x14 ];         // 0x2C
	Toshi::TNodeList<ASkinLowEnd::VertexBuffer> m_SomeList;              // 0x40
	DWORD                                       m_uiVertexShader;        // 0x50
	TUINT                                       m_iAlphaRef;             // 0x54
	TBOOL                                       m_bRenderEnvMap;         // 0x58
	Toshi::TOrderTable                          m_oOrderTable;           // 0x5C
	TBOOL                                       m_bHighEndSkinning;      // 0x70
	TBOOL                                       m_bLightScattering;      // 0x71
	TBOOL                                       m_bIsAlphaBlendMaterial; // 0x72
	TFLOAT                                      m_fAMDPatch1;            // 0x74
	TFLOAT                                      m_fAMDPatch2;            // 0x78
	TBOOL                                       m_bCPUSupportsSSE;       // 0x7C
	TBOOL                                       m_bCPUSupportsSSE2;      // 0x7D
	TBOOL                                       m_bPointLights;          // 0x7E
	TINT                                        m_iNumLights;            // 0x80
	Toshi::T2SList<Light>                       m_UsedLights;            // 0x84
	Toshi::T2SList<Light>                       m_FreeLights;            // 0x88
	Light*                                      m_pLights;               // 0x8C
	TFLOAT                                      m_fUnknown;              // 0x90
};
