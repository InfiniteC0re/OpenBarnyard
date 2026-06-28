#pragma once
#include <Math/TMatrix44.h>

class ASysMesh;
class ASysMaterial;
class AMaterialLibrary;
class AModelInstance;

class ASkyDome
{
public:
	struct DomeVertex
	{
		Toshi::TVector3 vPosition;
	};

	using TEXTURE = TUINT;
	enum TEXTURE_ : TEXTURE
	{
		TEXTURE_DAY,
		TEXTURE_NIGHT,
		TEXTURE_NUMOF
	};

public:
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
	TFLOAT            m_fUnk15c;
	TFLOAT            m_fUnk160;
	TFLOAT            m_fUnk164;
	TFLOAT            m_fUnk168;
	TFLOAT            m_fStarsOpacity;
	TFLOAT            m_fRotation;
	TUINT             m_uiUnk174;
	TFLOAT            m_fUnk178;
	Toshi::TVector4   m_vSunDirection;
	TFLOAT            m_fSunMin;
	TFLOAT            m_fSunMax;
	AModelInstance*   m_pStarsInstance;
	TUINT8            m_eFlags;
};

