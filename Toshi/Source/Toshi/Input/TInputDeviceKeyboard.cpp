#include "ToshiPCH.h"
#include "TInputDeviceKeyboard.h"

const char* Toshi::TInputDeviceKeyboard::GetButtonFromDoodad(int doodad) const
{
    return GET_KEY_FROM_DOODAD(doodad);
}
