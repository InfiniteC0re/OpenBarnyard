#include "pch.h"
#include "AInstanceManager2.h"
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

static TMatrix44 s_mInstanceTransform = {
	1.0f, 0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 1.0f, 0.0f,
	0.0f, -1.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f, 1.0f
};

static constexpr TUINT                   MAX_RENDERED_INSTANCES = 1024;
static TBOOL                             s_aInstanceVisMasks[ MAX_RENDERED_INSTANCES ];
static TUINT8                            s_aInstanceFlags[ MAX_RENDERED_INSTANCES ];
static TUINT8                            s_aInstanceLightIDs[ MAX_RENDERED_INSTANCES ];
static AInstanceManager::RenderListEntry s_aRenderList[ MAX_RENDERED_INSTANCES ];

TBOOL AInstanceManager2::Render()
{
	// Adjust parameters based on current settings
	m_flVisibilityDistanceBlendStart = g_oSettings.flInstanceRenderDistance;
	m_flVisibilityDistanceBlendEnd   = g_oSettings.flInstanceRenderDistance + 15.0f;

	if ( g_oSettings.bDisableInstanceRendering ) return TTRUE;

	TRenderContext* pRenderContext = g_pRender->GetCurrentContext();
	TMatrix44       matModelView   = pRenderContext->GetModelViewMatrix();

	// Reset render lists
	for ( TINT i = 0; i < m_iNumModels * MAX_NUM_LODS; i++ )
		m_pModelsRenderLists[ i ] = TNULL;

	const TFLOAT flVisibilityDistanceBlendStart = m_flVisibilityDistanceBlendStart;
	const TFLOAT flBlendFactor                  = 1.0f / ( m_flVisibilityDistanceBlendEnd - flVisibilityDistanceBlendStart );

	TINT iRenderListSize = 0;
	if ( m_uiFlags & 16 )
	{
		// Don't check terrain section visibility

		// Static instances
		FillRenderList_All( m_llUsedInstances, matModelView, s_aRenderList, iRenderListSize, TFALSE );

		// Dynamic instances
		if ( m_pDynamicInstances ) FillRenderList_All( m_llUsedDynamicInstances, matModelView, s_aRenderList, iRenderListSize, TTRUE );
	}
	else
	{
		// Check terrain section visibility

		// Static instances
		FillRenderList_Visible( m_llUsedInstances, matModelView, s_aRenderList, iRenderListSize, TFALSE );

		// Dynamic instances
		if ( m_pDynamicInstances ) FillRenderList_Visible( m_llUsedDynamicInstances, matModelView, s_aRenderList, iRenderListSize, TTRUE );
	}

	const TUINT uiOldClipFlags = pRenderContext->GetClipFlags();

	// Calculate light direction matrices
	TMatrix44 matLightDir = g_pRender->GetLightDirection();

	TMatrix44 aLightMatrices[ 16 ];
	for ( TINT i = 0; i < TARRAYSIZE( aLightMatrices ); i++ )
	{
		aLightMatrices[ i ].ScaleAll( matLightDir, i * ( 1.0f / ( TARRAYSIZE( aLightMatrices ) - 1 ) ) );
	}

	const TFLOAT flOldShadeCoeff = pRenderContext->GetShadeCoeff();

	TTODO( "Use DAT_0079b14c and a value from it as shade coeff" );
	pRenderContext->SetShadeCoeff( 0.0f );

	// Render everything
	for ( TINT i = 0; i < m_iNumModels * MAX_NUM_LODS; i++ )
	{
		if ( !m_pModelsRenderLists[ i ] ) continue;

		TModelInstance* pModelInstance = m_pModels[ i / MAX_NUM_LODS ].pSceneObject->GetInstance();
		TModel*         pModel         = pModelInstance->GetModel();

		const TINT iLODId = g_oSettings.bDisableInstanceLODs ? 0 : TMath::Min( i % MAX_NUM_LODS, pModel->GetNumLODs() - 1 );
		TModelLOD* pLOD   = &pModel->GetLOD( iLODId );

		pRenderContext->SetSkeletonInstance( pModelInstance->GetSkeletonInstance() );

		// Render every instance of this model
		RenderListEntry* pRenderList = m_pModelsRenderLists[ i ];
		while ( pRenderList )
		{
			// Set blending or don't draw instance that is completely blend out
			if ( pRenderList->matTransform.m_f43 >= flVisibilityDistanceBlendStart )
			{
				TFLOAT fBlend = ( pRenderList->matTransform.m_f43 - flVisibilityDistanceBlendStart ) * flBlendFactor;

				if ( fBlend < 1.0f )
				{
					pRenderContext->SetAlphaBlend( 1.0f - fBlend );
				}
				else
				{
					pRenderList = pRenderList->pNext;
					continue;
				}
			}
			else
			{
				pRenderContext->SetAlphaBlend( 1.0f );
			}

			pRenderContext->SetClipFlags( s_aInstanceVisMasks[ pRenderList - s_aRenderList ] );
			pRenderContext->SetModelViewMatrix( pRenderList->matTransform );

			for ( TINT k = 0; k < pLOD->iNumMeshes; k++ )
			{
				pLOD->ppMeshes[ k ]->Render();
			}

			pRenderList = pRenderList->pNext;
		}
	}

	// Restore context state
	pRenderContext->SetShadeCoeff( flOldShadeCoeff );
	pRenderContext->SetClipFlags( uiOldClipFlags );
	pRenderContext->SetAlphaBlend( 1.0f );
	pRenderContext->SetModelViewMatrix( matModelView );
	return TTRUE;
}

