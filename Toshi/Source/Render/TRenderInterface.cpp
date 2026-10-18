#include "ToshiPCH.h"
#include "TRenderInterface.h"
#include "File/TFile.h"
#include "Render/TShader.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

TDEFINE_CLASS_NORUNTIME( TRenderInterface );

// $Barnyard: FUNCTION 006bf490
TRenderInterface::TRenderInterface()
{
	m_bCreateSystemResources = TTRUE;
	m_bInScene               = TFALSE;
	m_bCreated               = TFALSE;
	m_bDisplayCreated        = TFALSE;
	m_ScreenOffset.Set( 0.0f, 0.0f );
	m_pCurrentContext    = TNULL;
	m_pCreatedContext    = TNULL;
	m_pDebugText         = TNULL;
	m_Unk2               = TNULL;
	m_pInvalidTexture    = TNULL;
	m_ResourceCount      = 1;
	m_iFrameCount        = 0;
	m_bHasDyingResources = TFALSE;

	m_Transforms.PushNull();
	m_Transforms.Top().Identity();

	TUtil::MemClear( m_SystemResources, sizeof( m_SystemResources ) );

	m_LightColour.Set(
	    1.0f, 1.0f, 1.0f, 0.0f,
	    0.0f, 0.0f, 0.0f, 0.0f,
	    0.0f, 0.0f, 0.0f, 0.0f,
	    0.0f, 0.0f, 0.0f, 0.0f
	);

	m_LightDirection.Set(
	    -0.47f, -0.74f, 0.47f, 0.0f,
	    0.0f, 0.0f, 0.0f, 0.0f,
	    0.0f, 0.0f, 0.0f, 0.0f,
	    0.0f, 0.0f, 0.0f, 0.0f
	);
}

// $Barnyard: FUNCTION 006bf6d0
TRenderInterface::~TRenderInterface()
{
}

// $Barnyard: FUNCTION 006be9f0
TBOOL TRenderInterface::Create()
{
	TASSERT( TFALSE == IsCreated(), "TRender already created" );
	m_pCreatedContext = CreateRenderContext();

#if defined( BARNYARD_COMMUNITY_PATCH ) && defined( TOSHI_SKU_WINDOWS )
	constexpr TSIZE uiMemBlockSize = 256 * 1024 * 1024;

	m_pMemBlockRaw = HeapAlloc( GetProcessHeap(), HEAP_ZERO_MEMORY, uiMemBlockSize );
	m_pMemBlock    = g_pMemory->CreateMemBlockInPlace( m_pMemBlockRaw, uiMemBlockSize, "TRenderInterface" );
#endif // BARNYARD_COMMUNITY_PATCH && TOSHI_SKU_WINDOWS

	if ( m_pCreatedContext )
	{
		SetCurrentRenderContext( m_pCreatedContext );
		m_Unk2     = CreateUnknown( "Render", 0, 0, 10 );
		m_bCreated = TTRUE;
		return TTRUE;
	}

	return TFALSE;
}

// $Barnyard: FUNCTION 006bea30
TBOOL TRenderInterface::Destroy()
{
	TASSERT( TTRUE == IsCreated(), "TRender is not created" );

#if defined( BARNYARD_COMMUNITY_PATCH ) && defined( TOSHI_SKU_WINDOWS )

	g_pMemory->DestroyMemBlock( m_pMemBlock );
	HeapFree( GetProcessHeap(), 0, m_pMemBlockRaw );

#endif // BARNYARD_COMMUNITY_PATCH && TOSHI_SKU_WINDOWS

	if ( m_Unk2 )
	{
		delete m_Unk2;
		m_Unk2 = TNULL;
	}

	if ( m_pCreatedContext )
	{
		delete m_pCreatedContext;
		m_pCreatedContext = TNULL;
	}

	m_pCurrentContext = TNULL;
	m_bCreated        = TFALSE;

	return TTRUE;
}

void TRenderInterface::RenderIndexPrimitive( TINT a_ePrimitiveType, TINT a_iNumVertices, TINT a_iNumPrimitives, TINT a_pIndices, TINT a_pVertices, TINT a_iVertexStride )
{
}

