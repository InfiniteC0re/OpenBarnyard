#pragma once
#include <File/TTRB.h>
#include <Render/TTransformObject.h>

struct ATerrainLocator
{
	void GetMatrix( Toshi::TMatrix44& a_rOutMatrix ) const;

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
	ATerrainLocatorList();
	~ATerrainLocatorList();

	// Returns a locator if it's currently visible
	ATerrainLocator* FindLocator( const TCHAR* a_pszLocatorName );

	// Loads a list of locators from a terrain's skel file that contains them
	void LoadFromTRB( Toshi::TTRB* a_pTRB );

	TINT GetNumVISSections() const;

	Toshi::TTRB*             GetTRB() { return m_pTRB; }
	Toshi::TTransformObject& GetWorldTransform() { return m_oWorldTransform; }

	TINT32                       GetNumLocators() const { return m_pLocatorsHeader->iNumLocators; }
	ATerrainLocatorTRBHeader*    GetHeader() const { return m_pLocatorsHeader; }
	ATerrainLocatorVISTRBHeader* GetVISHeader() const { return m_pLocatorVISHeader; }

	const ATerrainLocator* GetLocator( TUINT32 a_uiIndex ) const { return &m_pLocatorsHeader->pLocators[ a_uiIndex ]; }
	const TCHAR*           GetLocatorName( TUINT32 a_uiIndex ) const { return m_pLocatorsHeader->ppNames[ m_pLocatorsHeader->pLocators[ a_uiIndex ].uiNameId ]; }

private:
	Toshi::TTRB*                 m_pTRB;
	ATerrainLocatorTRBHeader*    m_pLocatorsHeader;
	ATerrainLocatorVISTRBHeader* m_pLocatorVISHeader;
	Toshi::TTransformObject      m_oWorldTransform;
	TBOOL                        m_bCreatedGrowings;
};
