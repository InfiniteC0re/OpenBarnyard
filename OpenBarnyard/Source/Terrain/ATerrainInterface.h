#pragma once
#include "ATerrainVIS.h"
#include "Assets/AModelLoaderJob.h"
#include "Assets/AMatLibLoaderJob.h"
#include "Assets/AKeyLibLoaderJob.h"
#include "Assets/ASkeletonDoneJob.h"
#include "Assets/ACollisionDoneJob.h"
#include "Assets/ASectionDoneJob.h"
#include "Physics/ACollisionCommon.h"

#include <Math/TOBBox.h>

class MODAPI ATerrainInterface : public Toshi::TSingleton<ATerrainInterface>
{
public:
	constexpr static TUINT NUM_LIGHT_MAGS                = 12;
	constexpr static TUINT MAX_NUM_MODEL_LOADER_JOBS     = 128;
	constexpr static TUINT MAX_NUM_MATLIB_LOADER_JOBS    = 10;
	constexpr static TUINT MAX_NUM_KEYLIB_LOADER_JOBS    = 10;
	constexpr static TUINT MAX_NUM_SECTION_LOADER_JOBS   = 32;
	constexpr static TUINT MAX_NUM_TRB_LOADER_JOBS       = 2;
	constexpr static TUINT MAX_NUM_SKELETON_LOADER_JOBS  = 2;
	constexpr static TUINT MAX_NUM_COLLISION_LOADER_JOBS = 64;
	constexpr static TUINT NUM_TOTAL_JOBS =
	    MAX_NUM_MODEL_LOADER_JOBS +
	    MAX_NUM_MATLIB_LOADER_JOBS +
	    MAX_NUM_KEYLIB_LOADER_JOBS +
	    MAX_NUM_SECTION_LOADER_JOBS +
	    MAX_NUM_TRB_LOADER_JOBS +
	    MAX_NUM_SKELETON_LOADER_JOBS +
	    MAX_NUM_COLLISION_LOADER_JOBS;

	using JobSlot = Toshi::T2SListNodeWrapper<AMainThreadJob*>;

	friend ATerrainVIS;
	friend class AGameLoader;
	friend class AGameTimeFX;
	friend class APrecisionSquirt;

	friend void RenderCellMeshWin( CellMeshSphere* a_pMeshSphere, RenderData* a_pRenderData );

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

	// Renders glowing models of the current and the visible sections (after the glowing models)
	void RenderGlowModels();

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

	// $Barnyard: FUNCTION 005e7e20
	ATerrainSection* GetSection( TINT a_iIndex ) const { return &m_pTerrainVIS->m_pSections[ a_iIndex ]; }

	/**
	 * Finds a locator (bone) by the specified name and obtains it current transform if found.
	 */
	void GetLocatorTransform( const TCHAR* a_pszLocatorName, Toshi::TMatrix44& a_rOutTransform );

	/**
	 * Finds a locator (bone) by the specified name and obtains it current position if found.
	 */
	Toshi::TVector4 GetLocatorPos( const TCHAR* a_pszLocatorName );

	/**
	 * Finds a locator (bone) by the specified name and obtains its position and direction (its negated Y axis). Returns TFALSE if not found.
	 */
	TBOOL GetLocatorPosAndDir( const TCHAR* a_pszLocatorName, Toshi::TVector4& a_rOutPosition, Toshi::TVector4& a_rOutDirection );

	/**
	 * Finds a locator (bone) by the specified name and obtains it current position and rotation if found.
	 */
	void GetLocatorPosAndRot( const TCHAR* a_pszLocatorName, Toshi::TVector4& a_rOutPosition, Toshi::TQuaternion& a_rOutRotation );

	/**
	 * Finds a locator (bone) by the specified name and obtains its position and the negated a_iAxis basis vector of its transform.
	 * NOTE: the locator must exist
	 */
	Toshi::TVector4& GetLocatorPosAndAxis( const TCHAR* a_pszLocatorName, Toshi::TVector4& a_rOutPosition, Toshi::TVector4& a_rOutAxis, TINT a_iAxis );

	/**
	 * Finds a locator (bone) by the specified name and returns it if found.
	 */
	ATerrainLocator* FindLocator( const TCHAR* a_pszLocatorName );

	ATRBLoaderJob*     GetFreeTRBLoaderJob();
	ASkeletonDoneJob*  GetFreeSkeletonLoaderJob();
	AMatLibLoaderJob*  GetFreeMatlibLoaderJob();
	ACollisionDoneJob* GetFreeCollisionLoaderJob();
	AModelLoaderJob*   GetFreeModelLoaderJob();
	ASectionDoneJob*   GetFreeSectionLoaderJob();

