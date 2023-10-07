#include "ToshiPCH.h"
#include "TRenderInterface.h"
#include "Toshi/File/TFile.h"
#include "Toshi2/T2ResourceManager.h"
#include "Toshi/Render/TModelManager.h"

namespace Toshi {

	TRenderInterface::TRenderInterface()
	{
		m_Unk1 = 0;
		m_bUnkFlag = TTRUE;
		m_bInScene = TFALSE;
		m_bCreated = TFALSE;
		m_bDisplayCreated = TFALSE;
		m_ScreenOffset.Set(0.0f, 0.0f);
		m_pCurrentContext = TNULL;
		m_pCreatedContext = TNULL;
		m_pDebugText = TNULL;
		m_Unk2 = TNULL;
		m_Unk3 = TNULL;
		m_ResourceCount = 1;
		m_iFrameCount = 0;
		m_bHasDyingResources = TFALSE;

		m_Transforms.PushNull();
		m_Transforms.Top().Identity();

		TUtil::MemClear(m_SystemResources, sizeof(m_SystemResources));

		m_LightColour.Set(
			1.0f, 1.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 0.0f
		);

		m_LightDirection.Set(
			-0.47f, -0.74f, 0.47f, 0.0f,
			0.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 0.0f
		);
	}

	TRenderInterface::~TRenderInterface()
	{
		m_Unk1 = -1;
	}

	TBOOL TRenderInterface::Create()
	{
		TASSERT(TFALSE == IsCreated(), "TRender already created");
		m_pCreatedContext = CreateRenderContext();
		
		if (m_pCreatedContext)
		{
			SetCurrentRenderContext(m_pCreatedContext);
			m_Unk2 = CreateUnknown("Render", 0, 0, 10);
			m_bCreated = TTRUE;
			return TTRUE;
		}

		return TFALSE;
	}

	TBOOL TRenderInterface::Destroy()
	{
		TASSERT(TTRUE == IsCreated(), "TRender is not created");

		if (m_Unk2)
		{
			delete m_Unk2;
			m_Unk2 = TNULL;
		}

		if (m_pCreatedContext)
		{
			delete m_pCreatedContext;
			m_pCreatedContext = TNULL;
		}

		m_pCurrentContext = TNULL;
		m_bCreated = TFALSE;

		return TTRUE;
	}

	void TRenderInterface::RenderIndexPrimitive(int param_2, int param_3, int param_4, int param_5, int param_6, int param_7)
	{

	}

	TBOOL TRenderInterface::CreateDisplay()
	{
		TASSERT(TTRUE == IsCreated(), "TRender must be created");
		TASSERT(TFALSE == IsDisplayCreated(), "Display already created");

		m_bDisplayCreated = TTRUE;
		return TTRUE;
	}

	void TRenderInterface::DestroyResource(TResource* resource)
	{
		TASSERT(TNULL != resource->GetTree(), "Resource doesn't have a tree");
		TASSERT(TFALSE != resource->IsDead(), "Resource is already dead");

		if (resource->IsDying() == TFALSE)
		{
			m_bHasDyingResources = TTRUE;
			resource->AddState(TResourceState_Dying);
			DestroyResourceRecurse(resource->GetAttached());
			resource->Invalidate();
		}
	}

	void TRenderInterface::DestroyResourceRecurse(TResource* resource)
	{
		if (resource == TNULL)
		{
			TResource* lastResource = resource->GetLastResource();

			while (resource != TNULL)
			{
				TResource* nextResource = (resource != lastResource) ? resource->GetNextResource() : TNULL;

				if (resource->IsDying() == TFALSE)
				{
					m_bHasDyingResources = TTRUE;
					resource->AddState(TResourceState_Dying);

					if (resource->GetAttached() != TNULL)
					{
						DestroyResourceRecurse(resource->GetAttached());
					}

					resource->Invalidate();
				}

				resource = nextResource;
			}
		}
	}

	TResource* TRenderInterface::CreateResource(TClass* pClass, char* name, TNodeTree<TResource>::TNode* parent)
	{
		TASSERT(pClass != TNULL, "TResource class is TNULL");
		TASSERT(pClass->IsA(TGetClass(TResource)), "TResource class is TNULL");

		TResource* pResource = static_cast<TResource*>(pClass->CreateObject());
		TASSERT(pResource != TNULL, "Couldn't create TResource");

		if (pResource != TNULL)
		{
			m_Resources.Remove(*pResource, TFALSE);

			if (parent == TNULL)
			{
				parent = m_Resources.GetRoot();
			}

			m_Resources.Insert(parent, pResource);
			pResource->SetUId(m_ResourceCount);
			pResource->SetRenderer(this);
			pResource->SetName(name);
			m_ResourceCount += 1;

			return pResource;
		}

		return pResource;
	}

