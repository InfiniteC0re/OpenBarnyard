#include "pch.h"
#include "AInstanceLightingHelper.h"
#include "Animation/AModel.h"
#include "Animation/AModelRepos.h"
#include "Terrain/ATerrainInterface.h"
#include "Terrain/ATerrainLocator.h"

#include <Toshi/TStringManager.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $Barnyard: FUNCTION 00607590
AInstanceLightingHelper::AInstanceLightingHelper( TINT a_iMaxSpheres )
{
	m_iMaxSpheres         = a_iMaxSpheres;
	m_iNumShadeSpheres    = 0;
	m_iNumLightSpheres    = 0;
	m_fShadeCoeff         = 0.0f;
	m_pShadeSpheres       = new TVector4[ a_iMaxSpheres ];
	m_pShadeValues        = new TFLOAT[ m_iMaxSpheres ];
	m_pLightSpheres       = new TVector4[ m_iMaxSpheres ];
	m_pLightValues        = new TFLOAT[ m_iMaxSpheres ];
	m_iNumSections        = 0;
	m_pShadeSectionRanges = TNULL;
	m_pLightSectionRanges = TNULL;
}

// $Barnyard: FUNCTION 00607630
AInstanceLightingHelper::~AInstanceLightingHelper()
{
	delete[] m_pShadeSpheres;
	delete[] m_pShadeValues;
	delete[] m_pLightSpheres;
	delete[] m_pLightValues;
}

