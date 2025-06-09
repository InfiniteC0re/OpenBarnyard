#pragma once
#include "ATerrainLODBlock.h"
#include "Animation/AModel.h"
#include "Assets/AMaterialLibrary.h"
#include "Render/AWorldShader/AWorldMaterial.h"
#include "World/AWorldVIS.h"
#include "Collision/ACollisionModelInstance.h"

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

		TBOOL           IsUsingLighting() const { return !ISZERO( m_eFlags & MNF_USE_LIGHTING ); }
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

	enum FLAGS : TUINT32
	{
		FLAGS_HIGH_LOD_LOADED   = BITFLAG( 0 ),
		FLAGS_LOW_LOD_LOADED    = BITFLAG( 1 ),
		FLAGS_HIGH_LOD_LOADING  = BITFLAG( 2 ),
		FLAGS_LOW_LOD_LOADING   = BITFLAG( 3 ),
		FLAGS_HIGH_LOD_QUEUED   = BITFLAG( 4 ),
		FLAGS_LOW_LOD_QUEUED    = BITFLAG( 5 ),
		FLAGS_COLLISION_LOADING = BITFLAG( 10 )
	};

	friend class ATerrainInterface;
	friend class ASectionDoneJob;
	friend class ACollisionDoneJob;

public:
	// Draws models of specified LOD
	// Note: if trying to draw high LOD that is not loaded, low LOW will draw
	void Draw( ATerrainLODType a_eLODType );

	void LoadCollision();
	void LoadModels( ATerrainLODType a_eLODType );
	void LoadMatlib( ATerrainLODType a_eLODType );

	void UnloadMatlib( ATerrainLODType a_eLODType );
	void DestroyLOD( ATerrainLODType a_eLODType );

	void RemoveFromStreamingQueue();

	TBOOL IsMatLibLoaded( ATerrainLODType a_eLODType ) const;

	TBOOL IsLODQueued( ATerrainLODType a_eLODType ) const { return ( m_eFlags & ( 16 << a_eLODType ) ); }
	void  SetLODQueued( ATerrainLODType a_eLODType, TBOOL a_bQueued );

	TBOOL IsLODLoaded( ATerrainLODType a_eLODType ) const { return m_eFlags & ( 1 << ( a_eLODType ) ); }
	void  SetLODLoaded( ATerrainLODType a_eLODType, TBOOL a_bLoaded );

	TBOOL IsLODLoading( ATerrainLODType a_eLODType ) const { return m_eFlags & ( 1 << ( a_eLODType + ATerrainLODType_NUMOF ) ); }
	void  SetLODLoading( ATerrainLODType a_eLODType, TBOOL a_bLoading );

	// TODO: The flag's name is wrong
	TBOOL IsLODEmpty( ATerrainLODType a_eLODType ) const { return ( m_eFlags & ( 64 << ( a_eLODType & 0x1f ) ) ); }
	void  SetLODEmpty( ATerrainLODType a_eLODType, TBOOL a_bEmpty );

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

	TBOOL IsOtherSectionVisible( TINT a_iSection ) const { return m_pVisibility[ a_iSection ] > ATerrainLODType_None; }

private:
	const TCHAR*  m_szName;
	const TCHAR** m_pszHighModelFiles;
	const TCHAR** m_pszLowModelFiles;
	TINT32        m_iNumHighModelFiles;
	TINT32        m_iNumLowModelFiles;

	TUINT8*      m_aLODFlags[ 2 ];
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
	TUINT32            m_eFlags;
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