	void TRenderInterface::DumpStats()
	{
		TIMPLEMENT();

		TFile* file = TFile::Create("TRenderInterface.txt", TFile::FileMode_CreateNew);

		if (file != TNULL)
		{
			file->CPrintf("-\r\n");
			char const* name = GetClass()->GetName();
			uint16_t versionMajor = GetClass()->GetVersionMajor();
			uint16_t versionMinor = GetClass()->GetVersionMinor();
			file->CPrintf("Toshi rendering interface through object[\"%s\"] Version : %u.%u\r\n", name, versionMajor, versionMinor);
			file->CPrintf("-\r\n");
		}
	}

	void TRenderInterface::GetScreenOffset(TVector2& a_rVec)
	{
		a_rVec.Set(m_ScreenOffset);
	}

	void TRenderInterface::SetScreenOffset(const TVector2& a_rVec)
	{
		m_ScreenOffset.Set(a_rVec);
	}

	float TRenderInterface::GetScreenAspectRatio()
	{
		return 4.0f / 3.0f;
	}

	float TRenderInterface::GetPixelAspectRatio()
	{
		return 1.0f;
	}

	TBOOL TRenderInterface::SetPixelAspectRatio(float a_fPixelAspectRatio)
	{
		return TFALSE;
	}

	void TRenderInterface::FlushOrderTables()
	{

	}

	TBOOL TRenderInterface::IsTextureFormatSupported(int a_eTextureFormat)
	{
		return TTRUE;
	}

	TBOOL TRenderInterface::Supports32BitTextures()
	{
		return TFALSE;
	}

	void TRenderInterface::SetLightDirectionMatrix(const TMatrix44& a_rMatrix)
	{
		m_LightDirection = a_rMatrix;
	}

	void TRenderInterface::SetLightColourMatrix(const TMatrix44& a_rMatrix)
	{
		m_LightColour = a_rMatrix;
	}

	void TRenderInterface::Unknown1(int a_iUnknown)
	{

	}

	void TRenderInterface::OnInitializationFailureDevice()
	{

	}

	void TRenderInterface::OnInitializationFailureDisplay()
	{

	}

	TBOOL TRenderInterface::CreateSystemResources()
	{
		return TTRUE;
	}

	void TRenderInterface::DestroySystemResources()
	{
		DestroyResourceRecurse(m_Resources.AttachedToRoot());
		FlushDyingResources();
	}

	void TRenderInterface::DestroyDyingResources()
	{
		auto attached = m_Resources.GetRoot()->Attached();

		if (attached != TNULL)
		{
			auto prev = attached->Prev() == attached ? TNULL : attached->Prev();
			if (attached->IsDying())
			{
				DestroyDyingResources(attached);
			}
			else
			{
				DeleteResource(prev);
			}
		}
	}

	void TRenderInterface::DestroyDyingResources(TResource* resources)
	{
		auto res = resources;

		while (resources != TNULL)
		{
			auto next = res->Next() == resources ? TNULL : res->Next();
			if (res->IsDying())
			{
				DestroyDyingResources(res->Attached());
			}
			else
			{
				if (res == resources)
				{
					resources = next;
				}
				DeleteResource(next);
				res = next;
			}
		}
	}

	void TRenderInterface::DeleteResource(TResource* resources)
	{
		DeleteResourceRecurse(resources->Attached());
		DeleteResourceAtomic(resources);
	}

	void TRenderInterface::DeleteResourceRecurse(TResource* resources)
	{
		auto res = resources;
		while (resources != TNULL)
		{
			auto next = res->Next() == resources ? TNULL : res->Next();
			DeleteResourceAtomic(resources);
			resources = next;
		}
	}

	void TRenderInterface::DeleteResourceAtomic(TResource* a_pResource)
	{
		auto attached = a_pResource->Attached();

		if (attached != TNULL)
		{
			DeleteResourceRecurse(attached);
		}

		TASSERT(TFALSE == a_pResource->IsValid());

		if (a_pResource->IsCreated())
		{
			a_pResource->OnDestroy();
		}

		m_Resources.Remove(a_pResource, TFALSE);
		delete a_pResource;
	}

}