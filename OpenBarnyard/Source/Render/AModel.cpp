#include "pch.h"
#include "AModel.h"

TOSHI_NAMESPACE_USING

AModel::AModel(const TPString8& a_rName, TTRB* a_pTRB)
{
	m_uiID = ms_uiNumCreated;
	m_pModelPtr = TNULL;
	m_Name = a_rName;
	m_uiNumModelInstances = 0;
	m_Vec1 = TVector3::VEC_ZERO;
	m_Vec2 = TVector3::VEC_ZERO;
	m_pModelPtr = AModel::Create(a_rName, a_pTRB);

	ms_uiNumCreated += 1;
}

AModel::~AModel()
{
	if (m_pModelPtr)
	{
		delete m_pModelPtr;
	}
}

void AModel::Update(TFLOAT a_fDeltaTime)
{
	TIMPLEMENT();

	for (TUINT i = 0; i < m_uiNumModelInstances; i++)
	{
		auto pInstance = m_aModelInstances[i].Get();

		if (pInstance->IsUpdatingSkeleton())
		{
			pInstance->GetT2Instance()->Update(a_fDeltaTime);
		}
	}
}

AModelInstanceRef* AModel::CreateInstance(AModelInstanceRef& a_rOutRef)
{
	// TODO: use this name in debug?
	TString8 instanceName;
	GenerateInstanceName(instanceName, m_Name);

	AModelInstanceRef modelInstanceRef = new AModelInstance(
		this,
		m_pModelPtr->CreateInstance(),
		TFALSE
	);

	m_aModelInstances[m_uiNumModelInstances++] = modelInstanceRef;
	a_rOutRef = modelInstanceRef;

	return &a_rOutRef;
}

TModelPtr* AModel::Create(const TPString8& a_rFilePath, TTRB* a_pTRB)
{
	// TODO: use this name in debug?
	TString8 modelName;
	GetNameFromPath(a_rFilePath, modelName);

	auto pModelPtr = new TModelPtr();

	if (pModelPtr)
	{
		TString8 filepath = a_rFilePath.GetString8();
		filepath.MakeLower();

		if (filepath.Find(".trb") < 0 &&
			filepath.Find(".trz") < 0)
		{
			filepath += ".trb";
		}

		pModelPtr->Create(filepath, a_pTRB);
		TFIXME("Call FUN_0060dcc0");
	}

	return pModelPtr;
}

void AModel::GetNameFromPath(const TPString8& a_FilePath, TString8& a_rName)
{
	TString8 name = a_FilePath.GetString8();

	if (name.Length() <= 8)
	{
		a_rName = name;
	}
	else
	{
		TUINT iPos1 = 0;
		TUINT iPos2;

		do {
			iPos2 = iPos1;
			iPos1 = name.Find('\\', iPos1 + 1);
		} while (-1 < iPos1);

		do {
			iPos1 = iPos2;
			iPos2 = name.Find('/', iPos1 + 1);
		} while (-1 < iPos2);

		if (iPos1 < 0 || name.Length() - 1 <= iPos1) iPos1 = 0;
		iPos1 += 1;
		iPos2 = name.Find('_', iPos1);
		if (iPos2 < 0) iPos2 = name.Length() - 1;

		TUINT iSize = iPos2 - iPos1;
		
		if (iSize < 1)
		{
			iPos1 = 0;
			iSize = 8;
		}
		else if (8 < iSize)
		{
			iSize = 8;
		}

		a_rName.Copy(name.GetString(iPos1), iSize);
	}
}

TString8* AModel::GenerateInstanceName(TString8& a_rOutName, const TPString8& a_FilePath)
{
	static TUINT ms_uiDebugObjectIndex = 0;

	TString8 name;
	GetNameFromPath(a_FilePath, name);

	TString8 debugName = TString8::Format("%s%i", name.GetString(), ms_uiDebugObjectIndex % 1000);
	ms_uiDebugObjectIndex += 1;

	a_rOutName = debugName;
	return &a_rOutName;
}

AModelInstance::AModelInstance(AModel* a_pModel, T2ModelInstance* a_pT2Instance, TBOOL a_bEnableSkeletonUpdate) :
	m_ChangeEmitter(this)
{
	TFIXME("Initialise some unknown members");

	m_Unknown1[0] = 1.0f;
	m_Unknown1[1] = 1.0f;
	m_Unknown1[2] = 1.0f;
	m_Unknown1[3] = 1.0f;
	m_pModel = a_pModel;
	m_pT2ModelInstance = a_pT2Instance;
	m_eFlags = 0b00001001;

	SetSkeletonUpdating(a_bEnableSkeletonUpdate);
	m_pT2ModelInstance->GetInstance()->SetCustomRenderMethod(RenderInstanceCallback, this);
}

AModelInstance::AModelInstance()
{
	TFIXME("Initialise some unknown members");

	m_Unknown1[0] = 1.0f;
	m_Unknown1[1] = 1.0f;
	m_Unknown1[2] = 1.0f;
	m_Unknown1[3] = 1.0f;
	m_pModel = TNULL;
	m_pT2ModelInstance = TNULL;
	m_eFlags = 0b00011000;
}

void AModelInstance::RenderInstanceCallback(TModelInstance* a_pInstance, void* a_pUserData)
{
	TIMPLEMENT();
}

void AModelInstance::SetSkeletonUpdating(TBOOL a_bUpdating)
{
	if (a_bUpdating)
	{
		if (!IsUpdatingSkeleton())
		{
			m_pT2ModelInstance->EnableSkeletonUpdate();
			m_ChangeEmitter.Throw(ChangeEvent_EnabledSkeletonUpdate);
		}

		m_eFlags |= Flags_UpdatingSkeleton;
	}
	else
	{
		if (IsUpdatingSkeleton())
		{
			m_pT2ModelInstance->DisableSkeletonUpdate();
			m_ChangeEmitter.Throw(ChangeEvent_DisabledSkeletonUpdate);
		}

		m_eFlags &= ~Flags_UpdatingSkeleton;
	}
}
