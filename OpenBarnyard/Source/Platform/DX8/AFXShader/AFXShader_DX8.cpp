#include "pch.h"
#include "AFXShader_DX8.h"
#include "AFXMaterial_DX8.h"
#include "AFXMesh_DX8.h"

#include <Platform/DX8/TRenderContext_DX8.h>
#include <Platform/DX8/TRenderInterface_DX8.h>
#include <Platform/DX8/TRenderAdapter_DX8.h>
#include <Platform/DX8/TVertexBlockResource_DX8.h>
#include <Platform/DX8/TVertexPoolResource_DX8.h>
#include <Platform/DX8/TIndexBlockResource_DX8.h>
#include <Platform/DX8/TIndexPoolResource_DX8.h>

#include <intrin.h>
#include <emmintrin.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $Barnyard: FUNCTION 005f9a90
// $Barnyard: FUNCTION 005fcbe0
// $Barnyard: FUNCTION 005fcc40
TDEFINE_CLASS( AFXShaderHAL );

// DAT_007789f0
static D3DMATERIAL8 s_oLowEndMaterial = ASkinLowEnd::DEFAULT_MATERIAL;

// DAT_00778a38
static D3DLIGHT8 s_oLowEndLight = ASkinLowEnd::DEFAULT_LIGHT;

// FLOAT_0079ade0, never written (the w of the normals)
static TFLOAT s_fNormalW = 0.0f;

// $Barnyard: FUNCTION 005fc540
AFXShaderHAL::AFXShaderHAL()
    : m_uiVertexShader( 0 )
    , m_iAlphaRef( 128 )
    , m_bHighEndSkinning( TTRUE )
    , m_bLightScattering( TTRUE )
    , m_bIsAlphaBlendMaterial( TFALSE )
    , m_fAMDPatch1( 0.0f )
    , m_fAMDPatch2( 1.0f )
    , m_bCPUSupportsSSE( TFALSE )
    , m_bCPUSupportsSSE2( TFALSE )
    , m_bPointLights( TFALSE )
    , m_iNumLights( 0 )
    , m_fUnknown( 0.0f )
{
	// NOTE: retail leaves it uninitialised and always tests it with == 1; a garbage non-zero value
	// would pick D3DCULL_CCW and cull the ASysShader quads drawn after it
	m_bRenderEnvMap = TFALSE;
}

// $Barnyard: FUNCTION 005fc6f0
// $Barnyard: FUNCTION 005fcc80
AFXShaderHAL::~AFXShaderHAL()
{
}

// $Barnyard: FUNCTION 005fbc90
void AFXShaderHAL::OnDestroy()
{
	m_FreeLights.Clear();
	m_UsedLights.Clear();

	// NOTE: Allocated with TMalloc in Create
	delete[] m_pLights;

	TShader::OnDestroy();
}

// $Barnyard: FUNCTION 005fb520
void AFXShaderHAL::Flush()
{
	if ( !IsValidated() ) return;

	if ( !IsHighEndSkinning() )
	{
		SetupLowEndMode();
		return;
	}

	auto pD3DDevice = TRenderD3DInterface::Interface()->GetDirect3DDevice();

	Validate();
	pD3DDevice->SetVertexShader( m_uiVertexShader );
	pD3DDevice->SetRenderState( D3DRS_ALPHATESTENABLE, 1 );
	pD3DDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, 1 );
	pD3DDevice->SetRenderState( D3DRS_ALPHAFUNC, 5 );

	DWORD bWasSpecularEnabled;
	pD3DDevice->GetRenderState( D3DRS_SPECULARENABLE, &bWasSpecularEnabled );
	pD3DDevice->SetRenderState( D3DRS_SPECULARENABLE, 0 );

	pD3DDevice->SetTextureStageState( 0, D3DTSS_COLOROP, 4 );
	pD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, 2 );
	pD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, 0 );
	pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP, 4 );
	pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, 2 );
	pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, 0 );
	pD3DDevice->SetTextureStageState( 1, D3DTSS_COLOROP, 1 );
	pD3DDevice->SetTextureStageState( 1, D3DTSS_ALPHAOP, 1 );

	if ( m_bRenderEnvMap == TTRUE )
		pD3DDevice->SetRenderState( D3DRS_CULLMODE, 3 );
	else
		pD3DDevice->SetRenderState( D3DRS_CULLMODE, 2 );

	m_oOrderTable.Render();

	pD3DDevice->SetRenderState( D3DRS_SPECULARENABLE, bWasSpecularEnabled );
	pD3DDevice->SetRenderState( D3DRS_FOGENABLE, 0 );
	TRenderD3DInterface::Interface()->GetDirect3DDevice()->SetPixelShader( 0 );
}

