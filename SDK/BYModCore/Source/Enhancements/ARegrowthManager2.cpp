#include "pch.h"
#include "ARegrowthManager2.h"
#include "ACoreSettings.h"
#include "StaticLights.h"

#include <BYardSDK/AGlowViewport.h>
#include <BYardSDK/SDKHooks.h>

#include <Toshi/TSceneObject.h>
#include <Render/TRenderInterface.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// Original game caps at 240 entries
static constexpr TINT MAX_RENDERED_INSTANCES = 2048;

static ARegrowthManager2::RenderListEntry s_aRenderEntries[ MAX_RENDERED_INSTANCES ];
static TINT8                              s_aEntryStaticLights[ MAX_RENDERED_INSTANCES ][ MAX_CELL_STATIC_LIGHTS ];
static TINT8                              s_aEntryLightIDs[ MAX_RENDERED_INSTANCES ][ 4 ];

// The original __ftol2 truncates to 64 bits and only the low half is used, so huge
// growth values (restored spawn times reach -1e8) wrap instead of going negative;
// saturate so fully grown objects don't fail the iFrame <= 0 check and vanish
static TINT GrowthToFrame( TFLOAT a_fGrowth )
{
	return ( a_fGrowth >= 2147483648.0f ) ? 0x7FFFFFFF : (TINT)a_fGrowth;
}

// $Barnyard: FUNCTION 005e3270
void ARegrowthManager2::FillRenderList()
{
	TRenderContext* pContext      = TRenderInterface::GetSingleton()->GetCurrentContext();
	TVector4        vecCamPos     = pContext->GetViewWorldMatrix().GetTranslation();
	TFLOAT          fTotalTime    = *(TFLOAT*)( *(TUINT*)0x00783d3c + 0x20 ); // AGameTimeManager::ms_pInstance->m_flTotalTime
	TUINT           uiNumMappings = *(TUINT*)( *(TUINT*)0x007962e8 + 8 );     // s_pBoneHelper->m_uiNumBones
	TINT            iCount        = 0;

	for ( TUINT i = 0; i < uiNumMappings; i++ )
	{
		GrowthMapping& rMap = m_pGrowthMappings[ i ];
		if ( rMap.uiModelIndex == 0xFF ) continue;

		TFLOAT fRadius = m_pModels[ rMap.uiModelIndex ].pManagedModel->GetModel()->GetRenderDistance() * 1.2f;

		T2_FOREACH( m_aObjectLists[ i ].llUsed, pObj )
		{
			TFLOAT fGrowth = ( fTotalTime - pObj->flSpawnTime ) * rMap.flGrowthRate;
			TINT   iFrame  = GrowthToFrame( fGrowth );
			TFLOAT fNorm   = 0.0f;

			if ( rMap.uiAnimType == 1 )
			{
				if ( fGrowth <= 0.001f ) continue;
				if ( iFrame < 1 ) iFrame = 1;
				fNorm = ( fGrowth - 0.001f ) * 1.001f;
			}
			else
			{
				if ( iFrame <= 0 ) continue;
			}

			TSphere sphere( pObj->vPosition.x, pObj->vPosition.y, pObj->vPosition.z, fRadius );
			TFLOAT  fDistSq = TVector4::DistanceSq( pObj->vPosition, vecCamPos );

			if ( fDistSq >= m_fRenderDistance * m_fRenderDistance ) continue;
			if ( !TRenderContext::CullSphereToFrustumSimple( sphere, pContext->GetWorldPlanes(), 6 ) ) continue;
			if ( iCount >= MAX_RENDERED_INSTANCES ) break;

			if ( rMap.iNumFrames <= iFrame ) iFrame = rMap.iNumFrames - 1;

			RenderListEntry* pEntry = &s_aRenderEntries[ iCount++ ];
			TMatrix44        matLocal;
			CALL( 0x005e2520, void, ARegrowthManager2::Object*, pObj, TMatrix44*, &matLocal );

			pEntry->oMatrix.Multiply( m_oModelViewMatrix, matLocal );

			if ( rMap.uiAnimType == 1 )
			{
				TFLOAT fMax    = (TFLOAT)( rMap.iNumFrames - 1 );
				TFLOAT fFactor = ( fNorm < fMax ) ? ( fNorm / fMax ) : 1.0f;
				if ( iFrame == rMap.iNumFrames - 2 && fFactor >= 0.997f )
				{
					iFrame++;
					fFactor = ( fFactor - 0.997f ) * 333.331f;
				}
				TFLOAT fScale = ( pObj->uiScalePacked * ( 1.0f / 255.0f ) * 0.4f + 0.8f ) * fFactor;
				pEntry->oMatrix.Scale( fScale, fScale, fScale );
			}

			TINT iBase               = ( (TINT)rMap.uiModelIndex + iFrame ) * 2;
			TINT iSlot               = ( fDistSq < m_fLODSwitchDistance * m_fLODSwitchDistance ) ? iBase - 2 : iBase - 1;
			pEntry->pPrev            = m_ppRenderLists[ iSlot ];
			m_ppRenderLists[ iSlot ] = pEntry;

			TINT iEntry = pEntry - s_aRenderEntries;

			TLightIDList oLightList;
			AGlowViewport::GetSingleton()->GetInfluencingLightIDs( sphere, oLightList );
			s_aEntryLightIDs[ iEntry ][ 0 ] = oLightList[ 0 ];
			s_aEntryLightIDs[ iEntry ][ 1 ] = oLightList[ 1 ];
			s_aEntryLightIDs[ iEntry ][ 2 ] = oLightList[ 2 ];
			s_aEntryLightIDs[ iEntry ][ 3 ] = oLightList[ 3 ];

			GatherStaticLights( sphere, s_aEntryStaticLights[ iEntry ] );
		}
	}
}

