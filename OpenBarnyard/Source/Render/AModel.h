#pragma once
#include <Toshi/File/TTRB.h>
#include <Toshi/Core/TRefCounted.h>
#include <Toshi/Render/TModelRegistry.h>
#include <Toshi/Strings/TPString8.h>

#include <Toshi2/T2DList.h>
#include <Toshi2/T2ModelInstance.h>

class AModel;

TOBJECT(AModelInstance, Toshi::TObject, TTRUE),
	public Toshi::TRefCounted
{
public:
	using Flags = TUINT8;
	enum Flags_ : Flags
	{
		Flags_None = 0,
		Flags_UpdatingSkeleton = BITFIELD(0),
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
	AModelInstance(AModel* a_pModel, Toshi::T2ModelInstance* a_pT2Instance, TBOOL a_bEnableSkeletonUpdate);

	void SetSkeletonUpdating(TBOOL a_bUpdating);

	AModel* GetModel() const { return m_pModel; }
	Toshi::T2ModelInstance* GetT2Instance() const { return m_pT2ModelInstance; }
	TUINT GetClipFlags() const { return m_uiClipFlags; }

	TBOOL IsUpdatingSkeleton() const { return m_eFlags & Flags_UpdatingSkeleton; }

private:
	static void RenderInstanceCallback(Toshi::TModelInstance* a_pInstance, void* a_pUserData);

private:
	TFLOAT m_Unknown1[4];
	AModel* m_pModel;
	TUINT m_uiClipFlags;
	// ...
	Toshi::T2ModelInstance* m_pT2ModelInstance;
	Toshi::TEmitter<AModelInstance, ChangeEvent> m_ChangeEmitter;
	Flags m_eFlags;
};

using AModelInstanceRef = Toshi::TRef<AModelInstance>;

class AModel :
	public Toshi::T2DList<AModel>::Node
{
public:
	static constexpr TUINT32 MAX_NUM_INSTANCES = 250;

public:
	AModel(const Toshi::TPString8& a_rName, Toshi::TTRB* a_pTRB);
	virtual ~AModel();

	void Update(TFLOAT a_fDeltaTime);
	void Render(TUINT8 a_uiFlags);

	AModelInstanceRef* CreateInstance(AModelInstanceRef& a_rOutRef);

	const Toshi::TPString8& GetName() const { return m_Name; }

public:
	static Toshi::TModelPtr* Create(const Toshi::TPString8& a_rFilePath, Toshi::TTRB* a_pTRB);
	static void GetNameFromPath(const Toshi::TPString8& a_FilePath, Toshi::TString8& a_rName);
	static Toshi::TString8* GenerateInstanceName(Toshi::TString8& a_rOutName, const Toshi::TPString8& a_FilePath);

private:
	inline static TUINT ms_uiNumCreated;

private:
	TUINT m_uiID;
	Toshi::TModelPtr* m_pModelPtr;
	Toshi::TPString8 m_Name;
	TUINT m_uiNumModelInstances;
	AModelInstanceRef m_aModelInstances[MAX_NUM_INSTANCES];
	Toshi::TVector3 m_Vec1;
	Toshi::TVector3 m_Vec2;
};
