#include "pch.h"
#include "AVehicleSkidManager.h"
#include "Assets/AMaterialLibraryManager.h"
#include "FX/AParticleSystem.h"
#include "Render/ASysShader/ASysShader.h"
#include "Render/ASysShader/ASysMaterial.h"
#include "Render/ASysShader/ASysMesh.h"

#include <Math/TPlane.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 006623a0, CreateObject 00662a10, CreateObjectInPlace 00662a90
TDEFINE_CLASS( AVehicleSkidManager );

static constexpr const TCHAR* TYRETRACK_TEXTURE = "Particles\\tyretrack.tga";

// Builds the rotation of a track: up = normal, forward = direction, right = up x forward
// $Barnyard: FUNCTION 00662520
static void BuildTrackMatrix( TMatrix44& a_rOutMatrix, const TVector4& a_rcNormal, const TVector4& a_rcDirection )
{
	a_rOutMatrix = TMatrix44::IDENTITY;

	TVector4& rRight   = a_rOutMatrix.AsBasisVector4( BASISVECTOR_RIGHT );
	TVector4& rUp      = a_rOutMatrix.AsBasisVector4( BASISVECTOR_UP );
	TVector4& rForward = a_rOutMatrix.AsBasisVector4( BASISVECTOR_FORWARD );

	rUp.Set( a_rcNormal.x, a_rcNormal.y, a_rcNormal.z, 0.0f );
	rForward.Set( a_rcDirection.x, a_rcDirection.y, a_rcDirection.z, 0.0f );

	rRight.CrossProduct( rUp, rForward );
	rRight.Normalise();
	rUp.CrossProduct( rForward, rRight );
	rUp.Normalise();
}

// $Barnyard: FUNCTION 00662460
AVehicleSkidManager::TyreTrack::TyreTrack()
    : pMesh( TNULL )
    , iUnk4( 0 )
    , vPosition( TVector4::VEC_ZERO )
    , vNormal( TVector4::VEC_NEGY )
    , vDirection( TVector4::VEC_POSX )
    , fWidth( 0.1f )
    , fLength( 0.1f )
    , uiColour( 0 )
    , fUVOffset( 0.0f )
    , fAge( TMath::MAXFLOAT )
    , fLifetime( 60.0f )
{
}

// $Barnyard: FUNCTION 00662500
AVehicleSkidManager::TyreTrack::~TyreTrack()
{
	if ( pMesh )
	{
		pMesh->DestroyResource();
		pMesh = TNULL;
	}
}

