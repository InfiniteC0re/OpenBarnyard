#include "ToshiPCH.h"
#include "TRenderInterface.h"
#include "Toshi/File/TFile.h"

namespace Toshi {

	TRenderInterface::TRenderInterface()
	{
		m_Unk1 = 0;
		m_bCreateSystemResources = TTRUE;
		m_bInScene = TFALSE;
		m_bCreated = TFALSE;
		m_bDisplayCreated = TFALSE;
		m_ScreenOffset.Set(0.0f, 0.0f);
		m_pCurrentContext = TNULL;
		m_pCreatedContext = TNULL;
		m_pDebugText = TNULL;
		m_Unk2 = TNULL;
		m_pInvalidTexture = TNULL;
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

	TBOOL TRenderInterface::CreateDisplay(const DISPLAYPARAMS& a_rParams)
	{
		m_bDisplayCreated = TTRUE;
		return TTRUE;
	}

	TBOOL TRenderInterface::BeginScene()
	{
		m_iFrameCount += 1;
		m_Transforms.Reset();
		m_Transforms.Top().Identity();
		return TTRUE;
	}

	void TRenderInterface::DestroyResource(TResource* resource)
	{
		TASSERT(TNULL != resource->GetTree(), "Resource doesn't have a tree");
		TASSERT(TFALSE == resource->IsDead(), "Resource is already dead");

		if (resource->IsDying() == TFALSE)
		{
			m_bHasDyingResources = TTRUE;
			resource->AddState(TResourceState_Dying);
			DestroyResourceRecurse(resource->GetChild());
			resource->Invalidate();
		}
	}

	void TRenderInterface::DestroyResourceRecurse(TResource* resource)
	{
		if (resource != TNULL)
		{
			TResource* lastResource = resource->GetLastResource();

			while (resource != TNULL)
			{
				TResource* nextResource = (resource != lastResource) ? resource->GetNextResource() : TNULL;

				if (resource->IsDying() == TFALSE)
				{
					m_bHasDyingResources = TTRUE;
					resource->AddState(TResourceState_Dying);

					if (resource->GetChild() != TNULL)
					{
						DestroyResourceRecurse(resource->GetChild());
					}

					resource->Invalidate();
				}

				resource = nextResource;
			}
		}
	}

	TResource* TRenderInterface::CreateResource(TClass* pClass, const char* name, TResource* parent)
	{
		TASSERT(pClass != TNULL, "TResource class is TNULL");
		TASSERT(pClass->IsA(&TGetClass(TResource)), "TResource class is TNULL");

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

			for (auto it = m_AdapterList.Begin(); it != m_AdapterList.End(); it++)
			{
				file->CPrintf("Adapter       : %d\r\n", it->GetAdapterIndex());
				file->CPrintf("  Driver      : %s\r\n", it->GetDriver());
				file->CPrintf("  Description : %s\r\n", it->GetDriverDescription());
				file->CPrintf("  Product ID  : %u\r\n", it->GetProductID());
				file->CPrintf("  Version     : %u\r\n", it->GetVersion());
				file->CPrintf("  Sub-version : %u\r\n", it->GetSubVersion());
				file->CPrintf("  Build       : %u\r\n", it->GetBuild());
				file->CPrintf("  Supports %d modes:\r\n", it->GetModeList()->Count());
			}

			file->Destroy();
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

	void TRenderInterface::Unknown1(TViewport* a_pViewport)
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
		DestroyResourceRecurse(m_Resources.ChildOfRoot());
		FlushDyingResources();
	}

	void TRenderInterface::DestroyAllShaderResources()
	{
		TIMPLEMENT();
	}

	void TRenderInterface::DestroyDyingResources(TResource* resources)
	{
		// TODO: refactor
		TResource* pTVar1;
		TResource* next;
		TResource* pRes1;
		TResource* pRes2;

		pRes1 = resources;
		pRes2 = resources;
		if (resources != (TResource*)0x0) {
			do {
				next = pRes2->GetNextResource();
				if (next == pRes1) {
					next = (TResource*)0x0;
				}
				if ((pRes2->m_State & 4) == 0) {
					pRes2 = pRes2->GetChild();
					if (pRes2 != (TResource*)0x0) {
						DestroyDyingResources(pRes2);
					}
				}
				else {
					if (pRes2 == pRes1) {
						pRes1 = next;
						resources = next;
					}
					pTVar1 = pRes2->GetChild();
					while (pTVar1 != (TResource*)0x0) {
						pRes1 = pTVar1->GetNextResource();
						if (pRes1 == pTVar1) {
							pRes1 = (TResource*)0x0;
						}
						DeleteResourceAtomic(pTVar1);
						pTVar1 = pRes1;
						pRes1 = resources;
					}
					DeleteResourceAtomic(pRes2);
				}
				pRes2 = next;
			} while (next != (TResource*)0x0);
		}
	}

	void TRenderInterface::DeleteResource(TResource* resources)
	{
		DeleteResourceRecurse(resources->Child());
		DeleteResourceAtomic(resources);
	}

	void TRenderInterface::DeleteResourceRecurse(TResource* resource)
	{
		while (resource)
		{
			auto next = resource->GetNextResource();
			if (next == resource) next = TNULL;

			DeleteResourceAtomic(resource);
			resource = next;
		}
	}

	void TRenderInterface::DeleteResourceAtomic(TResource* a_pResource)
	{
		if (a_pResource)
		{
			DeleteResourceRecurse(a_pResource->Child());

			TASSERT(TFALSE == a_pResource->IsValid());

			if (a_pResource->IsCreated())
			{
				a_pResource->OnDestroy();
			}

			m_Resources.Remove(a_pResource, TFALSE);

			if (a_pResource->IsExternal())
			{
				a_pResource->~TResource();
			}
			else
			{
				delete a_pResource;
			}
		}
	}

	TRenderAdapter::Mode::Device* TRenderInterface::FindDevice(const DISPLAYPARAMS& a_rDisplayParams)
	{
		auto pAdapter = GetAdapterList()->Begin();

		while (TTRUE)
		{
			if (pAdapter == GetAdapterList()->End())
			{
				TString8 errorString = "Unable to find the passed device!\n";
#ifdef TOSHI_DEBUG
				OutputDebugStringA(errorString);
#endif
				return TNULL;
			}

			for (auto pMode = pAdapter->GetModeList()->Begin(); pMode != pAdapter->GetModeList()->End(); pMode++)
			{
				auto uiNumSupportedDevices = pAdapter->GetNumSupportedDevices();

				for (TUINT32 i = 0; i < uiNumSupportedDevices; i++)
				{
					auto pDevice = pMode->GetDevice(i);

					if (pDevice->IsSoftware()) continue;

					TBOOL bPassedColourDepth = TTRUE;

					if (a_rDisplayParams.uiColourDepth == 32)
					{
						if (!pDevice->GetMode()->Is32Bit())
						{
							bPassedColourDepth = TFALSE;
						}
					}
					else if (a_rDisplayParams.uiColourDepth == 16)
					{
						if (!pDevice->GetMode()->Is16Bit())
						{
							bPassedColourDepth = TFALSE;
						}
					}

					TBOOL bPassedWindowed = TTRUE;

					if (a_rDisplayParams.bWindowed && !pDevice->CanRenderWindowed())
					{
						bPassedWindowed = TFALSE;
					}

					auto uiWidth = pDevice->GetMode()->GetWidth();
					auto uiHeight = pDevice->GetMode()->GetHeight();

					if (uiHeight < uiWidth || !a_rDisplayParams.bWindowed || a_rDisplayParams.uiWidth <= a_rDisplayParams.uiHeight)
					{
						if (uiWidth == a_rDisplayParams.uiWidth && uiHeight == a_rDisplayParams.uiHeight)
						{
							if (pDevice->IsDepthStencilFormatSupported(a_rDisplayParams.eDepthStencilFormat) &&
								bPassedColourDepth && bPassedWindowed)
							{
								return pDevice;
							}
						}
					}
					else
					{
						if (a_rDisplayParams.uiWidth <= uiWidth)
						{
							if (uiHeight < a_rDisplayParams.uiHeight) continue;

							if (pDevice->IsDepthStencilFormatSupported(a_rDisplayParams.eDepthStencilFormat) &&
								bPassedColourDepth && bPassedWindowed)
							{
								return pDevice;
							}
						}
					}
				}
			}

			pAdapter++;
		}
	}

}