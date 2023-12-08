#pragma once
#include "TTMDBase.h"
#include "TSkeleton.h"
#include "TModelCollision.h"
#include "TMesh.h"

#include "Toshi/Math/TSphere.h"
#include "Toshi/File/TTRB.h"

class AModelLoader;

namespace Toshi {

	struct TModelLOD
	{
		TSphere BoundingSphere = TSphere(0.0f, 0.0f, 0.0f, 0.0f);
		TINT iNumMeshes = 0;
		TMesh** ppMeshes = TNULL;
	};

	struct TTMD;
	class TModel;

	class TModelInstance
	{
	public:
		friend class TModel;

		using t_PreRenderCB = void(*)(TModelInstance* a_pInstance);
		using t_CustomRenderCB = void(*)(TModelInstance* a_pInstance, void* a_pUserData);

	public:
		TModelInstance(TModel* a_pModel);

		void Update(TFLOAT a_fDeltaTime)
		{
			m_pSkeletonInstance->UpdateTime(a_fDeltaTime);
		}

		void Render();

		void SetPreRenderCallback(t_PreRenderCB a_fnCallback)
		{
			m_fnPreRenderCb = a_fnCallback;
		}

		void SetCustomRenderMethod(t_CustomRenderCB a_fnCallback, void* a_pUserData)
		{
			m_fnCustomRenderCb = a_fnCallback;
			m_pCustomRenderCbUserData = a_pUserData;
		}

		void Delete();

	private:
		TModel* m_pModel;
		TSkeletonInstance* m_pSkeletonInstance;
		t_PreRenderCB m_fnPreRenderCb;
		t_CustomRenderCB m_fnCustomRenderCb;
		void* m_pCustomRenderCbUserData;
		TUINT32 m_iCurrentLOD;
	};

	class TModel
	{
	public:
		using Flags = TUINT32;
		enum Flags_ : Flags
		{
			Flags_None = 0,
			Flags_Created = BITFIELD(0),
			Flags_Loaded = BITFIELD(1),
		};

		using t_ModelLoaderTRBCallback = TBOOL(*)(TModel* a_pModel);
		using t_ModelLoaderTMDCallback = TBOOL(*)(TModel* a_pModel);

		static constexpr TUINT MAX_NUM_LODS = 5;

		friend TModelInstance;

	public:
		TModel();

		virtual TBOOL Create(TTMD* a_pTMD, TBOOL a_bLoad) = 0;
		virtual TBOOL Create(const char* a_szFileName, TBOOL a_bLoad) = 0;
		virtual void Delete() = 0;
		virtual TBOOL LoadFromFile(const char* a_szFileName) = 0;
		virtual void Unload() = 0;
		virtual ~TModel() = default;
		virtual TBOOL GetUnknownFlag() = 0;

		TBOOL LoadTRB();
		TBOOL LoadTRB(TTRB* a_pTRB);
		TBOOL LoadTRB(const char* a_szFileName, TTRB* a_pAssetTRB, TUINT8 a_ui8FileNameLen);
		void UnloadTRB();

		void* GetSymbol(const char* a_szSymbolName);

		TModelInstance* CreateInstance();

		TBOOL IsCreated() const { return m_eFlags & Flags_Created; }
		TBOOL IsLoaded() const { return m_eFlags & Flags_Loaded; }

	public:
		static TBOOL GetSkeletonAssetSymbolName(const char* a_szFileName, const char*& a_rSymbolName, TUINT8& a_rNameLen, TTRB* a_pTRB);
		
		static void SetLoaderTMDCallback(t_ModelLoaderTMDCallback a_cbLoader) { ms_cbModelLoaderTMD = a_cbLoader; }
		static void SetLoaderTRBCallback(t_ModelLoaderTRBCallback a_cbLoader) { ms_cbModelLoaderTRB = a_cbLoader; }

	private:
		inline static t_ModelLoaderTMDCallback ms_cbModelLoaderTMD;
		inline static t_ModelLoaderTRBCallback ms_cbModelLoaderTRB;

	protected:
		Flags m_eFlags;
		TINT m_iNumInstances;
		TINT m_iLODCount;
		TFLOAT m_fLODDistance; // ?
		TSkeleton* m_pSkeleton;
		TModelLOD m_LODs[MAX_NUM_LODS];
		TFLOAT m_aUnk1[4];
		void* m_pCollision;
		void* m_pCollisionData;
		TTRB* m_pTRB;
		const char* m_szSymbolPrefix;
		TUINT8 m_szSymbolPrefixLength;
		TBOOL m_bIsAssetFile;
	};
	
}