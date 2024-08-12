#pragma once
#include "Render/AWorldShader/AWorldMaterial.h"

#include <Render/TOrderTable.h>

class AWorldMaterialHAL : public AWorldMaterial
{
public:
    TDECLARE_CLASS( AWorldMaterialHAL, AWorldMaterial );

public:
    AWorldMaterialHAL();
    ~AWorldMaterialHAL();

    //-----------------------------------------------------------------------------
    // Toshi::TMaterial
    //-----------------------------------------------------------------------------
    virtual void OnDestroy() override;
    virtual void PreRender() override;
    virtual void PostRender() override;

    //-----------------------------------------------------------------------------
    // AWorldMaterial
    //-----------------------------------------------------------------------------
    virtual TBOOL Create( BLENDMODE a_eBlendMode ) override;
    virtual void  SetBlendMode( BLENDMODE a_eBlendMode ) override;

    //-----------------------------------------------------------------------------
    // Own methods
    //-----------------------------------------------------------------------------
    virtual void CopyToAlphaBlendMaterial();

    void SetOrderTable( Toshi::TOrderTable* a_pOrderTable );

    AWorldMaterialHAL* GetAlphaBlendMaterial() const { return m_pAlphaBlendMaterial; }
    void               SetAlphaBlendMaterial( AWorldMaterialHAL* val ) { m_pAlphaBlendMaterial = val; }

    TFLOAT GetUVOffsetX( TUINT a_uiTextureIndex )
    {
        TASSERT( a_uiTextureIndex < MAX_TEXTURES );
        return m_aUVOffsetsX[ a_uiTextureIndex ];
    }
    void SetUVOffsetX( TUINT a_uiTextureIndex, TFLOAT a_fOffset )
    {
        TASSERT( a_uiTextureIndex < MAX_TEXTURES );
        m_aUVOffsetsX[ a_uiTextureIndex ] = a_fOffset;
    }
    void AddUVOffsetX( TUINT a_uiTextureIndex, TFLOAT a_fOffset )
    {
        TASSERT( a_uiTextureIndex < MAX_TEXTURES );
        m_aUVOffsetsX[ a_uiTextureIndex ] += a_fOffset;
    }

    TFLOAT GetUVOffsetY( TUINT a_uiTextureIndex )
    {
        TASSERT( a_uiTextureIndex < MAX_TEXTURES );
        return m_aUVOffsetsY[ a_uiTextureIndex ];
    }
    void SetUVOffsetY( TUINT a_uiTextureIndex, TFLOAT a_fOffset )
    {
        TASSERT( a_uiTextureIndex < MAX_TEXTURES );
        m_aUVOffsetsY[ a_uiTextureIndex ] = a_fOffset;
    }
    void AddUVOffsetY( TUINT a_uiTextureIndex, TFLOAT a_fOffset )
    {
        TASSERT( a_uiTextureIndex < MAX_TEXTURES );
        m_aUVOffsetsY[ a_uiTextureIndex ] += a_fOffset;
    }

private:
    void SetupRenderer();

private:
    AWorldMaterialHAL*  m_pAlphaBlendMaterial;
    Toshi::TOrderTable* m_pAssignedOrderTable;
    TBOOL               m_aHasUVOffsets[ MAX_TEXTURES ];
    TFLOAT              m_aUVOffsetsX[ MAX_TEXTURES ];
    TFLOAT              m_aUVOffsetsY[ MAX_TEXTURES ];
};