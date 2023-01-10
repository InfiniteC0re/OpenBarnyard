#include "ToshiPCH.h"
#include "TRender.h"
#include "Toshi/File/TFile.h"
#include "Toshi2/T2ResourceManager.h"
#include "Toshi/Render/TModelManager.h"

namespace Toshi
{
	TRender::TRender()
	{
		// 0068ff70
		m_Unk1 = 0;
		m_bIsEnabled = true;
		m_ScreenOffset = { 0, 0 };
		m_Unk2 = 0;
		m_pRenderContext = TNULL;
		m_Unk5 = 0;
		m_HasDyingResources = false;

		// Initialize singletons
		T2ResourceManager::CreateSingleton(TRender::MAXNUMRESOURCES);
		TModelManager::CreateSingleton();

		TTODO("Create TRenderParamTable");

		// Reset array
		std::memset(m_SystemResources, '\0', sizeof(m_SystemResources));

		// Setting matrices up
		m_LightColour = {
			1.0, 1.0, 1.0, 0.0,
			0.0, 0.0, 0.0, 0.0,
			0.0, 0.0, 0.0, 0.0,
			0.0, 0.0, 0.0, 0.0
		};

		m_LightDirection = {
			-0.47, -0.74, 0.47, 0.0,
			0.0, 0.0, 0.0, 0.0,
			0.0, 0.0, 0.0, 0.0,
			0.0, 0.0, 0.0, 0.0
		};

		TIMPLEMENT();
	}

	bool TRender::Create()
	{
		TASSERT(TFALSE == IsCreated(), "TRender already created");
		TRenderContext* a_pRenderContext = CreateRenderContext();
		
		if (a_pRenderContext != TNULL)
		{
			SetCurrentRenderContext(a_pRenderContext);
			m_bCreated = true;
			return true;
		}
		return false;
	}

	bool TRender::Destroy()
	{
		return false;
	}

	bool TRender::CreateDisplay()
	{
		TASSERT(TTRUE == IsCreated(), "TRender must be created");
		TASSERT(TFALSE == IsDisplayCreated(), "Display already created");
		m_bDisplayCreated = true;
		return true;
	}

	void TRender::DestroyResource(TResource* resource)
	{
		// 00690fe0
		TASSERT(TNULL != resource->GetTree(), "Resource doesn't have a tree");
		TASSERT(TFALSE != resource->IsDead(), "Resource is already dead");

		if (resource->IsDying() == TFALSE)
		{
			m_HasDyingResources = true;
			resource->AddState(TResourceState_Dying);
			DestroyResourceRecurse(resource->GetAttached());
			resource->Invalidate();
		}
	}

	void TRender::DestroyResourceRecurse(TResource* resource)
	{
		// 00691280
		if (resource == TNULL)
		{
			TResource* lastResource = resource->GetLastResource();

			while (resource != TNULL)
			{
				TResource* nextResource = (resource != lastResource) ? resource->GetNextResource() : TNULL;

				if (resource->IsDying() == TFALSE)
				{
					m_HasDyingResources = true;
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

	TResource* TRender::CreateResource(TClass* pClass, char* name, TNodeTree<TResource>::TNode* parent)
	{
		TASSERT(pClass != TNULL, "TResource class is TNULL");
		TASSERT(pClass->IsA(TGetClass(TResource)), "TResource class is TNULL");

		TResource* pResource = static_cast<TResource*>(pClass->CreateObject());
		TASSERT(pResource != TNULL, "Couldn't create TResource");

		if (pResource != TNULL)
		{
			m_Resources.Remove(*pResource, false);

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

	void TRender::DumpStats()
	{
		TFile* file = TFile::Create("TRender.txt", TFile::FileMode_CreateNew);

		if (file != TNULL)
		{
			file->CPrintf("-\r\n");
			char const* name = GetName();
			uint16_t versionMajor = GetVersionMajor();
			uint16_t versionMinor = GetVersionMinor();
			file->CPrintf("Toshi rendering interface through object[\"%s\"] Version : %u.%u\r\n", name, versionMajor, versionMinor);
			file->CPrintf("-\r\n");

		}
	}

	void TRender::GetScreenOffset(TVector2* pOutVec)
	{
	}

	void TRender::SetScreenOffset(TVector2* pVec)
	{
	}

	void TRender::SetLightDirectionMatrix(TMatrix44* pMat)
	{
	}

	void TRender::SetLightColourMatrix(TMatrix44* pMat)
	{
	}

	bool TRender::CreateSystemResources()
	{
		return false;
	}

	void TRender::DestroySystemResources()
	{
	}

	float TRender::Update(float deltatime)
	{
		return 0.0f;
	}

	void TRender::BeginScene()
	{
	}

	void TRender::EndScene()
	{
	}
}