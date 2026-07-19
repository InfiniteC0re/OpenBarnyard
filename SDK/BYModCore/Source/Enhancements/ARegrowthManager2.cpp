#include "pch.h"
#include "ARegrowthManager2.h"
#include "StaticLights.h"

#include <Toshi/TSceneObject.h>
#include <Render/TRenderInterface.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

struct RegrowthRenderEntry
{
	TMatrix44            oMatrix;  // 0x00
	RegrowthRenderEntry* pPrev;   // 0x40
};
static_assert( sizeof( RegrowthRenderEntry ) == 0x44, "RegrowthRenderEntry must be 0x44 bytes" );

static RegrowthRenderEntry s_aRenderEntries[ 240 ];
static TINT8 s_aEntryStaticLights[ 240 ][ MAX_CELL_STATIC_LIGHTS ];

// $Barnyard: FUNCTION 005e3270
void ARegrowthManager2::FillRenderList()
{
	TRenderContext* pContext  = TRenderInterface::GetSingleton()->GetCurrentContext();
	TFLOAT fRenderDist        = *(TFLOAT*)( (TCHAR*)this + 0x133D4 );
	TFLOAT fLODDist           = *(TFLOAT*)( (TCHAR*)this + 0x133D0 );
	TMatrix44& rMVP           = *(TMatrix44*)( (TCHAR*)this + 0x133E0 );
	TVector4 vecCamPos        = pContext->GetViewWorldMatrix().GetTranslation();
	TFLOAT fTotalTime         = *(TFLOAT*)( *(TUINT*)0x00783d3c + 0x20 );
	TUINT uiNumBones          = *(TUINT*)( *(TUINT*)0x007962e8 + 8 );
	TUINT uiFieldBase         = *(TUINT*)( (TCHAR*)this + 0x08 );
	TCHAR* pGrowthBase        = *(TCHAR**)( (TCHAR*)this + 0x0C );
	TINT* pModelData          = *(TINT**)( (TCHAR*)this + 0x10 );
	TINT iCount               = 0;

	for ( TUINT i = 0; i < uiNumBones; i++ )
	{
		TCHAR*  pMap     = pGrowthBase + i * 0x28;
		TUINT8  uiModel  = *(TUINT8*)( pMap + 0x20 );
		if ( uiModel == 0xFF ) continue;

		TUINT8  animType  = *(TUINT8*)( pMap + 0x21 );
		TFLOAT  growthRate = *(TFLOAT*)( pMap + 0x00 );
		TINT    numFrames  = *(TINT*)( pMap + 0x04 );
		TUINT*  pSentinel  = (TUINT*)( (TCHAR*)this + 0x1C + i * 8 + 4 );  // &m_aObjectLists[i].llUsed
		TUINT*  pObj       = (TUINT*)*pSentinel;
		if ( pObj == pSentinel ) continue;

		// Pre-compute bounding radius: field2_0x8[modelIndex] -> * -> [+0x10] * 1.2
		TUINT uiRP1    = *(TUINT*)( uiFieldBase + uiModel * 8 );
		TUINT uiRP2    = *(TUINT*)uiRP1;
		TFLOAT fRadius = *(TFLOAT*)( uiRP2 + 0x10 ) * 1.2f;

		do
		{
			TFLOAT fGrowth = ( fTotalTime - *(TFLOAT*)( (TCHAR*)pObj + 0x24 ) ) * growthRate;
			TINT   iFrame  = (TINT)fGrowth;
			TFLOAT fNorm   = 0.0f;

			if ( animType == 1 )
			{
				if ( fGrowth <= 0.001f ) { pObj = (TUINT*)*pObj; continue; }
				if ( iFrame < 1 ) iFrame = 1;
				fNorm = ( fGrowth - 0.001f ) * 1.001f;
			}
			else
			{
				if ( iFrame <= 0 ) { pObj = (TUINT*)*pObj; continue; }
			}

			TFLOAT fPX = *(TFLOAT*)( (TCHAR*)pObj + 0x14 );
			TFLOAT fPY = *(TFLOAT*)( (TCHAR*)pObj + 0x18 );
			TFLOAT fPZ = *(TFLOAT*)( (TCHAR*)pObj + 0x1C );
			TSphere sphere( fPX, fPY, fPZ, fRadius );
			TFLOAT fDistSq = TVector4::DistanceSq( *(TVector4*)( (TCHAR*)pObj + 0x14 ), vecCamPos );

			if ( fDistSq >= fRenderDist * fRenderDist ) { pObj = (TUINT*)*pObj; continue; }
			if ( !TRenderContext::CullSphereToFrustumSimple( sphere, pContext->GetWorldPlanes(), 6 ) ) { pObj = (TUINT*)*pObj; continue; }
			if ( iCount >= 240 ) break;

			if ( numFrames <= iFrame ) iFrame = numFrames - 1;

			RegrowthRenderEntry* pEntry = &s_aRenderEntries[ iCount++ ];
			TMatrix44 matLocal;
			( (void( __stdcall* )( TUINT, TMatrix44* ))0x005e2520 )( (TUINT)pObj, &matLocal );
			pEntry->oMatrix.Multiply( rMVP, matLocal );

			if ( animType == 1 )
			{
				TFLOAT fMax    = (TFLOAT)( numFrames - 1 );
				TFLOAT fFactor = ( fNorm < fMax ) ? ( fNorm / fMax ) : 1.0f;
				if ( iFrame == numFrames - 2 && fFactor >= 0.997f )
				{
					iFrame++;
					fFactor = ( fFactor - 0.997f ) * 333.4f;
				}
				TUINT8 uiPacked = *(TUINT8*)( (TCHAR*)pObj + 0x2A );
				TFLOAT fScale   = ( uiPacked * ( 1.0f / 255.0f ) * 0.4f + 0.8f ) * fFactor;
				pEntry->oMatrix.Scale( fScale, fScale, fScale );
			}

			TINT iBase              = ( (TINT)uiModel + iFrame ) * 2;
			TINT iSlot              = ( fDistSq < fLODDist * fLODDist ) ? iBase - 2 : iBase - 1;
			pEntry->pPrev           = (RegrowthRenderEntry*)pModelData[ iSlot ];
			pModelData[ iSlot ]     = (TINT)pEntry;

			GatherStaticLights( sphere, s_aEntryStaticLights[ pEntry - s_aRenderEntries ] );

			pObj = (TUINT*)*pObj;
		}
		while ( pObj != pSentinel );
	}
}

