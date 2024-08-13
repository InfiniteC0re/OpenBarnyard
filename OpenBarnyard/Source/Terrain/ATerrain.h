#pragma once
#include "ATerrainVIS.h"
#include "Assets/AModelLoaderJob.h"
#include "Assets/AMatLibLoaderJob.h"
#include "Assets/AKeyLibLoaderJob.h"
#include "Assets/ASkeletonDoneJob.h"
#include "Assets/ACollisionDoneJob.h"
#include "Assets/ASectionDoneJob.h"

class ATerrainInterface :
	public Toshi::TSingleton<ATerrainInterface>
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
	using t_OnModelNodeReady   = void ( * )( ATerrainSection::ModelNode* a_pModelNode );

public:
	ATerrainInterface( TINT a_iUnused1, TINT a_iUnused2, TINT a_iPreloadTerrainBlockSize, TINT a_iStartVISGroup );
	virtual ~ATerrainInterface();

	// Updates the current state of terrain including loading
	void Update();

	// Renders current section
	void Render();

	// Return TTRUE if no any assets are in loading at the current moment
	TBOOL IsLoaded() const;

	/**
	 * Loads information about terrain.
	 * @param a_szFilePath path to the terrain info file
	 * @param a_bLoadLater if TFALSE, doesn't pause thread to load all resources
	 * @param a_bPreloadCollision if TFALSE, loads all collision objects at once
	 */
	void LoadFromFile( const TCHAR* a_szFilePath, TBOOL a_bLoadLater, TBOOL a_bStreamCollision );

	/* Blocks the thread until terrain is completely loaded. */
	void WaitUntilLoaded();

	/* Destroys terrain model */
	void DestroyModelData( ATerrainSection::ModelNode* a_pModelData );

	/**
	 * Sets visible blocks of specified LOD type to be used.
	 * @param a_eLODType type of LODs to load (can be ATerrainLODType_High or ATerrainLODType_Low)
	 */
	void UpdateUsedBlocks( ATerrainLODType a_eLODType );

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

	// Creates a model and setups rendering methods for it
	ATerrainSection::ModelNode* CreateModelInstance( ATerrainSection::ModelNode* a_pModelNode, const char* a_szModelName, const char* a_szType );

	/**
	 * Allocates free LODBlock of specified type and assigns it to the section.
	 * @param a_eLODType type of the block you want to allocate
	 * @param a_pVISGroup section you allocate the block for
	 * @return allocated block
	 */
	ATerrainLODBlock* AllocateLODBlock( ATerrainLODType a_eLODType, ATerrainSection* a_pVISGroup );

	/**
	 * @return TTRUE if collision is not preloaded
	 */
	TBOOL IsCollisionStreamed() const { return m_bStreamCollision; }

	/**
	 * @return visibility data of the currently loaded terrain
	 */
	ATerrainVIS* GetVIS() const { return m_pTerrainVIS; }

	ATRBLoaderJob*	   GetFreeTRBLoaderJob();
	ASkeletonDoneJob*  GetFreeSkeletonLoaderJob();
	AMatLibLoaderJob*  GetFreeMatlibLoaderJob();
	ACollisionDoneJob* GetFreeCollisionLoaderJob();
	AModelLoaderJob*   GetFreeModelLoaderJob();
	ASectionDoneJob*   GetFreeSectionLoaderJob();

	const Toshi::TVector4& GetDefaultShadowColor() const { return m_DefaultShadowColor; }
	const Toshi::TVector4& GetDefaultAmbientColor() const { return m_DefaultAmbientColor; }
	const Toshi::TVector4& GetLitShadowColor() const { return m_LitShadowColor; }
	const Toshi::TVector4& GetLitAmbientColor() const { return m_LitAmbientColor; }

	t_OnModelNodeReady GetOnModelNodeReadyCallback() const { return m_cbOnModelLoaded; }

private:
	void UpdateNightMaterials();
	void MoveAllFinishedJobs( Toshi::T2SList<JobSlot>& a_rFreeJobs, Toshi::T2SList<JobSlot>& a_rUsedJobs );

	/* Moves all finished jobs from used lists to free lists */
	void FlushJobs();

private:
	static TINT GetCurrentSectionID();
	static TINT GetPersistantSectionID();

	inline static TINT	ms_iPersistantVISGroupIndex = -1;
	inline static TBOOL ms_bAutoVIS					= TFALSE;

private:
	Toshi::TVector4							   m_LitShadowColor;
	Toshi::TVector4							   m_LitAmbientColor;
	Toshi::TVector4							   m_DefaultShadowColor;
	Toshi::TVector4							   m_DefaultAmbientColor;
	TFLOAT									   m_fUnused3;
	TBOOL									   m_bUnused4;
	Toshi::T2SList<ATerrainSection::ModelNode> m_ModelDatas;
	t_OnModelNodeReady						   m_cbOnCollsionModelLoaded;
	t_OnModelNodeReady						   m_cbOnModelLoaded;
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
	// ...
	TINT				 m_iCurrentSection;
	TINT				 m_iPreviousSection;
	TINT				 m_iPreloadTerrainBlockSize;
	t_GetCurrentVISGroup m_fnGetCurrentVISGroup;
	// ...
	TINT m_iNumChunks;
};
