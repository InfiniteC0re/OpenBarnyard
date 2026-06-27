#include "pch.h"
#include "AHooks.h"
#include "ModLoader.h"
#include "HookHelpers.h"
#include "AImGUI.h"
#include "World.h"

#include "BYardSDK/SDKHooks.h"
#include "BYardSDK/AGameStateController.h"

#include <Render/TTMDWin.h>
#include <Render/TIndexFactoryResourceInterface.h>
#include <Render/TVertexFactoryResourceInterface.h>
#include <Toshi/THPTimer.h>
#include <Toshi/TApplication.h>
#include <Toshi/TUtil.h>

#include <Platform/DX8/TRenderInterface_DX8.h>

#include <cstdio>
#include <windows.h>

#include <detours.h>

bool UnprotectMemory( void* memory, std::size_t size, unsigned long& oldProtect )
{
	return ( VirtualProtect( memory, size, PAGE_EXECUTE_READWRITE, &oldProtect ) != 0 );
}

bool WriteJump( void* jumpAddress, void* output )
{
#if defined( _X86_ )
	// Unprotect memory.
	if ( !UnprotectMemory( output, 5 ) )
	{
		return false;
	}

	// Write x86 jmp instruction.
	auto ouputBytes                                     = static_cast<std::uint8_t*>( output );
	*ouputBytes                                         = 0xE9;
	*reinterpret_cast<std::uint32_t*>( ouputBytes + 1 ) = ( reinterpret_cast<std::uintptr_t>( jumpAddress ) - ( reinterpret_cast<std::uintptr_t>( output ) + 5 ) );

	return true;
#else
// TODO: Support other architectures.
#  error "Unsupported architecture"
#endif
}

void InstallHook( void*& origFunc, void* hookFunc )
{
	DetourTransactionBegin();
	DetourUpdateThread( GetCurrentThread() );
	DetourAttach( &origFunc, hookFunc );
	DetourTransactionCommit();
}

void* InstallVTableHook( void** vtable, std::size_t virtualFuncIndex, void* hookFunc )
{
	// Get original function pointer.
	void* origFunc = vtable[ virtualFuncIndex ];

	// Set new function pointer and return original function pointer.
	vtable[ virtualFuncIndex ] = hookFunc;
	return origFunc;
}

TOSHI_NAMESPACE_USING

static T2CommandLine g_CommandLine;
const T2CommandLine* g_pCommandLine = &g_CommandLine;

class AModelLoader
{};

class AGameSystemManager
{};

static TBOOL s_bLoadedFrontEnd = TFALSE;

// Spoof AFrontEndMiniGamePCDemo with AFrontEndMainMenuState2
MEMBER_HOOK( 0x0042bd00, AGameStateController, AGameStateController_PushState, void, AGameState* a_pGameState )
{
	if ( !s_bLoadedFrontEnd && a_pGameState->GetClass() == (TClass*)0x00777074 )
	{
		s_bLoadedFrontEnd = TTRUE;

		AGameState* pFrontEndState = CALL( 0x00409620, AGameState* );
		return CallOriginal( pFrontEndState );
	}

	return CallOriginal( a_pGameState );
}

// Demo build doesn't support grass shader, so we must replace it to world shader type, because it's backward compatible
HOOK( 0x0060faa0, AModelLoader_AModelLoaderLoadTRBCallback, TBOOL, TModel* a_pModel )
{
	TTMDWin::TRBWinHeader* pHeader = a_pModel->CastSymbol<TTMDWin::TRBWinHeader>( "Header" );

	for ( TINT i = 0; i < pHeader->m_iNumLODs; i++ )
	{
		auto pTRBLod = pHeader->GetLOD( i );

		switch ( pTRBLod->m_eShader )
		{
			case TTMDBase::SHADERTYPE_GRASS:
				pTRBLod->m_eShader = TTMDBase::SHADERTYPE_WORLD;
				break;
		}
	}

	return CallOriginal( a_pModel );
}

// Bypass demo screen that appears after 1 game day
HOOK( 0x0042c0d0, ShowThanksForPlayingScreen, void )
{
}

// We must create AReplayManager on our own, because someone removed the code that does it from this build
MEMBER_HOOK( 0x004019f0, AGameSystemManager, AGameSystemManager_OnCreate, TBOOL )
{
	CallOriginal();

	// AReplayManager::ms_oClass.CreateObject();
	( (TClass*)( 0x0077ccbc ) )->CreateObject();

	return TTRUE;
}

struct DemoWorldVertex
{
	Toshi::TVector3 Position;
	Toshi::TVector3 Color;
	Toshi::TVector2 UV;
};

static DemoWorldVertex s_WorldVertexBuffer[ 1024 * 8 ];

