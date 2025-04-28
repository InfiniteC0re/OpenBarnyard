#pragma once
#include <File/TTRB.h>
#include <Render/TTransformObject.h>

class ATerrainLocator
{
public:
private:
	Toshi::TQuaternion m_Rotation;
	Toshi::TVector3    m_Position;
};

class ATerrainLocatorList
{
public:
	void LoadFromTRB( Toshi::TTRB* a_pTRB );

	Toshi::TTRB* GetTRB() { return m_pTRB; }

private:
	Toshi::TTRB*            m_pTRB;
	void*                   m_pLocators;
	void*                   m_pLocatorVIS;
	Toshi::TTransformObject m_TransformObject;
	TBOOL                   m_bCreatedGrowings;
};
