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
			const char name[4];
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
			{ { 't', 't', 'e', 'x'}, InitTex},
			{ { 't', 'f', 'x', 'l'}, InitFXLite},
			{ { 't', 'm', 'a', 't'}, ReloadMaterial},
			{ { 't', 'k', 'e', 'y'}, InitKeyframes},
			{ { 't', 'm', 'o', 'd'}, InitModel}
		};

		static FourCCFunction constexpr g_FourCCInitFunctions[12] = {
			{ { 't', 't', 'e', 'x'}, InitTex},
			{ { 't', 'v', 'd', 'c'}, InitVertexDecleration},
			{ { 't', 'f', 'x', 'l'}, InitFXLite},
			{ { 't', 'm', 'a', 't'}, InitMaterial},
			{ { 't', 'k', 'e', 'y'}, InitKeyframes},
			{ { 't', 'm', 'o', 'd'}, InitModel },
			{ { 't', 'x', 'u', 'i'}, InitXUI },
			{ { 't', 'f', 'n', 't'}, InitFont },
			{ { 't', 'w', 'l', 'd'}, InitWorld },
			{ { 't', 'p', 'm', 'a'}, InitParticleMaterial },
			{ { 't', 'p', 's', 'e'}, InitParticleSettings },
			{ { 't', 'p', 'e', 'f'}, InitParticleEffect },
		};

		static FourCCFunction constexpr g_FourCCDeinitFunctions[12] = {
			{ { 't', 'p', 'e', 'f'}, DeinitParticleEffect },
			{ { 't', 'p', 's', 'e'}, DeinitParticleSettings },
			{ { 't', 'p', 'm', 'a'}, DeinitParticleMaterial },
			{ { 't', 'w', 'l', 'd'}, DeinitWorld },
			{ { 't', 'f', 'n', 't'}, DeinitFont },
			{ { 't', 'x', 'u', 'i'}, DeinitXUI },
			{ { 't', 'm', 'o', 'd'}, DeinitModel },
			{ { 't', 'k', 'e', 'y'}, DeinitKeyframes},
			{ { 't', 'm', 'a', 't'}, DeinitMaterial},
			{ { 't', 'f', 'x', 'l'}, DeinitFXLite},
			{ { 't', 'v', 'd', 'c'}, DeinitVertexDecleration},
			{ { 't', 't', 'e', 'x'}, DeinitTex},
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