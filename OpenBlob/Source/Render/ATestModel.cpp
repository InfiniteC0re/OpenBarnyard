#include "pch.h"
#include "ATestModel.h"

#include <d3dtypes.h>
#include <Toshi/Render/TAssetInit.h>
#include <Toshi/Shaders/SysShader/TSysShaderHAL.h>

struct SysMeshVertex
{
    Toshi::TVector3 Position;
    uint32_t Color;
    Toshi::TVector2 UV;
};

ATestModel::ATestModel()
{
    m_Position = { 0.0f, 0.0f, 6.0f, 1.0f };

    m_AssetTRB.Load("Data\\LEVELS\\PRODUCTION\\Singleplayer\\Abyss\\RegionAssets.trb");
    Toshi::TAssetInit::InitAssets(m_AssetTRB, TTRUE, TFALSE);

    CreateCube();
    CreatePlane();
}

ATestModel::~ATestModel()
{
    Toshi::TAssetInit::DeinitAssets(m_AssetTRB);

    m_pCube->DestroyResource();
    m_pPlane->DestroyResource();
    delete m_pCubeMaterial;
    delete m_pPlaneMaterial;
}

void ATestModel::Render(float deltaTime)
{
    auto pRenderContext = Toshi::TRender::GetSingletonWeak()->GetCurrentRenderContext();

    Toshi::TMatrix44 modelView;
    Toshi::TMatrix44 worldView = pRenderContext->GetModelViewMatrix();

    modelView = DirectX::XMMatrixRotationY(m_CubeRotation) * DirectX::XMMatrixTranslationFromVector(m_Position.XMM()) * worldView.XMM();
    pRenderContext->SetModelViewMatrix(modelView);

    m_pCube->Render();
    m_CubeRotation += deltaTime;

    modelView = DirectX::XMMatrixScaling(10, 1, 10) * DirectX::XMMatrixTranslationFromVector((m_Position + Toshi::TVector4(0, 0.1f, 0, 0.0f)).XMM()) * worldView.XMM();
    pRenderContext->SetModelViewMatrix(modelView);

    m_pPlane->Render();
}

void ATestModel::CreateCube()
{
    static SysMeshVertex s_Vertices[] = {
        { { -1.0f,  1.0f,  -1.0f, }, 0xFFFFFFFF, { 0.0f, 0.0f } },
        { {  1.0f,  1.0f,  -1.0f },  0xFFFFFFFF, { 1.0f, 0.0f } },
        { { -1.0f, -1.0f,  -1.0f },  0xFFFFFFFF, { 0.0f, 1.0f } },
        { {  1.0f, -1.0f,  -1.0f },  0xFFFFFFFF, { 1.0f, 1.0f } },
        { {  -1.0f, -1.0f, 1.0f },   0xFFFFFFFF, { 0.0f, 1.0f } },
        { {  1.0f, -1.0f,  1.0f },   0xFFFFFFFF, { 1.0f, 1.0f } },
        { {  -1.0f,  1.0f,  1.0f },  0xFFFFFFFF, { 0.0f, 0.0f } },
        { {  1.0f,  1.0f,  1.0f },   0xFFFFFFFF, { 1.0f, 0.0f } },
    };

    static uint16_t s_Indices[] = {
        0, 1, 2, 3, 4, 5, 6, 7, 0, 1
    };

    auto pTexManager = Toshi::TTextureManager::GetSingleton();
    auto pSysShader = Toshi::TSysShader::GetSingleton();

    m_pCubeMaterial = pSysShader->CreateMaterial();
    m_pCubeMaterial->SetFlag(Toshi::TMaterial::Flags_AlphaUpdate, TTRUE);
    m_pCubeMaterial->Create(Toshi::TSysMaterialHAL::BlendMode::Default);

    auto pTexture = pTexManager->FindTexture("prop_fountain_02.tga");
    pTexture->SetAlphaEnabled(TFALSE);
    m_pCubeMaterial->SetTexture(pTexture);

    auto pMesh = pSysShader->CreateMesh("test_cube");
    pMesh->Create(0, sizeof(s_Vertices) / sizeof(SysMeshVertex), sizeof(s_Indices) / sizeof(uint16_t));

    Toshi::TSysMesh::TLockBuffer lock;

    if (pMesh->Lock(&lock))
    {
        Toshi::TUtil::MemCopy(lock.pVertexBufferData, s_Vertices, sizeof(s_Vertices));
        Toshi::TUtil::MemCopy(lock.pIndexBufferData, s_Indices, sizeof(s_Indices));
        pMesh->Unlock(sizeof(s_Vertices) / sizeof(SysMeshVertex), sizeof(s_Indices) / sizeof(uint16_t));
    }

    pMesh->SetMaterial(m_pCubeMaterial);
    m_pCube = pMesh;
}

void ATestModel::CreatePlane()
{
    static SysMeshVertex s_Vertices[] = {
        { { -1.0f, 1.0f,  -1.0f },  0xFFFFFFFF, { 0.0f, 0.0f } },
        { {  1.0f, 1.0f,  -1.0f },  0xFFFFFFFF, { 12.0f, 0.0f } },
        { {  -1.0f, 1.0f, 1.0f },   0xFFFFFFFF, { 0.0f, 12.0f } },
        { {  1.0f, 1.0f,  1.0f },   0xFFFFFFFF, { 12.0f, 12.0f } },
    };

    static uint16_t s_Indices[] = {
        0, 1, 2, 3
    };

    auto pTexManager = Toshi::TTextureManager::GetSingleton();
    auto pSysShader = Toshi::TSysShader::GetSingleton();

    m_pPlaneMaterial = pSysShader->CreateMaterial();
    m_pPlaneMaterial->SetFlag(Toshi::TMaterial::Flags_AlphaUpdate, TTRUE);
    m_pPlaneMaterial->Create(Toshi::TSysMaterialHAL::BlendMode::Default);

    auto pTexture = pTexManager->FindTexture("slum_island_colour.tga");
    pTexture->SetAlphaEnabled(TFALSE);
    pTexture->SetSamplerId(3);
    m_pPlaneMaterial->SetTexture(pTexture);

    auto pMesh = pSysShader->CreateMesh("test_cube");
    pMesh->Create(0, sizeof(s_Vertices) / sizeof(SysMeshVertex), sizeof(s_Indices) / sizeof(uint16_t));

    Toshi::TSysMesh::TLockBuffer lock;

    if (pMesh->Lock(&lock))
    {
        Toshi::TUtil::MemCopy(lock.pVertexBufferData, s_Vertices, sizeof(s_Vertices));
        Toshi::TUtil::MemCopy(lock.pIndexBufferData, s_Indices, sizeof(s_Indices));
        pMesh->Unlock(sizeof(s_Vertices) / sizeof(SysMeshVertex), sizeof(s_Indices) / sizeof(uint16_t));
    }

    pMesh->SetMaterial(m_pPlaneMaterial);
    m_pPlane = pMesh;
}