TBOOL TRenderInterface::CreateDisplay()
{
	TASSERT( TTRUE == IsCreated(), "TRender must be created" );
	TASSERT( TFALSE == IsDisplayCreated(), "Display already created" );

	m_bDisplayCreated = TTRUE;
	return TTRUE;
}

TBOOL TRenderInterface::CreateDisplay( const DISPLAYPARAMS& a_rParams )
{
	m_bDisplayCreated = TTRUE;
	return TTRUE;
}

// $Barnyard: FUNCTION 006bea70
TBOOL TRenderInterface::BeginScene()
{
	m_iFrameCount += 1;
	m_Transforms.Reset();
	m_Transforms.PushNull().Identity();
	return TTRUE;
}

// $Barnyard: FUNCTION 006bec20
void TRenderInterface::DestroyResource( TResource* resource )
{
	TASSERT( TNULL != resource->Tree(), "Resource doesn't have a tree" );
	TASSERT( TFALSE == resource->IsDead(), "Resource is already dead" );

	if ( resource->IsDying() == TFALSE )
	{
		m_bHasDyingResources = TTRUE;
		resource->AddState( TResourceState_Dying );
		DestroyResourceRecurse( resource->Child() );
		resource->Invalidate();
	}
}

// $Barnyard: FUNCTION 006beaa0
void TRenderInterface::DestroyResourceRecurse( TResource* resource )
{
	if ( resource != TNULL )
	{
		TResource* lastResource = resource->Prev();

		while ( resource != TNULL )
		{
			TResource* nextResource = ( resource != lastResource ) ? resource->Next() : TNULL;

			if ( resource->IsDying() == TFALSE )
			{
				m_bHasDyingResources = TTRUE;
				resource->AddState( TResourceState_Dying );

				if ( resource->Child() != TNULL )
				{
					DestroyResourceRecurse( resource->Child() );
				}

				resource->Invalidate();
			}

			resource = nextResource;
		}
	}
}

// $Barnyard: FUNCTION 006be8f0
void TRenderInterface::SetResourceExplicit( TResource* resource, SYSRESOURCE systemResource )
{
	TASSERT( systemResource >= 0 && systemResource < SYSRESOURCE_NUMOF, "Unknown resource" );
	TASSERT( m_SystemResources[ systemResource ] == TNULL, "This resource has already been assigned!" );
	m_SystemResources[ systemResource ] = resource;
}

// $Barnyard: FUNCTION 006bf240
TResource* TRenderInterface::CreateResource( const TClass* pClass, const TCHAR* name, TResource* parent )
{
	TASSERT( pClass != TNULL, "TResource class is TNULL" );
	TASSERT( pClass->IsA( &TGetClass( TResource ) ), "TResource class is TNULL" );

	TResource* pResource = static_cast<TResource*>( pClass->CreateObject() );
	TASSERT( pResource != TNULL, "Couldn't create TResource" );

	if ( pResource != TNULL )
	{
		m_Resources.Remove( *pResource, TFALSE );

		if ( parent == TNULL )
		{
			parent = m_Resources.GetRoot();
		}

		m_Resources.Insert( parent, pResource );
		pResource->SetUId( m_ResourceCount++ );
		pResource->SetRenderer( this );
		pResource->SetName( name );

		return pResource;
	}

	return pResource;
}

