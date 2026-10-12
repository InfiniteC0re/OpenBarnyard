#include "pch.h"
#include "AMilkManager.h"
#include "ARandom.h"
#include "Assets/AAssetLoader.h"

#include "Helpers/AInstanceLightingHelper.h"

#include <Render/TModelManager.h>
#include <Render/TRenderInterface.h>
#include <Render/TModel.h>
#include <Render/TMesh.h>
#include <Toshi/TStringManager.h>
#include <GUI/T2GUIFontManager.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

#ifdef BARNYARD_COMMUNITY_PATCH
// Skeleton instances the particle models are rendered with, one per type (see Render)
static TSkeletonInstance** s_ppSkeletonInstances = TNULL;
#endif

// NOTE: inlined PackColour (004083c0, static in ABikeSteer.cpp), packs a colour (0-1 components) into ARGB
static TUINT32 PackColour( const TVector4& a_rcColour )
{
	return ( ( ( ( TUINT32( a_rcColour.w * 255.0f ) << 8 ) | TUINT32( a_rcColour.x * 255.0f ) ) << 8 | TUINT32( a_rcColour.y * 255.0f ) ) << 8 ) | TUINT32( a_rcColour.z * 255.0f );
}

// $Barnyard: FUNCTION 00666e50
AMilkManager::AMilkManager()
    : m_pData( TNULL )
    , m_ppModels( TNULL )
    , m_pParticles( TNULL )
    , m_pFreeParticles( TNULL )
    , m_ppUsedParticles( TNULL )
    , m_iNumFreeParticles( -1 )
    , m_pNumUsedParticles( TNULL )
    , m_bDebugInfo( TFALSE )
    , m_pTypeTextBoxes( TNULL )
    , m_pTypeStrings( TNULL )
{
}

// $Barnyard: FUNCTION 006684c0
AMilkManager::~AMilkManager()
{
	Unload();
}

// $Barnyard: FUNCTION 006685b0
AMilkManager* AMilkManager::Load( const TCHAR* a_szFileName )
{
	if ( GetSingleton() )
		return TNULL;

	// NOTE: TSingleton's constructor sets the singleton (retail assigns s_Instance here)
	new AMilkManager();

	if ( !GetSingleton()->LoadImpl( a_szFileName ) )
	{
		// NOTE: TSingleton's destructor clears the singleton
		delete GetSingleton();
	}

	return GetSingleton();
}

