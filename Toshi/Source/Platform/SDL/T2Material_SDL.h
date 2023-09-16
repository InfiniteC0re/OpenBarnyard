#pragma once
#include "Toshi2/T2ResourceManager.h"

namespace Toshi {

	class TTexture;

	class T2Material : public T2Resource
	{
	public:
		typedef enum {
			Flags_InMaterialManager = BITFIELD(0),
		} Flags;

		struct Texture
		{
			TTexture* m_pTexture;
			const char* m_pName;
		};

		struct Pass
		{
			uint32_t m_uiUnk1;
			uint32_t m_uiBlendMode;
			uint32_t m_uiUnk3;
			Texture* m_pTexture;
			uint32_t m_uiUnk4;
			uint32_t m_uiUnk5;
			uint32_t m_uiUnk6;
			uint32_t m_uiUnk7;
			uint32_t m_uiUnk8;
			uint32_t m_uiUnk9;
			uint32_t m_uiUnk10;
		};

		using t_TRBLoadCallback = void(*)(T2Material* pMetaMat, void* pData);
		using t_TRBUnloadCallback = void(*)(T2Material* pMetaMat, void* pData);

		friend class TMaterialManager;

	public:
		void Init();
		void Deinit();

		TTexture* GetTexture(int a_iId) const
		{
			TASSERT(a_iId < m_iNumTex);
			return m_aTextures[a_iId].m_pTexture;
		}

		const char* GetTextureName(int a_iId) const
		{
			TASSERT(a_iId < m_iNumTex);
			return m_aTextures[a_iId].m_pName;
		}

		Pass* GetPassInfo(int a_iPass) const
		{
			TASSERT(a_iPass < m_iNumPasses);
			return &m_aPasses[a_iPass];
		}

		uint32_t GetPassBlendMode(int a_iPass) const
		{
			TASSERT(a_iPass < m_iNumPasses);
			return m_aPasses[a_iPass].m_uiBlendMode;
		}

		TTexture* GetPassTexture(int a_iPass) const
		{
			TASSERT(a_iPass < m_iNumPasses);
			return m_aPasses[a_iPass].m_pTexture->m_pTexture;
		}

		const char* GetPassTextureName(int a_iPass) const
		{
			TASSERT(a_iPass < m_iNumPasses);
			return m_aPasses[a_iPass].m_pTexture->m_pName;
		}

		const char* GetMaterialName() const
		{
			return m_pMaterialName;
		}

		int GetNumTextures() const
		{
			return m_iNumTex;
		}

		uint32_t GetNumPasses() const
		{
			return m_iNumPasses;
		}

		TBOOL HasFlag(Flags flag)
		{
			return HASFLAG(m_uiFlags & flag);
		}

	private:
		void InitData();

		static void* ResourceCallback(void* pCustomData, TTRB* trb, TBOOL bCreated);

	private:
		inline static t_TRBLoadCallback sm_pTRBLoadCallback = TNULL;
		inline static t_TRBUnloadCallback sm_pTRBUnloadCallback = TNULL;
		inline static void* sm_pTRBLoadCallbackData = TNULL;
		inline static void* sm_pTRBUnloadCallbackData = TNULL;

	private:
		uint32_t m_uiUnk1;
		uint32_t m_uiFlags;
		const char* m_pMaterialName;
		int32_t m_iNumTex;
		Texture* m_aTextures;
		uint32_t m_uiUnk2;
		uint32_t m_uiUnk3;
		Pass* m_aPasses;
		int32_t m_iNumPasses;
		uint32_t m_uiUnk4;
		T2Material* m_pNext;
		T2Material* m_pPrev;
	};

	DEFINE_T2FLAGS(T2Material::Flags);

	class TMaterialManager : public TSingleton<TMaterialManager>
	{
	public:
		TMaterialManager() = default;

		void AddMaterial(T2Material* a_pMat);
		void RemoveMaterial(T2Material* a_pMat);

		T2Material* FindMaterial(const char* a_pName) const;

	private:
		T2Material* m_pHeadMaterial = TNULL;
	};

}
