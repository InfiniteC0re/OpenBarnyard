#pragma once
#include "ATerrainVIS.h"
#include "Assets/AModelLoaderJob.h"
#include "Assets/AMatLibLoaderJob.h"
#include "Assets/AKeyLibLoaderJob.h"
#include "Assets/ASkeletonDoneJob.h"
#include "Assets/ACollisionDoneJob.h"
#include "Assets/ASectionDoneJob.h"

#include <Toshi/T2SimpleArray.h>

class ATerrain :
	public Toshi::TSingleton<ATerrain>
{
public:
	constexpr static TUINT MAX_NUM_MODEL_LOADER_JOBS = 128;
	constexpr static TUINT MAX_NUM_MATLIB_LOADER_JOBS = 10;
	constexpr static TUINT MAX_NUM_KEYLIB_LOADER_JOBS = 10;
	constexpr static TUINT MAX_NUM_SECTION_LOADER_JOBS = 32;
	constexpr static TUINT MAX_NUM_TRB_LOADER_JOBS = 2;
	constexpr static TUINT MAX_NUM_SKELETON_LOADER_JOBS = 2;
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
	friend class ATerrainManager;

	using t_GetCurrentVISGroup = TINT(*)();
	using t_OnVISGroupChanged = void(*)(ATerrainSection* a_pCurrent, ATerrainSection* a_pPrevious);

public:
	ATerrain(TINT a_iUnused1, TINT a_iUnused2, TINT a_iPreloadTerrainBlockSize, TINT a_iStartVISGroup);
	
	virtual ~ATerrain();

	/* Updates the current state of terrain including loading. */
	void Update();

	/**
	 * @return TTRUE if no any assets are in loading at the current moment.
	 */
	TBOOL IsLoaded() const;

	/**
	 * Loads information about terrain.
	 * @param a_szFilePath path to the terrain info file
	 * @param a_bLoadLater if TFALSE, doesn't pause thread to load all resources
	 * @param a_bPreloadCollision if TFALSE, loads all collision objects at once
	 */
	void LoadFromFile(const TCHAR* a_szFilePath, TBOOL a_bLoadLater, TBOOL a_bStreamCollision);
	
	/* Blocks the thread until terrain is completely loaded. */
	void WaitUntilLoaded();

	/* Destroys terrain model */
	void DestroyModelData(ATerrainSection::ModelData* a_pModelData);

	/**
	 * Sets visible blocks of specified LOD type to be used.
	 * @param a_eLODType type of LODs to load (can be ATerrainLODType_High or ATerrainLODType_Low)
	 */
	void UpdateUsedBlocks(ATerrainLODType a_eLODType);

	/* Queues needed LODs to be loaded based on the current section. */
	void QueueStreamingAssets();

	/* Cancels streaming of LODs in the current section. */
	void UnqueueStreamingAssets();

	/**
	 * @return TTRUE if any LODs are planned to be loaded
	 */
	TBOOL HasAnyLODsQueued();

	/* Cancels jobs that are not needed anymore (can happen after moving to other section). */
	void CancelUnrequiredJobs();
	
	/**
	 * Allocates free LODBlock of specified type and assigns it to the section.
	 * @param a_eLODType type of the block you want to allocate
	 * @param a_pVISGroup section you allocate the block for
	 * @return allocated block
	 */
	ATerrainLODBlock* AllocateLODBlock(ATerrainLODType a_eLODType, ATerrainSection* a_pVISGroup);

	/**
	 * @return TTRUE if collision is not preloaded
	 */
	TBOOL IsCollisionStreamed() const { return m_bStreamCollision; }
	
	/**
	 * @return visibility data of the currently loaded terrain
	 */
	ATerrainVIS* GetVIS() const { return m_pTerrainVIS; }

	ATRBLoaderJob* GetFreeTRBLoaderJob();
	ASkeletonDoneJob* GetFreeSkeletonLoaderJob();
	AMatLibLoaderJob* GetFreeMatlibLoaderJob();
	ACollisionDoneJob* GetFreeCollisionLoaderJob();
	AModelLoaderJob* GetFreeModelLoaderJob();
	ASectionDoneJob* GetFreeSectionLoaderJob();

private:
	void UpdateNightMaterials();
	void MoveAllFinishedJobs(Toshi::T2SList<JobSlot>& a_rFreeJobs, Toshi::T2SList<JobSlot>& a_rUsedJobs);
	
	/* Moves all finished jobs from used lists to free lists */
	void FlushJobs();

private:
	static TINT GetCurrentSectionID();
	static TINT GetPersistantSectionID();

	inline static TINT ms_iPersistantVISGroupIndex = -1;
	inline static TBOOL ms_bAutoVIS = TFALSE;

private:
	Toshi::TVector4 m_Vector1;
	Toshi::TVector4 m_Vector2;
	Toshi::TVector4 m_Vector3;
	Toshi::TVector4 m_Vector4;
	// ...
	Toshi::T2SList<ATerrainSection::ModelData> m_ModelDatas;
	// ...
	t_OnVISGroupChanged m_cbOnVISGroupChanged;
	AModelLoaderJob m_aModelLoaderJobs[MAX_NUM_MODEL_LOADER_JOBS];
	AMatLibLoaderJob m_aMatlibLoaderJobs[MAX_NUM_MATLIB_LOADER_JOBS];
	AKeyLibLoaderJob m_aKeylibLoaderJobs[MAX_NUM_KEYLIB_LOADER_JOBS];
	ASectionDoneJob m_aSectionDoneJobs[MAX_NUM_SECTION_LOADER_JOBS];
	ATRBLoaderJob m_aTRBLoaderJobs[MAX_NUM_TRB_LOADER_JOBS];
	ASkeletonDoneJob m_aSkeletonDoneJobs[MAX_NUM_SKELETON_LOADER_JOBS];
	ACollisionDoneJob m_aCollisionDoneJobs[MAX_NUM_COLLISION_LOADER_JOBS];
	JobSlot* m_pJobs;
	Toshi::T2SList<JobSlot> m_UsedModelLoaderJobs;
	Toshi::T2SList<JobSlot> m_FreeModelLoaderJobs;
	Toshi::T2SList<JobSlot> m_UsedUnknownLoaderJobs;
	Toshi::T2SList<JobSlot> m_FreeUnknownLoaderJobs;
	Toshi::T2SList<JobSlot> m_UsedSectionLoaderJobs;
	Toshi::T2SList<JobSlot> m_FreeSectionLoaderJobs;
	Toshi::T2SList<JobSlot> m_UsedKeylibLoaderJobs;
	Toshi::T2SList<JobSlot> m_FreeKeylibLoaderJobs;
	Toshi::T2SList<JobSlot> m_UsedMatlibLoaderJobs;
	Toshi::T2SList<JobSlot> m_FreeMatlibLoaderJobs;
	Toshi::T2SList<JobSlot> m_UsedTRBLoaderJobs;
	Toshi::T2SList<JobSlot> m_FreeTRBLoaderJobs;
	Toshi::T2SList<JobSlot> m_UsedSkeletonLoaderJobs;
	Toshi::T2SList<JobSlot> m_FreeSkeletonLoaderJobs;
	Toshi::T2SList<JobSlot> m_UsedCollisionLoaderJobs;
	Toshi::T2SList<JobSlot> m_FreeCollisionLoaderJobs;
	TBOOL m_bStreamCollision;
	TBOOL m_bIsLoaded;
	Toshi::TTRB m_VISTRB;
	Toshi::TTRB m_TRB2;
	ATerrainVIS* m_pTerrainVIS;
	ATerrainOrderDVIS* m_pOrderDVIS;
	// ...
	TINT m_iCurrentSection;
	TINT m_iPreviousSection;
	TINT m_iPreloadTerrainBlockSize;
	t_GetCurrentVISGroup m_fnGetCurrentVISGroup;
	// ...
	TINT m_iNumChunks;
};

class ATerrainManager
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
		const TCHAR* szName;
		TUINT32 uiUnk;
	};
	
	inline static constexpr TerrainInfo ms_aTerrains[Terrain_NUMOF] = {
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
	static void SetTerrain(TINT a_eTerrain, TBOOL a_bLoadLater, TBOOL a_bStreamModels, TINT a_iUnused1, TINT a_iUnused2, TINT a_iPreloadTerrainBlockSize, TINT a_iStartVISGroup);
	static void StartLoading();
	
	static constexpr const TCHAR* GetTerrainName(Terrain a_eTerrain) { return ms_aTerrains[a_eTerrain].szName; }


private:
	inline static ATerrain* ms_pCurrentTerrain;
	inline static Terrain ms_eCurrentLevel = Terrain_EnvBeadyFarm;
};