// $Barnyard: FUNCTION 00667280
TBOOL AMilkManager::LoadImpl( const TCHAR* a_szFileName )
{
	if ( !a_szFileName || m_pData )
		return TFALSE;

	TTRB::ERROR eError = m_oTRB.Load( a_szFileName, 0 );

	// NOTE: only errors ERROR_NO_HEADER to ERROR_PARSE_ERROR are treated as failures
	if ( eError >= TTRB::ERROR_NO_HEADER && eError <= TTRB::ERROR_PARSE_ERROR )
		return TFALSE;

	if ( !m_oTRB.GetSymbolAddress( "MilkParticleData" ) )
		return TFALSE;

	m_pData = TSTATICCAST( Data, m_oTRB.GetSymbolAddress( "MilkParticleData" ) );

	// Models of the particle types
	m_ppModels = new TManagedModel*[ m_pData->iNumTypes ];

	for ( TINT i = 0; i < m_pData->iNumTypes; i++ )
	{
		m_ppModels[ i ] = new TManagedModel();
		m_ppModels[ i ]->Create( m_pData->pszModels[ i ], AAssetLoader::GetAssetTRB( AAssetType_AssetPack ) );
	}

	// Pool of the particles, all of them are free
	m_pParticles = new Particle[ m_pData->iNumParticles ];

	for ( TINT i = 0; i < m_pData->iNumParticles; i++ )
	{
		m_pParticles[ i ].pNext = &m_pParticles[ i ] + 1;
		m_pParticles[ i ].pPrev = &m_pParticles[ i ] - 1;
	}

	m_pParticles[ 0 ].pPrev                          = TNULL;
	m_pParticles[ m_pData->iNumParticles - 1 ].pNext = TNULL;
	m_iNumFreeParticles                              = m_pData->iNumParticles;
	m_pFreeParticles                                 = m_pParticles;

	m_ppUsedParticles   = new Particle*[ m_pData->iNumTypes ];
	m_pNumUsedParticles = new TINT[ m_pData->iNumTypes ];

	for ( TINT i = 0; i < m_pData->iNumTypes; i++ )
	{
		m_ppUsedParticles[ i ]   = TNULL;
		m_pNumUsedParticles[ i ] = 0;
	}

	if ( a_szFileName != m_szFileName )
		TStringManager::String8Copy( m_szFileName, a_szFileName, sizeof( m_szFileName ) );

	// The debug overlay: particle counts and the lighting of the particles
	T2GUIFont* pFont = T2GUIFontManager::FindFont( "Rekord18" );

	m_oLactStats.Create( pFont, pFont->GetTextWidth( L"LactStats" ) );
	m_oLactStats.SetText( L"LactStats:" );
	m_oLactStats.SetTextAlign( T2GUIFont::TextAlign_Left );
	m_oLactStats.SetAttachment( T2GUIATTACHMENT_TOPLEFT, T2GUIATTACHMENT_TOPLEFT );
	m_oStatsElement.AddChildTail( m_oLactStats );

	// Ambient colour: the text, the colour used for the particles and the colour of the scene
	m_oAmbient.Create( pFont, pFont->GetTextWidth( L"Ambient: 000 000 000" ) );
	m_oAmbient.SetAttachment( T2GUIATTACHMENT_BOTTOMLEFT, T2GUIATTACHMENT_TOPLEFT );
	TStringManager::String16Format( m_wszAmbient, 32, L"Ambient: %3d %3d %3d", 255, 255, 255 );
	m_oAmbient.SetText( m_wszAmbient );
	m_oAmbient.SetTextAlign( T2GUIFont::TextAlign_Left );
	m_oLactStats.AddChildTail( m_oAmbient );

	m_oAmbientBackground.SetDimensions( m_oAmbient.GetHeight(), m_oAmbient.GetHeight() );
	m_oAmbientBackground.SetAttachment( T2GUIATTACHMENT_TOPRIGHT, T2GUIATTACHMENT_TOPLEFT );
	m_oAmbient.AddChildTail( m_oAmbientBackground );

	m_oAmbientColour.SetDimensions( m_oAmbient.GetHeight(), m_oAmbient.GetHeight() );
	m_oAmbientColour.SetAttachment( T2GUIATTACHMENT_TOPRIGHT, T2GUIATTACHMENT_TOPLEFT );
	m_oAmbientBackground.AddChildTail( m_oAmbientColour );

	// Diffuse colour
	m_oDiffuse.Create( pFont, pFont->GetTextWidth( L"Diffuse: 000 000 000" ) );
	m_oDiffuse.SetAttachment( T2GUIATTACHMENT_BOTTOMLEFT, T2GUIATTACHMENT_TOPLEFT );
	TStringManager::String16Format( m_wszDiffuse, 32, L"Diffuse: %3d %3d %3d", 255, 255, 255 );
	m_oDiffuse.SetText( m_wszDiffuse );
	m_oDiffuse.SetTextAlign( T2GUIFont::TextAlign_Left );
	m_oAmbient.AddChildTail( m_oDiffuse );

	m_oDiffuseBackground.SetDimensions( m_oDiffuse.GetHeight(), m_oDiffuse.GetHeight() );
	m_oDiffuseBackground.SetAttachment( T2GUIATTACHMENT_TOPRIGHT, T2GUIATTACHMENT_TOPLEFT );
	m_oDiffuse.AddChildTail( m_oDiffuseBackground );

	m_oDiffuseColour.SetDimensions( m_oDiffuse.GetHeight(), m_oDiffuse.GetHeight() );
	m_oDiffuseColour.SetAttachment( T2GUIATTACHMENT_TOPRIGHT, T2GUIATTACHMENT_TOPLEFT );
	m_oDiffuseBackground.AddChildTail( m_oDiffuseColour );

	// Particle counts
	m_oAllParticles.Create( pFont, pFont->GetTextWidth( L"All particles: F=00000, U=00000, T=00000" ) );
	m_oAllParticles.SetText( m_wszAllParticles );
	TStringManager::String16Format( m_wszAllParticles, 48, L"All particles: F=%5d, U=%5d, T=%5d", m_iNumFreeParticles, 0, TINT( m_pData->iNumParticles ) );
	m_oAllParticles.SetTextAlign( T2GUIFont::TextAlign_Left );
	m_oAllParticles.SetAttachment( T2GUIATTACHMENT_BOTTOMLEFT, T2GUIATTACHMENT_TOPLEFT );
	m_oDiffuse.AddChildTail( m_oAllParticles );

	T2GUIElement* pPrevElement = &m_oAllParticles;

	T2GUITransform oScale;
	oScale.SetFromScale( 0.75f, 0.75f );
	m_oStatsElement.GetTransform().PreMultiply( oScale );
	m_oStatsElement.SetAttachment( T2GUIATTACHMENT_TOPRIGHT, T2GUIATTACHMENT_TOPRIGHT );

	m_pTypeTextBoxes = new AGUI2TextBox[ m_pData->iNumTypes ];
	m_pTypeStrings   = new TWCHAR*[ m_pData->iNumTypes ];

	for ( TINT i = 0; i < m_pData->iNumTypes; i++ )
	{
		m_pTypeTextBoxes[ i ].Create( pFont, pFont->GetTextWidth( L"Type 00 Used: 00000" ) );
		m_pTypeTextBoxes[ i ].SetAttachment( T2GUIATTACHMENT_BOTTOMLEFT, T2GUIATTACHMENT_TOPLEFT );

		m_pTypeStrings[ i ] = new TWCHAR[ 32 ];
		TStringManager::String16Format( m_pTypeStrings[ i ], 32, L"Type %d Used: %5d", i, m_pNumUsedParticles[ i ] );

		m_pTypeTextBoxes[ i ].SetText( m_pTypeStrings[ i ] );
		m_pTypeTextBoxes[ i ].SetTextAlign( T2GUIFont::TextAlign_Left );

		// Each one below the previous one
		pPrevElement->AddChildTail( m_pTypeTextBoxes[ i ] );
		pPrevElement = &m_pTypeTextBoxes[ i ];
	}

	// Big enough for all of them
	TFLOAT fWidth  = TMath::Max( m_oAllParticles.GetWidth(), m_pTypeTextBoxes[ 0 ].GetWidth() );
	TFLOAT fHeight = m_pTypeTextBoxes[ 0 ].GetHeight() * TFLOAT( m_pData->iNumTypes );
	fHeight += m_oAllParticles.GetHeight();
	fHeight += m_oAmbient.GetHeight();
	fHeight += m_oLactStats.GetHeight();
	fHeight += m_oDiffuse.GetHeight();

	m_oStatsElement.SetDimensions( fWidth, fHeight );

	return TTRUE;
}