	// Section whose collision is under a_rcPoint (from 40 above it), TNULL if none. a_bUnused isn't used.
	ATerrainSection* FindSectionAtPoint( const Toshi::TVector4& a_rcPoint, TBOOL a_bUnused );

	// Index of the section under a_rcPoint or -1
	TINT GetSectionIndexAtPoint( const Toshi::TVector4& a_rcPoint, TBOOL a_bUnused );

	// Returns the localised name of the area the section a_szSectionName belongs to or TNULL
	const TWCHAR* GetSectionLocalisedName( const TCHAR* a_szSectionName );

	// Replaces the locators with the ones of the skeleton a_szSkelFilePath and waits until they are loaded
	void ReloadSkeleton( const TCHAR* a_szSkelFilePath );

	// Cancels the jobs of the asset streaming
	void CancelStreaming();

	const TCHAR* GetSkeletonFilePath() const { return m_pTerrainVIS->m_szSkelFilePath; }

	// $Barnyard: FUNCTION 005e82e0
	ATerrainLocatorList* GetLocatorList() const
	{
		TVALIDPTR( m_pTerrainVIS );
		return m_pTerrainVIS->GetLocatorList();
	}

	TINT GetSectionID() const { return m_iCurrentSection; }

	void SetLightMag( TINT a_iIndex, TFLOAT a_fMagnitude ) { m_afLightMags[ a_iIndex ] = a_fMagnitude; }
	void SetLightOverride( TBOOL a_bOverride ) { m_bLightOverride = a_bOverride; }

	Toshi::TCollisionCommon::TOSHICGROUPFLAG GetSectionRayIgnoreGroups() const { return m_eSectionRayIgnoreGroups; }

	void SetGetCurrentVISGroupCallback( t_GetCurrentVISGroup a_fnCallback ) { m_fnGetCurrentVISGroup = a_fnCallback; }
	void SetIsLoaded( TBOOL a_bLoaded ) { m_bIsLoaded = a_bLoaded; }

	const Toshi::TVector4& GetDefaultShadowColor() const { return m_DefaultShadowColor; }
	const Toshi::TVector4& GetDefaultAmbientColor() const { return m_DefaultAmbientColor; }
	const Toshi::TVector4& GetLitShadowColor() const { return m_LitShadowColor; }
	const Toshi::TVector4& GetLitAmbientColor() const { return m_LitAmbientColor; }

	t_OnModelNodeReady GetOnModelNodeReadyCallback() const { return m_cbOnModelLoaded; }
	t_OnModelNodeReady GetOnCollisionModelReadyCallback() const { return m_cbOnCollisionModelLoaded; }

	// $Barnyard: FUNCTION 005e8230
	void SetCollisionModelLoadCallback( t_OnModelNodeReady a_fnCallback ) { m_cbOnCollisionModelLoaded = a_fnCallback; }

	// $Barnyard: FUNCTION 004a3920
	void SetOnVISGroupChangedCallback( t_OnVISGroupChanged a_fnCallback ) { m_cbOnVISGroupChanged = a_fnCallback; }

	// Returns index of the section whose collision model is a_pCollisionModel
	TINT GetCollisionModelSectionIndex( ATerrainSection::ModelNode* a_pCollisionModel ) const;

	//-----------------------------------------------------------------------------
	// Collision queries against the collision models of the loaded sections
	//-----------------------------------------------------------------------------

	TBOOL CollideRay( const Toshi::TVector4& a_rcOrigin, const Toshi::TVector4& a_rcDirection, TFLOAT& a_rfMaxDistance, Toshi::TCollisionCommon::TOSHICGROUPFLAG a_eIgnoreGroups, TBOOL a_bCulling );
	TBOOL CollideRay( const Toshi::TVector4& a_rcOrigin, const Toshi::TVector4& a_rcDirection, TFLOAT& a_rfMaxDistance, ACollisionModelRayHitList* a_pHitList, Toshi::TCollisionCommon::TOSHICGROUPFLAG a_eIgnoreGroups, TBOOL a_bCulling );

	// Collects the triangles touching the sphere (xyz is the center, w is the radius).
	TBOOL CollideSphere( const Toshi::TVector4& a_rcSphere, ACollisionModelRayHitList* a_pHitList, TINT a_iCollGroup, Toshi::TCollisionCommon::TOSHICGROUPFLAG a_eIgnoreGroups );

