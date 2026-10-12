#pragma once
#include "Render/TModelManager.h"
#include "Render/TTransformObject.h"

TOSHI_NAMESPACE_START

class TSceneObject
{
public:
	using FLAGS = TUINT8;
	enum FLAGS_ : FLAGS
	{
		FLAGS_NONE             = 0,
		FLAGS_SKELETON_UPDATE  = BITFLAG( 0 ), // also required by RenderIfVisible
		FLAGS_DISTANCE_CULLING = BITFLAG( 1 ),
		FLAGS_OPACITY          = BITFLAG( 2 ),
	};

public:
	TSceneObject();
	~TSceneObject();

	void Create( TManagedModel* a_pModelPtr );

	void Delete();
	void DestroyModelInstance();

	void  Update( TFLOAT a_fDeltaTime );
	TBOOL Render( TUINT a_uiClipFlags, const TVector3& a_rBounding );
	TBOOL RenderIfVisible();

	// Returns TTRUE if the skeleton update is enabled and the bounding sphere is in the frustum of the current context
	TBOOL ShouldAnimateThisFrame();

	TManagedModel*     GetModelRef() { return m_pModelRef; }
	TModelInstance*    GetInstance() { return m_pModelInstance; }
	TSkeletonInstance* GetSkeletonInstance() { return GetInstance()->GetSkeletonInstance(); }
	TTransformObject&  GetTransform() { return m_TransformObject; }

	void  EnableSkeletonUpdate() { m_eFlags |= FLAGS_SKELETON_UPDATE; }
	void  DisableSkeletonUpdate() { m_eFlags &= ~FLAGS_SKELETON_UPDATE; }
	TBOOL IsSkeletonUpdateEnabled() const { return m_eFlags & FLAGS_SKELETON_UPDATE; }

	void EnableOpacity() { m_eFlags |= FLAGS_OPACITY; }
	void DisableOpacity() { m_eFlags &= ~FLAGS_OPACITY; }

	void EnableDistanceCulling() { m_eFlags |= FLAGS_DISTANCE_CULLING; }
	void DisableDistanceCulling() { m_eFlags &= ~FLAGS_DISTANCE_CULLING; }

	FLAGS GetFlags() const { return m_eFlags; }
	void  SetFlags( FLAGS a_eFlags ) { m_eFlags = a_eFlags; }

	TVector2& GetFadeDistances() { return m_vFadeDistances; }
	TVector2& GetFadeParams() { return m_vFadeParams; }

	// Shade coefficient of the render context while the model is rendered, see AInstanceLightingHelper
	TFLOAT GetShadeCoeff() const { return m_fShadeCoeff; }
	void   SetShadeCoeff( TFLOAT a_fShadeCoeff ) { m_fShadeCoeff = a_fShadeCoeff; }

private:
	TManagedModel*   m_pModelRef      = TNULL;                                       // 0x00
	TModelInstance*  m_pModelInstance = TNULL;                                       // 0x04
	TTransformObject m_TransformObject;                                              // 0x08
	TUINT8           m_aUnk1[ 8 ];                                                   // 0x68, set by Create
	TVector2         m_vFadeDistances;                                               // 0x70, x: start, y: end (culled beyond y with EnableDistanceCulling)
	TVector2         m_vFadeParams;                                                  // 0x78, x: 1 / ( end - start ), y: opacity (used with EnableOpacity)
	TFLOAT           m_fShadeCoeff = 0.0f;                                           // 0x80
	FLAGS            m_eFlags      = FLAGS_SKELETON_UPDATE | FLAGS_DISTANCE_CULLING; // 0x84
};

TOSHI_NAMESPACE_END