// $Barnyard: FUNCTION 005e35d0
void ARegrowthManager2::FillRenderList_Visible()
{
	TRenderContext* pContext  = TRenderInterface::GetSingleton()->GetCurrentContext();
	TFLOAT fRenderDist        = *(TFLOAT*)( (TCHAR*)this + 0x133D4 );
	TFLOAT fLODDist           = *(TFLOAT*)( (TCHAR*)this + 0x133D0 );
	TMatrix44& rMVP           = *(TMatrix44*)( (TCHAR*)this + 0x133E0 );
	TVector4 vecCamPos        = pContext->GetViewWorldMatrix().GetTranslation();
	TFLOAT fTotalTime         = *(TFLOAT*)( *(TUINT*)0x00783d3c + 0x20 );
	TUINT uiNumBones          = *(TUINT*)( *(TUINT*)0x007962e8 + 8 );
	TUINT uiFieldBase         = *(TUINT*)( (TCHAR*)this + 0x08 );
	TCHAR* pGrowthBase        = *(TCHAR**)( (TCHAR*)this + 0x0C );
	TINT* pModelData          = *(TINT**)( (TCHAR*)this + 0x10 );
	TFLOAT fWaterLevel        = *(TFLOAT*)0x0071f754;
	TINT   iWaterMode         = *(TINT*)0x00773a64;
	TINT   iCount             = 0;

	for ( TUINT i = 0; i < uiNumBones; i++ )
	{
		TCHAR*  pMap     = pGrowthBase + i * 0x28;
		TUINT8  uiModel  = *(TUINT8*)( pMap + 0x20 );
		if ( uiModel == 0xFF ) continue;

		TUINT8  animType   = *(TUINT8*)( pMap + 0x21 );
		TFLOAT  growthRate  = *(TFLOAT*)( pMap + 0x00 );
		TINT    numFrames   = *(TINT*)( pMap + 0x04 );
		TBOOL   bForceScale = ( *(TUINT8*)( pMap + 0x22 ) & ( 1u << 5 ) ) != 0;
		TUINT*  pSentinel   = (TUINT*)( (TCHAR*)this + 0x1C + i * 8 + 4 );
		TUINT*  pObj        = (TUINT*)*pSentinel;
		if ( pObj == pSentinel ) continue;

		TUINT uiRP1    = *(TUINT*)( uiFieldBase + uiModel * 8 );
		TUINT uiRP2    = *(TUINT*)uiRP1;
		TFLOAT fRadius = *(TFLOAT*)( uiRP2 + 0x10 ) * 1.2f;

		do
		{
			TFLOAT fGrowth = ( fTotalTime - *(TFLOAT*)( (TCHAR*)pObj + 0x24 ) ) * growthRate;
			TINT   iFrame  = (TINT)fGrowth;
			TFLOAT fNorm   = 0.0f;

			if ( animType == 1 )
			{
				if ( fGrowth <= 0.001f ) { pObj = (TUINT*)*pObj; continue; }
				if ( iFrame < 1 ) iFrame = 1;
				fNorm = ( fGrowth - 0.001f ) * 1.001f;
			}
			else
			{
				if ( iFrame <= 0 ) { pObj = (TUINT*)*pObj; continue; }
			}

			TFLOAT fPX = *(TFLOAT*)( (TCHAR*)pObj + 0x14 );
			TFLOAT fPY = *(TFLOAT*)( (TCHAR*)pObj + 0x18 );
			TFLOAT fPZ = *(TFLOAT*)( (TCHAR*)pObj + 0x1C );
			TSphere sphere( fPX, fPY, fPZ, fRadius );
			TFLOAT fDistSq = TVector4::DistanceSq( *(TVector4*)( (TCHAR*)pObj + 0x14 ), vecCamPos );

			if ( fDistSq >= fRenderDist * fRenderDist ) { pObj = (TUINT*)*pObj; continue; }

			// Water-level filter (DAT_0071f754 / DAT_00773a64)
			if ( !( fPY <= fWaterLevel || iWaterMode == 0 ) ) { pObj = (TUINT*)*pObj; continue; }
			if ( !( fPY > fWaterLevel || iWaterMode == 1 || iWaterMode == 2 ) ) { pObj = (TUINT*)*pObj; continue; }

			if ( !TRenderContext::CullSphereToFrustumSimple( sphere, pContext->GetWorldPlanes(), 6 ) ) { pObj = (TUINT*)*pObj; continue; }
			if ( iCount >= 240 ) break;

			if ( numFrames <= iFrame ) iFrame = numFrames - 1;

			RegrowthRenderEntry* pEntry = &s_aRenderEntries[ iCount++ ];
			TMatrix44 matLocal;
			( (void( __stdcall* )( TUINT, TMatrix44* ))0x005e2520 )( (TUINT)pObj, &matLocal );
			pEntry->oMatrix.Multiply( rMVP, matLocal );

			if ( animType == 1 )
			{
				TFLOAT fMax    = (TFLOAT)( numFrames - 1 );
				TFLOAT fFactor = ( fNorm < fMax ) ? ( fNorm / fMax ) : 1.0f;
				if ( iFrame == numFrames - 2 && fFactor >= 0.997f )
				{
					iFrame++;
					fFactor = ( fFactor - 0.997f ) * 333.4f;
				}
				TFLOAT fBaseScale = bForceScale ? 1.0f : *(TUINT8*)( (TCHAR*)pObj + 0x2A ) * ( 1.0f / 255.0f ) * 0.4f + 0.8f;
				TFLOAT fScale     = fBaseScale * fFactor;
				pEntry->oMatrix.Scale( fScale, fScale, fScale );
			}

			TINT iBase          = ( (TINT)uiModel + iFrame ) * 2;
			TINT iSlot          = ( fDistSq < fLODDist * fLODDist ) ? iBase - 2 : iBase - 1;
			pEntry->pPrev       = (RegrowthRenderEntry*)pModelData[ iSlot ];
			pModelData[ iSlot ] = (TINT)pEntry;

			GatherStaticLights( sphere, s_aEntryStaticLights[ pEntry - s_aRenderEntries ] );

			pObj = (TUINT*)*pObj;
		}
		while ( pObj != pSentinel );
	}
}

