#pragma once
#include "ATerrainVIS.h"
#include "AModelLoaderJob.h"
#include "AMatLibLoaderJob.h"
#include "AKeyLibLoaderJob.h"
#include "ASkeletonDoneJob.h"
#include "ACollisionDoneJob.h"
#include "ASectionDoneJob.h"
#include "THookedSingleton.h"

class ATerrainInterface :
	public THookedSingleton<ATerrainInterface, 0x00796300>
{
public:
	constexpr static TUINT MAX_NUM_MODEL_LOADER_JOBS	 = 128;
	constexpr static TUINT MAX_NUM_MATLIB_LOADER_JOBS	 = 10;
	constexpr static TUINT MAX_NUM_KEYLIB_LOADER_JOBS	 = 10;
	constexpr static TUINT MAX_NUM_SECTION_LOADER_JOBS	 = 32;
	constexpr static TUINT MAX_NUM_TRB_LOADER_JOBS		 = 2;
	constexpr static TUINT MAX_NUM_SKELETON_LOADER_JOBS	 = 2;
	constexpr static TUINT MAX_NUM_COLLISION_LOADER_JOBS = 64;
	constexpr static TUINT NUM_TOTAL_JOBS =
		MAX_NUM_MODEL_LOADER_JOBS +
		MAX_NUM_MATLIB_LOADER_JOBS +
		MAX_NUM_KEYLIB_LOADER_JOBS +
		MAX_NUM_SECTION_LOADER_JOBS +
		MAX_NUM_TRB_LOADER_JOBS +
		MAX_NUM_SKELETON_LOADER_JOBS +
		MAX_NUM_COLLISION_LOADER_JOBS;

	struct JobSlot :
		public Toshi::T2SList<JobSlot>::Node
	{
		AMainThreadJob* pJob = TNULL;
	};

	friend ATerrainVIS;
	friend class ABYardTerrainManager;

	using t_GetCurrentVISGroup = TINT ( * )();
	using t_OnVISGroupChanged  = void ( * )( ATerrainSection* a_pCurrent, ATerrainSection* a_pPrevious );

public:
	virtual ~ATerrainInterface() = default;

	void Update();

	TBOOL IsLoaded() const;

	void LoadFromFile( const char* a_szFilePath, TBOOL a_bLoadLater, TBOOL a_bPersistantCollision );

	void WaitUntilLoaded();

	void DestroyModelData( ATerrainSection::ModelNode* a_pModelData );

	void UpdateUsedBlocks( ATerrainLODType a_eLODType );

	void QueueStreamingAssets();

	void UnqueueStreamingAssets();

	TBOOL HasAnyLODsQueued();

	void CancelUnrequiredJobs();

	ATerrainLODBlock* AllocateLODBlock( ATerrainLODType a_eLODType, ATerrainSection* a_pVISGroup );

	TBOOL IsCollisionStreamed() const { return m_bStreamCollision; }

	ATerrainVIS* GetVIS() const { return m_pTerrainVIS; }

	void FlushJobs();

	ATRBLoaderJob* GetFreeTRBLoaderJob()
	{
		TASSERT( m_FreeTRBLoaderJobs.Size() > 0, "No free ATRBLoaderJobs left!" );

		if ( m_FreeTRBLoaderJobs.Size() > 0 )
		{
			auto pTRBJobSlot = m_FreeTRBLoaderJobs.PopFront();
			m_UsedTRBLoaderJobs.PushFront( pTRBJobSlot );

			return TSTATICCAST( ATRBLoaderJob, pTRBJobSlot->pJob );
		}
		else
		{
			return TNULL;
		}
	}

	ASkeletonDoneJob* GetFreeSkeletonLoaderJob()
	{
		TASSERT( m_FreeSkeletonLoaderJobs.Size() > 0, "No free ASkeletonDoneJobs left!" );

		if ( m_FreeSkeletonLoaderJobs.Size() > 0 )
		{
			auto pTRBJobSlot = m_FreeSkeletonLoaderJobs.PopFront();
			m_UsedSkeletonLoaderJobs.PushFront( pTRBJobSlot );

			return TSTATICCAST( ASkeletonDoneJob, pTRBJobSlot->pJob );
		}
		else
		{
			return TNULL;
		}
	}

	AMatLibLoaderJob* GetFreeMatlibLoaderJob()
	{
		TASSERT( m_FreeMatlibLoaderJobs.Size() > 0, "No free AMatlibLoaderJobs left!" );

		if ( m_FreeMatlibLoaderJobs.Size() > 0 )
		{
			auto pTRBJobSlot = m_FreeMatlibLoaderJobs.PopFront();
			m_UsedMatlibLoaderJobs.PushFront( pTRBJobSlot );

			return TSTATICCAST( AMatLibLoaderJob, pTRBJobSlot->pJob );
		}
		else
		{
			return TNULL;
		}
	}

	ACollisionDoneJob* GetFreeCollisionLoaderJob()
	{
		TASSERT( m_FreeCollisionLoaderJobs.Size() > 0, "No free ACollisionDoneJob left!" );

		if ( m_FreeCollisionLoaderJobs.Size() > 0 )
		{
			auto pTRBJobSlot = m_FreeCollisionLoaderJobs.PopFront();
			m_UsedCollisionLoaderJobs.PushFront( pTRBJobSlot );

			return TSTATICCAST( ACollisionDoneJob, pTRBJobSlot->pJob );
		}
		else
		{
			return TNULL;
		}
	}

	AModelLoaderJob* GetFreeModelLoaderJob()
	{
		TASSERT( m_FreeModelLoaderJobs.Size() > 0, "No free AModelLoaderJob left!" );

		if ( m_FreeModelLoaderJobs.Size() > 0 )
		{
			auto pTRBJobSlot = m_FreeModelLoaderJobs.PopFront();
			m_UsedModelLoaderJobs.PushFront( pTRBJobSlot );

			return TSTATICCAST( AModelLoaderJob, pTRBJobSlot->pJob );
		}
		else
		{
			return TNULL;
		}
	}

	ASectionDoneJob* GetFreeSectionLoaderJob()
	{
		TASSERT( m_FreeSectionLoaderJobs.Size() > 0, "No free ASectionDoneJob left!" );

		if ( m_FreeSectionLoaderJobs.Size() > 0 )
		{
			auto pTRBJobSlot = m_FreeSectionLoaderJobs.PopFront();
			m_UsedSectionLoaderJobs.PushFront( pTRBJobSlot );

			return TSTATICCAST( ASectionDoneJob, pTRBJobSlot->pJob );
		}
		else
		{
			return TNULL;
		}
	}

private:
	void UpdateNightMaterials();
	void MoveAllFinishedJobs( Toshi::T2SList<JobSlot>& a_rFreeJobs, Toshi::T2SList<JobSlot>& a_rUsedJobs );

private:
	static TINT GetCurrentVISGroupIndex();
	static TINT GetPersistantVISGroupIndex();

	inline static TINT	ms_iPersistantVISGroupIndex = -1;
	inline static TBOOL ms_bAutoVIS					= TFALSE;

public:
	Toshi::TVector4							   m_LitShadowColor;
	Toshi::TVector4							   m_LitAmbientColor;
	Toshi::TVector4							   m_DefaultShadowColor;
	Toshi::TVector4							   m_DefaultAmbientColor;
	char									   PAD1[ 8 ];
	Toshi::T2SList<ATerrainSection::ModelNode> m_ModelDatas;
	char									   PAD2[ 8 ];
	t_OnVISGroupChanged						   m_cbOnVISGroupChanged;
	AModelLoaderJob							   m_aModelLoaderJobs[ MAX_NUM_MODEL_LOADER_JOBS ];
	AMatLibLoaderJob						   m_aMatlibLoaderJobs[ MAX_NUM_MATLIB_LOADER_JOBS ];
	AKeyLibLoaderJob						   m_aKeylibLoaderJobs[ MAX_NUM_KEYLIB_LOADER_JOBS ];
	ASectionDoneJob							   m_aSectionDoneJobs[ MAX_NUM_SECTION_LOADER_JOBS ];
	ATRBLoaderJob							   m_aTRBLoaderJobs[ MAX_NUM_TRB_LOADER_JOBS ];
	ASkeletonDoneJob						   m_aSkeletonDoneJobs[ MAX_NUM_SKELETON_LOADER_JOBS ];
	ACollisionDoneJob						   m_aCollisionDoneJobs[ MAX_NUM_COLLISION_LOADER_JOBS ];
	JobSlot*								   m_pJobs;
	Toshi::T2SList<JobSlot>					   m_UsedModelLoaderJobs;
	Toshi::T2SList<JobSlot>					   m_FreeModelLoaderJobs;
	Toshi::T2SList<JobSlot>					   m_UsedUnknownLoaderJobs;
	Toshi::T2SList<JobSlot>					   m_FreeUnknownLoaderJobs;
	Toshi::T2SList<JobSlot>					   m_UsedSectionLoaderJobs;
	Toshi::T2SList<JobSlot>					   m_FreeSectionLoaderJobs;
	Toshi::T2SList<JobSlot>					   m_UsedKeylibLoaderJobs;
	Toshi::T2SList<JobSlot>					   m_FreeKeylibLoaderJobs;
	Toshi::T2SList<JobSlot>					   m_UsedMatlibLoaderJobs;
	Toshi::T2SList<JobSlot>					   m_FreeMatlibLoaderJobs;
	Toshi::T2SList<JobSlot>					   m_UsedTRBLoaderJobs;
	Toshi::T2SList<JobSlot>					   m_FreeTRBLoaderJobs;
	Toshi::T2SList<JobSlot>					   m_UsedSkeletonLoaderJobs;
	Toshi::T2SList<JobSlot>					   m_FreeSkeletonLoaderJobs;
	Toshi::T2SList<JobSlot>					   m_UsedCollisionLoaderJobs;
	Toshi::T2SList<JobSlot>					   m_FreeCollisionLoaderJobs;
	TBOOL									   m_bStreamCollision;
	TBOOL									   m_bIsLoaded;
	Toshi::TTRB								   m_VISTRB;
	Toshi::TTRB								   m_TRB2;
	ATerrainVIS*							   m_pTerrainVIS;
	ATerrainOrderDVIS*						   m_pOrderDVIS;
	char									   PAD3[ 100 ];
	TINT									   m_iCurrentGroup;
	TINT									   m_iPreviousGroup;
	TINT									   m_iPreloadTerrainBlockSize;
	t_GetCurrentVISGroup					   m_fnGetCurrentVISGroup;
	char									   PAD4[ 4 ];
	TINT									   m_iNumChunks;
};