	// Outputs the distance to the closest triangle touching the sphere and its normal.
	TBOOL CollideSphere( const Toshi::TVector4& a_rcSphere, TFLOAT& a_rfDistance, Toshi::TVector4& a_rNormal, TINT a_iCollGroup, Toshi::TCollisionCommon::TOSHICGROUPFLAG a_eIgnoreGroups );

	// Collects the triangles touching a box in a_pHitList.
	TBOOL CollideBox( const Toshi::TOBBox& a_rcBox, ACollisionModelRayHitList* a_pHitList, TINT a_iCollGroup, Toshi::TCollisionCommon::TOSHICGROUPFLAG a_eIgnoreGroups );

	// Collects the triangles touching the sphere around a box moving from a_rcPrevBox to a_rcBox in a_pHitList.
	TBOOL CollideSweptBox( const Toshi::TOBBox& a_rcPrevBox, const Toshi::TOBBox& a_rcBox, ACollisionModelRayHitList* a_pHitList, TINT a_iCollGroup, Toshi::TCollisionCommon::TOSHICGROUPFLAG a_eIgnoreGroups );

	// Pushes the sphere out of the triangles of every section (each section starts from a_rcSphere).
	TBOOL PushOutSphere( const Toshi::TVector4& a_rcSphere, Toshi::TVector4& a_rOutPosition, TINT a_iCollGroup, Toshi::TCollisionCommon::TOSHICGROUPFLAG a_eIgnoreGroups );

	TBOOL CollideSweptSphere( const Toshi::TVector4& a_rcCenter, TFLOAT a_fRadius, const Toshi::TVector4& a_rcDirection, TFLOAT& a_rfDistance, Toshi::TVector4& a_rNormal, TFLOAT a_fMaxStepHeight, TFLOAT& a_rfStepHeight, TFLOAT& a_rfPenetration, TINT a_iUnused, Toshi::TCollisionCommon::TOSHICGROUPFLAG a_eIgnoreGroups );

	// Same as the other CollideSweptSphere, without the unused parameter
	TBOOL CollideSweptSphere( const Toshi::TVector4& a_rcCenter, TFLOAT a_fRadius, const Toshi::TVector4& a_rcDirection, TFLOAT& a_rfDistance, Toshi::TVector4& a_rNormal, TFLOAT a_fMaxStepHeight, TFLOAT& a_rfStepHeight, TFLOAT& a_rfPenetration, Toshi::TCollisionCommon::TOSHICGROUPFLAG a_eIgnoreGroups );

	// Sweeps a sphere and slides it along the hit triangles (up to 15 times). Outputs the end position and the last
	// slide direction. Returns TFALSE if nothing was hit.
	TBOOL SlideSweptSphere( const Toshi::TVector4& a_rcCenter, TFLOAT a_fRadius, const Toshi::TVector4& a_rcDirection, TFLOAT& a_rfDistance, TFLOAT a_fMaxStepHeight, TFLOAT a_fSpeedScale, TFLOAT& a_rfStepHeight, Toshi::TVector4& a_rOutPosition, Toshi::TVector4& a_rOutDirection, TFLOAT a_fMinSlope, TFLOAT a_fMaxSlope );

	// Same as the other SlideSweptSphere, without the slide direction
	TBOOL SlideSweptSphere( const Toshi::TVector4& a_rcCenter, TFLOAT a_fRadius, const Toshi::TVector4& a_rcDirection, TFLOAT& a_rfDistance, TFLOAT a_fMaxStepHeight, TFLOAT a_fSpeedScale, TFLOAT& a_rfStepHeight, Toshi::TVector4& a_rOutPosition, TFLOAT a_fMinSlope, TFLOAT a_fMaxSlope );

	// Scales a_rfSpeed down on steep slopes (a_rcNormal.y between a_fMinSlope and a_fMaxSlope).
	// NOTE: like ACollisionManager::ScaleSpeedBySlope, but with a softer curve
	static void ScaleSpeedBySlopeSoft( TFLOAT& a_rfSpeed, const Toshi::TVector4& a_rcNormal, TFLOAT a_fMinSlope, TFLOAT a_fMaxSlope );

