#pragma once
#include <Render/TShader.h>
#include <Toshi/TSingleton.h>
#include <Math/TVector3.h>
#include <Math/TVector4.h>

class AFXMaterial;
class AFXMesh;

//-----------------------------------------------------------------------------
// Skinned shader for effect models.
// NOTE: A copy of ASkinShader in the original code (same virtual methods and
// mostly identical HAL code). Nothing in the retail game creates an AFXShaderHAL,
// AModelLoader::CreateMaterial only handles it if it exists.
//-----------------------------------------------------------------------------
class MODAPI AFXShader
    : public Toshi::TShader
    , public Toshi::TSingleton<AFXShader>
{
public:
	TDECLARE_CLASS( AFXShader, Toshi::TShader );

public:
	AFXShader();
	~AFXShader();

	//-----------------------------------------------------------------------------
	// Own methods
	//-----------------------------------------------------------------------------
	virtual void         EnableRenderEnvMap( TBOOL a_bEnable );                                                  // +0x34
	virtual TBOOL        IsHighEndSkinning();                                                                    // +0x38
	virtual void         EnableHighEndSkinning( TBOOL a_bEnable );                                               // +0x3C
	virtual TBOOL        IsCapableHighEndSkinning();                                                             // +0x40
	virtual TBOOL        IsLightScattering();                                                                    // +0x44
	virtual void         SetLightScattering( TBOOL a_bEnable );                                                  // +0x48
	virtual TBOOL        IsAlphaBlendMaterial();                                                                 // +0x4C
	virtual void         SetAlphaBlendMaterial( TBOOL a_bIsAlphaBlendMaterial );                                 // +0x50
	virtual AFXMaterial* CreateMaterial( const TCHAR* a_szName )                                            = 0; // +0x54
	virtual AFXMesh*     CreateMesh( const TCHAR* a_szName )                                                = 0; // +0x58
	virtual TINT         AddLight( const Toshi::TVector3& a_rPosition, TFLOAT a_fIntensity )                = 0; // +0x5C
	virtual void         SetLight( TINT a_iIndex, const Toshi::TVector3& a_rPosition, TFLOAT a_fIntensity ) = 0; // +0x60
	virtual void         RemoveLight( TINT a_iIndex )                                                       = 0; // +0x64
	virtual void         EnableClouds( TBOOL a_bEnabled );                                                       // +0x68
	virtual TBOOL        IsEnabledClouds();                                                                      // +0x6C
	virtual TBOOL        IsCapablePointLights();                                                                 // +0x70
	virtual void         SetPointLights( TBOOL a_bEnabled );                                                     // +0x74
	virtual TBOOL        IsPointLightsEnabled();                                                                 // +0x78

protected:
	union
	{
		TUINT8 m_SomeColourComponents[ 4 ];
		TINT32 m_SomeColour;
	};                       // 0x0C
	TINT            m_Unk2;  // 0x10
	TFLOAT          m_Unk3;  // 0x14
	TINT            m_Unk4;  // 0x18
	Toshi::TVector4 m_vUnk5; // 0x1C
};
