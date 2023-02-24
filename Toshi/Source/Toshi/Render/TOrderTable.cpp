#include "ToshiPCH.h"
#include "TOrderTable.h"

namespace Toshi
{
    void TOrderTable::CreateStaticData(uint32_t maxMaterials, uint32_t maxRenderPackets)
    {
        TIMPLEMENT();

        s_uiOrigMaxRenderPackets = s_uiMaxRenderPackets = maxRenderPackets;
        s_uiNumRenderPackets = 0;

        s_pRenderPackets = (TRenderPacket*)TMemalign(32, maxRenderPackets * sizeof(TRenderPacket));

        s_uiMaxMaterials = maxMaterials;
        /*s_pRegMaterials = new TRegMaterial[maxMaterials];

        for (size_t i = 0; i < s_uiMaxMaterials; i++)
        {
            s_pRegMaterials[i].SetOrderTable(TNULL);
            
        }*/
    }
}