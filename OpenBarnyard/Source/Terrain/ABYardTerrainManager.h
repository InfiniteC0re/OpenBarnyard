#pragma once
#include "ATerrain.h"

class ABYardTerrainManager
{
public:
    using Terrain = TUINT32;
    enum Terrain_ : Terrain
    {
        Terrain_EnvMain,
        Terrain_GateCrash,
        Terrain_EnvBarnInt,
        Terrain_Golf1,
        Terrain_Golf2,
        Terrain_Golf3,
        Terrain_Golf4,
        Terrain_Golf5,
        Terrain_Golf6,
        Terrain_Golf7,
        Terrain_Golf8,
        Terrain_Golf9,
        Terrain_Golf10,
        Terrain_Golf11,
        Terrain_Golf12,
        Terrain_Golf13,
        Terrain_Golf14,
        Terrain_Golf15,
        Terrain_Golf16,
        Terrain_Golf17,
        Terrain_Golf18,
        Terrain_PeckRange,
        Terrain_FrontEnd,
        Terrain_Statue,
        Terrain_EnvBeadyInt,
        Terrain_EnvWoods,
        Terrain_EnvBarnIntDay,
        Terrain_EnvBlackMarket,
        Terrain_CowTipping,
        Terrain_VeggieDef,
        Terrain_EnvPoolTable,
        Terrain_EnvPoolBoat,
        Terrain_EnvWacARac,
        Terrain_Tappers,
        Terrain_EnvCarRace,
        Terrain_Kitchen,
        Terrain_Cocktails,
        Terrain_ChasingChicks2,
        Terrain_EnvBensHill,
        Terrain_EnvBeadyFarm,
        Terrain_EnvCoopInt,
        Terrain_EnvFarmerInt,
        Terrain_EnvDartsLarge,
        Terrain_EnvSSDarts,
        Terrain_test1,
        Terrain_ss01,
        Terrain_ss02,
        Terrain_ss03,
        Terrain_ss04,
        Terrain_ss05,
        Terrain_ss06,
        Terrain_ss07a,
        Terrain_ss07b,
        Terrain_ss08,
        Terrain_ss09,
        Terrain_ss10,
        Terrain_ss11,
        Terrain_NUMOF
    };

    struct TerrainInfo
    {
        const TCHAR* szName;
        TUINT32      uiUnk;
    };

    inline static constexpr TerrainInfo ms_aTerrains[ Terrain_NUMOF ] = {
        { "EnvMain", 0x400000 },
        { "gatecrash", 0x500000 },
        { "envbarnint", 0x400000 },
        { "golf1", 0x600000 },
        { "golf2", 0x600000 },
        { "golf3", 0x600000 },
        { "golf4", 0x600000 },
        { "golf5", 0x600000 },
        { "golf6", 0x600000 },
        { "golf7", 0x600000 },
        { "golf8", 0x600000 },
        { "golf9", 0x600000 },
        { "golf10", 0x600000 },
        { "golf11", 0x600000 },
        { "golf12", 0x600000 },
        { "golf13", 0x600000 },
        { "golf14", 0x600000 },
        { "golf15", 0x600000 },
        { "golf16", 0x600000 },
        { "golf17", 0x600000 },
        { "golf18", 0x600000 },
        { "peckrange", 0x900000 },
        { "frontend", 0x300000 },
        { "statue", 0x400000 },
        { "env_beadyint", 0x400000 },
        { "envwoods", 0x800000 },
        { "envbarnintday", 0x400000 },
        { "envblackmarket", 0x100000 },
        { "cowtipping", 0x600000 },
        { "veggiedef", 0x600000 },
        { "EnvPoolTable", 0x400000 },
        { "envpoolboat", 0x400000 },
        { "EnvWacARac", 0x100000 },
        { "tappers", 0x500000 },
        { "envcarrace", 0xB00000 },
        { "kitchen", 0x400000 },
        { "cocktail", 0x400000 },
        { "chasingchicks2", 0x800000 },
        { "envbenshill", 0x800000 },
        { "envbeadyfarm", 0x800000 },
        { "envcoopint", 0x400000 },
        { "envfarmerint", 0x400000 },
        { "envdarts_large", 0x400000 },
        { "EnvSSDarts", 0x400000 },
        { "test1", 0x400000 },
        { "ss01", 0x400000 },
        { "ss02", 0x400000 },
        { "ss03", 0x400000 },
        { "ss04", 0x400000 },
        { "ss05", 0x400000 },
        { "ss06", 0x400000 },
        { "ss07a", 0x400000 },
        { "ss07b", 0x400000 },
        { "ss08", 0x400000 },
        { "ss09", 0x400000 },
        { "ss11", 0x400000 },
    };

public:
    static void SetTerrain( TINT a_eTerrain, TBOOL a_bLoadLater, TBOOL a_bStreamModels, TINT a_iUnused1, TINT a_iUnused2, TINT a_iPreloadTerrainBlockSize, TINT a_iStartVISGroup );
    static void StartLoading();

    static constexpr const TCHAR* GetTerrainName( Terrain a_eTerrain ) { return ms_aTerrains[ a_eTerrain ].szName; }

private:
    inline static ATerrainInterface* ms_pCurrentTerrain;
    inline static Terrain            ms_eCurrentLevel = Terrain_EnvBeadyFarm;
};
