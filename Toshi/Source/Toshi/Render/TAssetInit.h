#pragma once
#include "Toshi/File/TTRB.h"
#include "Toshi/Xui/TXUI.h"

namespace Toshi
{
	class TAssetInit
	{
		

	public:

		using t_fourCCFunction = void (*)(void* a_pData);

		struct FourCCFunction
		{
			uint32_t name;
			t_fourCCFunction func;
		};

		static t_fourCCFunction constexpr InitTex = [](void* a_pData)
		{

		};

		static t_fourCCFunction constexpr InitFXLite = [](void* a_pData)
		{

		};

		static t_fourCCFunction constexpr InitMaterial = [](void* a_pData)
		{

		};

		static t_fourCCFunction constexpr ReloadMaterial = [](void* a_pData)
		{

		};

		static t_fourCCFunction constexpr InitKeyframes = [](void* a_pData)
		{

		};

		static t_fourCCFunction constexpr InitModel = [](void* a_pData)
		{

		};

		// Empty ( de blob only calls a func where TASSERT("TFALSE","..\\..\\Source\\Render\\TVertexDecl_DX11.cpp",0x1f,"TASSERT"); gets called
		static t_fourCCFunction constexpr InitVertexDecleration = [](void* a_pData)
		{

		};

		static t_fourCCFunction constexpr InitXUI = [](void* a_pData)
		{
			reinterpret_cast<TXUIResourceTRB*>(a_pData)->Init();
		};

		static t_fourCCFunction constexpr InitFont = [](void* a_pData)
		{

		};

		static t_fourCCFunction constexpr InitWorld = [](void* a_pData)
		{

		};

		static t_fourCCFunction constexpr InitParticleMaterial = [](void* a_pData)
		{

		};

		static t_fourCCFunction constexpr InitParticleSettings = [](void* a_pData)
		{

		};

		static t_fourCCFunction constexpr InitParticleEffect = [](void* a_pData)
		{

		};

		static t_fourCCFunction constexpr DeinitParticleEffect = [](void* a_pData)
		{

		};

		static t_fourCCFunction constexpr DeinitParticleSettings = [](void* a_pData)
		{

		};

		static t_fourCCFunction constexpr DeinitParticleMaterial = [](void* a_pData)
		{

		};

		static t_fourCCFunction constexpr DeinitWorld = [](void* a_pData)
		{

		};

		static t_fourCCFunction constexpr DeinitFont = [](void* a_pData)
		{

		};

		static t_fourCCFunction constexpr DeinitXUI = [](void* a_pData)
		{

		};

		static t_fourCCFunction constexpr DeinitModel = [](void* a_pData)
		{

		};

		static t_fourCCFunction constexpr DeinitKeyframes = [](void* a_pData)
		{

		};

		static t_fourCCFunction constexpr DeinitMaterial = [](void* a_pData)
		{

		};

		static t_fourCCFunction constexpr DeinitFXLite = [](void* a_pData)
		{

		};

		static t_fourCCFunction constexpr DeinitVertexDecleration = [](void* a_pData)
		{

		};

		static t_fourCCFunction constexpr DeinitTex = [](void* a_pData)
		{

		};

		static FourCCFunction constexpr g_FourCCReloadFunctions[5] = {
			{ TMAKEFOUR("ttex"), InitTex},
			{ TMAKEFOUR("tfxl"), InitFXLite},
			{ TMAKEFOUR("tmat"), ReloadMaterial},
			{ TMAKEFOUR("tkey"), InitKeyframes},
			{ TMAKEFOUR("tmod"), InitModel}
		};

		static FourCCFunction constexpr g_FourCCInitFunctions[12] = {
			{ TMAKEFOUR("ttex"), InitTex},
			{ TMAKEFOUR("tvdc"), InitVertexDecleration},
			{ TMAKEFOUR("tfxl"), InitFXLite},
			{ TMAKEFOUR("tmat"), InitMaterial},
			{ TMAKEFOUR("tkey"), InitKeyframes},
			{ TMAKEFOUR("tmod"), InitModel },
			{ TMAKEFOUR("txui"), InitXUI },
			{ TMAKEFOUR("tfnt"), InitFont },
			{ TMAKEFOUR("twld"), InitWorld },
			{ TMAKEFOUR("tpma"), InitParticleMaterial },
			{ TMAKEFOUR("tpse"), InitParticleSettings },
			{ TMAKEFOUR("tpef"), InitParticleEffect },
		};

		static FourCCFunction constexpr g_FourCCDeinitFunctions[12] = {
			{ TMAKEFOUR("tpef"), DeinitParticleEffect },
			{ TMAKEFOUR("tpse"), DeinitParticleSettings },
			{ TMAKEFOUR("tpma"), DeinitParticleMaterial },
			{ TMAKEFOUR("twld"), DeinitWorld },
			{ TMAKEFOUR("tfnt"), DeinitFont },
			{ TMAKEFOUR("txui"), DeinitXUI },
			{ TMAKEFOUR("tmod"), DeinitModel },
			{ TMAKEFOUR("tkey"), DeinitKeyframes},
			{ TMAKEFOUR("tmat"), DeinitMaterial},
			{ TMAKEFOUR("tfxl"), DeinitFXLite},
			{ TMAKEFOUR("tvdc"), DeinitVertexDecleration},
			{ TMAKEFOUR("ttex"), DeinitTex},
		};

	public:
		static void InitAssets(TTRB& a_trb, bool createResources, bool allowCrossTRBReferences);
		static void DeinitAssets(TTRB& a_trb);
		static void Init(TTRB& a_trb, uint32_t a_unk, t_fourCCFunction a_fourCCFunc);
		

	public:
		static bool g_bCreateResources;
		static bool g_bAllowCrossTRBReferences;
		static TTRB* g_pCurrentTRB;
		static TMemoryHeap* g_pMemHeap;
	};
}