class ABYardTerrainManager
{
public:
	using Terrain = TUINT32;
	enum Terrain_ : Terrain
	{
		Terrain_EnvMain,
		Terrain_GateCrash,
		Terrain_EnvBarnInt,
		Terrain_Golf1,
		Terrain_Golf2,
		Terrain_Golf3,
		Terrain_Golf4,
		Terrain_Golf5,
		Terrain_Golf6,
		Terrain_Golf7,
		Terrain_Golf8,
		Terrain_Golf9,
		Terrain_Golf10,
		Terrain_Golf11,
		Terrain_Golf12,
		Terrain_Golf13,
		Terrain_Golf14,
		Terrain_Golf15,
		Terrain_Golf16,
		Terrain_Golf17,
		Terrain_Golf18,
		Terrain_PeckRange,
		Terrain_FrontEnd,
		Terrain_Statue,
		Terrain_EnvBeadyInt,
		Terrain_EnvWoods,
		Terrain_EnvBarnIntDay,
		Terrain_EnvBlackMarket,
		Terrain_CowTipping,
		Terrain_VeggieDef,
		Terrain_EnvPoolTable,
		Terrain_EnvPoolBoat,
		Terrain_EnvWacARac,
		Terrain_Tappers,
		Terrain_EnvCarRace,
		Terrain_Kitchen,
		Terrain_Cocktails,
		Terrain_ChasingChicks2,
		Terrain_EnvBensHill,
		Terrain_EnvBeadyFarm,
		Terrain_EnvCoopInt,
		Terrain_EnvFarmerInt,
		Terrain_EnvDartsLarge,
		Terrain_EnvSSDarts,
		Terrain_test1,
		Terrain_ss01,
		Terrain_ss02,
		Terrain_ss03,
		Terrain_ss04,
		Terrain_ss05,
		Terrain_ss06,
		Terrain_ss07a,
		Terrain_ss07b,
		Terrain_ss08,
		Terrain_ss09,
		Terrain_ss10,
		Terrain_ss11,
		Terrain_NUMOF
	};

