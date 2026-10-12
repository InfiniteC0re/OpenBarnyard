#pragma once
#include <Toshi/TSingleton.h>
#include <Math/TVector4.h>

class AModel;
class ATerrainLocatorList;

//-----------------------------------------------------------------------------
// Lights the model instances with the "sphereshade" and "spherelight" locators
// of the terrain. Each sphere darkens (shade, used in the day) or lightens
// (light, used at night) the instances inside of it, the result is the shade
// coefficient of the render context while the instance is rendered.
// m_fShadeCoeff is the time of day (0 is day, 1 is night), set by AGameTimeFX.
//-----------------------------------------------------------------------------
class AInstanceLightingHelper
    : public Toshi::TSingleton<AInstanceLightingHelper>
{
public:
	// Spheres of a terrain section: [iStart, iEnd)
	struct SectionRange
	{
		TINT iStart;
		TINT iEnd;
	};

public:
	AInstanceLightingHelper( TINT a_iMaxSpheres );
	~AInstanceLightingHelper();

	// Creates the spheres from the locators of a_pLocatorList
	void CreateSpheres( ATerrainLocatorList* a_pLocatorList );

	// Forgets the spheres of the locators
	void DestroySpheres( ATerrainLocatorList* a_pLocatorList );

	// Updates the shade coefficient of every model instance
	void Update();

	// Shade coefficient the sphereshade locators give at a_rcPosition (day)
	TFLOAT GetShadeValue( const Toshi::TVector4& a_rcPosition );

	// Shade coefficient the spherelight locators give at a_rcPosition (night)
	TFLOAT GetLightValue( const Toshi::TVector4& a_rcPosition );

	TFLOAT GetShadeCoeff() const { return m_fShadeCoeff; }
	void   SetShadeCoeff( TFLOAT a_fShadeCoeff ) { m_fShadeCoeff = a_fShadeCoeff; }

private:
	// Shade coefficient of the spheres a_iStart to a_iEnd at a_rcPosition (m_fShadeCoeff if there is none)
	TFLOAT GetValueInRange( TINT a_iStart, TINT a_iEnd, const Toshi::TVector4& a_rcPosition );

	// Shade coefficient of the spheres of the current terrain section (or the ones visible from it) at a_rcPosition
	TFLOAT GetValueAtPosition( const Toshi::TVector4& a_rcPosition );

	void UseShadeSpheres()
	{
		m_iNumCurSpheres    = m_iNumShadeSpheres;
		m_pCurSpheres       = m_pShadeSpheres;
		m_pCurValues        = m_pShadeValues;
		m_pCurSectionRanges = m_pShadeSectionRanges;
		m_bShade            = TTRUE;
	}

	void UseLightSpheres()
	{
		m_iNumCurSpheres    = m_iNumLightSpheres;
		m_pCurSpheres       = m_pLightSpheres;
		m_pCurValues        = m_pLightValues;
		m_pCurSectionRanges = m_pLightSectionRanges;
		m_bShade            = TFALSE;
	}

	// AModelRepos::ForEachModel callbacks
	static void UpdateModel( AModel* a_pModel );
	static void UpdateModelBlended( AModel* a_pModel );

private:
	TINT             m_iMaxSpheres;         // 0x00
	TINT             m_iNumShadeSpheres;    // 0x04
	Toshi::TVector4* m_pShadeSpheres;       // 0x08, position and radius
	TFLOAT*          m_pShadeValues;        // 0x0c
	TINT             m_iNumLightSpheres;    // 0x10
	Toshi::TVector4* m_pLightSpheres;       // 0x14, position and radius
	TFLOAT*          m_pLightValues;        // 0x18
	TFLOAT           m_fShadeCoeff;         // 0x1c
	TINT             m_iNumCurSpheres;      // 0x20
	Toshi::TVector4* m_pCurSpheres;         // 0x24
	TFLOAT*          m_pCurValues;          // 0x28
	SectionRange*    m_pCurSectionRanges;   // 0x2c
	TBOOL            m_bShade;              // 0x30
	TBOOL            m_bInSphere;           // 0x31, set by GetValueInRange
	TINT             m_iNumSections;        // 0x34
	SectionRange*    m_pShadeSectionRanges; // 0x38
	SectionRange*    m_pLightSectionRanges; // 0x3c
};
