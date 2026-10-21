#pragma once
#include "AStaticInstanceMaterial.h"
#include "AStaticInstanceMesh.h"

#include <Toshi/Toshi.h>
#include <Render/TShader.h>

//-----------------------------------------------------------------------------
// Purpose: Shader of the static instance meshes ("si_" materials).
// NOTE: the singleton (0x0079aba4) is only used to destroy the shader, the
// meshes and materials are created through the shader passed to AModelLoader.
//-----------------------------------------------------------------------------
class MODAPI AStaticInstanceShader
    : public Toshi::TShader
    , public Toshi::TSingleton<AStaticInstanceShader>
{
public:
	TDECLARE_CLASS( AStaticInstanceShader, Toshi::TShader );

public:
	AStaticInstanceShader();
	~AStaticInstanceShader();

	//-----------------------------------------------------------------------------
	// Own methods
	//-----------------------------------------------------------------------------
	virtual AStaticInstanceMaterial* CreateMaterial( const TCHAR* a_szName ) = 0; // +0x34
	virtual AStaticInstanceMesh*     CreateMesh( const TCHAR* a_szName )     = 0; // +0x38
};
