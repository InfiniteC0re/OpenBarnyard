#pragma once
#include <Render/TShader.h>

class ASysMaterial;
class ASysMesh;

class MODAPI ASysShader : public Toshi::TShader
    , public Toshi::TSingleton<ASysShader>
{
public:
	TDECLARE_CLASS( ASysShader, Toshi::TShader );

public:
	ASysShader();
	~ASysShader();
	virtual ASysMaterial* CreateMaterial( const TCHAR* a_szName ) = 0;
	virtual ASysMesh*     CreateMesh( const TCHAR* a_szName )     = 0;
	virtual TBOOL         IsLightScattering();
	virtual void          SetLightScattering( TBOOL a_bEnabled );
};