void AInstanceManager2::FillRenderList_All( const Toshi::T2SList<StaticInstanceEntry>& a_rcInstanceList, const Toshi::TMatrix44& a_rcWorldTransform, RenderListEntry* a_pOutRenderData, TINT& a_rNumInstances, TBOOL a_bDynamic )
{
	const TFLOAT flLOD0DistanceSq      = m_flLOD0Distance * m_flLOD0Distance;
	const TFLOAT flMaxRenderDistanceSq = m_flVisibilityDistanceBlendEnd * m_flVisibilityDistanceBlendEnd;

	TRenderContext*  pRenderContext = g_pRender->GetCurrentContext();
	const TMatrix44& matViewWorld   = pRenderContext->GetViewWorldMatrix();
	TVector4         vecCameraPos   = matViewWorld.GetTranslation();

	T2_FOREACH( a_rcInstanceList, it )
	{
		const TVector3* pInstancePosition = ( a_bDynamic ) ? (TVector3*)( (TCHAR*)it.Get() + 0x3C ) : &it->pLocatorList->m_pLocatorsHeader->pLocators[ it->uiLocatorIndex ].vecPosition;

		// Check whether the instance is visible
		TSphere vecInstanceBounding(
		    pInstancePosition->x,
		    -pInstancePosition->z,
		    pInstancePosition->y,
		    g_oSettings.flInstanceBoundingMultiplier * m_pModels[ it->uiModelIndex ].pManagedModel->GetModel()->GetRenderDistance()
		);

		const TFLOAT flInstanceDistanceSq = TVector4::DistanceSq( vecInstanceBounding.AsVector4(), vecCameraPos );

		const TINT& DAT_00773a64 = *(TINT*)0x00773a64;

		// There's some odd hack I don't currently understand
		if ( flInstanceDistanceSq < flMaxRenderDistanceSq &&
		     ( vecInstanceBounding.GetOrigin().y <= 3.246f || DAT_00773a64 == 0 ) &&
		     ( vecInstanceBounding.GetOrigin().y >= 3.246f || DAT_00773a64 == 1 || DAT_00773a64 == 2 ) )
		{
			TINT iLODIndex = flInstanceDistanceSq >= flLOD0DistanceSq ? 1 : 0;

			const TBOOL bVisibleInFrustum = pRenderContext->CullSphereToFrustumSimple( vecInstanceBounding, pRenderContext->GetWorldPlanes(), 6 );
			if ( bVisibleInFrustum )
			{
				TMatrix44 matLocatorLocalTransform;

				if (a_bDynamic)
				{
					matLocatorLocalTransform = *(TMatrix44*)( (TCHAR*)it.Get() + 0xC );
				}
				else
				{
					it->pLocatorList->m_pLocatorsHeader->pLocators[ it->uiLocatorIndex ].GetMatrix( matLocatorLocalTransform );
				}

				if ( a_rNumInstances >= g_oSettings.iInstanceMaxRendered ) return;

				// Collect light sources
				Toshi::TLightIDList oLightList;
				AGlowViewport::GetSingleton()->GetInfluencingLightIDs( vecInstanceBounding, oLightList );

				// Store data about the instance
				s_aInstanceVisMasks[ a_rNumInstances ] = TTRUE;
				s_aInstanceFlags[ a_rNumInstances ]    = it->uiPackedScaleYaw >> 4;
				s_aInstanceLightIDs[ a_rNumInstances ] = oLightList[ 0 ];

				const TINT   iModelSlot     = ( it->uiModelIndex * MAX_NUM_LODS ) + iLODIndex;
				const TFLOAT flLocatorScale = ( it->uiPackedScaleYaw & 0b1111 ) * 0.2f;

				// Scale and apply global transform
				TMatrix44 matLocatorTransform;
				matLocatorTransform.Multiply( s_mInstanceTransform, matLocatorLocalTransform );
				matLocatorTransform.Scale( flLocatorScale, flLocatorScale, flLocatorScale );

				// Add to the render list, applying world transform
				RenderListEntry* pRenderListEntry = &a_pOutRenderData[ a_rNumInstances++ ];
				pRenderListEntry->matTransform.Multiply( a_rcWorldTransform, matLocatorTransform );
				pRenderListEntry->pNext = m_pModelsRenderLists[ iModelSlot ];

				m_pModelsRenderLists[ iModelSlot ] = pRenderListEntry;
			}
		}
	}
}