// DOES_NOTHING
void AFXShaderHAL::StartFlush()
{
}

// DOES_NOTHING
void AFXShaderHAL::EndFlush()
{
}

// $Barnyard: FUNCTION 005fbc10
TBOOL AFXShaderHAL::Create()
{
	m_oOrderTable.Create( this, -370 );

	// NOTE: Retail allocates the lights with TMalloc( MAX_LIGHTS * sizeof( Light ) ) and constructs the nodes in place
	m_pLights = new Light[ MAX_LIGHTS ];

	for ( TINT i = 0; i < MAX_LIGHTS; i++ )
	{
		m_FreeLights.PushFront( &m_pLights[ i ] );
		m_pLights[ i ].iIndex = i;
	}

	return TShader::Create();
}

// $Barnyard: FUNCTION 005fbcc0
TBOOL AFXShaderHAL::Validate()
{
	if ( IsValidated() )
		return TTRUE;

	m_uiVertexShader = ASkinLowEnd::STATIC_FVF;
	EnableHighEndSkinning( TTRUE );
	ApplyGPUSpecificPatches();
	CheckCPUSupportsSSE();
	CheckCPUSupportsSSE2();

	return TShader::Validate();
}

// $Barnyard: FUNCTION 005fbf70
void AFXShaderHAL::Invalidate()
{
	if ( !IsValidated() ) return;

	if ( !IsHighEndSkinning() )
		m_SomeList.DeleteAll();

	TShader::Invalidate();
}

// NOTE: Folded with ASysShaderHAL::TryInvalidate (005f3690)
TBOOL AFXShaderHAL::TryInvalidate()
{
	Invalidate();
	return TTRUE;
}

// NOTE: Folded with ASysShaderHAL::TryValidate (005f36a0)
TBOOL AFXShaderHAL::TryValidate()
{
	Validate();
	return TTRUE;
}