// $Barnyard: FUNCTION 005e35d0
void ARegrowthManager2::FillRenderList_Visible()
{
	TRenderContext* pContext      = TRenderInterface::GetSingleton()->GetCurrentContext();
	TVector4        vecCamPos     = pContext->GetViewWorldMatrix().GetTranslation();
	TFLOAT          fTotalTime    = *(TFLOAT*)( *(TUINT*)0x00783d3c + 0x20 ); // AGameTimeManager::ms_pInstance->m_flTotalTime
	TUINT           uiNumMappings = *(TUINT*)( *(TUINT*)0x007962e8 + 8 );     // s_pBoneHelper->m_uiNumBones
	TFLOAT          fWaterLevel   = *(TFLOAT*)0x0071f754;
	TINT            iWaterMode    = *(TINT*)0x00773a64;
	TINT            iCount        = 0;

	for ( TUINT i = 0; i < uiNumMappings; i++ )
	{
		GrowthMapping& rMap = m_pGrowthMappings[ i ];
		if ( rMap.uiModelIndex == 0xFF ) continue;

		TBOOL  bForceScale = ( rMap.uiFlags & ( 1u << 5 ) ) != 0;
		TFLOAT fRadius     = m_pModels[ rMap.uiModelIndex ].pManagedModel->GetModel()->GetRenderDistance() * 1.2f;

		T2_FOREACH( m_aObjectLists[ i ].llUsed, pObj )
		{
			TFLOAT fGrowth = ( fTotalTime - pObj->flSpawnTime ) * rMap.flGrowthRate;
			TINT   iFrame  = GrowthToFrame( fGrowth );
			TFLOAT fNorm   = 0.0f;

			if ( rMap.uiAnimType == 1 )
			{
				if ( fGrowth <= 0.001f ) continue;
				if ( iFrame < 1 ) iFrame = 1;
				fNorm = ( fGrowth - 0.001f ) * 1.001f;
			}
			else
			{
				if ( iFrame <= 0 ) continue;
			}

			TSphere sphere( pObj->vPosition.x, pObj->vPosition.y, pObj->vPosition.z, fRadius );
			TFLOAT  fDistSq = TVector4::DistanceSq( pObj->vPosition, vecCamPos );

			if ( fDistSq >= m_fRenderDistance * m_fRenderDistance ) continue;

			// Water-level filter (DAT_0071f754 / DAT_00773a64)
			if ( !( pObj->vPosition.y <= fWaterLevel || iWaterMode == 0 ) ) continue;
			if ( !( pObj->vPosition.y > fWaterLevel || iWaterMode == 1 || iWaterMode == 2 ) ) continue;

			if ( !TRenderContext::CullSphereToFrustumSimple( sphere, pContext->GetWorldPlanes(), 6 ) ) continue;
			if ( iCount >= MAX_RENDERED_INSTANCES ) break;

			if ( rMap.iNumFrames <= iFrame ) iFrame = rMap.iNumFrames - 1;

			RenderListEntry* pEntry = &s_aRenderEntries[ iCount++ ];
			TMatrix44        matLocal;
			CALL( 0x005e2520, void, ARegrowthManager2::Object*, pObj, TMatrix44*, &matLocal );
			pEntry->oMatrix.Multiply( m_oModelViewMatrix, matLocal );

			if ( rMap.uiAnimType == 1 )
			{
				TFLOAT fMax    = (TFLOAT)( rMap.iNumFrames - 1 );
				TFLOAT fFactor = ( fNorm < fMax ) ? ( fNorm / fMax ) : 1.0f;
				if ( iFrame == rMap.iNumFrames - 2 && fFactor >= 0.997f )
				{
					iFrame++;
					fFactor = ( fFactor - 0.997f ) * 333.331f;
				}
				TFLOAT fBaseScale = bForceScale ? 1.0f : pObj->uiScalePacked * ( 1.0f / 255.0f ) * 0.4f + 0.8f;
				TFLOAT fScale     = fBaseScale * fFactor;
				pEntry->oMatrix.Scale( fScale, fScale, fScale );
			}

			TINT iBase               = ( (TINT)rMap.uiModelIndex + iFrame ) * 2;
			TINT iSlot               = ( fDistSq < m_fLODSwitchDistance * m_fLODSwitchDistance ) ? iBase - 2 : iBase - 1;
			pEntry->pPrev            = m_ppRenderLists[ iSlot ];
			m_ppRenderLists[ iSlot ] = pEntry;

			TINT iEntry = pEntry - s_aRenderEntries;

			TLightIDList oLightList;
			AGlowViewport::GetSingleton()->GetInfluencingLightIDs( sphere, oLightList );
			s_aEntryLightIDs[ iEntry ][ 0 ] = oLightList[ 0 ];
			s_aEntryLightIDs[ iEntry ][ 1 ] = oLightList[ 1 ];
			s_aEntryLightIDs[ iEntry ][ 2 ] = oLightList[ 2 ];
			s_aEntryLightIDs[ iEntry ][ 3 ] = oLightList[ 3 ];

			GatherStaticLights( sphere, s_aEntryStaticLights[ iEntry ] );
		}
	}
}