void AInstanceManager2::FillRenderList_Visible( const Toshi::T2SList<StaticInstanceEntry>& a_rcInstanceList, const Toshi::TMatrix44& a_rcWorldTransform, RenderListEntry* a_pOutRenderData, TINT& a_rNumInstances, TBOOL a_bDynamic )
{
	//-----------------------------------------------------------------------------
	// 1. Gather visible sections
	//-----------------------------------------------------------------------------

	// Get terrain related objects
	ATerrainInterface* pTerrainInterface = ATerrainInterface::GetSingleton();
	ATerrainVIS*       pTerrainVIS       = pTerrainInterface->GetVIS();
	TINT               iNumSections      = pTerrainVIS->m_iNumSections;
	ATerrainSection*   pCurrentSection   = &pTerrainVIS->m_pSections[ pTerrainInterface->m_iCurrentSection ];

	ATerrainLocatorList* pLocatorList = pTerrainInterface->m_pTerrainVIS->GetLocatorList();
	if ( !pLocatorList->m_pLocatorVISHeader ) return;

	TINT iLocatorsNumVISSections = pLocatorList->m_pLocatorVISHeader->iNumSections;

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

	//-----------------------------------------------------------------------------
	// 2. Fill the render list
	//-----------------------------------------------------------------------------

	const TFLOAT flLOD0DistanceSq      = m_flLOD0Distance * m_flLOD0Distance;
	const TFLOAT flMaxRenderDistanceSq = m_flVisibilityDistanceBlendEnd * m_flVisibilityDistanceBlendEnd;

	TRenderContext*  pRenderContext = g_pRender->GetCurrentContext();
	const TMatrix44& matViewWorld   = pRenderContext->GetViewWorldMatrix();
	TVector4         vecCameraPos   = matViewWorld.GetTranslation();

	T2_FOREACH( a_rcInstanceList, it )
	{
		const TUINT uiLocatorIndex = it->uiLocatorIndex;

		TUINT uiNumRenderedSections = 0;
		for ( TUINT i = 0; i < uiNumSectionsToRender; i++ )
		{
			const SectionRenderData& rSectionData = aSectionRenderData[ i ];

			// Skip all other sections if rendered the required number
			if ( uiNumRenderedSections >= uiNumSectionsToRender ) break;

			if ( uiLocatorIndex >= rSectionData.uiStart && uiLocatorIndex < rSectionData.uiEnd )
			{
				const TVector3* pInstancePosition = ( a_bDynamic ) ? (TVector3*)( (TCHAR*)it.Get() + 0x3C ) : &it->pLocatorList->m_pLocatorsHeader->pLocators[ it->uiLocatorIndex ].vecPosition;

				// Check whether the instance is visible
				TSphere vecInstanceBounding(
				    pInstancePosition->x,
				    -pInstancePosition->z,
				    pInstancePosition->y,
				    g_oSettings.flInstanceBoundingMultiplier * m_pModels[ it->uiModelIndex ].pManagedModel->GetModel()->GetRenderDistance()
				);

				const TFLOAT flInstanceDistanceSq = TVector4::DistanceSq( vecInstanceBounding.AsVector4(), vecCameraPos );

				if ( flInstanceDistanceSq < flMaxRenderDistanceSq )
				{
					TINT iLODIndex = flInstanceDistanceSq >= flLOD0DistanceSq ? 1 : 0;

					const TBOOL bVisibleInFrustum = pRenderContext->CullSphereToFrustumSimple( vecInstanceBounding, pRenderContext->GetWorldPlanes(), 6 );
					if ( bVisibleInFrustum )
					{
						TMatrix44 matLocatorLocalTransform;

						if ( a_bDynamic )
						{
							matLocatorLocalTransform = *(TMatrix44*)( (TCHAR*)it.Get() + 0xC );
						}
						else
						{
							it->pLocatorList->m_pLocatorsHeader->pLocators[ it->uiLocatorIndex ].GetMatrix( matLocatorLocalTransform );
						}

						if ( a_rNumInstances >= g_oSettings.iInstanceMaxRendered ) return;

						// Collect light sources
						Toshi::TLightIDList oLightList;
						AGlowViewport::GetSingleton()->GetInfluencingLightIDs( vecInstanceBounding, oLightList );

						// Store data about the instance
						s_aInstanceVisMasks[ a_rNumInstances ] = TTRUE;
						s_aInstanceFlags[ a_rNumInstances ]    = it->uiPackedScaleYaw >> 4;
						s_aInstanceLightIDs[ a_rNumInstances ] = oLightList[ 0 ];

						const TINT   iModelSlot     = ( it->uiModelIndex * MAX_NUM_LODS ) + iLODIndex;
						const TFLOAT flLocatorScale = ( it->uiPackedScaleYaw & 0b1111 ) * 0.2f;

						// Scale and apply global transform
						TMatrix44 matLocatorTransform;
						matLocatorTransform.Multiply( s_mInstanceTransform, matLocatorLocalTransform );
						matLocatorTransform.Scale( flLocatorScale, flLocatorScale, flLocatorScale );

						// Add to the render list, applying world transform
						RenderListEntry* pRenderListEntry = &a_pOutRenderData[ a_rNumInstances++ ];
						pRenderListEntry->matTransform.Multiply( a_rcWorldTransform, matLocatorTransform );
						pRenderListEntry->pNext = m_pModelsRenderLists[ iModelSlot ];

						m_pModelsRenderLists[ iModelSlot ] = pRenderListEntry;
					}
				}
			}
		}
	}
}