// $Barnyard: FUNCTION 005fc770
void AFXShaderHAL::Render( Toshi::TRenderPacket* a_pRenderPacket )
{
	if ( !IsHighEndSkinning() )
	{
		RenderLowEnd( a_pRenderPacket );
		return;
	}

	if ( !a_pRenderPacket ) return;

	auto pMesh = TSTATICCAST( AFXMeshHAL, a_pRenderPacket->GetMesh() );
	if ( !pMesh || pMesh->GetNumSubMeshes() == 0 ) return;

	auto pRenderInterface = TRenderD3DInterface::Interface();
	auto pD3DDevice       = pRenderInterface->GetDirect3DDevice();
	auto pCurrentContext  = TRenderContextD3D::Upcast( pRenderInterface->GetCurrentContext() );
	auto pBones           = a_pRenderPacket->GetSkeletonInstance()->GetBones();

	Validate();
	pD3DDevice->SetVertexShader( m_uiVertexShader );

	TMatrix44 oModelViewProjection;
	D3DXMatrixMultiply( oModelViewProjection, a_pRenderPacket->GetModelViewMatrix(), pCurrentContext->GetProjectionMatrix() );
	D3DXMatrixTranspose( oModelViewProjection, oModelViewProjection );

	// DAT_00778aa0
	static TVector4 s_vUnknown = TVector4( 256.0f, 256.0f, 256.0f, 256.0f );
	pD3DDevice->SetVertexShaderConstant( 0, oModelViewProjection, 4 );
	pD3DDevice->SetVertexShaderConstant( 4, &s_vUnknown, 1 );

	TFLOAT fLightDirX = -a_pRenderPacket->GetLightDirection().x;
	TFLOAT fLightDirY = -a_pRenderPacket->GetLightDirection().y;
	TFLOAT fLightDirZ = -a_pRenderPacket->GetLightDirection().z;

	TVector4 vAmbientColour = TVector4( a_pRenderPacket->GetAmbientColour().x, a_pRenderPacket->GetAmbientColour().y, a_pRenderPacket->GetAmbientColour().z, 1.0f );
	TVector4 vLightColour   = TVector4( a_pRenderPacket->GetLightColour().x, a_pRenderPacket->GetLightColour().y, a_pRenderPacket->GetLightColour().z, 1.0f );
	TVector4 vUnkVector     = TVector4( 0.6f, 0.4f, 0.0f, 1.0f );

	TMatrix44 oModelView = a_pRenderPacket->GetModelViewMatrix();
	TMatrix44 oWorldModelView;

	for ( TINT i = 0; i < 4; i++ )
	{
		oWorldModelView.AsBasisVector4( i ).x = oModelView.m_f11 * sm_oWorldViewMatrix.AsBasisVector3( i ).x + oModelView.m_f12 * sm_oWorldViewMatrix.AsBasisVector3( i ).y + oModelView.m_f13 * sm_oWorldViewMatrix.AsBasisVector3( i ).z;
		oWorldModelView.AsBasisVector4( i ).y = oModelView.m_f21 * sm_oWorldViewMatrix.AsBasisVector3( i ).x + oModelView.m_f22 * sm_oWorldViewMatrix.AsBasisVector3( i ).y + oModelView.m_f23 * sm_oWorldViewMatrix.AsBasisVector3( i ).z;
		oWorldModelView.AsBasisVector4( i ).z = oModelView.m_f31 * sm_oWorldViewMatrix.AsBasisVector3( i ).x + oModelView.m_f32 * sm_oWorldViewMatrix.AsBasisVector3( i ).y + oModelView.m_f33 * sm_oWorldViewMatrix.AsBasisVector3( i ).z;
		oWorldModelView.AsBasisVector4( i ).w = oModelView.m_f14 * sm_oWorldViewMatrix.AsBasisVector3( i ).x + oModelView.m_f24 * sm_oWorldViewMatrix.AsBasisVector3( i ).y + oModelView.m_f34 * sm_oWorldViewMatrix.AsBasisVector3( i ).z;
	}

	// NOTE: Retail leaves w of the light direction uninitialised
	TVector4 vLightDirWorld;
	vLightDirWorld.x = oWorldModelView.m_f11 * fLightDirX + oWorldModelView.m_f21 * fLightDirY + oWorldModelView.m_f31 * fLightDirZ;
	vLightDirWorld.y = oWorldModelView.m_f12 * fLightDirX + oWorldModelView.m_f22 * fLightDirY + oWorldModelView.m_f32 * fLightDirZ;
	vLightDirWorld.z = oWorldModelView.m_f13 * fLightDirX + oWorldModelView.m_f23 * fLightDirY + oWorldModelView.m_f33 * fLightDirZ;
	vLightDirWorld.w = 0.0f;
	vLightDirWorld.Normalise();

	pD3DDevice->SetVertexShaderConstant( 89, &vLightDirWorld, 1 );

	vAmbientColour.w = a_pRenderPacket->GetAlpha();
	pD3DDevice->SetVertexShaderConstant( 90, &vAmbientColour, 1 );
	pD3DDevice->SetVertexShaderConstant( 91, &vLightColour, 1 );
	pD3DDevice->SetVertexShaderConstant( 93, &vUnkVector, 1 );

	TUINT uiAlpha = TUINT( a_pRenderPacket->GetAlpha() * 128.0f );
	if ( uiAlpha > m_iAlphaRef ) uiAlpha = m_iAlphaRef;
	pD3DDevice->SetRenderState( D3DRS_ALPHAREF, uiAlpha );

	auto pVertexPool = TSTATICCAST( TVertexPoolResource, pMesh->GetVertexPool() );

	TVertexBlockResource::HALBuffer vertexHALBuffer;
	pVertexPool->GetHALBuffer( &vertexHALBuffer );
	pD3DDevice->SetStreamSource( 0, vertexHALBuffer.apVertexBuffers[ 0 ], sizeof( ASkinLowEnd::SkinVertex ) );

	TIndexBlockResource::HALBuffer indexHALBuffer;

	for ( TUINT i = 0; i < pMesh->GetNumSubMeshes(); i++ )
	{
		auto pSubMesh = pMesh->GetSubMesh( i );

		TINT iCurrentRegister = 5;

		for ( TUINT k = 0; k < pSubMesh->uiNumBones; k++ )
		{
			TMatrix44 oBoneTransform;
			D3DXMatrixTranspose( oBoneTransform, pBones[ pSubMesh->aBones[ k ] ].m_Transform );
			pD3DDevice->SetVertexShaderConstant( iCurrentRegister, oBoneTransform, 3 );

			iCurrentRegister += 3;
		}

		auto pIndexPool = TSTATICCAST( TIndexPoolResource, pSubMesh->pIndexPool );
		pIndexPool->GetHALBuffer( &indexHALBuffer );

		pD3DDevice->SetIndices( indexHALBuffer.pIndexBuffer, vertexHALBuffer.uiVertexOffset );
		pD3DDevice->DrawIndexedPrimitive(
		    D3DPT_TRIANGLESTRIP,
		    0,
		    pVertexPool->GetNumVertices(),
		    indexHALBuffer.uiIndexOffset,
		    pIndexPool->GetNumIndices() - 2
		);
	}
}

