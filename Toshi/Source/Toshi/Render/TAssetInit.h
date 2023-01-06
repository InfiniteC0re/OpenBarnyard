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

		static void InitAssets(TTRB& a_trb, bool createResources, bool allowCrossTRBReferences);
		static void Init(TTRB& a_trb, uint32_t a_unk, t_fourCCFunction a_fourCCFunc);
		

	public:
		static bool g_bCreateResources;
		static bool g_bAllowCrossTRBReferences;
		static TTRB* g_pCurrentTRB;
	};
}