// $Barnyard: FUNCTION 00667a60
void AMilkManager::Unload()
{
	if ( !m_pData )
		return;

#ifdef BARNYARD_COMMUNITY_PATCH
	if ( s_ppSkeletonInstances )
	{
		for ( TINT i = 0; i < m_pData->iNumTypes; i++ )
		{
			if ( s_ppSkeletonInstances[ i ] )
				s_ppSkeletonInstances[ i ]->Delete();
		}

		delete[] s_ppSkeletonInstances;
		s_ppSkeletonInstances = TNULL;
	}
#endif

	if ( m_ppModels )
	{
		for ( TINT i = 0; i < m_pData->iNumTypes; i++ )
		{
			if ( m_ppModels[ i ] )
			{
				delete m_ppModels[ i ];
				m_ppModels[ i ] = TNULL;
			}
		}

		delete[] m_ppModels;
		m_ppModels = TNULL;
	}

	if ( m_pParticles )
	{
		delete[] m_pParticles;
		m_pParticles = TNULL;
	}

	if ( m_ppUsedParticles )
	{
		delete[] m_ppUsedParticles;
		m_ppUsedParticles = TNULL;
	}

	if ( m_pNumUsedParticles )
	{
		delete[] m_pNumUsedParticles;
		m_pNumUsedParticles = TNULL;
	}

	// NOTE: m_oAmbient is never unlinked by the original code
	m_oStatsElement.RemoveSelf();
	m_oLactStats.RemoveSelf();
	m_oAllParticles.RemoveSelf();
	m_oAmbientBackground.RemoveSelf();
	m_oAmbientColour.RemoveSelf();
	m_oDiffuse.RemoveSelf();
	m_oDiffuseBackground.RemoveSelf();
	m_oDiffuseColour.RemoveSelf();

	if ( m_pTypeTextBoxes )
	{
		for ( TINT i = 0; i < m_pData->iNumTypes; i++ )
			m_pTypeTextBoxes[ i ].RemoveSelf();

		delete[] m_pTypeTextBoxes;
		m_pTypeTextBoxes = TNULL;
	}

	if ( m_pTypeStrings )
	{
		for ( TINT i = 0; i < m_pData->iNumTypes; i++ )
		{
			if ( m_pTypeStrings[ i ] )
			{
				delete[] m_pTypeStrings[ i ];
				m_pTypeStrings[ i ] = TNULL;
			}
		}

		// NOTE: the original code doesn't reset m_pTypeStrings
		delete[] m_pTypeStrings;
	}

	m_oTRB.Close();
	m_pData = TNULL;
}