static void LoadTreeIntersect( CellSphereTreeBranchNode* a_pRenderGroup, Cell*& a_ppModel, TModelLOD* a_pModelLOD )
{
	TRenderD3DInterface::SetSingletonExplicit( *(TRenderD3DInterface**)0x007c753c );

	TVALIDPTR( a_pRenderGroup );

	// Load sub groups
	auto pShader      = *TREINTERPRETCAST( AWorldShader**, 0x00793714 );
	auto pRenderGroup = a_pRenderGroup;
	while ( !pRenderGroup->IsLeaf() )
	{
		LoadTreeIntersect( pRenderGroup->GetSubNode(), a_ppModel, a_pModelLOD );
		pRenderGroup = pRenderGroup->m_pRight;
	}

	TASSERT( pRenderGroup->IsLeaf() );
	auto pRenderData = pRenderGroup->GetLeafNode();

	for ( TUINT i = 0; i < pRenderData->m_uiNumMeshes; i++ )
	{
		auto pTerrainMesh = a_ppModel->ppCellMeshSpheres[ pRenderData->GetMeshIndex( i ) ]->m_pCellMesh;
		auto pMesh        = pShader->CreateMesh( TNULL );

		CALL_THIS( 0x005f4910, AWorldMesh*, void, pMesh, TUINT32, 0, TUINT16, pTerrainMesh->uiNumVertices1 );
		pTerrainMesh->pMesh = pMesh;

		TINT& s_iMeshIndex = *(TINT*)0x007ad4cc;

		pMesh->SetMaterial( CALL( 0x0060e330, TMaterial*, TShader*, pShader, const TCHAR*, pTerrainMesh->szMaterialName ) );
		a_pModelLOD->ppMeshes[ s_iMeshIndex++ ] = pMesh;

		auto pSubMesh             = pMesh->GetSubMesh( 0 );
		pSubMesh->pAssociatedMesh = pMesh;

		// Create index pool
		if ( pSubMesh->pIndexPool == TNULL )
		{
			auto pIndexFactory = TRenderInterface::GetSingleton()->GetSystemResource<TIndexFactoryResourceInterface>( SYSRESOURCE_IFSYS );
			TVALIDPTR( pIndexFactory );

			pSubMesh->pIndexPool = pIndexFactory->CreatePoolResource( pTerrainMesh->uiNumIndices, 9 );
		}

		pSubMesh->uiNumVertices = pTerrainMesh->uiNumVertices2;

		// Write vertices
		if ( pTerrainMesh->uiNumVertices1 != 0 )
		{
			AWorldMesh::LockBuffer lockBuffer;
			TBOOL                  bLocked = pMesh->Lock( lockBuffer );

			if ( bLocked )
			{
				TUINTPTR uiAssumedVertexSize = ( TUINTPTR( pTerrainMesh->pIndices ) - TUINTPTR( pTerrainMesh->pVertices ) ) / pTerrainMesh->uiNumVertices1;

				if ( uiAssumedVertexSize < 44 )
				{
					TUtil::MemCopy( lockBuffer.apStreams[ 0 ], pTerrainMesh->pVertices, 32 * pTerrainMesh->uiNumVertices1 );
				}
				else
				{
					for (TUINT k = 0; k < pTerrainMesh->uiNumVertices1; k++)
					{
						s_WorldVertexBuffer[ k ].Position = pTerrainMesh->pVertices[ k ].Position;
						s_WorldVertexBuffer[ k ].Color    = pTerrainMesh->pVertices[ k ].Color;
						s_WorldVertexBuffer[ k ].UV       = pTerrainMesh->pVertices[ k ].UV;
					}

					TUtil::MemCopy( lockBuffer.apStreams[ 0 ], s_WorldVertexBuffer, 32 * pTerrainMesh->uiNumVertices1 );
				}

				pMesh->Unlock( pTerrainMesh->uiNumVertices1 );
			}
			else
			{
				TASSERT( !"Couldn't lock buffer" );
			}
		}

		// Write indices
		if ( pTerrainMesh->uiNumIndices != 0 )
		{
			TVALIDPTR( pSubMesh->pIndexPool );
			TIndexPoolResourceInterface::LockBuffer lockBuffer;

			pSubMesh->pIndexPool->Lock( &lockBuffer );
			TUtil::MemCopy( lockBuffer.pBuffer, pTerrainMesh->pIndices, pTerrainMesh->uiNumIndices * sizeof( TUINT16 ) );
			pSubMesh->pIndexPool->Unlock( pTerrainMesh->uiNumIndices );
		}
	}
}

HOOK( 0x0060fe20, LoadTreeIntersectHook, void, CellSphereTreeBranchNode* a_pRenderGroup, Cell*& a_ppModel, TModelLOD* a_pModelLOD )
{
	LoadTreeIntersect( a_pRenderGroup, a_ppModel, a_pModelLOD );
}

DWORD APIENTRY DllMain( HMODULE hModule, DWORD reason, LPVOID reserved )
{
	switch ( reason )
	{
		case DLL_PROCESS_ATTACH:
		{
			TMemory::Initialise( 2 * 1024 * 1024, 0 );

			TUtil::TOSHIParams toshiParams;
			toshiParams.szCommandLine = "";
			toshiParams.szLogFileName = "demo-unlocker";
			toshiParams.szLogAppName  = "BYDemoUnlocker";

			TUtil::ToshiCreate( toshiParams );

			g_CommandLine.Create( GetCommandLineA() );

			// Initialise hooks
			InstallHook<AGameStateController_PushState>();
			InstallHook<AModelLoader_AModelLoaderLoadTRBCallback>();
			InstallHook<AGameSystemManager_OnCreate>();
			InstallHook<LoadTreeIntersectHook>();
			InstallHook<ShowThanksForPlayingScreen>();

			return TTRUE;
		}
		default:
			return TFALSE;
	}
}