// $Barnyard: FUNCTION 006becf0
void TRenderInterface::DumpStats()
{
	static const TCHAR* s_aDepthStencilFormats[] = { "D16_S0", "D15_S1", "D24_S0", "D24_S8", "D32_S0" }; // 0x0077cf68

	TFile* pFile = TFile::Create( "TRenderInterface.txt", TFILEMODE_CREATENEW );

	if ( pFile != TNULL )
	{
		pFile->CPrintf( "-\r\n" );

		// NOTE: retail passes the minor version before the major one
		pFile->CPrintf( "Toshi rendering interface through object [\"%s\"] Version : %u.%u\r\n", GetClass()->GetName(), GetClass()->GetVersionMinor(), GetClass()->GetVersionMajor() );
		pFile->CPrintf( "-\r\n" );

		for ( auto pAdapter = m_AdapterList.Begin(); pAdapter != m_AdapterList.End(); pAdapter++ )
		{
			pFile->CPrintf( "Adapter       : %d\r\n", pAdapter->GetAdapterIndex() );
			pFile->CPrintf( "  Driver      : %s\r\n", pAdapter->GetDriver().GetString() );
			pFile->CPrintf( "  Description : %s\r\n", pAdapter->GetDriverDescription().GetString() );
			pFile->CPrintf( "  Product ID  : %u\r\n", pAdapter->GetProductID() );
			pFile->CPrintf( "  Version     : %u\r\n", pAdapter->GetVersion() );
			pFile->CPrintf( "  Sub-version : %u\r\n", pAdapter->GetSubVersion() );
			pFile->CPrintf( "  Build       : %u\r\n", pAdapter->GetBuild() );

			// NOTE: retail prints the address of Data4 as the last number
			const GUID* pGUID = TSTATICCAST( const GUID, pAdapter->GetDeviceIdentifier() );
			pFile->CPrintf( "  GUID        : %u-%u-%u-%u\r\n", pGUID->Data1, pGUID->Data2, pGUID->Data3, pGUID->Data4 );

			pFile->CPrintf( "  Supports %d modes:\r\n", pAdapter->GetModeList()->Count() );

			for ( auto pMode = pAdapter->GetModeList()->Begin(); pMode != pAdapter->GetModeList()->End(); pMode++ )
			{
				const TCHAR* sz32Bit = pMode->Is32Bit() == TTRUE ? "Yes" : "No";
				const TCHAR* sz16Bit = pMode->Is16Bit() == TTRUE ? "Yes" : "No";

				pFile->CPrintf( "    Mode %d: %ux%u , %uhz: 16/32 bit Frame Buffer [%s/%s]\r\n", pMode->GetModeIndex(), pMode->GetWidth(), pMode->GetHeight(), pMode->GetRefreshRate(), sz16Bit, sz32Bit );

				for ( TUINT32 i = 0; i < pAdapter->GetNumSupportedDevices(); i++ )
				{
					TRenderAdapter::Mode::Device* pDevice = pMode->GetDevice( i );

					pFile->CPrintf( "      Supported Device [\"%s\"]: \r\n", pDevice->GetTypeString() );
					pFile->CPrintf( "        Supports Hardware TnL? [\"%s\"]: \r\n", pDevice->SupportsHardwareTransfomations() == TTRUE ? "Yes!" : "No" );
					pFile->CPrintf( "        Supports Windowed? [\"%s\"]: \r\n", pDevice->CanRenderWindowed() == TTRUE ? "Yes!" : "No" );
					pFile->CPrintf( "        Supported Depth/Stencil Modes : " );

					for ( TUINT32 k = 0; k < TARRAYSIZE( s_aDepthStencilFormats ); k++ )
					{
						if ( pDevice->IsDepthStencilFormatSupported( k ) == TTRUE )
							pFile->CPrintf( "\"%s\" ", s_aDepthStencilFormats[ k ] );
					}

					pFile->CPrintf( "\r\n" );
				}
			}
		}

		pFile->Destroy();
	}
}

// $Barnyard: FUNCTION 006be9b0
void TRenderInterface::GetScreenOffset( TVector2& a_rVec )
{
	a_rVec.Set( m_ScreenOffset );
}

// $Barnyard: FUNCTION 006be9d0
void TRenderInterface::SetScreenOffset( const TVector2& a_rVec )
{
	m_ScreenOffset.Set( a_rVec );
}

// $Barnyard: FUNCTION 006bf670
TFLOAT TRenderInterface::GetScreenAspectRatio()
{
	return 4.0f / 3.0f;
}

TFLOAT TRenderInterface::GetPixelAspectRatio()
{
	return 1.0f;
}