// $Barnyard: FUNCTION 00667080
void AMilkManager::Update( TFLOAT a_fDeltaTime )
{
	for ( TINT i = 0; i < m_pData->iNumTypes; i++ )
	{
		for ( Particle* pParticle = m_ppUsedParticles[ i ]; pParticle != TNULL; pParticle = pParticle->pNext )
		{
			TVector4 vOffset;
			vOffset.Multiply( pParticle->vVelocity, a_fDeltaTime );

			pParticle->vPosition.x += vOffset.x;
			pParticle->vPosition.z += vOffset.z;
			pParticle->vPosition.y = a_fDeltaTime * a_fDeltaTime * 4.9f + vOffset.y + pParticle->vPosition.y;
			pParticle->vVelocity.y += a_fDeltaTime * 9.8f;
		}
	}

	if ( m_bDebugInfo )
	{
		TINT iNumUsed = 0;

		for ( TINT i = 0; i < m_pData->iNumTypes; i++ )
		{
			TStringManager::String16Format( m_pTypeStrings[ i ], 32, L"Type %2d Used: %5d", i, m_pNumUsedParticles[ i ] );
			iNumUsed += m_pNumUsedParticles[ i ];
		}

		TStringManager::String16Format( m_wszAllParticles, 48, L"All particles: F=%5d, U=%5d, T=%5d", m_iNumFreeParticles, iNumUsed, TINT( m_pData->iNumParticles ) );
	}
}

