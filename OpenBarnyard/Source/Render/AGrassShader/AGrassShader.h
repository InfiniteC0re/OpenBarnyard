#pragma once
#include "AGrassMaterial.h"
#include "AGrassMesh.h"

#include <Toshi/Toshi.h>
#include <Render/TShader.h>

class AGrassShader
    : public Toshi::TShader
    , public Toshi::TSingleton<AGrassShader>
{
public:
	TDECLARE_CLASS( AGrassShader, Toshi::TShader );

public:
	AGrassShader();
	~AGrassShader();

	//-----------------------------------------------------------------------------
	// Own methods
	//-----------------------------------------------------------------------------
	virtual void            EnableRenderEnvMap( TBOOL a_bEnable );
	virtual TBOOL           IsHighEndSkinning();
	virtual void            EnableHighEndSkinning( TBOOL a_bEnable );
	virtual TBOOL           IsCapableHighEndSkinning();
	virtual TBOOL           IsLightScattering();
	virtual void            SetLightScattering( TBOOL a_bEnable );
	virtual TBOOL           IsAlphaBlendMaterial();
	virtual void            SetAlphaBlendMaterial( TBOOL a_bIsAlphaBlendMaterial );
	virtual AGrassMaterial* CreateMaterial( const TCHAR* a_szName ) = 0;
	virtual AGrassMesh*     CreateMesh( const TCHAR* a_szName )     = 0;

private:
	TUINT  m_Unk1;
	TFLOAT m_Unk2;
	TUINT  m_Unk3;
};