// $Barnyard: FUNCTION 005fb510
void AFXShaderHAL::EnableRenderEnvMap( TBOOL a_bEnable )
{
	m_bRenderEnvMap = a_bEnable;
}

// $Barnyard: FUNCTION 005fc660
TBOOL AFXShaderHAL::IsHighEndSkinning()
{
	return m_bHighEndSkinning;
}

// $Barnyard: FUNCTION 005f9ac0
void AFXShaderHAL::EnableHighEndSkinning( TBOOL a_bEnable )
{
	m_bHighEndSkinning = IsCapableHighEndSkinning() ? a_bEnable : TFALSE;
	m_bLightScattering = a_bEnable && IsCapableLightScattering();
}

// NOTE: Folded with ASkinShaderHAL::IsCapableHighEndSkinning (005f1810)
TBOOL AFXShaderHAL::IsCapableHighEndSkinning()
{
	D3DCAPS8 caps;

	auto pRenderInterface = TRenderD3DInterface::Interface();
	auto pDevice          = pRenderInterface->GetDirect3DDevice();

	if ( !pDevice )
	{
		auto pDirect3D = pRenderInterface->GetDirect3D();
		if ( !pDirect3D ) return TFALSE;

		pDirect3D->GetDeviceCaps( 0, D3DDEVTYPE_HAL, &caps );
	}
	else
	{
		pDevice->GetDeviceCaps( &caps );
	}

	return ( ( caps.VertexShaderVersion & 0xFFFF ) > 256 ) && ( caps.DevCaps & 0x10000 );
}

// $Barnyard: FUNCTION 005fc5f0
TBOOL AFXShaderHAL::IsLightScattering()
{
	return m_bLightScattering;
}

// $Barnyard: FUNCTION 005f9dd0
void AFXShaderHAL::SetLightScattering( TBOOL a_bEnable )
{
	m_bLightScattering = a_bEnable && IsCapableLightScattering();
}

// $Barnyard: FUNCTION 005fc600
TBOOL AFXShaderHAL::IsAlphaBlendMaterial()
{
	return m_bIsAlphaBlendMaterial;
}

// $Barnyard: FUNCTION 005fc650
void AFXShaderHAL::SetAlphaBlendMaterial( TBOOL a_bIsAlphaBlendMaterial )
{
	m_bIsAlphaBlendMaterial = a_bIsAlphaBlendMaterial;
}

