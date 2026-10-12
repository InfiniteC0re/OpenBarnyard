#pragma once
#include <Toshi/Toshi.h>
#include <Toshi/TSingleton.h>
#include <Render/TShader.h>
#include <Render/TMaterial.h>
#include <Render/TMesh.h>
#include <Math/TVector3.h>

class ALineMaterial
    : public Toshi::TMaterial
{
public:
	TDECLARE_CLASS( ALineMaterial, Toshi::TMaterial );

public:
	ALineMaterial();
	~ALineMaterial();
};

class ALineMesh
    : public Toshi::TMesh
{
public:
	TDECLARE_CLASS( ALineMesh, Toshi::TMesh );

public:
	ALineMesh();
};

//-----------------------------------------------------------------------------
// Purpose: Draws lists of coloured lines and triangles (debug drawing).
// Note: The game creates it but never adds any lines.
//-----------------------------------------------------------------------------
class ALineShader
    : public Toshi::TShader
    , public Toshi::TSingleton<ALineShader>
{
public:
	TDECLARE_CLASS( ALineShader, Toshi::TShader );

public:
	ALineShader();
	~ALineShader();

	//-----------------------------------------------------------------------------
	// Toshi::TShader
	//-----------------------------------------------------------------------------
	virtual TBOOL Create() override;

	//-----------------------------------------------------------------------------
	// Own methods
	//-----------------------------------------------------------------------------
	// NOTE: the HAL returns TNULL from both, ALineMaterialHAL and ALineMeshHAL are never created
	virtual Toshi::TMaterial* CreateMaterial( const TCHAR* a_szName ) = 0; // +0x34
	virtual Toshi::TMesh*     CreateMesh( const TCHAR* a_szName )     = 0; // +0x38

	// Draws and clears the lines and the triangles
	virtual void FlushLines() = 0; // +0x3C

	virtual void AddLineDirect( const Toshi::TVector3& a_rcStart, const Toshi::TVector3& a_rcEnd, TUINT32 a_uiStartColour, TUINT32 a_uiEndColour ) = 0; // +0x40

	// Adds a_iNumLines lines, a_pStarts and a_pEnds are arrays of TVector3, a_pStartColours and a_pEndColours arrays of TUINT32
	virtual void AddLines( const Toshi::TVector3* a_pStarts, const Toshi::TVector3* a_pEnds, const TUINT32* a_pStartColours, const TUINT32* a_pEndColours, TINT a_iNumLines ); // +0x44

	virtual void AddTriangle( const Toshi::TVector3 a_aVertices[ 3 ], TUINT32 a_uiColour ) = 0; // +0x48
};
