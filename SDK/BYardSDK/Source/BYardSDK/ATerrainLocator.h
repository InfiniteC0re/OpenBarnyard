#pragma once
#include <File/TTRB.h>
#include <Render/TTransformObject.h>

struct ATerrainLocator
{
	void GetMatrix( Toshi::TMatrix44& a_rOutMatrix ) const
	{
		a_rOutMatrix.SetFromQuaternion( quatRotation );
		a_rOutMatrix.AsBasisVector3( Toshi::BASISVECTOR_TRANSLATION ) = vecPosition;
	}

	Toshi::TQuaternion quatRotation;
	Toshi::TVector3    vecPosition;
	TUINT16            uiNameId;
	TUINT8             uiFlags1;
	TINT8              iFlags2;

	TFLOAT GetScale() const { return ( uiFlags1 & 0xF ) * 0.2f; }
	TUINT8 GetFlags1() const { return uiFlags1 >> 4; }
};

struct ATerrainLocatorTRBHeader
{
	TINT32           iNumLocators;
	ATerrainLocator* pLocators;
	TUINT32          uiNameCount;
	const TCHAR**    ppNames;
};

struct ATerrainLocatorVISSection
{
	const TCHAR* pName;
	TUINT32      uiFirstLocator;
	TUINT32      uiNumLocators;
};

struct ATerrainLocatorVISTRBHeader
{
	TINT32                     iNumSections;
	ATerrainLocatorVISSection* pSections;
};

class ATerrainLocatorList
{
public:
	void LoadFromTRB( Toshi::TTRB* a_pTRB );

	Toshi::TTRB* GetTRB() { return m_pTRB; }

public:
	Toshi::TTRB*                 m_pTRB;
	ATerrainLocatorTRBHeader*    m_pLocatorsHeader;
	ATerrainLocatorVISTRBHeader* m_pLocatorVISHeader;
	Toshi::TTransformObject      m_oWorldTransform;
	TBOOL                        m_bCreatedGrowings;
};