// $Barnyard: FUNCTION 00607670
void AInstanceLightingHelper::CreateSpheres( ATerrainLocatorList* a_pLocatorList )
{
	ATerrainInterface* pTerrain = ATerrainInterface::GetSingleton();

	m_iNumSections        = pTerrain->GetVIS()->GetNumSections();
	m_pShadeSectionRanges = new SectionRange[ m_iNumSections ];
	m_pLightSectionRanges = new SectionRange[ m_iNumSections ];

	for ( TINT i = 0; i < m_iNumSections; i++ )
	{
		m_pShadeSectionRanges[ i ].iStart = -1;
		m_pShadeSectionRanges[ i ].iEnd   = -1;
		m_pLightSectionRanges[ i ].iStart = -1;
		m_pLightSectionRanges[ i ].iEnd   = -1;
	}

	// NOTE: the sections of the locators are the ones of the locator list of the terrain
	ATerrainLocatorList* pTerrainLocators = pTerrain->GetLocatorList();
	ATerrainVIS*         pVIS             = pTerrain->GetVIS();
	TINT                 iNumVISSections  = pTerrainLocators->GetNumVISSections();

	TMatrix44 matWorld;
	a_pLocatorList->GetWorldTransform().GetLocalMatrixImp( matWorld );

	TINT iNumLocators = a_pLocatorList->GetNumLocators();
	TINT iSectionEnd  = -1;
	TINT iSection     = -1;
	TINT iVISSection  = -1;

	for ( TINT i = 0; i < iNumLocators; i++ )
	{
		if ( i >= iSectionEnd && ++iVISSection < iNumVISSections )
		{
			// The locators of the next VIS section, find the terrain section with the same name
			ATerrainLocatorVISSection* pVISSection = &pTerrainLocators->GetVISHeader()->pSections[ iVISSection ];

			for ( TINT k = 0; k < m_iNumSections; k++ )
			{
				ATerrainSection* pSection = &pVIS->GetSections()[ k ];

				if ( pSection->GetOtherSectionVisibility( k ) >= ATerrainLODType_High && TStringManager::String8CompareNoCase( pSection->GetName(), pVISSection->pName ) == 0 )
				{
					iSectionEnd                       = pVISSection->uiFirstLocator + pVISSection->uiNumLocators;
					m_pShadeSectionRanges[ k ].iStart = m_iNumShadeSpheres;
					m_pShadeSectionRanges[ k ].iEnd   = m_iNumShadeSpheres;
					m_pLightSectionRanges[ k ].iStart = m_iNumLightSpheres;
					m_pLightSectionRanges[ k ].iEnd   = m_iNumLightSpheres;
					iSection                          = k;
					break;
				}
			}
		}

		const ATerrainLocator* pLocator = a_pLocatorList->GetLocator( i );
		const TCHAR*           szName   = a_pLocatorList->GetLocatorName( i );

		if ( TStringManager::String8CompareNoCase( "sphereshade", szName ) == 0 )
		{
			TMatrix44 matLocator;
			pLocator->GetMatrix( matLocator );

			TMatrix44 matTransform;
			matTransform.Multiply( matWorld, matLocator );

			TFLOAT fRadius = ( pLocator->uiFlags1 & 0xF ) * 0.3f;
			TFLOAT fValue  = 1.0f - ( pLocator->uiFlags1 >> 4 ) * 0.06666666f;

			if ( m_iNumShadeSpheres < m_iMaxSpheres )
			{
				const TVector3& rcPosition = matTransform.AsBasisVector3( BASISVECTOR_TRANSLATION );
				m_pShadeSpheres[ m_iNumShadeSpheres ].Set( rcPosition.x, rcPosition.y, rcPosition.z, fRadius );
				m_pShadeValues[ m_iNumShadeSpheres++ ] = fValue;
			}

#ifdef BARNYARD_COMMUNITY_PATCH
			// NOTE: retail writes before the array if no section was found yet
			if ( iSection >= 0 )
#endif
				m_pShadeSectionRanges[ iSection ].iEnd = m_iNumShadeSpheres;
		}
		else if ( TStringManager::String8CompareNoCase( "spherelight", szName ) == 0 )
		{
			TMatrix44 matLocator;
			pLocator->GetMatrix( matLocator );

			TMatrix44 matTransform;
			matTransform.Multiply( matWorld, matLocator );

			TFLOAT fRadius = ( pLocator->uiFlags1 & 0xF ) * 0.3f;
			TFLOAT fValue  = 1.0f - ( pLocator->uiFlags1 >> 4 ) * 0.06666666f;

			if ( m_iNumLightSpheres < m_iMaxSpheres )
			{
				const TVector3& rcPosition = matTransform.AsBasisVector3( BASISVECTOR_TRANSLATION );
				m_pLightSpheres[ m_iNumLightSpheres ].Set( rcPosition.x, rcPosition.y, rcPosition.z, fRadius );
				m_pLightValues[ m_iNumLightSpheres++ ] = fValue;
			}

#ifdef BARNYARD_COMMUNITY_PATCH
			// NOTE: retail writes before the array if no section was found yet
			if ( iSection >= 0 )
#endif
				m_pLightSectionRanges[ iSection ].iEnd = m_iNumLightSpheres;
		}
	}

	if ( iNumVISSections == 0 )
	{
		// No sections, every sphere is everywhere
		for ( TINT i = 0; i < m_iNumSections; i++ )
		{
			m_pShadeSectionRanges[ i ].iStart = 0;
			m_pShadeSectionRanges[ i ].iEnd   = 0;
			m_pLightSectionRanges[ i ].iStart = 0;
			m_pLightSectionRanges[ i ].iEnd   = 0;
		}
	}
}

// $Barnyard: FUNCTION 00607550
void AInstanceLightingHelper::DestroySpheres( ATerrainLocatorList* a_pLocatorList )
{
	m_iNumShadeSpheres = 0;
	m_iNumLightSpheres = 0;
	m_iNumSections     = 0;

	delete[] m_pShadeSectionRanges;
	delete[] m_pLightSectionRanges;

#ifdef BARNYARD_COMMUNITY_PATCH
	// NOTE: retail keeps the dangling pointers
	m_pShadeSectionRanges = TNULL;
	m_pLightSectionRanges = TNULL;
#endif
}

