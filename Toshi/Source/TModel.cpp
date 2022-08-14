#include "TModel.h"
bool TModel::LoadTMD(char* fileName)
{
    char cVar1;
    TCString* extraout_EAX;
    TFile* file;
    void* pvVar2;
    bool bVar3;
    TCString string;
    char acStack88[4];
    undefined local_54[4];
    undefined auStack80[12];
    uint uStack68;
    int iStack64;
    int iStack60;
    int iStack56;
    int iStack52;
    int iStack48;
    int iStack44;
    TKeyframeLibrary* pTStack40;

    /* 0x9730  884  ?LoadTMD@TModel@Toshi@@IAE_NPBD@Z */
    bVar3 = false;
    Toshi::TCString::TCString(&string, param_1);
    file = Toshi::TFile::Create(extraout_EAX, 1);
    Toshi::TCString::_TCString(&string);
    if (file != (TFile*)0x0) {
        (*(code*)*file->field_0x0)(local_54, 0xc);
        if (_string == 0x4c444d54) {
            (*(code*)*file->field_0x0)(auStack80, 0x30);
            pvVar2 = tmalloc(iStack64, (char*)0x0, -1);
            *(void**)(this + 0x14) = pvVar2;
            if (pvVar2 != (void*)0x0) {
                (*(code*)*file->field_0x0)(pvVar2, iStack64);
                TSkeleton::Create(*(TSkeleton**)(this + 0x14), uStack68);
                pTStack40 = TKeyframeLibraryManager::GetLibrary
                ((TKeyframeLibraryManager*)(TRenderInterface::s_Interface + 0x150),
                    acStack88);
                TKeyframeLibraryInstance::CreateEx
                ((TKeyframeLibraryInstance*)(*(int*)(this + 0x14) + 0x10), pTStack40, iStack64,
                    iStack60, iStack56, iStack52, iStack48, iStack44);
                cVar1 = (**(code**)(*(int*)this + 0x14))(file);
                if (cVar1 != '\0') {
                    bVar3 = true;
                }
            }
        }
        Toshi::TFile::Destroy(file);
        return bVar3;
    }
    return false;
}