	// Finds the ground below a_rcPosition (up to a_fMaxDistance away) to place an object (or its shadow) on it.
	// a_rOutPosition gets the ground point raised by a_fHeightOffset, a_rOutAlpha 255..0 by the distance to the ground.
	// If the a_fRadius sphere at the ground point touches triangles, a_rOutNormal gets their average (upwards facing)
	// normal and a_rOutRotation the euler angles of the matrix aligned with it. Returns TFALSE if the ground is too far.
	TBOOL FindGroundPlacement( const Toshi::TVector4& a_rcPosition, TFLOAT a_fHeightOffset, Toshi::TVector4& a_rOutPosition, TINT& a_rOutAlpha, Toshi::TVector3& a_rOutRotation, Toshi::TVector4& a_rOutNormal, TFLOAT a_fRadius, TFLOAT a_fMaxDistance );

	// Casts a ray down from the point (up to 40 units) and returns the y of the ground or 0.0f if nothing was hit.
	TFLOAT GetGroundHeight( TFLOAT a_fX, TFLOAT a_fY, TFLOAT a_fZ, Toshi::TCollisionCommon::TOSHICGROUPFLAG a_eIgnoreGroups );
	TFLOAT GetGroundHeight( const Toshi::TVector4& a_rcPosition, Toshi::TCollisionCommon::TOSHICGROUPFLAG a_eIgnoreGroups );

	// Casts a ray from y = -40 down to y = 40 at x, z and returns the y of the first hit or 0.0f if nothing was hit.
	TFLOAT GetGroundHeightTop( TFLOAT a_fX, TFLOAT a_fZ, Toshi::TCollisionCommon::TOSHICGROUPFLAG a_eIgnoreGroups );

	// Sets the y of a_rPosition to GetGroundHeightTop at its x, z.
	void SetToGroundHeightTop( Toshi::TVector4& a_rPosition, Toshi::TCollisionCommon::TOSHICGROUPFLAG a_eIgnoreGroups );

	// Casts a ray from y = 40 up to y = -40 at x, z and returns the y of the first hit or 0.0f if nothing was hit.
	TFLOAT GetGroundHeightBottom( TFLOAT a_fX, TFLOAT a_fZ, Toshi::TCollisionCommon::TOSHICGROUPFLAG a_eIgnoreGroups );

	// Sets the y of a_rPosition to GetGroundHeightBottom at its x, z.
	void SetToGroundHeightBottom( Toshi::TVector4& a_rPosition, Toshi::TCollisionCommon::TOSHICGROUPFLAG a_eIgnoreGroups );

	// Casts a ray (CollideRay) and returns the distance of the hit or a_fMaxDistance if nothing was hit.
	TFLOAT GetRayHitDistance( Toshi::TVector4 a_vOrigin, Toshi::TVector4 a_vDirection, TFLOAT a_fMaxDistance, Toshi::TCollisionCommon::TOSHICGROUPFLAG a_eIgnoreGroups, TBOOL a_bCulling );

	// Finds the ground below a_rcPosition (up to a_fMaxDistance) for a blob shadow. Outputs the ground position (moved up
	// by a_fHeightOffset), an alpha (255 at the position, 0 at a_fMaxDistance) and, if the sphere of a_fRadius at the ground
	// touches anything, the average normal of the triangles facing up and its rotation. Returns TFALSE if there's no ground.
	TBOOL GetGroundShadowInfo( const Toshi::TVector4& a_rcPosition, TFLOAT a_fHeightOffset, Toshi::TVector4& a_rOutPosition, TINT& a_rOutAlpha, Toshi::TVector3& a_rOutRotation, Toshi::TVector4& a_rOutNormal, TFLOAT a_fRadius, TFLOAT a_fMaxDistance, Toshi::TCollisionCommon::TOSHICGROUPFLAG a_eIgnoreGroups );

private:
	void UpdateNightMaterials();
	void MoveAllFinishedJobs( Toshi::T2SList<JobSlot>& a_rFreeJobs, Toshi::T2SList<JobSlot>& a_rUsedJobs );

	/* Moves all finished jobs from used lists to free lists */
	void FlushJobs();

public:
	static TINT GetSectionPlayerIsIn();

	// Returns index of a_pSection in the sections of the VIS
	TINT GetSectionIndex( const ATerrainSection* a_pSection ) const;

	// Returns the section called a_szName (case insensitive), TNULL if there's none
	ATerrainSection* FindSection( const TCHAR* a_szName ) const;

	// Returns the index of the section called a_szName, -1 if there's none
	TINT GetSectionIndex( const TCHAR* a_szName ) const;

	// Streams the terrain around section a_iSection (instead of the one the camera is in) until
	// it's loaded, a_bWait blocks until then
	void ForceSection( TINT a_iSection, TBOOL a_bWait );

private:
	// m_fnGetCurrentVISGroup while a section is forced
	static TINT GetForcedSection();

