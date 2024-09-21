#pragma once
#include <File/TTRB.h>
#include <Toshi/TRefCounted.h>
#include <Render/TModelManager.h>
#include <Toshi/TPString8.h>

#include <Toshi/T2DList.h>
#include <Toshi/T2Vector.h>
#include <Toshi/TSceneObject.h>

class AModel;

class AModelInstance :
    public Toshi::TObject,
    public Toshi::TRefCounted
{
public:
	TDECLARE_CLASS( AModelInstance, Toshi::TObject );

public:
	using Flags = TUINT8;
	enum Flags_ : Flags
	{
		Flags_None             = 0,
		Flags_UpdatingSkeleton = BITFLAG( 0 ),
		Flags_Unknown1    = BITFLAG( 2 ),
		Flags_ReceivesLight    = BITFLAG( 3 ),
	};

	enum ChangeEvent : TUINT32
	{
		ChangeEvent_UNK1,
		ChangeEvent_UNK2,
		ChangeEvent_EnabledSkeletonUpdate,
		ChangeEvent_DisabledSkeletonUpdate,
	};

	friend class AModel;

public:
	AModelInstance();
	AModelInstance( AModel* a_pModel, Toshi::TSceneObject* a_pSceneObject, TBOOL a_bEnableSkeletonUpdate );

	void SetSkeletonUpdating( TBOOL a_bUpdating );

	AModel*              GetModel() const { return m_pModel; }
	Toshi::TSceneObject* GetSceneObject() const { return m_pSceneObject; }
	TUINT                GetClipFlags() const { return m_uiClipFlags; }

	const Toshi::TVector4& GetScale() const { return m_Scale; }

	TBOOL IsUpdatingSkeleton() const { return m_eFlags & Flags_UpdatingSkeleton; }
	TBOOL IsUnknown1() const { return m_eFlags & Flags_Unknown1; }
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

class AModel :
    public Toshi::T2DList<AModel>::Node
{
public:
	static constexpr TUINT32 MAX_NUM_INSTANCES = 250;

public:
	AModel( const Toshi::TPString8& a_rName, Toshi::TTRB* a_pTRB );
	virtual ~AModel();

	void Update( TFLOAT a_fDeltaTime );
	void Render( TUINT8 a_uiFlags );

	AModelInstanceRef* CreateInstance( AModelInstanceRef& a_rOutRef );

	TSIZE GetNumInstances() const;

	const Toshi::TPString8& GetName() const { return m_Name; }

public:
	static Toshi::TModelPtr* Create( const Toshi::TPString8& a_rFilePath, Toshi::TTRB* a_pTRB );
	static void              GetNameFromPath( const Toshi::TPString8& a_FilePath, Toshi::TString8& a_rName );
	static Toshi::TString8*  GenerateInstanceName( Toshi::TString8& a_rOutName, const Toshi::TPString8& a_FilePath );

private:
	inline static TUINT ms_uiNumCreated;

private:
	TUINT                                                 m_uiID;
	Toshi::TModelPtr*                                     m_pModelPtr;
	Toshi::TPString8                                      m_Name;
	Toshi::T2Vector<AModelInstanceRef, MAX_NUM_INSTANCES> m_vecInstanceRefs;
	Toshi::TVector3                                       m_Vec1;
	Toshi::TVector3                                       m_Vec2;
};