// $Barnyard: FUNCTION 005e3990
void ARegrowthManager2::Render()
{
	if ( ( m_eFlags & 1 ) == 0 )
		return;

	// Adjust parameters based on current settings (original defaults: LOD 30, render 60, blend 50)
	m_fLODSwitchDistance = ( g_oSettings.bDisableInstanceLODs ) ? 9999.0f : 30.0f;
	m_fBlendingDistance  = g_oSettings.flInstanceRenderDistance;
	m_fRenderDistance    = g_oSettings.flInstanceRenderDistance + 15.0f;
	m_fBlendingFactor    = 1.0f / 15.0f; // 1 / (render - blend)

	TRenderInterface* pRI      = TRenderInterface::GetSingleton();
	TRenderContext*   pContext = pRI->GetCurrentContext();

	m_oModelViewMatrix = pContext->GetModelViewMatrix();

	pContext->ClearLightIDs();
	ClearStaticLights( pContext );

	for ( TINT i = 0; i < m_iNumModels * 2; i++ )
		m_ppRenderLists[ i ] = TNULL;

	// Point render buffer at our static storage
	m_pRenderBuffer = s_aRenderEntries;

	// Perspective far-clip cap: clamp render distance for glow-light shadow passes.
	// Orthographic CSM passes are not affected.
	const TBOOL  bIsPerspective    = pContext->GetCameraMode() == TRenderContext::CameraMode_Perspective;
	const TFLOAT flFarClipLimit    = bIsPerspective ? pContext->GetProjectionParams().m_fFarClip : FLT_MAX;
	const TFLOAT flSavedRenderDist = m_fRenderDistance;
	m_fRenderDistance              = TMath::Min( m_fRenderDistance, flFarClipLimit );

	if ( m_eFlags < 0 )
		FillRenderList_Visible();
	else
		FillRenderList();

	m_fRenderDistance = flSavedRenderDist;

	const TFLOAT flOldShadeCoeff = pContext->GetShadeCoeff();
	pContext->SetShadeCoeff( 0.0f );

	const TUINT uiOldClipFlags = pContext->GetClipFlags();

	for ( TINT i = 0; i < m_iNumModels * 2; i++ )
	{
		if ( m_ppRenderLists[ i ] == TNULL ) continue;

		TSceneObject*   pSceneObj  = m_pModels[ i >> 1 ].pSceneObject;
		TModelInstance* pModelInst = pSceneObj->GetInstance();
		TModel*         pModel     = pModelInst->GetModel();
		TINT            iLOD       = TMath::Min( i & 1, pModel->GetNumLODs() - 1 );
		TModelLOD&      rLOD       = pModel->GetLOD( iLOD );

		pContext->SetSkeletonInstance( pModelInst->GetSkeletonInstance() );

		for ( RenderListEntry* pEntry = m_ppRenderLists[ i ];
		      pEntry != TNULL;
		      pEntry = pEntry->pPrev )
		{
			TFLOAT fViewZ = pEntry->oMatrix.m_f43;
			if ( fViewZ >= m_fBlendingDistance )
			{
				TFLOAT fBlend = 1.0f - ( fViewZ - m_fBlendingDistance ) * m_fBlendingFactor;
				if ( fBlend <= 0.0f ) continue;
				pContext->SetAlphaBlend( fBlend );
			}
			else
			{
				pContext->SetAlphaBlend( 1.0f );
			}

			TINT iEntry = pEntry - s_aRenderEntries;

			pContext->ClearLightIDs();
			TINT8* pLightIds = s_aEntryLightIDs[ iEntry ];
			if ( pLightIds[ 0 ] != -1 ) pContext->AddLight( pLightIds[ 0 ] );
			if ( pLightIds[ 1 ] != -1 ) pContext->AddLight( pLightIds[ 1 ] );
			if ( pLightIds[ 2 ] != -1 ) pContext->AddLight( pLightIds[ 2 ] );
			if ( pLightIds[ 3 ] != -1 ) pContext->AddLight( pLightIds[ 3 ] );

			ClearStaticLights( pContext );
			AddStaticLights( pContext, s_aEntryStaticLights[ iEntry ] );

			pContext->SetModelViewMatrix( pEntry->oMatrix );
			for ( TINT k = 0; k < rLOD.iNumMeshes; k++ )
				rLOD.ppMeshes[ k ]->Render();
		}
	}

	pContext->SetClipFlags( uiOldClipFlags );
	pContext->SetShadeCoeff( flOldShadeCoeff );
	pContext->SetAlphaBlend( 1.0f );
	pContext->SetModelViewMatrix( m_oModelViewMatrix );
	pContext->ClearLightIDs();
	ClearStaticLights( pContext );
}
