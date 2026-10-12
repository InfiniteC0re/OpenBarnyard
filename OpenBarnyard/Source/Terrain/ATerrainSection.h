#pragma once
#include "ATerrainLODBlock.h"
#include "Animation/AModel.h"
#include "Assets/AMaterialLibrary.h"
#include "Render/AWorldShader/AWorldMaterial.h"
#include "World/AWorldVIS.h"
#include "Physics/ACollisionModelInstance.h"

#include <Render/TModelManager.h>
#include <Toshi/T2SList.h>

class ACollisionModelSet;

class ATerrainSection
{
public:
	class ModelNode : public Toshi::T2SList<ModelNode>::Node
	{
	public:
		using ModelNodeFlags = TUINT8;
		enum ModelNodeFlags_ : ModelNodeFlags
		{
			MNF_NONE         = 0,
			MNF_USE_LIGHTING = BITFLAG( 0 ),
			MNF_GLOW         = BITFLAG( 1 ),
		};

		static constexpr TUINT TYPE_NAME_MAX_SIZE = 14;

	public:
		ModelNode();
		~ModelNode();

		void Render();

		void SetUseLighting( TBOOL a_bUseLighting );
		void SetGlow( TBOOL a_bIsGlow );

		TBOOL IsUsingLighting() const { return !ISZERO( m_eFlags & MNF_USE_LIGHTING ); }
		TBOOL IsGlow() const { return !ISZERO( m_eFlags & MNF_GLOW ); }

		AWorldMaterial* GetAnimatedMaterial() const { return m_pAnimatedMaterial; }

	public:
		Toshi::TManagedModel    m_ModelRef;
		Toshi::TSceneObject*    m_pSceneObject;
		AWorldVis               m_WorldVis;
		AWorldMaterial*         m_pAnimatedMaterial;
		ACollisionModelSet*     m_pCollisionModelSet;
		ACollisionModelInstance m_oCollisionInstance;
		char                    m_szType[ TYPE_NAME_MAX_SIZE + 1 ];
		ModelNodeFlags          m_eFlags;
		TBOOL                   m_bCreated;
	};

	using FLAGS = TUINT32;
	enum FLAGS_ : FLAGS
	{
		FLAGS_NONE              = 0,
		FLAGS_HIGH_LOD_LOADED   = BITFLAG( 0 ),
		FLAGS_LOW_LOD_LOADED    = BITFLAG( 1 ),
		FLAGS_HIGH_LOD_LOADING  = BITFLAG( 2 ),
		FLAGS_LOW_LOD_LOADING   = BITFLAG( 3 ),
		FLAGS_HIGH_LOD_QUEUED   = BITFLAG( 4 ),
		FLAGS_LOW_LOD_QUEUED    = BITFLAG( 5 ),
		FLAGS_HIGH_LOD_READY    = BITFLAG( 6 ),
		FLAGS_LOW_LOD_READY     = BITFLAG( 7 ),
		FLAGS_COLLISION_LOADING = BITFLAG( 10 )
	};

	// Flags of the models of a LOD (m_aLODFlags)
	using LODFLAGS = TUINT8;
	enum LODFLAGS_ : LODFLAGS
	{
		LODFLAGS_NONE = 0,
		LODFLAGS_GLOW = BITFLAG( 0 ), // the model uses lighting and glows
	};

	friend class ATerrainInterface;
	friend class ATerrainVIS;
	friend class ASectionDoneJob;
	friend class ACollisionDoneJob;

public:
	TBOOL IsPointInside( const Toshi::TVector4& a_rcPoint );

	// Casts a ray down through a_rcPosition against this section's collision and returns the first section, named by
	// the hit collision groups, whose high LOD isn't loaded yet (TNULL if there's none). Used to decide if the terrain
	// has to stall until the ground under the camera is loaded.
	ATerrainSection* FindUnloadedSectionBelow( const Toshi::TVector4& a_rcPosition ) const;

	void LoadCollision();
	void LoadModels( ATerrainLODType a_eLODType );
	void LoadMatlib( ATerrainLODType a_eLODType );

	void UnloadMatlib( ATerrainLODType a_eLODType );
	void DestroyLOD( ATerrainLODType a_eLODType );

	void RemoveFromStreamingQueue();

	// Makes the terrain stream this section (ATerrainInterface::ForceSection) unless a_eLODType is already loaded
	void ForceLoad( ATerrainLODType a_eLODType );

	TBOOL IsMatLibLoaded( ATerrainLODType a_eLODType ) const;

