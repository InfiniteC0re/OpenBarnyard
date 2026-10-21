#pragma once
#include <Render/TShader.h>
#include <Toshi/TSingleton.h>

class AFOBMaterial;
class AFOBMesh;

//-----------------------------------------------------------------------------
// Shader of FOBs (foliage objects: the leaf clusters of the trees placed by
// ATreeManager, models like tree_standard_fob.trb, foblittle.trb, Fob_pond.trb).
// NOTE: PC retail only keeps the TClass objects of AFOBShader/AFOBMaterial/AFOBMesh
// (sizes 0x18/0x70/0x2C) and the IsA checks in AModelLoader. No vtable, constructor
// or HAL implementation was linked, so no instance is ever created on PC and the
// FOB models are drawn with the other shaders. The Wii build still has
// AFOBShaderHAL/AFOBMaterialHAL/AFOBMeshHAL and TSingleton<AFOBShader>.
// The virtuals mirror AWorldShader: AModelLoader calls CreateMaterial at +0x54.
//-----------------------------------------------------------------------------
class MODAPI AFOBShader
    : public Toshi::TShader
    , public Toshi::TSingleton<AFOBShader>
{
public:
	TDECLARE_CLASS( AFOBShader, Toshi::TShader );

public:
	//-----------------------------------------------------------------------------
	// Own methods
	//-----------------------------------------------------------------------------
	virtual void          EnableRenderEnvMap( TBOOL a_bEnable )                  = 0; // +0x34
	virtual TBOOL         IsHighEndSkinning()                                    = 0; // +0x38
	virtual void          EnableHighEndSkinning( TBOOL a_bEnable )               = 0; // +0x3C
	virtual TBOOL         IsCapableHighEndSkinning()                             = 0; // +0x40
	virtual TBOOL         IsLightScattering()                                    = 0; // +0x44
	virtual void          SetLightScattering( TBOOL a_bEnable )                  = 0; // +0x48
	virtual TBOOL         IsAlphaBlendMaterial()                                 = 0; // +0x4C
	virtual void          SetAlphaBlendMaterial( TBOOL a_bIsAlphaBlendMaterial ) = 0; // +0x50
	virtual AFOBMaterial* CreateMaterial( const TCHAR* a_szName )                = 0; // +0x54
	virtual AFOBMesh*     CreateMesh( const TCHAR* a_szName )                    = 0; // +0x58

private:
	TUINT8 PADDING[ 0xC ]; // 0x0C
};
