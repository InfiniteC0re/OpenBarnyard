#include "ToshiPCH.h"
#include "TOrderTable.h"

namespace Toshi
{
    void TOrderTable::CreateStaticData(uint32_t maxMaterials, uint32_t maxRenderPackets)
    {
        s_uiOrigMaxRenderPackets = s_uiMaxRenderPackets = maxRenderPackets;
        s_uiNumRenderPackets = 0;
        TTODO("INT_009a89c8 = 0;");

        s_pRenderPackets = (TRenderPacket*)TMemalign(32, maxRenderPackets * sizeof(TRenderPacket));

        s_uiMaxMaterials = maxMaterials;
        s_pRegMaterials = new TRegMaterial[s_uiMaxMaterials];

        for (size_t i = 0; i < s_uiMaxMaterials; i++)
        {
            s_pRegMaterials[i].SetOrderTable(TNULL);
            s_RegMatFreeList.InsertTail(s_pRegMaterials[i]);
        }
    }
}