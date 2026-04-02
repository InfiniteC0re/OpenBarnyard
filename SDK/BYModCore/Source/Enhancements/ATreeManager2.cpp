#include "pch.h"
#include "ATreeManager2.h"
#include "ACoreSettings.h"

#include <BYardSDK/AGlowViewport.h>
#include <BYardSDK/ATerrainInterface.h>

#include <Render/TRenderInterface.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

static constexpr TUINT                       MAX_RENDERED_INSTANCES = 240;
static ATreeManager2::TreeInstanceRenderData s_aInstanceMatrices[ MAX_RENDERED_INSTANCES ];

void ATreeManager2::Render()
{
	// NOTE: just a stub used to spoof settings
	// Adjust parameters based on current settings

	// m_fLODSwitchDistance
	*(TFLOAT*)( (TCHAR*)this + 0x6698 ) = ( g_oSettings.bDisableInstanceLODs ) ? 9999.0f : 30.0f;

	// m_fBlendingDistance
	*(TFLOAT*)( (TCHAR*)this + 0x66A0 ) = g_oSettings.flInstanceRenderDistance;

	// m_fRenderDistance
	*(TFLOAT*)( (TCHAR*)this + 0x669C ) = g_oSettings.flInstanceRenderDistance + 15.0f;

	AGlowViewport*    pGlowViewport    = AGlowViewport::GetSingleton();
	TRenderInterface* pRenderInterface = TRenderInterface::GetSingleton();
	TRenderContext*   pRenderContext   = pRenderInterface->GetCurrentContext();

	TMatrix44 matModelView = pRenderContext->GetModelViewMatrix();
	TMatrix44 matLightDir  = pRenderInterface->GetLightDirection();

	TINT iNumLocatorsToRender = 0;

	// Reset render data
	for ( TINT i = 0; i < m_iNumTrunks * 2; i++ )
		m_pRenderData[ i ] = 0;

	// Get terrain related objects
	ATerrainInterface* pTerrainInterface = ATerrainInterface::GetSingleton();
	ATerrainVIS*       pTerrainVIS       = pTerrainInterface->GetVIS();
	TINT               iNumSections      = pTerrainVIS->m_iNumSections;
	ATerrainSection*   pCurrentSection   = &pTerrainVIS->m_pSections[ pTerrainInterface->m_iCurrentSection ];

	ATerrainLocatorList* pLocatorList            = pTerrainInterface->m_pTerrainVIS->GetLocatorList();
	TINT                 iLocatorsNumVISSections = pLocatorList->m_pLocatorVISHeader->iNumSections;

	// Collect info about the locators we should render
	constexpr TUINT MAX_SECTIONS = 64;

	TUINT uiNumSectionsToRender = 0;
	struct SectionRenderData
	{
		TUINT uiStart;
		TUINT uiEnd;
	} aSectionRenderData[ MAX_SECTIONS ];

	for ( TINT i = 0; i < iLocatorsNumVISSections; i++ )
	{
		ATerrainLocatorVISSection* pLocatorVISSection = &pLocatorList->m_pLocatorVISHeader->pSections[ i ];

		if ( !TStringManager::String8CompareNoCase( "undefined", pLocatorVISSection->pName ) )
		// undefined cell - always visible
		{
			TUINT uiFirstLocator = pLocatorVISSection->uiFirstLocator;
			TUINT uiNumLocators  = pLocatorVISSection->uiNumLocators;

			aSectionRenderData[ uiNumSectionsToRender ].uiStart = uiFirstLocator;
			aSectionRenderData[ uiNumSectionsToRender ].uiEnd   = uiFirstLocator + uiNumLocators;

			uiNumSectionsToRender++;
		}
		else
		// some other cell - should check visibility
		{
			for ( TINT k = 0; k < iNumSections; k++ )
			{
				// If the section is visible from the current one and locator VIS name matches it, add to the draw list
				if ( pCurrentSection->IsOtherSectionVisible( k ) &&
				     !TStringManager::String8CompareNoCase( pTerrainVIS->m_pSections[ k ].m_szName, pLocatorVISSection->pName ) )
				{
					TUINT uiFirstLocator = pLocatorVISSection->uiFirstLocator;
					TUINT uiNumLocators  = pLocatorVISSection->uiNumLocators;

					aSectionRenderData[ uiNumSectionsToRender ].uiStart = uiFirstLocator;
					aSectionRenderData[ uiNumSectionsToRender ].uiEnd   = uiFirstLocator + uiNumLocators;

					uiNumSectionsToRender++;

					break;
				}
			}
		}
	}

	if ( iLocatorsNumVISSections == 0 )
	{
		// Compare against all locators if no VIS data is provided
		aSectionRenderData[ uiNumSectionsToRender ].uiStart = 0;
		aSectionRenderData[ uiNumSectionsToRender ].uiEnd   = 2147483647;
		uiNumSectionsToRender += 1;
	}

	TASSERT( uiNumSectionsToRender < MAX_SECTIONS );

	// Do culling
	TFLOAT fRenderDistanceSq    = m_fRenderDistance * m_fRenderDistance;
	TFLOAT fLODSwitchDistanceSq = m_fLODSwitchDistance * m_fLODSwitchDistance;
	TFLOAT fBlendingFactor      = 1.0f / ( m_fRenderDistance - m_fBlendingDistance );

	TMatrix44 matGlobalTransform;
	matGlobalTransform.m_f11 = 1.0f;
	matGlobalTransform.m_f12 = 0.0f;
	matGlobalTransform.m_f13 = 0.0f;
	matGlobalTransform.m_f14 = 0.0f;
	matGlobalTransform.m_f21 = 0.0f;
	matGlobalTransform.m_f22 = 0.0f;
	matGlobalTransform.m_f23 = 1.0f;
	matGlobalTransform.m_f24 = 0.0f;
	matGlobalTransform.m_f31 = 0.0f;
	matGlobalTransform.m_f32 = -1.0f;
	matGlobalTransform.m_f33 = 0.0f;
	matGlobalTransform.m_f34 = 0.0f;
	matGlobalTransform.m_f41 = 0.0f;
	matGlobalTransform.m_f42 = 0.0f;
	matGlobalTransform.m_f43 = 0.0f;
	matGlobalTransform.m_f44 = 1.0f;

	TVector4 vecViewPos = pRenderContext->GetViewWorldMatrix().GetTranslation();

	struct LocatorRenderData
	{
		TUINT8 uiFlags;
		TUINT8 uiUnused;
		TUINT8 uiLightId;
	} aLocatorRenderData[ MAX_RENDERED_INSTANCES ];

	if ( !m_llUsedTreeInstances.IsEmpty() )
	{
		TINT iInstanceIndex = 0;

		T2_FOREACH( m_llUsedTreeInstances, pTreeInstance )
		{
			TUINT uiInstanceLocatorId = pTreeInstance->uiLocatorId;

			for ( TUINT i = 0; i < uiNumSectionsToRender; i++ )
			{
				if ( aSectionRenderData[ i ].uiStart <= uiInstanceLocatorId && uiInstanceLocatorId < aSectionRenderData[ i ].uiEnd )
				// this tree instance relates to this VIS section
				{
					Model*           pTrunk    = &m_pTrunks[ pTreeInstance->iTrunkIndex ];
					ATerrainLocator* pLocators = pTreeInstance->pLocatorList->m_pLocatorsHeader->pLocators;

					ATerrainLocator* pLocator = &pLocators[ uiInstanceLocatorId ];
					TFLOAT           fScale   = pLocator->GetScale();

					TSphere boundingVolume( pLocator->vecPosition.x, -pLocator->vecPosition.z + pTrunk->m_fGroundOffset, pLocator->vecPosition.y, fScale * pTrunk->m_fRadius );
					TFLOAT  fDistanceSq = TVector4::DistanceSq( boundingVolume.AsVector4(), vecViewPos );

					if ( fDistanceSq < fRenderDistanceSq )
					{
						const TBOOL bIsLocatorFar = fLODSwitchDistanceSq <= fDistanceSq;

						if ( TRenderContext::CullSphereToFrustumSimple( boundingVolume, pRenderContext->GetWorldPlanes(), 6 ) &&
						     ( !bIsLocatorFar || bIsLocatorFar ) // yeah, that's stupid, but it's probably a leftover from testing
						)
						{
							// Get transform of this locator
							TMatrix44 matLocator;
							pLocator->GetMatrix( matLocator );

							// Go render if exceeded the max limit of instances to render
							if ( iInstanceIndex >= MAX_RENDERED_INSTANCES )
								goto START_RENDERING;

							TreeInstanceRenderData* pInstanceRenderData = &s_aInstanceMatrices[ iInstanceIndex ];
							TINT                    iRenderDataIndex    = pTreeInstance->iTrunkIndex * 2;

							// Use second LOD if the locator is away
							if ( bIsLocatorFar ) iRenderDataIndex += 1;

							aLocatorRenderData[ iNumLocatorsToRender ].uiFlags  = pLocators[ uiInstanceLocatorId ].GetFlags1();
							aLocatorRenderData[ iNumLocatorsToRender ].uiUnused = 0x3F;

							// Get list of lights
							TLightIDList oLightIdList;
							pGlowViewport->GetInfluencingLightIDs( boundingVolume, oLightIdList );

							aLocatorRenderData[ iNumLocatorsToRender ].uiFlags |= pTreeInstance->iColor << 4;
							aLocatorRenderData[ iNumLocatorsToRender ].uiLightId = oLightIdList[ 0 ];

							iNumLocatorsToRender += 1;
							iInstanceIndex += 1;

							// Build instance matrix
							TMatrix44 matTransform;
							matTransform.Multiply( matGlobalTransform, matLocator );

							if ( pTreeInstance->unk )
							{
								TASSERT( TFALSE && "It's time to implement this" );
							}

							pInstanceRenderData->oMatrix.Multiply( matModelView, matTransform );
							pInstanceRenderData->oMatrix.Scale( fScale );

							pInstanceRenderData->pPrev        = m_pRenderData[ iRenderDataIndex ];
							m_pRenderData[ iRenderDataIndex ] = pInstanceRenderData;
						}
					}
				}
			}
		}
	}

START_RENDERING:
	static TVector4 s_vecUnused1 = TVector4( 0.7372549f, 0.8156863f, 0.5254902f );
	static TVector4 s_vecUnused2 = TVector4( 0.54509807f, 0.60784316f, 0.47058824f );
	static TVector4 s_vecUnused3 = TVector4( 0.9529412f, 0.75686276f, 0.54509807f );

	// Calculate light direction matrices
	TMatrix44 aLightMatrices[ 16 ];
	for ( TINT i = 0; i < TARRAYSIZE( aLightMatrices ); i++ )
	{
		aLightMatrices[ i ].ScaleAll( matLightDir, i * ( 1.0f / ( TARRAYSIZE( aLightMatrices ) - 1 ) ) );
	}

	// Finally, do the rendering!!!
	for ( TINT i = 0; i < m_iNumTrunks * 2; i++ )
	{
		if ( m_pRenderData[ i ] == TNULL )
			continue;

		Model*        pTrunkModel  = &m_pTrunks[ i / 2 ];
		TSceneObject* pSceneObject = pTrunkModel->m_pSceneObject;

		// Decide which LOD to render
		TINT iLODIndex = i & 1;

		// Setup skeleton instance and make sure the LOD exists
		if ( pSceneObject )
		{
			iLODIndex = TMath::Min( pSceneObject->GetInstance()->GetModel()->GetNumLODs() - 1, iLODIndex );
			TASSERT( iLODIndex >= 0 );

			pRenderContext->SetSkeletonInstance( pSceneObject->GetInstance()->GetSkeletonInstance() );
		}

		// Render recursively
		TreeInstanceRenderData* pInstanceRenderData = m_pRenderData[ i ];
		while ( pInstanceRenderData != TNULL )
		{
			// Set blending or don't draw instance that is completely blend out
			if ( pInstanceRenderData->oMatrix.m_f43 >= m_fBlendingDistance )
			{
				TFLOAT fBlend = ( pInstanceRenderData->oMatrix.m_f43 - m_fBlendingDistance ) * fBlendingFactor;

				if ( fBlend < 1.0f )
				{
					pRenderContext->SetAlphaBlend( 1.0f - fBlend );
				}
				else
				{
					pInstanceRenderData = pInstanceRenderData->pPrev;
					continue;
				}
			}
			else
			{
				pRenderContext->SetAlphaBlend( 1.0f );
			}

			// Setup lighting settings
			TINT iInstanceIndex = pInstanceRenderData - s_aInstanceMatrices;
			pRenderInterface->SetLightDirectionMatrix( aLightMatrices[ aLocatorRenderData[ iInstanceIndex ].uiFlags % TARRAYSIZE( aLightMatrices ) ] );

			// Setup model view matrix
			pRenderContext->SetModelViewMatrix( pInstanceRenderData->oMatrix );

			// Render trunk model
			if ( pSceneObject )
			{
				TModel*    pModel = pSceneObject->GetInstance()->GetModel();
				TModelLOD& rLOD   = pModel->GetLOD( iLODIndex );

				for ( TINT k = 0; k < rLOD.iNumMeshes; k++ )
					rLOD.ppMeshes[ k ]->Render();
			}

			// Render FOB model
			Model* pFOBModel = &m_pFOBs[ pTrunkModel->m_uiID * 2 ];
			pRenderContext->SetSkeletonInstance( pFOBModel->m_pSceneObject->GetInstance()->GetSkeletonInstance() );

			for ( TINT k = 0; k < pTrunkModel->m_iNumFOBs; k++ )
			{
				TMatrix44 matFOBTransform;
				matFOBTransform.Multiply( pInstanceRenderData->oMatrix, pTrunkModel->m_pFOBMatrices[ k ] );

				pRenderContext->SetModelViewMatrix( matFOBTransform );

				TModel*    pModel = pFOBModel->m_pSceneObject->GetInstance()->GetModel();
				TModelLOD& rLOD   = pModel->GetLOD( 0 );

				for ( TINT k = 0; k < rLOD.iNumMeshes; k++ )
					rLOD.ppMeshes[ k ]->Render();
			}

			pInstanceRenderData = pInstanceRenderData->pPrev;
		}
	}

	pRenderInterface->SetLightDirectionMatrix( matLightDir );
	pRenderContext->SetAlphaBlend( 1.0f );
	pRenderContext->SetModelViewMatrix( matModelView );
}
