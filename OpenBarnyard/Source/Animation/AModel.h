#pragma once
#include <File/TTRB.h>
#include <Toshi/TRefCounted.h>
#include <Render/TModelManager.h>
#include <Toshi/TPString8.h>

#include <Toshi/T2DList.h>
#include <Toshi/T2Vector.h>
#include <Toshi/TSceneObject.h>

class AModel;

class AModelInstance : public Toshi::TObject
    , public Toshi::TRefCounted
{
public:
	TDECLARE_CLASS( AModelInstance, Toshi::TObject );

public:
	using Flags = TUINT8;
	enum Flags_ : Flags
	{
		Flags_None                     = 0,
		Flags_UpdatingSkeleton         = BITFLAG( 0 ),
		Flags_Visible                  = BITFLAG( 1 ),
		Flags_DrawWithDefaultMaterials = BITFLAG( 2 ),
		Flags_ReceivesLight            = BITFLAG( 3 ),
		Flags_Unknown                  = BITFLAG( 4 ),
	};

	enum ChangeEvent : TUINT32
	{
		ChangeEvent_Visibility,
		ChangeEvent_UNK2,
		ChangeEvent_EnabledSkeletonUpdate,
		ChangeEvent_DisabledSkeletonUpdate,
	};

	friend class AModel;

public:
	AModelInstance();
	AModelInstance( AModel* a_pModel, Toshi::TSceneObject* a_pSceneObject, TBOOL a_bEnableSkeletonUpdate );

	void SetTransform( const Toshi::TMatrix44& a_rcTransform );
	void SetSkeletonUpdating( TBOOL a_bUpdating );
	void SetReceivesLight( TBOOL a_bReceives );
	void SetVisible( TBOOL a_bVisible );

	AModel*                  GetModel() const { return m_pModel; }
	const Toshi::TPString8&  GetModelName() const;
	Toshi::TSceneObject*     GetSceneObject() const { return m_pSceneObject; }
	Toshi::TTransformObject& GetTransform() const { return m_pSceneObject->GetTransform(); }
	Toshi::TSkeleton*        GetSkeleton() const { return GetSceneObject()->GetInstance()->GetSkeletonInstance()->GetSkeleton(); }
	TUINT                    GetClipFlags() const { return m_uiClipFlags; }
	void                     SetClipFlags( TUINT a_uiClipFlags ) { m_uiClipFlags = a_uiClipFlags; }

	const Toshi::TVector4& GetScale() const { return m_Scale; }

	TBOOL IsUpdatingSkeleton() const { return m_eFlags & Flags_UpdatingSkeleton; }
	TBOOL IsVisible() const { return m_eFlags & Flags_Visible; }
	TBOOL DrawWithDefaultMaterials() const { return m_eFlags & Flags_DrawWithDefaultMaterials; }
	TBOOL ReceivesLight() const { return m_eFlags & Flags_ReceivesLight; }

private:
	static void RenderInstanceCallback( Toshi::TModelInstance* a_pInstance, void* a_pUserData );

private:
	Toshi::TVector4 m_Scale;
	AModel*         m_pModel;
	TUINT           m_uiClipFlags;
	// ...
	Toshi::TSceneObject*                         m_pSceneObject;
	Toshi::TEmitter<AModelInstance, ChangeEvent> m_ChangeEmitter;
	Flags                                        m_eFlags;
};

using AModelInstanceRef = Toshi::TRef<AModelInstance>;

class AModel : public Toshi::T2DList<AModel>::Node
{
public:
	static constexpr TUINT32 MAX_NUM_INSTANCES = 250;

public:
	AModel( const Toshi::TPString8& a_rName, Toshi::TTRB* a_pTRB );
	virtual ~AModel();

	void Update( TFLOAT a_fDeltaTime );
	void Render( TBOOL a_bIsSomething );

	AModelInstanceRef CreateInstance();
	void              DestroyInstance( AModelInstance* a_pInstance );

	TSIZE                                                  GetNumInstances() const;
	Toshi::T2Vector<AModelInstanceRef, MAX_NUM_INSTANCES>& GetInstances() { return m_vecInstanceRefs; }

	const Toshi::TPString8& GetName() const { return m_Name; }
	Toshi::TSkeleton*       GetSkeleton() const { return m_pManagedModel->GetModel()->GetSkeleton(); }

public:
	static Toshi::TManagedModel* Create( const Toshi::TPString8& a_rFilePath, Toshi::TTRB* a_pTRB );
	static void                  GetNameFromPath( const Toshi::TPString8& a_FilePath, Toshi::TString8& a_rName );
	static Toshi::TString8       GenerateInstanceName( const Toshi::TPString8& a_FilePath );

private:
	inline static TUINT ms_uiNumCreated;

private:
	TUINT                                                 m_uiID;
	Toshi::TManagedModel*                                 m_pManagedModel;
	Toshi::TPString8                                      m_Name;
	Toshi::T2Vector<AModelInstanceRef, MAX_NUM_INSTANCES> m_vecInstanceRefs;
	Toshi::TVector3                                       m_Vec1;
	Toshi::TVector3                                       m_Vec2;
};