// $Barnyard: FUNCTION 00667d70
void AMilkManager::Render()
{
	TRenderInterface* pRenderInterface = TRenderInterface::GetSingleton();
	TRenderContext*   pRenderContext   = pRenderInterface->GetCurrentContext();

	TVector4  vAmbient     = pRenderContext->GetAmbientColour();
	TMatrix44 mLightColour = pRenderInterface->GetLightColour();

	if ( m_bDebugInfo )
	{
		m_oAmbientColour.SetColour( PackColour( vAmbient ) );
		m_oDiffuseColour.SetColour( PackColour( mLightColour.AsBasisVector4( BASISVECTOR_RIGHT ) ) );
	}

	if ( m_iNumFreeParticles == m_pData->iNumParticles )
		return;

	const TPlane* pPlanes = pRenderContext->GetWorldPlanes();

	TMatrix44 mOldLightColour = mLightColour;
	TVector4  vOldAmbient     = vAmbient;
	TFLOAT    fOldAlphaBlend  = pRenderContext->GetAlphaBlend();
	TUINT     uiOldClipFlags  = pRenderContext->GetClipFlags();
	TMatrix44 mModelView      = pRenderContext->GetModelViewMatrix();
	TFLOAT    fOldShadeCoeff  = pRenderContext->GetShadeCoeff();
#ifdef BARNYARD_COMMUNITY_PATCH
	TSkeletonInstance* pOldSkeletonInstance = pRenderContext->GetSkeletonInstance();
#endif

	pRenderContext->SetShadeCoeff( AInstanceLightingHelper::GetSingleton()->GetShadeCoeff() );
	pRenderContext->SetAlphaBlend( 1.0f );

	// Desaturate the ambient and the diffuse colours towards their brightest component and scale them
	const TFLOAT* pRenderParams = m_pData->pRenderParams;

	TFLOAT   fMax = TMath::Max( TMath::Max( vAmbient.x, vAmbient.y ), vAmbient.z );
	TVector4 vOffset;
	vOffset.Multiply( TVector4( fMax - vAmbient.x, fMax - vAmbient.y, fMax - vAmbient.z, 0.0f ), pRenderParams[ 0 ] );
	vAmbient.x += vOffset.x;
	vAmbient.y += vOffset.y;
	vAmbient.z += vOffset.z;

	TVector4& rDiffuse = mLightColour.AsBasisVector4( BASISVECTOR_RIGHT );
	fMax               = TMath::Max( TMath::Max( rDiffuse.x, rDiffuse.y ), rDiffuse.z );
	vOffset.Multiply( TVector4( fMax - rDiffuse.x, fMax - rDiffuse.y, fMax - rDiffuse.z, 0.0f ), pRenderParams[ 2 ] );
	rDiffuse.x += vOffset.x;
	rDiffuse.y += vOffset.y;
	rDiffuse.z += vOffset.z;

	vAmbient.Multiply( pRenderParams[ 1 ] );
	rDiffuse.Multiply( pRenderParams[ 3 ] );

	pRenderContext->SetAmbientColour( vAmbient );
	pRenderInterface->SetLightColourMatrix( mLightColour );

	if ( m_bDebugInfo )
	{
		TStringManager::String16Format( m_wszAmbient, 32, L"Ambient: %3d %3d %3d", TINT( vAmbient.x * 255.0f ), TINT( vAmbient.y * 255.0f ), TINT( vAmbient.z * 255.0f ) );
		TStringManager::String16Format( m_wszDiffuse, 32, L"Diffuse: %3d %3d %3d", TINT( rDiffuse.x * 255.0f ), TINT( rDiffuse.y * 255.0f ), TINT( rDiffuse.z * 255.0f ) );
		m_oAmbientBackground.SetColour( PackColour( vAmbient ) );
		m_oDiffuseBackground.SetColour( PackColour( rDiffuse ) );
	}

	for ( TINT i = 0; i < m_pData->iNumTypes; i++ )
	{
		TMesh* pMesh = m_ppModels[ i ]->GetModel()->GetLOD( 0 ).ppMeshes[ 0 ];

		if ( !pMesh )
			continue;

#ifdef BARNYARD_COMMUNITY_PATCH
		// The particle models are skinned, but retail renders them with whatever skeleton instance
		// the render context was left with. Use one in the base pose of the model's own skeleton
		// instead, the leftover one makes the particles invisible here
		if ( !s_ppSkeletonInstances )
		{
			s_ppSkeletonInstances = new TSkeletonInstance*[ m_pData->iNumTypes ];

			for ( TINT k = 0; k < m_pData->iNumTypes; k++ )
			{
				TSkeleton* pSkeleton       = m_ppModels[ k ]->GetModel()->GetSkeleton();
				s_ppSkeletonInstances[ k ] = pSkeleton ? pSkeleton->CreateInstance( TTRUE ) : TNULL;
			}
		}

		if ( s_ppSkeletonInstances[ i ] )
			pRenderContext->SetSkeletonInstance( s_ppSkeletonInstances[ i ] );
#endif

		for ( Particle* pParticle = m_ppUsedParticles[ i ]; pParticle; pParticle = pParticle->pNext )
		{
			// Skip the particles outside of the frustum
			TBOOL bVisible = TTRUE;

			for ( TINT j = 0; j < 6 && bVisible; j++ )
			{
				const TPlane& rcPlane = pPlanes[ j ];
				bVisible              = rcPlane.GetNormal().x * pParticle->vPosition.x + rcPlane.GetNormal().y * pParticle->vPosition.y + rcPlane.GetNormal().z * pParticle->vPosition.z - rcPlane.GetD() <= 1.0f;
			}

			if ( !bVisible )
				continue;

			pRenderContext->SetClipFlags( TRenderContext::CLIPFLAGS_ALL );

			// Face the direction the particle moves in
			TVector4 vDirection = pParticle->vVelocity;
			vDirection.Normalise();

			TMatrix44 mRotation;
			mRotation.Identity();
			mRotation.LookAtDirection( vDirection, TVector4::VEC_NEGY );

			TMatrix44 mTransform = mModelView;
			TMatrix44::TransformVector( mTransform.AsBasisVector4( BASISVECTOR_TRANSLATION ), mTransform, pParticle->vPosition );
			mTransform.Multiply( mRotation );

			pRenderContext->SetModelViewMatrix( mTransform );
			pMesh->Render();
		}
	}

#ifdef BARNYARD_COMMUNITY_PATCH
	pRenderContext->SetSkeletonInstance( pOldSkeletonInstance );
#endif

	pRenderInterface->SetLightColourMatrix( mOldLightColour );
	pRenderContext->SetAmbientColour( vOldAmbient );
	pRenderContext->SetModelViewMatrix( mModelView );
	pRenderContext->SetAlphaBlend( fOldAlphaBlend );
	pRenderContext->SetShadeCoeff( fOldShadeCoeff );
	pRenderContext->SetClipFlags( uiOldClipFlags );
}