// $Barnyard: FUNCTION 00607ff0
void AInstanceLightingHelper::Update()
{
	if ( m_fShadeCoeff > 0.02f )
	{
		if ( m_fShadeCoeff < 0.98f )
		{
			// Dusk or dawn
			AModelRepos::GetSingleton()->ForEachModel( UpdateModelBlended );
		}
		else
		{
			UseLightSpheres();
			AModelRepos::GetSingleton()->ForEachModel( UpdateModel );
		}
	}
	else
	{
		UseShadeSpheres();
		AModelRepos::GetSingleton()->ForEachModel( UpdateModel );
	}
}

// $Barnyard: FUNCTION 00608080
TFLOAT AInstanceLightingHelper::GetShadeValue( const TVector4& a_rcPosition )
{
	UseShadeSpheres();

	TFLOAT fShadeCoeff = m_fShadeCoeff;
	m_fShadeCoeff      = 0.0f;
	TFLOAT fValue      = GetValueInRange( 0, m_iNumShadeSpheres, a_rcPosition );
	m_fShadeCoeff      = fShadeCoeff;

	return fValue;
}

// $Barnyard: FUNCTION 006080d0
TFLOAT AInstanceLightingHelper::GetLightValue( const TVector4& a_rcPosition )
{
	UseLightSpheres();

	TFLOAT fShadeCoeff = m_fShadeCoeff;
	m_fShadeCoeff      = 1.0f;
	TFLOAT fValue      = GetValueInRange( 0, m_iNumLightSpheres, a_rcPosition );
	m_fShadeCoeff      = fShadeCoeff;

	return fValue;
}

// $Barnyard: FUNCTION 00607a30
TFLOAT AInstanceLightingHelper::GetValueInRange( TINT a_iStart, TINT a_iEnd, const TVector4& a_rcPosition )
{
	if ( a_iStart < 0 )
		return m_fShadeCoeff;

	TFLOAT fResult = m_fShadeCoeff;
	m_bInSphere    = TFALSE;

	for ( TINT i = a_iStart; i < a_iEnd; i++ )
	{
		TFLOAT fDistanceSq = TVector4::DistanceSq( m_pCurSpheres[ i ], a_rcPosition );
		TFLOAT fRadius     = m_pCurSpheres[ i ].w;

		if ( fDistanceSq < fRadius * fRadius )
		{
			// Full effect up to 70% of the radius, then fades out
			TFLOAT fFade = ( TMath::Sqrt( fDistanceSq ) / fRadius - 0.7f ) * 3.3333333f;

			if ( fFade < 0.0f )
				fFade = 0.0f;

			TFLOAT fValue = TMath::LERP( m_pCurValues[ i ], m_fShadeCoeff, fFade );

			if ( m_bShade )
				fResult = ( fValue < fResult ) ? fResult : fValue;
			else
				fResult = ( fResult < fValue ) ? fResult : fValue;

			m_bInSphere = TTRUE;
		}
	}

	return fResult;
}

// $Barnyard: FUNCTION 00607b40
TFLOAT AInstanceLightingHelper::GetValueAtPosition( const TVector4& a_rcPosition )
{
	ATerrainInterface* pTerrain = ATerrainInterface::GetSingleton();
	TINT               iSection = pTerrain->GetSectionID();
	ATerrainVIS*       pVIS     = pTerrain->GetVIS();
	ATerrainSection*   pSection = &pVIS->GetSections()[ iSection ];

	TFLOAT fValue = GetValueInRange( m_pCurSectionRanges[ iSection ].iStart, m_pCurSectionRanges[ iSection ].iEnd, a_rcPosition );

	if ( !m_bInSphere )
	{
		// Look in the sections visible from the current one
		for ( TINT i = 0; i < pVIS->GetNumSections(); i++ )
		{
			if ( pSection->GetOtherSectionVisibility( i ) == ATerrainLODType_High && i != iSection )
			{
				fValue = GetValueInRange( m_pCurSectionRanges[ i ].iStart, m_pCurSectionRanges[ i ].iEnd, a_rcPosition );

				if ( m_bInSphere )
					break;
			}
		}
	}

	return fValue;
}

