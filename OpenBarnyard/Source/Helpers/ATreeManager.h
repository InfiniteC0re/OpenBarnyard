#pragma once
#include "Render/ALockedMesh.h"
#include "Physics/ACollisionObjectSimple.h"
#include "Terrain/ATerrainLocator.h"

#include <Toshi/T2SList.h>
#include <Toshi/TNodeList.h>
#include <Render/TAnimation.h>

class ACollisionModelSet;
class ACollisionModelInstance;

class ATreeManager
    : public Toshi::TSingleton<ATreeManager>
{
public:
	struct TreeInstance
	    : Toshi::T2SList<TreeInstance>::Node
	{
		// $Barnyard: FUNCTION 005ee010
		TreeInstance() {}

		ATerrainLocatorList* pLocatorList;
		TUINT16              uiLocatorId;

		struct
		{
			TUINT8 iTrunkIndex : 6;
			TUINT8 iColor : 2;
		};

		TUINT8 uiRegrowthGroup; // the group of the regrowth objects (fruits) of the tree, see ARegrowthManager

		// The position of the tree (the locators are Z up)
		// $Barnyard: FUNCTION 005edda0
		Toshi::TVector4 GetPosition() const
		{
			const Toshi::TVector3& rcPosition = pLocatorList->GetLocator( uiLocatorId )->vecPosition;
			return Toshi::TVector4( rcPosition.x, -rcPosition.z, rcPosition.y, 1.0f );
		}

		ACollisionObjectSimple*  pSimpleCollisionObjects = TNULL; // 0x0c, collisions of the FOBs
		ACollisionModelInstance* pTrunkCollision         = TNULL; // 0x10, collision model of the trunk (when the collision sets are enabled)
	};

	struct TreeInstanceRenderData
	{
		Toshi::TMatrix44        oMatrix;
		TreeInstanceRenderData* pPrev;
	};

	class Model
	{
	public:
		friend ATreeManager;

	public:
		Model();
		~Model();

		void Load( const TCHAR* a_szModelName );

	private:
		Toshi::TManagedModel* m_pManagedModel;
		Toshi::TSceneObject*  m_pSceneObject;
		TUINT                 m_uiID;
		TINT                  m_iNumFOBs;
		Toshi::TMatrix44*     m_pFOBMatrices;
		TFLOAT                m_fGroundOffset;
		TFLOAT                m_fRadius;
	};

	// Objects at +0x6670, only destroyed by the original (nothing creates them)
	class Unk6670 : public Toshi::TObject
	{
	public:
		virtual void Destroy() = 0; // +0x0c
	};

	// Nodes of the two lists at +0x6678 and +0x6688, only destroyed by the original
	struct UnkNode : public Toshi::TNodeList<UnkNode>::TNode
	{
	};

	inline static constexpr TINT MAX_INSTANCES      = 1302;
	inline static constexpr TINT MAX_COLLISION_SETS = 5;

public:
	ATreeManager();
	~ATreeManager();

	// Initialises models and returns TTRUE if succeded
	TBOOL Initialise();

	void CreateInstances( ATerrainLocatorList* a_pLocatorList );

	// Removes the instances created from a_pLocatorList
	void DestroyInstances( ATerrainLocatorList* a_pLocatorList );

	void Render();

	// $Barnyard: FUNCTION 005edd20
	void GetDistances( TFLOAT& a_rLODSwitchDistance, TFLOAT& a_rRenderDistance, TFLOAT& a_rBlendingDistance ) const
	{
		a_rLODSwitchDistance = m_fLODSwitchDistance;
		a_rRenderDistance    = m_fRenderDistance;
		a_rBlendingDistance  = m_fBlendingDistance;
	}

	// $Barnyard: FUNCTION 005edd50
	void SetDistances( TFLOAT a_fLODSwitchDistance, TFLOAT a_fRenderDistance, TFLOAT a_fBlendingDistance )
	{
		m_fLODSwitchDistance = a_fLODSwitchDistance;
		m_fRenderDistance    = a_fRenderDistance;
		m_fBlendingDistance  = a_fBlendingDistance;
	}

	// Creates (a_bEnable) or destroys the collision model sets of the trunks
	void SetCollisionEnabled( TBOOL a_bEnable );

	TBOOL GetCreateCollision() const { return m_bCreateCollision; }
	void  SetCreateCollision( TBOOL a_bCreateCollision ) { m_bCreateCollision = a_bCreateCollision; }

	// Set by SetCollisionEnabled (creates/destroys the collision model sets of the trees)
	TBOOL IsCollisionEnabled() const { return m_bCollisionEnabled; }

	// The tree with FOBs in front of a_rcPosition (a_rcDirection is the XZ direction) within a_fMaxDistance
	TreeInstance* FindTreeInFront( const Toshi::TVector4& a_rcPosition, const Toshi::TVector2& a_rcDirection, TFLOAT a_fMaxDistance );

	// The tree whose TreeInstance::pTrunkCollision is a_pCollisionInstance (the ACollisionModelInstance of a hit collision object)
	TreeInstance* FindTreeByCollision( void* a_pCollisionInstance );

	// Number of FOBs (leaf clusters) of the trunk model of a_pTree
	TINT GetNumFOBs( const TreeInstance* a_pTree ) const;

	// World position of the FOB a_iFOB of a_pTree
	Toshi::TVector4 GetFOBPosition( const TreeInstance* a_pTree, TINT a_iFOB ) const;

private:
	// Destroys the models, the keyframe libraries and the collisions of the trees
	void Destroy();

	// Destroys the collision model sets of the trunks
	void DestroyCollisionSets();

	// The collision model set of the trunk a_iTrunk or TNULL
	ACollisionModelSet* FindCollisionSet( TINT a_iTrunk ) const;

private:
	TINT                      m_iNumKeyLibs;            // 0x00
	Toshi::TKeyframeLibrary** m_ppKeyLibs;              // 0x04
	TINT                      m_iNumTrunks;             // 0x08
	Model*                    m_pTrunks;                // 0x0c
	TINT                      m_iNumFOBs;               // 0x10
	Model*                    m_pFOBs;                  // 0x14
	TreeInstanceRenderData**  m_pRenderData;            // 0x18
	ACollisionObjectSimple*   m_pTrunkCollisionObjects; // 0x1c

	TreeInstance                 m_aTreeInstances[ MAX_INSTANCES ];       // 0x20
	Toshi::T2SList<TreeInstance> m_llUsedTreeInstances;                   // 0x65d8
	Toshi::T2SList<TreeInstance> m_llFreeTreeInstances;                   // 0x65dc
	ALockedMesh                  m_aLockedMeshes[ 2 ];                    // 0x65e0
	Unk6670*                     m_apUnk6670[ 2 ];                        // 0x6670
	Toshi::TNodeList<UnkNode>    m_oUnkList6678;                          // 0x6678
	Toshi::TNodeList<UnkNode>    m_oUnkList6688;                          // 0x6688
	TFLOAT                       m_fLODSwitchDistance;                    // 0x6698
	TFLOAT                       m_fRenderDistance;                       // 0x669c
	TFLOAT                       m_fBlendingDistance;                     // 0x66a0
	TBOOL                        m_bCreateCollision;                      // 0x66a4
	TBOOL                        m_bCollisionEnabled;                     // 0x66a5, the collision sets are enabled
	TINT                         m_iNumCollisionSets;                     // 0x66a8
	ACollisionModelSet*          m_apCollisionSets[ MAX_COLLISION_SETS ]; // 0x66ac
};