// $Barnyard: FUNCTION 00667c70
TINT AMilkManager::SpawnParticle( const Toshi::TVector4& a_rcPosition, const Toshi::TVector4& a_rcVelocity )
{
	if ( m_iNumFreeParticles == 0 )
		return -1;

	// Take the first free particle
	Particle* pParticle = m_pFreeParticles;
	Particle* pNext     = pParticle->pNext;

	if ( !pNext )
	{
		m_pFreeParticles = TNULL;
	}
	else
	{
		m_pFreeParticles = pNext;
		pNext->pPrev     = TNULL;
	}

	pParticle->pPrev = TNULL;
	pParticle->pNext = TNULL;
	m_iNumFreeParticles--;

	pParticle->iType     = ARandom::GetSingleton()->m_oRandom.GetInt( m_pData->iNumTypes );
	pParticle->vPosition = a_rcPosition;
	pParticle->vVelocity = a_rcVelocity;

	// Add it to the list of its type
	pParticle->pNext = m_ppUsedParticles[ pParticle->iType ];

	if ( m_ppUsedParticles[ pParticle->iType ] )
		m_ppUsedParticles[ pParticle->iType ]->pPrev = pParticle;

	m_ppUsedParticles[ pParticle->iType ] = pParticle;
	m_pNumUsedParticles[ pParticle->iType ]++;

	return TINT( pParticle - m_pParticles );
}

// $Barnyard: FUNCTION 006671e0
void AMilkManager::UpdateParticle( TINT a_iParticle, TFLOAT a_fTime )
{
	if ( a_iParticle > -1 && a_iParticle < m_pData->iNumParticles )
	{
		Particle* pParticle = &m_pParticles[ a_iParticle ];

		TVector4 vOffset;
		vOffset.Multiply( pParticle->vVelocity, a_fTime );

		pParticle->vPosition.x += vOffset.x;
		pParticle->vPosition.z += vOffset.z;
		pParticle->vPosition.y = a_fTime * a_fTime * 4.9f + pParticle->vPosition.y + vOffset.y;
		pParticle->vVelocity.y += a_fTime * 9.8f;
	}
}

// $Barnyard: FUNCTION 00666fb0
void AMilkManager::ReleaseParticles( TINT a_iNumParticles, const TINT* a_pParticles )
{
	if ( a_iNumParticles < 0 || !a_pParticles )
		return;

	for ( TINT i = 0; i < a_iNumParticles; i++ )
	{
		TINT iParticle = a_pParticles[ i ];

		if ( iParticle > -1 && iParticle < m_pData->iNumParticles )
		{
			Particle* pParticle = &m_pParticles[ iParticle ];

			// Remove it from the list of its type
			if ( pParticle->pPrev )
				pParticle->pPrev->pNext = pParticle->pNext;

			if ( pParticle->pNext )
				pParticle->pNext->pPrev = pParticle->pPrev;

			if ( m_ppUsedParticles[ pParticle->iType ] == pParticle )
				m_ppUsedParticles[ pParticle->iType ] = pParticle->pNext;

			pParticle->pNext = TNULL;
			pParticle->pPrev = TNULL;
			m_pNumUsedParticles[ pParticle->iType ]--;

			// Add it to the free ones
			pParticle->pNext = m_pFreeParticles;

			if ( m_pFreeParticles )
				m_pFreeParticles->pPrev = pParticle;

			m_pFreeParticles = pParticle;
			m_iNumFreeParticles++;
		}
	}
}
