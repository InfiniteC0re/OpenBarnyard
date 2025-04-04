#pragma once
#include <File/TTRB.h>
#include <Render/TTransformObject.h>

struct ATerrainLocator
{
	void GetMatrix( Toshi::TMatrix44& a_rOutMatrix );

	Toshi::TQuaternion quatRotation;
	Toshi::TVector3    vecPosition;
	TUINT16            uiNameId;
	TUINT16            uiFlags;
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

class ATerrainLocatorManager
{
public:
	ATerrainLocatorManager();
	~ATerrainLocatorManager();

	// Returns a locator if it's currently visible
	ATerrainLocator* FindLocator( const TCHAR* a_pszLocatorName );

	// Loads a list of locators from a terrain's skel file that contains them
	void LoadFromTRB( Toshi::TTRB* a_pTRB );

	Toshi::TTRB* GetTRB() { return m_pTRB; }
	Toshi::TTransformObject& GetWorldTransform() { return m_oWorldTransform; }

private:
	Toshi::TTRB*                 m_pTRB;
	ATerrainLocatorTRBHeader*    m_pLocatorsHeader;
	ATerrainLocatorVISTRBHeader* m_pLocatorVISHeader;
	Toshi::TTransformObject      m_oWorldTransform;
	TBOOL                        m_bCreatedGrowings;
};
