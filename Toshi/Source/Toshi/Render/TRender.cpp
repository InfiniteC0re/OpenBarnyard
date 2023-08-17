#include "ToshiPCH.h"
#include "TRender.h"
#include "Toshi/File/TFile.h"
#include "Toshi2/T2ResourceManager.h"
#include "Toshi/Render/TModelManager.h"

namespace Toshi {

	TRender::TRender()
	{
		// 0068ff70
		m_Unk1 = 0;
		m_bIsEnabled = TTRUE;
		m_ScreenOffset = { 0, 0 };
		m_eAspectRatio = ASPECT_RATIO_4_3;
		m_pRenderContext = TNULL;
		m_pCreatedRenderContext = TNULL;
		m_HasDyingResources = TFALSE;
		m_bDisplayCreated = TFALSE;
		m_bCreated = TFALSE;
		m_bInScene = TFALSE;
		// Create things
		T2ResourceManager::CreateSingleton(TRender::MAXNUMRESOURCES);
		m_ParamTable = new TRenderParamTable;
		TModelManager::CreateSingleton();

		// Reset array
		std::memset(m_SystemResources, '\0', sizeof(m_SystemResources));

		// Setting matrices up
		m_LightColour = {
			1.0f, 1.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 0.0f
		};

		m_LightDirection = {
			-0.47f, -0.74f, 0.47f, 0.0f,
			0.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 0.0f
		};
	}

	TRender::~TRender()
	{
		if (m_ParamTable)
			delete m_ParamTable;
		
		TModelManager::DestroySingleton();
		T2ResourceManager::DestroySingleton();
		m_Unk1 = -1;
	}

	TBOOL TRender::Create()
	{
		TASSERT(TFALSE == IsCreated(), "TRender already created");
		m_pCreatedRenderContext = CreateRenderContext();
		
		if (m_pCreatedRenderContext != TNULL)
		{
			SetCurrentRenderContext(m_pCreatedRenderContext);
			m_bCreated = TTRUE;
			return TTRUE;
		}

		return TFALSE;
	}

	TBOOL TRender::Destroy()
	{
		m_pRenderContext = TNULL;

		if (m_pCreatedRenderContext != TNULL)
		{
			delete m_pCreatedRenderContext;
			m_pCreatedRenderContext = TNULL;
		}

		m_bCreated = TFALSE;
		return TTRUE;
	}

	TBOOL TRender::CreateDisplay()
	{
		TASSERT(TTRUE == IsCreated(), "TRender must be created");
		TASSERT(TFALSE == IsDisplayCreated(), "Display already created");

		m_bDisplayCreated = TTRUE;
		return TTRUE;
	}