// $Barnyard: FUNCTION 005fbd80
AFXMaterial* AFXShaderHAL::CreateMaterial( const TCHAR* a_szName )
{
	// NOTE: The name is ignored and every material gets an alpha blend copy
	Validate();

	auto pMaterial       = new AFXMaterialHAL();
	pMaterial->m_pShader = this;
	m_oOrderTable.RegisterMaterial( pMaterial );

	auto pAlphaBlendMaterial         = new AFXMaterialHAL();
	pMaterial->m_pAlphaBlendMaterial = pAlphaBlendMaterial;
	pAlphaBlendMaterial->m_pShader   = this;
	m_oOrderTable.RegisterMaterial( pMaterial->m_pAlphaBlendMaterial );
	pMaterial->m_pAlphaBlendMaterial->Create( 1 );

	return pMaterial;
}

// $Barnyard: FUNCTION 005fbd00
AFXMesh* AFXShaderHAL::CreateMesh( const TCHAR* a_szName )
{
	// NOTE: The name is ignored
	Validate();

	auto pMesh = new AFXMeshHAL();
	pMesh->SetOwnerShader( this );

	return pMesh;
}

// $Barnyard: FUNCTION 005fbea0
TINT AFXShaderHAL::AddLight( const Toshi::TVector3& a_rPosition, TFLOAT a_fIntensity )
{
	if ( m_FreeLights.IsEmpty() )
		return -1;

	Light* pLight      = m_FreeLights.PopFront();
	pLight->vPosition  = a_rPosition;
	pLight->fIntensity = a_fIntensity;
	m_UsedLights.PushFront( pLight );
	m_iNumLights++;

	return pLight->iIndex;
}

// $Barnyard: FUNCTION 005fc6a0
void AFXShaderHAL::SetLight( TINT a_iIndex, const Toshi::TVector3& a_rPosition, TFLOAT a_fIntensity )
{
	if ( a_iIndex != -1 )
	{
		m_pLights[ a_iIndex ].vPosition  = a_rPosition;
		m_pLights[ a_iIndex ].fIntensity = a_fIntensity;
	}
}

// $Barnyard: FUNCTION 005fbf20
void AFXShaderHAL::RemoveLight( TINT a_iIndex )
{
	if ( a_iIndex != -1 )
	{
		m_UsedLights.Erase( &m_pLights[ a_iIndex ] );
		m_FreeLights.PushFront( &m_pLights[ a_iIndex ] );
		m_iNumLights--;
	}
}

// $Barnyard: FUNCTION 005f9b10
TBOOL AFXShaderHAL::IsCapablePointLights()
{
	auto pDevice = TSTATICCAST( TD3DAdapter::Mode::Device, TRenderD3DInterface::Interface()->GetCurrentDevice() );
	return pDevice->GetD3DCaps().MaxVertexShaderConst > 101;
}

// $Barnyard: FUNCTION 005f9af0
void AFXShaderHAL::SetPointLights( TBOOL a_bEnabled )
{
	m_bPointLights = a_bEnabled && IsCapablePointLights();
}

// $Barnyard: FUNCTION 005fc610
TBOOL AFXShaderHAL::IsPointLightsEnabled()
{
	return m_bPointLights;
}

// $Barnyard: FUNCTION 005f9b40
TBOOL AFXShaderHAL::IsCapableLightScattering()
{
	D3DCAPS8 caps;

	auto pRenderInterface = TRenderD3DInterface::Interface();

	if ( auto pDevice = pRenderInterface->GetDirect3DDevice() )
	{
		pDevice->GetDeviceCaps( &caps );
	}
	else
	{
		auto pDirect3D = pRenderInterface->GetDirect3D();
		if ( !pDirect3D ) return TFALSE;

		pDirect3D->GetDeviceCaps( 0, D3DDEVTYPE_HAL, &caps );
	}

	if ( ( caps.PixelShaderVersion & 0xFFFF ) >= 0x101 )
		return IsCapableHighEndSkinning();

	return TFALSE;
}

// $Barnyard: FUNCTION 005fc620
void AFXShaderHAL::SetUnknownFloat( TFLOAT a_fValue )
{
	m_fUnknown = a_fValue;
}

// $Barnyard: FUNCTION 005fc630
TFLOAT AFXShaderHAL::GetUnknownFloat()
{
	return m_fUnknown;
}

