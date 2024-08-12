#pragma once
#include "Math/TVector3.h"

namespace Toshi
{

namespace TTMDBase
{

struct FileHeader
{
    TUINT32 m_uiMagic;
    TUINT32 m_uiZero1;
    TUINT16 m_uiVersionMajor;
    TUINT16 m_uiVersionMinor;
    TUINT32 m_uiZero2;
};

struct SkeletonHeader
{
    TCHAR  m_szTKLName[ 16 ];
    TINT32 m_iTKeyCount;
    TINT32 m_iQKeyCount;
    TINT32 m_iSKeyCount;
    TINT32 m_iTBaseIndex;
    TINT32 m_iQBaseIndex;
    TINT32 m_iSBaseIndex;
};

struct Material
{
    TCHAR szMatName[ 104 ];
    TCHAR szTextureFile[ 192 ];
};

struct MaterialsHeader
{
    TUINT32 uiZero1;
    TUINT32 uiZero2;
    TINT32  iNumMaterials;
    TUINT32 uiSectionSize;

    Material* GetMaterial( TINT a_iIndex )
    {
        TASSERT( a_iIndex >= 0 && a_iIndex < iNumMaterials );
        return TREINTERPRETCAST( Material*, this + 1 ) + a_iIndex;
    }
};

struct CollisionModel
{
    TUINT32   m_Unk1;
    TVector3* m_pVertices;
    TUINT32   m_uiNumVertices;
    TUINT16*  m_pIndices;
    TUINT32   m_uiNumIndices;
    TUINT32   m_uiNumCollTypes;
    TCHAR**   m_pszCollName;
    TUINT32   m_Unk3;
};

struct Collision
{
    TINT            m_iNumModels;
    CollisionModel* m_pModels;
};

struct Mesh
{
};
} // namespace TTMDBase

} // namespace Toshi