TBOOL TRenderInterface::SetPixelAspectRatio( TFLOAT a_fPixelAspectRatio )
{
	return TFALSE;
}

void TRenderInterface::FlushOrderTables()
{
}

TBOOL TRenderInterface::IsTextureFormatSupported( TTEXTURERESOURCEFORMAT a_eTextureFormat )
{
	return TTRUE;
}

TBOOL TRenderInterface::Supports32BitTextures()
{
	return TFALSE;
}

// $Barnyard: FUNCTION 006bf690
void TRenderInterface::SetLightDirectionMatrix( const TMatrix44& a_rMatrix )
{
	m_LightDirection = a_rMatrix;
}

// $Barnyard: FUNCTION 006bf6b0
void TRenderInterface::SetLightColourMatrix( const TMatrix44& a_rMatrix )
{
	m_LightColour = a_rMatrix;
}

void TRenderInterface::ConnectDefaultViewportHandlers( TViewport* a_pViewport )
{
}

void TRenderInterface::OnInitializationFailureDevice()
{
}

void TRenderInterface::OnInitializationFailureDisplay()
{
}

TBOOL TRenderInterface::CreateSystemResources()
{
	return TTRUE;
}

// $Barnyard: FUNCTION 006bf800
void TRenderInterface::DestroySystemResources()
{
	DestroyResourceRecurse( m_Resources.ChildOfRoot() );
	FlushDyingResources();
}

// $Barnyard: FUNCTION 006bf230
void TRenderInterface::DestroyAllShaderResources()
{
	DestroyResourcesOfClassRecurse( &TGetClass( TShader ), m_SystemResources[ SYSRESOURCE_Shaders ] );
}

// $Barnyard: FUNCTION 006bec60
void TRenderInterface::DestroyResourcesOfClassRecurse( const TClass* a_pClass, TResource* a_pResource )
{
	if ( a_pResource == TNULL )
		return;

	TResource* pResource = a_pResource;

	while ( TTRUE )
	{
		TResource* pNext = pResource->Next();

		if ( pNext == a_pResource )
			pNext = TNULL;

		if ( pResource->Child() )
			DestroyResourcesOfClassRecurse( a_pClass, pResource->Child() );

		if ( !pResource->IsDying() && pResource->GetClass()->IsA( a_pClass ) && !pResource->IsDying() )
		{
			m_bHasDyingResources = TTRUE;
			pResource->AddState( TResourceState_Dying );
			DestroyResourceRecurse( pResource->Child() );
			pResource->Invalidate();
		}

		pResource = pNext;

		if ( pNext == TNULL )
			break;
	}
}

// $Barnyard: FUNCTION 006bf750
void TRenderInterface::DestroyDyingResources( TResource* a_pResources )
{
	// The resources are a circular list of siblings starting at a_pResources
	TResource* pFirst    = a_pResources;
	TResource* pResource = a_pResources;

	while ( pResource )
	{
		TResource* pNext = pResource->Next();
		if ( pNext == pFirst ) pNext = TNULL;

		if ( !pResource->IsDying() )
		{
			if ( TResource* pChild = pResource->Child() )
				DestroyDyingResources( pChild );
		}
		else
		{
			// The list starts at the next resource once this one is gone
			if ( pResource == pFirst )
				pFirst = pNext;

			TResource* pChild = pResource->Child();

			while ( pChild )
			{
				TResource* pNextChild = pChild->Next();
				if ( pNextChild == pChild ) pNextChild = TNULL;

				DeleteResourceAtomic( pChild );
				pChild = pNextChild;
			}

			DeleteResourceAtomic( pResource );
		}

		pResource = pNext;
	}
}

void TRenderInterface::DeleteResource( TResource* resources )
{
	DeleteResourceRecurse( resources->Child() );
	DeleteResourceAtomic( resources );
}

void TRenderInterface::DeleteResourceRecurse( TResource* resource )
{
	while ( resource )
	{
		auto next = resource->Next();
		if ( next == resource ) next = TNULL;

		DeleteResourceAtomic( resource );
		resource = next;
	}
}

