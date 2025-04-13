#pragma once
#include "AObjectHash.h"

class AObjectHashMain
    : public AObjectHash
    , public Toshi::TSingleton<AObjectHashMain>
{

};