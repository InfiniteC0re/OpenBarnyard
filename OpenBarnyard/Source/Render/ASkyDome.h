#pragma once
#include <Math/TMatrix44.h>
#include <Math/TVector4.h>
#include <Render/TTexture.h>

class ASysMesh;
class ASysMaterial;
class AMaterialLibrary;
class AModelInstance;

class MODAPI ASkyDome
{
public:
	struct DomeVertex
	{
		Toshi::TVector3 vPosition;
	};

	using FLAGS = TUINT8;
	enum FLAGS_ : FLAGS
	{
		FLAGS_NONE   = 0,
		FLAGS_FROZEN = BITFLAG( 0 ), // Colour transitions don't advance
		FLAGS_RENDER = BITFLAG( 1 ),
		FLAGS_ROTATE = BITFLAG( 2 ),
	};

	using TEXTURE = TUINT;
	enum TEXTURE_ : TEXTURE
	{
		TEXTURE_DAY,
		TEXTURE_NIGHT,
		TEXTURE_NUMOF
	};

public:
	ASkyDome();
	virtual ~ASkyDome();

	void Create( TFLOAT a_fRadius, TFLOAT a_fHeight, TINT a_iSides, TINT a_iRings );
	void Render1();
	void Render2();
	void SetSkyColors( TFLOAT a_fDuration, const Toshi::TMatrix44* a_pColorMatrix, const Toshi::TMatrix44* a_pLightMatrix, TFLOAT a_fTargetBlend, TINT a_iNewSkyIndex, TFLOAT a_fElapsed );
	void SetDirty();

	TBOOL Update( TFLOAT a_fDeltaTime );

	// Loads the stars model (data\models\stars) from the startup library
	void LoadStars();
	void UnloadStars();

	// $Barnyard: FUNCTION 0042c940
	void SetStarsOpacity( TFLOAT a_fOpacity ) { m_fStarsOpacity = a_fOpacity; }
	TFLOAT GetStarsOpacity() const { return m_fStarsOpacity; }

	// The dome is drawn once its meshes are created
	TBOOL IsCreated() const { return m_pFancyDome != TNULL; }

	void SetRotationSpeed( TFLOAT a_fSpeed ) { m_fRotationSpeed = a_fSpeed; }
	void SetSunDirection( const Toshi::TVector4& a_rcDirection ) { m_vSunDirection = a_rcDirection; }
	void SetSunMin( TFLOAT a_fSunMin ) { m_fSunMin = a_fSunMin; }
	void SetSunMax( TFLOAT a_fSunMax ) { m_fSunMax = a_fSunMax; }

	void SetTint( const Toshi::TVector4& a_rcTint )
	{
		m_fTintR = a_rcTint.x;
		m_fTintG = a_rcTint.y;
		m_fTintB = a_rcTint.z;
		m_fTintA = a_rcTint.w;
	}

	FLAGS GetFlags() const { return m_eFlags; }
	void  SetFlags( FLAGS a_eFlags ) { m_eFlags = a_eFlags; }

private:
	void GenerateDomeVertices( TFLOAT a_fRadius, TFLOAT a_fHeight );
	void ResetMaterials();
	void DestroyMeshes();
	void DestroyResources();
	void CreateMesh1();
	void CreateMesh2();
	void DrawFancyDome( const Toshi::TVector4& a_rColour1, const Toshi::TVector4& a_rColour2, const Toshi::TVector4& a_rColour3, const Toshi::TVector4& a_rColour4, ASysMaterial* a_pMaterial, TFLOAT a_fOpacity, TINT a_iMode );
	void UpdateCachedMeshData( TINT a_iSkyIndex );
	void Render1_Impl();
	void Render2_Impl();

private:
	TINT              m_iNumSomeMeshes;
	TINT              m_iNumMeshes2;
	TINT              m_iRings;
	TINT              m_iSides;
	TFLOAT            m_fHeight;
	TFLOAT            m_fRadius;
	TINT              m_iPrevSkyIndex;
	TINT              m_iTexturedSkyIndex;
	Toshi::TMatrix44  m_oColorMatrix;
	Toshi::TMatrix44  m_oLightMatrix;
	TFLOAT            m_fBlend;
	Toshi::TMatrix44  m_oTargetColorMatrix;
	Toshi::TMatrix44  m_oTargetLightMatrix;
	TFLOAT            m_fTargetBlend;
	ASysMesh**        m_ppDomeMeshes;
	ASysMesh*         m_pFancyDome;
	AMaterialLibrary* m_pSkyLibrary;
	ASysMaterial*     m_pUntexturedSky;
	ASysMaterial*     m_apTexturedSkyMaterials[ TEXTURE_NUMOF ];
	Toshi::TTexture*  m_apSkyTextures[ TEXTURE_NUMOF ];
	DomeVertex*       m_pVertexData;
	TFLOAT            m_fTransitionDuration;
	TFLOAT            m_fTransitionElapsed;
	TBOOL             m_bIsDay;
	TBOOL             m_bNeedsUpdate;
	TFLOAT            m_fTintR;
	TFLOAT            m_fTintG;
	TFLOAT            m_fTintB;
	TFLOAT            m_fTintA;
	TFLOAT            m_fStarsOpacity;
	TFLOAT            m_fRotation;
	TFLOAT            m_fRotation2;
	TFLOAT            m_fRotationSpeed;
	Toshi::TVector4   m_vSunDirection;
	TFLOAT            m_fSunMin;
	TFLOAT            m_fSunMax;
	AModelInstance*   m_pStarsInstance;
	FLAGS             m_eFlags;
};