// $Barnyard: FUNCTION 00607bf0
void AInstanceLightingHelper::UpdateModel( AModel* a_pModel )
{
	AInstanceLightingHelper* pThis = GetSingleton();

	for ( TINT i = 0; i < a_pModel->GetInstances().Size(); i++ )
	{
		AModelInstance* pInstance = a_pModel->GetInstances()[ i ].Get();

		if ( !pInstance->IsUpdatingSkeleton() )
			continue;

		TVector4 vecPosition;
		pInstance->GetPosition( vecPosition );

		TFLOAT fShadeCoeff = 0.0f;

		switch ( pInstance->GetShadowMode() )
		{
			case AModelInstance::SHADOWMODE_DEFAULT:
			case AModelInstance::SHADOWMODE_UNLIT:
				fShadeCoeff = pThis->m_bShade ? pInstance->GetShadeValue() : pInstance->GetLightValue();
				break;
			case AModelInstance::SHADOWMODE_DYNAMIC:
				fShadeCoeff = pThis->GetValueAtPosition( vecPosition );
				break;
			case AModelInstance::SHADOWMODE_SECTION:
			{
				SectionRange& rRange = pThis->m_pCurSectionRanges[ pInstance->GetLightingSection() ];
				fShadeCoeff          = pThis->GetValueInRange( rRange.iStart, rRange.iEnd, vecPosition );
				break;
			}
		}

		pInstance->GetSceneObject()->SetShadeCoeff( fShadeCoeff );
	}
}

// $Barnyard: FUNCTION 00607da0
void AInstanceLightingHelper::UpdateModelBlended( AModel* a_pModel )
{
	AInstanceLightingHelper* pThis = GetSingleton();
	TFLOAT                   fTime = pThis->m_fShadeCoeff;

	// NOTE: an instance with an unknown shadow mode uses the values of the previous one
	TFLOAT fShade = 0.0f;
	TFLOAT fLight = 1.0f;

	for ( TINT i = 0; i < a_pModel->GetInstances().Size(); i++ )
	{
		AModelInstance* pInstance = a_pModel->GetInstances()[ i ].Get();

		if ( !pInstance->IsUpdatingSkeleton() )
			continue;

		TVector4 vecPosition;
		pInstance->GetPosition( vecPosition );

		switch ( pInstance->GetShadowMode() )
		{
			case AModelInstance::SHADOWMODE_DEFAULT:
			case AModelInstance::SHADOWMODE_UNLIT:
				fShade = pInstance->GetShadeValue();
				fLight = pInstance->GetLightValue();
				break;
			case AModelInstance::SHADOWMODE_DYNAMIC:
				pThis->UseShadeSpheres();
				fShade = pThis->GetValueAtPosition( vecPosition );
				pThis->UseLightSpheres();
				fLight = pThis->GetValueAtPosition( vecPosition );
				break;
			case AModelInstance::SHADOWMODE_SECTION:
			{
				pThis->UseShadeSpheres();
				SectionRange& rShadeRange = pThis->m_pCurSectionRanges[ pInstance->GetLightingSection() ];
				fShade                    = pThis->GetValueInRange( rShadeRange.iStart, rShadeRange.iEnd, vecPosition );

				pThis->UseLightSpheres();
				SectionRange& rLightRange = pThis->m_pCurSectionRanges[ pInstance->GetLightingSection() ];
				fLight                    = pThis->GetValueInRange( rLightRange.iStart, rLightRange.iEnd, vecPosition );
				break;
			}
		}

		pInstance->GetSceneObject()->SetShadeCoeff( TMath::LERP( fShade, fLight, fTime ) );
	}
}
