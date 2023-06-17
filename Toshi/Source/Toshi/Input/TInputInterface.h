#pragma once
#include "TInputDeviceKeyboard.h"
#include "TInputDeviceMouse.h"

namespace Toshi {

    class TInputInterface :
        public TGenericClassDerived<TInputInterface, TObject, "TInputInterface", TMAKEVERSION(1, 0), TFALSE>,
        public TSingleton<TInputInterface>
    {
    public:
        TInputInterface() 
        {
            m_bIsExclusiveMode = TFALSE;
        }

        TInputDevice* GetDeviceByIndex(TClass* pClass, size_t index);
        TInputDeviceKeyboard* GetKeyboardByIndex(size_t index);
        TInputDeviceMouse* GetMouseByIndex(size_t index);

        TClass* GetSpecificDeviceByIndex(TClass* pClass, size_t index)
        {
            TInputDevice* pDevice = GetDeviceByIndex(pClass, index);
            TASSERT(pDevice == TNULL || pDevice->GetClass()->IsA(pClass));
            if (pDevice == TNULL) return TNULL;
            return pDevice->GetClass();
        }

        void AddDevice(TInputDevice* device);
        void RemoveDevice(TInputDevice* device);

        virtual TBOOL Initialise() { return TTRUE; }
        virtual TBOOL Deinitialise();

        virtual TBOOL AcquireAll();
        virtual TBOOL UnacquireAll();
        virtual TBOOL FlushAll();
        virtual void SetExclusiveMode(TBOOL mode);
        virtual TBOOL GetExclusiveMode() const;
        virtual int ProcessEvents(float fUnk);
        virtual void StopAllRepeats();

    private:
        TBOOL m_bIsExclusiveMode;             // 0x04 
        TNodeList<TInputDevice> m_DeviceList; // 0x08
        TGenericEmitter m_Emitter1;           // 0x24
        TGenericEmitter m_Emitter2;           // 0x28
    };

}