// $Barnyard: FUNCTION 005e3990
void ARegrowthManager2::Render()
{
	if ( ( *(TUINT*)( (TCHAR*)this + 0x13424 ) & 1U ) == 0 )
		return;

	TRenderInterface* pRI     = TRenderInterface::GetSingleton();
	TRenderContext*   pContext = pRI->GetCurrentContext();

	// Save context ModelViewMatrix into m_oModelViewMatrix
	TMatrix44& rSavedMVP = *(TMatrix44*)( (TCHAR*)this + 0x133E0 );
	rSavedMVP = pContext->GetModelViewMatrix();

	pContext->ClearLightIDs();
	ClearStaticLights( pContext );

	// Clear render list heads
	TINT  iNumModels = *(TINT*)( (TCHAR*)this + 0x04 );
	TINT* pModelData = *(TINT**)( (TCHAR*)this + 0x10 );
	for ( TINT i = 0; i < iNumModels * 2; i++ )
		pModelData[ i ] = 0;

	// Point render buffer at our static storage
	*(void**)( (TCHAR*)this + 0x133DC ) = s_aRenderEntries;

	// Perspective far-clip cap: clamp render distance for glow-light shadow passes.
	// Orthographic CSM passes are not affected.
	const TBOOL  bIsPerspective    = pContext->GetCameraMode() == TRenderContext::CameraMode_Perspective;
	const TFLOAT flFarClipLimit    = bIsPerspective ? pContext->GetProjectionParams().m_fFarClip : FLT_MAX;
	TFLOAT& flRenderDist           = *(TFLOAT*)( (TCHAR*)this + 0x133D4 );
	const TFLOAT flSavedRenderDist = flRenderDist;
	flRenderDist                   = TMath::Min( flRenderDist, flFarClipLimit );

	if ( *(TINT*)( (TCHAR*)this + 0x13424 ) < 0 )
		FillRenderList_Visible();
	else
		FillRenderList();

	flRenderDist = flSavedRenderDist;

	const TFLOAT flOldShadeCoeff = pContext->GetShadeCoeff();
	TTODO( "Use AInstanceLightingHelper::ms_pSingleton->m_flShadingCoeff as shade coeff" );
	pContext->SetShadeCoeff( 0.0f );

	const TUINT uiOldClipFlags = pContext->GetClipFlags();
	TFLOAT fBlendDist          = *(TFLOAT*)( (TCHAR*)this + 0x133D8 );
	TFLOAT fBlendFactor        = *(TFLOAT*)( (TCHAR*)this + 0x13420 );
	TUINT  uiFieldBase         = *(TUINT*)( (TCHAR*)this + 0x08 );

	for ( TINT i = 0; i < iNumModels * 2; i++ )
	{
		if ( pModelData[ i ] == 0 ) continue;

		// field2_0x8[(i>>1)] = { radiusDataPtr*, TSceneObject* }; TSceneObject at +4
		TSceneObject*   pSceneObj  = *(TSceneObject**)( uiFieldBase + 4 + ( i >> 1 ) * 8 );
		TModelInstance* pModelInst = pSceneObj->GetInstance();
		TModel*         pModel     = pModelInst->GetModel();
		TINT iLOD                  = TMath::Min( i & 1, pModel->GetNumLODs() - 1 );
		TModelLOD& rLOD            = pModel->GetLOD( iLOD );

		pContext->SetSkeletonInstance( pModelInst->GetSkeletonInstance() );

		for ( RegrowthRenderEntry* pEntry = (RegrowthRenderEntry*)pModelData[ i ];
		      pEntry != TNULL;
		      pEntry = pEntry->pPrev )
		{
			TFLOAT fViewZ = pEntry->oMatrix.m_f43;
			if ( fViewZ >= fBlendDist )
			{
				TFLOAT fBlend = 1.0f - ( fViewZ - fBlendDist ) * fBlendFactor;
				if ( fBlend <= 0.0f ) continue;
				pContext->SetAlphaBlend( fBlend );
			}
			else
			{
				pContext->SetAlphaBlend( 1.0f );
			}

			ClearStaticLights( pContext );
			AddStaticLights( pContext, s_aEntryStaticLights[ pEntry - s_aRenderEntries ] );

			pContext->SetModelViewMatrix( pEntry->oMatrix );
			for ( TINT k = 0; k < rLOD.iNumMeshes; k++ )
				rLOD.ppMeshes[ k ]->Render();
		}
	}

	pContext->SetClipFlags( uiOldClipFlags );
	pContext->SetShadeCoeff( flOldShadeCoeff );
	pContext->SetAlphaBlend( 1.0f );
	pContext->SetModelViewMatrix( rSavedMVP );
	ClearStaticLights( pContext );
}