	TBOOL IsLODQueued( ATerrainLODType a_eLODType ) const { return ( m_eFlags & ( FLAGS_HIGH_LOD_QUEUED << a_eLODType ) ); }
	void  SetLODQueued( ATerrainLODType a_eLODType, TBOOL a_bQueued );

	TBOOL IsLODLoaded( ATerrainLODType a_eLODType ) const { return m_eFlags & ( FLAGS_HIGH_LOD_LOADED << ( a_eLODType ) ); }
	void  SetLODLoaded( ATerrainLODType a_eLODType, TBOOL a_bLoaded );

	TBOOL IsLODLoading( ATerrainLODType a_eLODType ) const { return m_eFlags & ( FLAGS_HIGH_LOD_LOADED << ( a_eLODType + ATerrainLODType_NUMOF ) ); }
	void  SetLODLoading( ATerrainLODType a_eLODType, TBOOL a_bLoading );

	// Set once the LOD has finished loading (or has nothing to load) and can be rendered.
	// A loaded LOD without it is destroyed, and the streaming clears it to give an in-use block a second chance before reusing it.
	TBOOL IsLODReady( ATerrainLODType a_eLODType ) const { return ( m_eFlags & ( FLAGS_HIGH_LOD_READY << ( a_eLODType & 0x1f ) ) ); }
	void  SetLODReady( ATerrainLODType a_eLODType, TBOOL a_bReady );

	void GetLODNames( ATerrainLODType a_eLODType, const TCHAR**& a_rLODs, TINT& a_rNumLODs )
	{
		if ( a_eLODType == ATerrainLODType_High )
		{
			a_rLODs    = m_pszHighModelFiles;
			a_rNumLODs = m_iNumHighModelFiles;
		}
		else
		{
			a_rLODs    = m_pszLowModelFiles;
			a_rNumLODs = m_iNumLowModelFiles;
		}
	}

	void GetLODBlocks( ATerrainLODType a_eLODType, ATerrainLODBlock**& a_rMemBlocks, TUINT16& a_rNumMemBlocks )
	{
		if ( a_eLODType == ATerrainLODType_High )
		{
			a_rMemBlocks    = m_ppHighLODBlocks;
			a_rNumMemBlocks = m_iNumHighMemBlocksUsed;
		}
		else
		{
			a_rMemBlocks    = m_ppLowLODBlocks;
			a_rNumMemBlocks = m_iNumLowMemBlocksUsed;
		}
	}

	const TCHAR* GetName() const { return m_szName; }

	TINT GetLODCount( ATerrainLODType a_eLODType ) { return ( a_eLODType == ATerrainLODType_High ) ? m_iNumHighModelFiles : m_iNumLowModelFiles; }

	TBOOL           IsOtherSectionVisible( TINT a_iSection ) const { return m_pVisibility[ a_iSection ] > ATerrainLODType_None; }
	ATerrainLODType GetOtherSectionVisibility( TINT a_iSection ) const { return m_pVisibility[ a_iSection ]; }
	void            SetOtherSectionVisibility( TINT a_iSection, ATerrainLODType a_eLOD ) { m_pVisibility[ a_iSection ] = a_eLOD; }

private:
	const TCHAR*  m_szName;
	const TCHAR** m_pszHighModelFiles;
	const TCHAR** m_pszLowModelFiles;
	TINT32        m_iNumHighModelFiles;
	TINT32        m_iNumLowModelFiles;

	LODFLAGS*    m_aLODFlags[ 2 ];
	const TCHAR* m_szCollisionFilename;
	ModelNode*   m_pCollisionModelData;
	ModelNode**  m_ppLODModelsData[ ATerrainLODType_NUMOF ];

	const TCHAR*      m_szHighMatLibFilename;
	Toshi::TTRB*      m_pMatLibHighTRB;
	AMaterialLibrary* m_pMatLibHigh;

	const TCHAR*      m_szLowMatLibFilename;
	Toshi::TTRB*      m_pMatLibLowTRB;
	AMaterialLibrary* m_pMatLibLow;

	ATerrainLODType*   m_pVisibility;
	FLAGS              m_eFlags;
	TUINT16            m_iNumHighMemBlocksUsed;
	TUINT16            m_iNumLowMemBlocksUsed;
	ATerrainLODBlock** m_ppHighLODBlocks;
	ATerrainLODBlock** m_ppLowLODBlocks;
	TUINT8*            m_pHighLODToBlock;
	TUINT8*            m_pLowLODToBlock;
	TUINT8             m_iCollisionMemBlockID;
	TUINT8             m_iHighMatLibMemBlockID;
	TUINT8             m_iLowMatLibMemBlockID;
};