// $Barnyard: FUNCTION 005fc640
TBOOL AFXShaderHAL::IsEnableRenderEnvMap()
{
	return m_bRenderEnvMap;
}

// NOTE: Folded with ASkinShaderHAL::SetSomeColour (005fc670)
void AFXShaderHAL::SetSomeColour( TUINT a_uiR, TUINT a_uiG, TUINT a_uiB, TUINT a_uiA )
{
	m_SomeColour = ( ( a_uiA << 8 | a_uiB ) << 8 | a_uiG ) << 8 | a_uiR;
}

// NOTE: Folded with ASkinShaderHAL::SetUnknown1 (005f3640)
TINT AFXShaderHAL::SetUnknown1( TINT a_Unknown, TUINT8 a_fAlpha )
{
	if ( m_SomeColourComponents[ 1 ] < 2 )
		m_Unk3 = a_fAlpha * ( 1.0f / 128.0f );
	else if ( m_SomeColourComponents[ 0 ] == 0 )
		m_Unk3 = 1.0f;
	else
		m_Unk3 = 0.0f;

	return m_Unk2;
}

// NOTE: folded with ASkinShaderHAL::SetUnknown2 (006639a0)
void AFXShaderHAL::SetUnknown2( TINT a_Unknown )
{
	m_Unk2 = a_Unknown;
}

// $Barnyard: FUNCTION 005fbfa0
void AFXShaderHAL::RenderLowEnd( Toshi::TRenderPacket* a_pRenderPacket )
{
	ASkinLowEnd::Render<AFXShaderHAL, AFXMeshHAL>( this, a_pRenderPacket, m_SomeList, s_oLowEndMaterial, s_oLowEndLight, sm_oWorldViewMatrix, m_Unk4 != 0 );
}

// Skins every vertex of the mesh with up to 4 bones
// $Barnyard: FUNCTION 005f9fc0
void AFXShaderHAL::SkinVerticesCPU( Toshi::TRenderPacket* a_pRenderPacket, const TBYTE* a_pSrcVertices, TBYTE* a_pDstVertices )
{
	ASkinLowEnd::SkinVertices<AFXMeshHAL>( a_pRenderPacket, a_pSrcVertices, a_pDstVertices, m_bCPUSupportsSSE, s_fNormalW );
}

// Transforms every vertex by the first bone it's attached to
// $Barnyard: FUNCTION 005fb340
void AFXShaderHAL::TransformVerticesRigid( Toshi::TRenderPacket* a_pRenderPacket, const TBYTE* a_pSrcVertices, TBYTE* a_pDstVertices )
{
	ASkinLowEnd::TransformVerticesRigid<AFXMeshHAL>( a_pRenderPacket, a_pSrcVertices, a_pDstVertices, s_fNormalW );
}

// $Barnyard: FUNCTION 005f9df0
void AFXShaderHAL::SetupLowEndMode()
{
	auto pRenderInterface = TRenderD3DInterface::Interface();
	auto pCurrentContext  = TRenderContextD3D::Upcast( pRenderInterface->GetCurrentContext() );
	auto pDevice          = pRenderInterface->GetDirect3DDevice();

	pDevice->SetVertexShader( ASkinLowEnd::STATIC_FVF );
	pDevice->SetPixelShader( 0 );
	pDevice->SetRenderState( D3DRS_LIGHTING, 1 );
	pDevice->SetRenderState( D3DRS_FOGENABLE, 0 );
	pDevice->SetRenderState( D3DRS_ALPHATESTENABLE, 1 );
	pDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, 1 );
	pDevice->SetRenderState( D3DRS_SRCBLEND, 5 );
	pDevice->SetRenderState( D3DRS_DESTBLEND, 6 );
	pDevice->SetRenderState( D3DRS_ALPHAFUNC, 5 );

	DWORD bWasSpecularEnabled;
	pDevice->GetRenderState( D3DRS_SPECULARENABLE, &bWasSpecularEnabled );
	pDevice->SetRenderState( D3DRS_SPECULARENABLE, 0 );

	pDevice->SetTextureStageState( 0, D3DTSS_COLOROP, 4 );
	pDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, 2 );
	pDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, 0 );
	pDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP, 4 );
	pDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, 2 );
	pDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, 0 );
	pDevice->SetTextureStageState( 1, D3DTSS_COLOROP, 1 );
	pDevice->SetTextureStageState( 1, D3DTSS_ALPHAOP, 1 );

	pDevice->SetRenderState( D3DRS_CULLMODE, m_bRenderEnvMap == TTRUE ? 3 : 2 );

	// DAT_0079aeb8
	static TMatrix44 s_IdentityMatrix = TMatrix44::IDENTITY;

	pDevice->SetTransform( D3DTS_VIEW, s_IdentityMatrix );
	pDevice->SetTransform( D3DTS_PROJECTION, pCurrentContext->GetProjectionMatrix() );
	m_oOrderTable.Render();

	pDevice->SetRenderState( D3DRS_SPECULARENABLE, bWasSpecularEnabled );
	pDevice->SetRenderState( D3DRS_FOGENABLE, 0 );
}

