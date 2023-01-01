#include "ToshiPCH.h"
#include "TRender.h"
#include "TResource.h"
#include "Toshi/File/TFile.h"

namespace Toshi
{
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
			m_UnkFlag1 = true;
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
					m_UnkFlag1 = true;
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

	void TRender::DumpStats()
	{
		TFile* file = TFile::Create("TRender.txt", TFile::OpenFlags_CreateNew);

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