	struct TerrainInfo
	{
		const char* szName;
		TUINT32		uiUnk;
	};

	inline static constexpr TerrainInfo ms_aTerrains[ Terrain_NUMOF ] = {
		{ "EnvMain", 0x400000 },
		{ "gatecrash", 0x500000 },
		{ "envbarnint", 0x400000 },
		{ "golf1", 0x600000 },
		{ "golf2", 0x600000 },
		{ "golf3", 0x600000 },
		{ "golf4", 0x600000 },
		{ "golf5", 0x600000 },
		{ "golf6", 0x600000 },
		{ "golf7", 0x600000 },
		{ "golf8", 0x600000 },
		{ "golf9", 0x600000 },
		{ "golf10", 0x600000 },
		{ "golf11", 0x600000 },
		{ "golf12", 0x600000 },
		{ "golf13", 0x600000 },
		{ "golf14", 0x600000 },
		{ "golf15", 0x600000 },
		{ "golf16", 0x600000 },
		{ "golf17", 0x600000 },
		{ "golf18", 0x600000 },
		{ "peckrange", 0x900000 },
		{ "frontend", 0x300000 },
		{ "statue", 0x400000 },
		{ "env_beadyint", 0x400000 },
		{ "envwoods", 0x800000 },
		{ "envbarnintday", 0x400000 },
		{ "envblackmarket", 0x100000 },
		{ "cowtipping", 0x600000 },
		{ "veggiedef", 0x600000 },
		{ "EnvPoolTable", 0x400000 },
		{ "envpoolboat", 0x400000 },
		{ "EnvWacARac", 0x100000 },
		{ "tappers", 0x500000 },
		{ "envcarrace", 0xB00000 },
		{ "kitchen", 0x400000 },
		{ "cocktail", 0x400000 },
		{ "chasingchicks2", 0x800000 },
		{ "envbenshill", 0x800000 },
		{ "envbeadyfarm", 0x800000 },
		{ "envcoopint", 0x400000 },
		{ "envfarmerint", 0x400000 },
		{ "envdarts_large", 0x400000 },
		{ "EnvSSDarts", 0x400000 },
		{ "test1", 0x400000 },
		{ "ss01", 0x400000 },
		{ "ss02", 0x400000 },
		{ "ss03", 0x400000 },
		{ "ss04", 0x400000 },
		{ "ss05", 0x400000 },
		{ "ss06", 0x400000 },
		{ "ss07a", 0x400000 },
		{ "ss07b", 0x400000 },
		{ "ss08", 0x400000 },
		{ "ss09", 0x400000 },
		{ "ss11", 0x400000 },
	};

public:
	static void					 SetTerrain( TINT a_eTerrain, TBOOL a_bLoadLater, TBOOL a_bStreamModels, TINT a_iUnused1, TINT a_iUnused2, TINT a_iPreloadTerrainBlockSize, TINT a_iStartVISGroup );
	static constexpr const char* GetTerrainName( Terrain a_eTerrain ) { return ms_aTerrains[ a_eTerrain ].szName; }

	static void StartLoading();

public:
	inline static ATerrainInterface* ms_pCurrentTerrain;
	inline static Terrain*			 ms_pCurrentLevel = TREINTERPRETCAST( Terrain*, 0x00772800 );
	inline static Terrain&			 ms_eCurrentLevel = *ms_pCurrentLevel;
};