// $Barnyard: FUNCTION 005f9d80
void AFXShaderHAL::ApplyGPUSpecificPatches()
{
	auto pRenderInterface = TRenderD3DInterface::Interface();
	auto pDevice          = pRenderInterface->GetCurrentDevice();
	auto pMode            = pDevice->GetMode();
	auto pAdapter         = pMode->GetAdapter();

	if ( T2String8::CompareNoCase( pAdapter->GetDriverDescription(), "RADEON 8500 / RADEON 8500LE" ) == 0 )
	{
		m_fAMDPatch1 = 0.005f;
		m_fAMDPatch2 = 0.99995f;
	}
}

// $Barnyard: FUNCTION 005f9bb0
void AFXShaderHAL::CheckCPUSupportsSSE()
{
	DWORD uiExceptionCode;

	__try
	{
		TINT aCPUInfo[ 4 ];
		__cpuid( aCPUInfo, 1 );

		// EDX bit 25
		if ( !( aCPUInfo[ 3 ] & BITFLAG( 25 ) ) )
		{
			m_bCPUSupportsSSE = TFALSE;
			return;
		}
	}
	__except ( uiExceptionCode = GetExceptionCode(), EXCEPTION_EXECUTE_HANDLER )
	{
		m_bCPUSupportsSSE = TFALSE;
		return;
	}

	__try
	{
		// Executes an SSE instruction (xorps)
		volatile __m128 vZero = _mm_setzero_ps();
		(void)vZero;
	}
	__except ( uiExceptionCode = GetExceptionCode(), EXCEPTION_EXECUTE_HANDLER )
	{
		if ( uiExceptionCode == STATUS_ILLEGAL_INSTRUCTION )
		{
			m_bCPUSupportsSSE = TFALSE;
			return;
		}
	}

	m_bCPUSupportsSSE = TTRUE;
}

// $Barnyard: FUNCTION 005f9c90
void AFXShaderHAL::CheckCPUSupportsSSE2()
{
	DWORD uiExceptionCode;

	__try
	{
		TINT aCPUInfo[ 4 ];
		__cpuid( aCPUInfo, 1 );

		// EDX bit 26
		if ( !( aCPUInfo[ 3 ] & BITFLAG( 26 ) ) )
		{
			m_bCPUSupportsSSE2 = TFALSE;
			return;
		}
	}
	__except ( uiExceptionCode = GetExceptionCode(), EXCEPTION_EXECUTE_HANDLER )
	{
		m_bCPUSupportsSSE2 = TFALSE;
		return;
	}

	__try
	{
		// Executes an SSE2 instruction (xorpd)
		volatile __m128d vZero = _mm_setzero_pd();
		(void)vZero;
	}
	__except ( uiExceptionCode = GetExceptionCode(), EXCEPTION_EXECUTE_HANDLER )
	{
		if ( uiExceptionCode == STATUS_ILLEGAL_INSTRUCTION )
		{
			m_bCPUSupportsSSE2 = TFALSE;
			return;
		}
	}

	m_bCPUSupportsSSE2 = TTRUE;
}
