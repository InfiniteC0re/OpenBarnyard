#pragma once
#include "Render/AFXShader/AFXShader.h"

#include <Toshi/T2SList.h>
#include <Render/TOrderTable.h>

#include <d3d11.h>

class MODAPI AFXShaderHAL : public AFXShader
{
public:
	TDECLARE_CLASS( AFXShaderHAL, AFXShader );

	friend class AFXMaterialHAL;
	friend class AFXMeshHAL;

	static constexpr TUINT MAX_LIGHTS = 256;
	static constexpr TUINT MAX_BONES  = 28;

	enum CONSTANT_ : TINT
	{
		CONSTANT_MVP            = 0,
		CONSTANT_BONEINDEXSCALE = 4,
		CONSTANT_BONES          = 5,
		CONSTANT_LIGHTDIRECTION = 89,
		CONSTANT_AMBIENTCOLOUR  = 90,
		CONSTANT_LIGHTCOLOUR    = 91,
		CONSTANT_LIGHTINGRANGE  = 93,
		CONSTANT_ALPHATEST      = 96,
		CONSTANT_STAGES         = 97,
	};

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
	virtual TBOOL  IsCapableLightScattering();
	virtual void   SetUnknownFloat( TFLOAT a_fValue );
	virtual TFLOAT GetUnknownFloat();
	virtual TBOOL  IsEnableRenderEnvMap();
	virtual void   SetSomeColour( TUINT a_uiR, TUINT a_uiG, TUINT a_uiB, TUINT a_uiA );
	virtual TINT   SetUnknown1( TINT a_Unknown, TUINT8 a_fAlpha );
	virtual void   SetUnknown2( TINT a_Unknown );

	inline static Toshi::TMatrix44 sm_oWorldViewMatrix;

private:
	ID3D11VertexShader*   m_pVertexShader;
	ID3D11PixelShader*    m_pPixelShader;
	ID3D11InputLayout*    m_pInputLayout;
	TINT                  m_iAlphaRef;
	TBOOL                 m_bRenderEnvMap;
	Toshi::TOrderTable    m_oOrderTable;
	TBOOL                 m_bHighEndSkinning;
	TBOOL                 m_bLightScattering;
	TBOOL                 m_bIsAlphaBlendMaterial;
	TBOOL                 m_bPointLights;
	TINT                  m_iNumLights;
	Toshi::T2SList<Light> m_UsedLights;
	Toshi::T2SList<Light> m_FreeLights;
	Light*                m_pLights;
	TFLOAT                m_fUnknown;
};