// $Barnyard: FUNCTION 006625f0
TBOOL AVehicleSkidManager::TyreTrack::IsVisible() const
{
	AParticleSystem* pParticleSystem = AParticleSystem::GetSingleton();

	if ( !pParticleSystem )
		return TFALSE;

	const TPlane* pPlanes    = pParticleSystem->GetFrustumPlanes();
	TINT          iNumPlanes = pParticleSystem->GetNumFrustumPlanes();

	for ( TINT i = 0; i < iNumPlanes; i++ )
	{
		if ( -fLength > TVector4::DotProduct3( pPlanes[ i ].AsVector4(), vPosition ) - pPlanes[ i ].GetD() )
			return TFALSE;
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 00662780
void AVehicleSkidManager::TyreTrack::Render()
{
	if ( !pMesh )
		return;

	ASysMesh::LockBuffer oLockBuffer;
	pMesh->Lock( oLockBuffer );

	TIndexType*       pIndices  = oLockBuffer.IndexLock.pBuffer;
	ASysMesh::Vertex* pVertices = TREINTERPRETCAST( ASysMesh::Vertex*, oLockBuffer.VertexLock.apStreams[ 0 ] );

	TMatrix44 oMatrix;
	BuildTrackMatrix( oMatrix, vNormal, vDirection );

	// Lift the track a bit above the ground
	TVector4 vLift;
	vLift.Multiply( TVector4::VEC_NEGY, 0.05f );

	TVector4& rTranslation = oMatrix.AsBasisVector4( BASISVECTOR_TRANSLATION );
	rTranslation.x         = vLift.x + vPosition.x;
	rTranslation.y         = vLift.y + vPosition.y;
	rTranslation.z         = vLift.z + vPosition.z;
	rTranslation.w         = vPosition.w;

	TVector4 vCorner;
	TVector4 vTransformed;

	vCorner.Set( fWidth * -0.5f, 0.0f, 0.0f, 1.0f );
	TMatrix44::TransformVector( vTransformed, oMatrix, vCorner );
	pVertices[ 0 ].vPosition.Set( vTransformed.x, vTransformed.y, vTransformed.z );

	vCorner.Set( fWidth * -0.5f, 0.0f, fLength, 1.0f );
	TMatrix44::TransformVector( vTransformed, oMatrix, vCorner );
	pVertices[ 1 ].vPosition.Set( vTransformed.x, vTransformed.y, vTransformed.z );

	vCorner.Set( fWidth * 0.5f, 0.0f, 0.0f, 1.0f );
	TMatrix44::TransformVector( vTransformed, oMatrix, vCorner );
	pVertices[ 2 ].vPosition.Set( vTransformed.x, vTransformed.y, vTransformed.z );

	vCorner.Set( fWidth * 0.5f, 0.0f, fLength, 1.0f );
	TMatrix44::TransformVector( vTransformed, oMatrix, vCorner );
	pVertices[ 3 ].vPosition.Set( vTransformed.x, vTransformed.y, vTransformed.z );

	TFLOAT fStartV = fLength / fWidth + fUVOffset;
	pVertices[ 0 ].vUV.Set( 1.0f, fStartV );
	pVertices[ 1 ].vUV.Set( 1.0f, fUVOffset );
	pVertices[ 2 ].vUV.Set( 0.0f, fStartV );
	pVertices[ 3 ].vUV.Set( 0.0f, fUVOffset );

	pVertices[ 0 ].uiColor = uiColour;
	pVertices[ 1 ].uiColor = uiColour;
	pVertices[ 2 ].uiColor = uiColour;
	pVertices[ 3 ].uiColor = uiColour;

	pIndices[ 0 ] = 0;
	pIndices[ 1 ] = 1;
	pIndices[ 2 ] = 2;
	pIndices[ 3 ] = 3;

	pMesh->Unlock( 4, 4 );
	pMesh->Validate();
	pMesh->Render();
}

// $Barnyard: FUNCTION 00662a90
AVehicleSkidManager::AVehicleSkidManager()
    : m_pMaterial( TNULL )
    , m_pTexture( TNULL )
    , m_pTracks( TNULL )
    , m_pOldestTrack( TNULL )
    , m_iNumTracks( 0 )
{
}

// $Barnyard: FUNCTION 00662b10
// $Barnyard: FUNCTION 00662cf0
AVehicleSkidManager::~AVehicleSkidManager()
{
	if ( m_pTracks )
		delete[] m_pTracks;

	m_pTracks  = TNULL;
	m_pTexture = TNULL;

	if ( m_pMaterial )
	{
		m_pMaterial->Destroy();
		m_pMaterial = TNULL;
	}
}

// $Barnyard: FUNCTION 006626a0
TBOOL AVehicleSkidManager::OnUpdate( TFLOAT a_fDeltaTime )
{
	for ( TINT i = 0; i < m_iNumTracks; i++ )
	{
		if ( m_pTracks[ i ].fAge < m_pTracks[ i ].fLifetime )
			m_pTracks[ i ].fAge += a_fDeltaTime;
	}

	UpdateOldestTrack();

	return TTRUE;
}

// $Barnyard: FUNCTION 00662bb0
void AVehicleSkidManager::Create( TINT a_iNumTracks )
{
	if ( !m_pMaterial )
	{
		m_pMaterial = ASysShader::GetSingleton()->CreateMaterial( TNULL );

		if ( m_pMaterial && m_pMaterial->Create( ASysMaterial::BLENDMODE_1 ) )
		{
			m_pMaterial->SetTextureNum( 1 );
			m_pMaterial->SetTexture( AMaterialLibraryManager::GetSingleton()->FindTexture( TYRETRACK_TEXTURE ) );
		}
	}

	m_iNumTracks = a_iNumTracks;
	m_pTracks    = new TyreTrack[ a_iNumTracks ];

	for ( TINT i = 0; i < m_iNumTracks; i++ )
	{
		ASysMaterial* pMaterial = m_pMaterial;
		TyreTrack*    pTrack    = &m_pTracks[ i ];

		pTrack->pMesh = ASysShader::GetSingleton()->CreateMesh( TNULL );

		if ( pTrack->pMesh && pTrack->pMesh->CreatePools( ASysMesh::FLAGS_DEFAULT, 4, 4 ) && pMaterial )
			pTrack->pMesh->SetMaterial( pMaterial );
	}

	m_pOldestTrack = m_pTracks;
}

// $Barnyard: FUNCTION 00662670
AVehicleSkidManager::TyreTrack* AVehicleSkidManager::GetFreeTrack()
{
	if ( !m_pTexture || !m_pMaterial )
		return TNULL;

	TyreTrack* pTrack = m_pOldestTrack;

	if ( pTrack )
	{
		pTrack->fAge      = 0.0f;
		pTrack->fUVOffset = 0.0f;
	}

	UpdateOldestTrack();
	return pTrack;
}

// $Barnyard: FUNCTION 00662430
void AVehicleSkidManager::HideAllTracks()
{
	for ( TINT i = 0; i < m_iNumTracks; i++ )
	{
		TyreTrack* pTrack = &m_pTracks[ i ];

		if ( pTrack )
			pTrack->fAge = pTrack->fLifetime;
	}
}

// $Barnyard: FUNCTION 00662720
void AVehicleSkidManager::LoadTyreTrackTexture()
{
	if ( m_pTexture )
		return;

	m_pTexture = AMaterialLibraryManager::GetSingleton()->FindTexture( TYRETRACK_TEXTURE );

	if ( m_pTexture && m_pMaterial )
	{
		m_pMaterial->SetTexture( m_pTexture );

		for ( TINT i = 0; i < m_iNumTracks; i++ )
		{
			if ( m_pTracks[ i ].pMesh )
				m_pTracks[ i ].pMesh->SetMaterial( m_pMaterial );
		}
	}
}

// $Barnyard: FUNCTION 006623f0
void AVehicleSkidManager::ReleaseTyreTrackTexture()
{
	HideAllTracks();
	m_pTexture = TNULL;
}

// $Barnyard: FUNCTION 00662d10
void AVehicleSkidManager::Render()
{
	for ( TINT i = 0; i < m_iNumTracks; i++ )
	{
		TyreTrack* pTrack = &m_pTracks[ i ];

		if ( pTrack->fAge < pTrack->fLifetime && pTrack->IsVisible() )
			pTrack->Render();
	}
}

// $Barnyard: FUNCTION 006623b0
void AVehicleSkidManager::UpdateOldestTrack()
{
	TFLOAT fHighestAge = -TMath::MAXFLOAT;

	for ( TINT i = 0; i < m_iNumTracks; i++ )
	{
		if ( fHighestAge < m_pTracks[ i ].fAge )
		{
			fHighestAge    = m_pTracks[ i ].fAge;
			m_pOldestTrack = &m_pTracks[ i ];
		}
	}
}