	// The forced section (0x00778820) and the callback it replaced (0x007962fc)
	static TINT                 ms_iForcedSection;
	static t_GetCurrentVISGroup ms_fnUnforcedGetCurrentVISGroup;

private:
private:
	Toshi::TVector4                            m_LitShadowColor;
	Toshi::TVector4                            m_LitAmbientColor;
	Toshi::TVector4                            m_DefaultShadowColor;
	Toshi::TVector4                            m_DefaultAmbientColor;
	TFLOAT                                     m_fUnused3;
	TBOOL                                      m_bUnused4;
	Toshi::T2SList<ATerrainSection::ModelNode> m_llModelNodes;
	t_OnModelNodeReady                         m_cbOnCollisionModelLoaded;
	t_OnModelNodeReady                         m_cbOnModelLoaded;
	t_OnVISGroupChanged                        m_cbOnVISGroupChanged;
	AModelLoaderJob                            m_aModelLoaderJobs[ MAX_NUM_MODEL_LOADER_JOBS ];
	AMatLibLoaderJob                           m_aMatlibLoaderJobs[ MAX_NUM_MATLIB_LOADER_JOBS ];
	AKeyLibLoaderJob                           m_aKeylibLoaderJobs[ MAX_NUM_KEYLIB_LOADER_JOBS ];
	ASectionDoneJob                            m_aSectionDoneJobs[ MAX_NUM_SECTION_LOADER_JOBS ];
	ATRBLoaderJob                              m_aTRBLoaderJobs[ MAX_NUM_TRB_LOADER_JOBS ];
	ASkeletonDoneJob                           m_aSkeletonDoneJobs[ MAX_NUM_SKELETON_LOADER_JOBS ];
	ACollisionDoneJob                          m_aCollisionDoneJobs[ MAX_NUM_COLLISION_LOADER_JOBS ];
	JobSlot*                                   m_pJobs;
	Toshi::T2SList<JobSlot>                    m_UsedModelLoaderJobs;
	Toshi::T2SList<JobSlot>                    m_FreeModelLoaderJobs;
	Toshi::T2SList<JobSlot>                    m_UsedUnknownLoaderJobs;
	Toshi::T2SList<JobSlot>                    m_FreeUnknownLoaderJobs;
	Toshi::T2SList<JobSlot>                    m_UsedSectionLoaderJobs;
	Toshi::T2SList<JobSlot>                    m_FreeSectionLoaderJobs;
	Toshi::T2SList<JobSlot>                    m_UsedKeylibLoaderJobs;
	Toshi::T2SList<JobSlot>                    m_FreeKeylibLoaderJobs;
	Toshi::T2SList<JobSlot>                    m_UsedMatlibLoaderJobs;
	Toshi::T2SList<JobSlot>                    m_FreeMatlibLoaderJobs;
	Toshi::T2SList<JobSlot>                    m_UsedTRBLoaderJobs;
	Toshi::T2SList<JobSlot>                    m_FreeTRBLoaderJobs;
	Toshi::T2SList<JobSlot>                    m_UsedSkeletonLoaderJobs;
	Toshi::T2SList<JobSlot>                    m_FreeSkeletonLoaderJobs;
	Toshi::T2SList<JobSlot>                    m_UsedCollisionLoaderJobs;
	Toshi::T2SList<JobSlot>                    m_FreeCollisionLoaderJobs;
	TBOOL                                      m_bStreamCollision;
	TBOOL                                      m_bIsLoaded;
	Toshi::TTRB                                m_VISTRB;
	Toshi::TTRB                                m_TRB2;
	ATerrainVIS*                               m_pTerrainVIS;
	ATerrainOrderDVIS*                         m_pOrderDVIS;
	Toshi::TMaterial*                          m_apLightMagMaterials[ NUM_LIGHT_MAGS ];
	TFLOAT                                     m_afLightMags[ NUM_LIGHT_MAGS ];
	TBOOL                                      m_bLightOverride;
	TINT                                       m_iCurrentSection;
	TINT                                       m_iPreviousSection;
	TINT                                       m_iPreloadTerrainBlockSize;
	t_GetCurrentVISGroup                       m_fnGetCurrentVISGroup;
	Toshi::TCollisionCommon::TOSHICGROUPFLAG   m_eSectionRayIgnoreGroups; // 0x4cd0, see ATerrainSection::FindUnloadedSectionBelow
	// ...
	TINT m_iNumChunks;
};