	void TRender::DestroyResource(TResource* resource)
	{
		// 00690fe0
		TASSERT(TNULL != resource->GetTree(), "Resource doesn't have a tree");
		TASSERT(TFALSE != resource->IsDead(), "Resource is already dead");

		if (resource->IsDying() == TFALSE)
		{
			m_HasDyingResources = TTRUE;
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
					m_HasDyingResources = TTRUE;
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

	void TRender::DumpStats()
	{
		TFile* file = TFile::Create("TRender.txt", TFile::FileMode_CreateNew);

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

	void TRender::GetScreenOffset(TVector2& a_rVec)
	{
		a_rVec.Set(m_ScreenOffset);
	}

	void TRender::SetScreenOffset(const TVector2& a_rVec)
	{
		m_ScreenOffset.Set(a_rVec);
	}

	void TRender::SetLightDirectionMatrix(const TMatrix44& a_rMatrix)
	{
		m_LightDirection = a_rMatrix;
	}

	void TRender::SetLightColourMatrix(const TMatrix44& a_rMatrix)
	{
		m_LightColour = a_rMatrix;
	}

	TBOOL TRender::CreateSystemResources()
	{
		return TTRUE;
	}

	void TRender::DestroySystemResources()
	{
		DestroyResourceRecurse(m_Resources.AttachedToRoot());
		FlushDyingResources();
	}

	void TRender::DestroyDyingResources()
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

	void TRender::DestroyDyingResources(TResource* resources)
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

	void TRender::DeleteResource(TResource* resources)
	{
		DeleteResourceRecurse(resources->Attached());
		DeleteResourceAtomic(resources);
	}

	void TRender::DeleteResourceRecurse(TResource* resources)
	{
		auto res = resources;
		while (resources != TNULL)
		{
			auto next = res->Next() == resources ? TNULL : res->Next();
			DeleteResourceAtomic(resources);
			resources = next;
		}
	}

	void TRender::DeleteResourceAtomic(TResource* a_pResource)
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

	void TRender::Update(float deltatime)
	{
		FlushDyingResources();
	}

	void TRender::BeginScene()
	{
		TASSERT(TTRUE == IsCreated());
		TASSERT(TTRUE == IsDisplayCreated());

		m_iFrameCount += 1;
		m_Transforms.Reset();
		m_Transforms.Top().Identity();
	}

	void TRender::EndScene()
	{
		TASSERT(TTRUE == IsCreated());
		TASSERT(TTRUE == IsDisplayCreated());
	}

	TRenderContext::TRenderContext(TRender* pRender)
	{
		TIMPLEMENT();
		m_eFlags = 0;
		m_pRender = pRender;

		m_oParams.fX = 0;
		m_oParams.fY = 0;
		m_oParams.fMaxZ = 1.0f;
		m_oParams.fMinZ = 1.0f;

		m_eCameraMode = CameraMode_Perspective;
		m_ProjParams.m_fNearClip = 1.0f;
		m_ProjParams.m_fFarClip = 1000.0f;
		m_oModelViewMatrix.Identity();
		m_oWorldViewMatrix.Identity();

		auto pDevice = pRender->GetCurrentDevice();
		
		if (pDevice == TNULL)
		{
			m_oParams.fWidth = 640.0f;
			m_oParams.fHeight = 480.0f;
		}
		else
		{
			TASSERT(TFALSE, "Not used in De blob");
		}
	}

	void TRenderContext::SetModelViewMatrix(const TMatrix44& a_rMatrix)
	{
		m_eFlags |= (FLAG_DIRTY_WORLDMODELMATRIX | FLAG_DIRTY_VIEWMODELMATRIX);
		m_oModelViewMatrix = a_rMatrix;
		m_eFlags &= ~(FLAG_HAS_MODELWORLDMATRIX | FLAG_UNK3);
		
		TRender::GetSingleton()->GetParamTable()->SetParameterM44(TRenderParamTable::M44PARAM_MODELVIEW, a_rMatrix);
	}

	void TRenderContext::SetWorldViewMatrix(const TMatrix44& a_rMatrix)
	{
		m_eFlags |= FLAG_DIRTY_VIEWMODELMATRIX;
		m_oWorldViewMatrix = a_rMatrix;
		m_eFlags &= ~(FLAG_HAS_MODELWORLDMATRIX | FLAG_HAS_VIEWWORLDMATRIX | FLAG_UNK4 | FLAG_HAS_WORLDPLANES | FLAG_UNK6);
	}

	void TRenderContext::SetProjectionParams(const PROJECTIONPARAMS& params)
	{
		TASSERT(params.m_Proj.x != 0.0f);
		TASSERT(params.m_Proj.y != 0.0f);
		TASSERT(TMath::IsFinite(params.m_Proj.x) && (!TMath::IsNaN(params.m_Proj.x)));
		TASSERT(TMath::IsFinite(params.m_Proj.y) && (!TMath::IsNaN(params.m_Proj.y)));
		TASSERT(TMath::IsFinite(params.m_Centre.x) && (!TMath::IsNaN(params.m_Centre.x)));
		TASSERT(TMath::IsFinite(params.m_Centre.y) && (!TMath::IsNaN(params.m_Centre.y)));

		m_ProjParams = params;
		m_eFlags = (m_eFlags & (~(FLAG_UNK3 | FLAG_UNK4 | FLAG_HAS_WORLDPLANES | FLAG_UNK6))) | FLAG_DIRTY;
	}

	const TPlane* TRenderContext::GetWorldPlanes()
	{
		if (!HASFLAG(m_eFlags & FLAG_HAS_WORLDPLANES))
		{
			auto& viewWorld = GetViewWorldMatrix();

			for (size_t i = 0; i < 6; i++)
			{
				TMatrix44::TransformPlaneOrthogonal(m_aWorldPlanes[i], viewWorld, m_aFrustumPlanes1[i]);
			}

			m_eFlags |= FLAG_HAS_WORLDPLANES;
		}

		return m_aWorldPlanes;
	}

	const TMatrix44& TRenderContext::GetViewWorldMatrix()
	{
		if (!HASFLAG(m_eFlags & FLAG_HAS_VIEWWORLDMATRIX))
		{
			m_oViewWorldMatrix.InvertOrthogonal(m_oWorldViewMatrix);
			m_eFlags |= FLAG_HAS_VIEWWORLDMATRIX;
		}

		return m_oViewWorldMatrix;
	}

	const TMatrix44& TRenderContext::GetModelWorldMatrix()
	{
		if (!HASFLAG(m_eFlags & FLAG_HAS_MODELWORLDMATRIX))
		{
			m_oModelWorldMatrix.Multiply(GetViewWorldMatrix(), m_oModelViewMatrix);
			m_eFlags |= FLAG_HAS_MODELWORLDMATRIX;
		}

		TRender::GetSingleton()->GetParamTable()->SetParameterM44(TRenderParamTable::M44PARAM_MODELWORLD, m_oModelWorldMatrix);
		return m_oModelWorldMatrix;
	}

	const TMatrix44& TRenderContext::GetViewModelMatrix()
	{
		if (HASFLAG(m_eFlags & FLAG_DIRTY_VIEWMODELMATRIX))
		{
			m_oViewModelMatrix.Invert(m_oModelViewMatrix);
			m_eFlags &= ~FLAG_DIRTY_VIEWMODELMATRIX;
		}
		
		return m_oViewModelMatrix;
	}

	const TMatrix44& TRenderContext::GetWorldModelMatrix()
	{
		if (HASFLAG(m_eFlags & FLAG_DIRTY_WORLDMODELMATRIX))
		{
			m_oViewModelMatrix.Invert(m_oModelViewMatrix);
			m_eFlags &= ~FLAG_DIRTY_WORLDMODELMATRIX;
		}

		return m_oWorldModelMatrix;
	}

}