// $Barnyard: FUNCTION 006bf420
void TRenderInterface::DeleteResourceAtomic( TResource* a_pResource )
{
	if ( a_pResource )
	{
		DeleteResourceRecurse( a_pResource->Child() );

		TASSERT( TFALSE == a_pResource->IsValid() );

		if ( a_pResource->IsCreated() )
		{
			a_pResource->OnDestroy();
		}

		m_Resources.Remove( a_pResource, TFALSE );

		if ( a_pResource->IsExternal() )
		{
			a_pResource->~TResource();
		}
		else
		{
			delete a_pResource;
		}
	}
}

// $Barnyard: FUNCTION 006bf030
TRenderAdapter::Mode::Device* TRenderInterface::FindDevice( const DISPLAYPARAMS& a_rDisplayParams )
{
	auto pAdapter = GetAdapterList()->Begin();

	while ( TTRUE )
	{
		if ( pAdapter == GetAdapterList()->End() )
		{
			TString8 errorString = "Unable to find the passed device!\n";
#ifdef TOSHI_DEBUG
			OutputDebugStringA( errorString );
#endif
			return TNULL;
		}

		for ( auto pMode = pAdapter->GetModeList()->Begin(); pMode != pAdapter->GetModeList()->End(); pMode++ )
		{
			auto uiNumSupportedDevices = pAdapter->GetNumSupportedDevices();

			for ( TUINT32 i = 0; i < uiNumSupportedDevices; i++ )
			{
				auto pDevice = pMode->GetDevice( i );

				if ( pDevice->IsSoftware() ) continue;

				TBOOL bPassedColourDepth = TTRUE;

				if ( a_rDisplayParams.uiColourDepth == 32 )
				{
					if ( !pDevice->GetMode()->Is32Bit() )
					{
						bPassedColourDepth = TFALSE;
					}
				}
				else if ( a_rDisplayParams.uiColourDepth == 16 )
				{
					if ( !pDevice->GetMode()->Is16Bit() )
					{
						bPassedColourDepth = TFALSE;
					}
				}

				TBOOL bPassedWindowed = TTRUE;

				if ( a_rDisplayParams.bWindowed && !pDevice->CanRenderWindowed() )
				{
					bPassedWindowed = TFALSE;
				}

				auto uiWidth  = pDevice->GetMode()->GetWidth();
				auto uiHeight = pDevice->GetMode()->GetHeight();

				if ( uiHeight < uiWidth || !a_rDisplayParams.bWindowed || a_rDisplayParams.uiWidth <= a_rDisplayParams.uiHeight )
				{
					if ( uiWidth == a_rDisplayParams.uiWidth && uiHeight == a_rDisplayParams.uiHeight )
					{
						if ( pDevice->IsDepthStencilFormatSupported( a_rDisplayParams.eDepthStencilFormat ) &&
						     bPassedColourDepth && bPassedWindowed )
						{
							return pDevice;
						}
					}
				}
				else
				{
					if ( a_rDisplayParams.uiWidth <= uiWidth )
					{
						if ( uiHeight < a_rDisplayParams.uiHeight ) continue;

						if ( pDevice->IsDepthStencilFormatSupported( a_rDisplayParams.eDepthStencilFormat ) &&
						     bPassedColourDepth && bPassedWindowed )
						{
							return pDevice;
						}
					}
				}
			}
		}

		pAdapter++;
	}
}

// $Barnyard: FUNCTION 006be910
TBOOL TRenderInterface::DestroyDisplay()
{
	m_bDisplayCreated = TFALSE;
	return TTRUE;
}

// $Barnyard: FUNCTION 006bf850
TBOOL TRenderInterface::Update( TFLOAT a_fDeltaTime )
{
	FlushDyingResources();
	return TTRUE;
}

// $Barnyard: FUNCTION 006bf7d0
void TRenderInterface::FlushDyingResources()
{
	while ( m_bHasDyingResources )
	{
		m_bHasDyingResources = TFALSE;
		DestroyDyingResources( m_Resources.ChildOfRoot() );
	}
}

TOSHI_NAMESPACE_END
