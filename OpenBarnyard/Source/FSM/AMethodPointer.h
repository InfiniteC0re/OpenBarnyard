#pragma once
#include "AFunctionTable.h"

#include <Plugins/PPropertyParser/PPropertyValue.h>
#include <Plugins/PPropertyParser/PPropertyValueArray.h>

#include <type_traits>
#include <utility>

//-----------------------------------------------------------------------------
// Base of all script-callable member functions (vtable 0072ccb4).
//-----------------------------------------------------------------------------
class AMethodPointer
    : public AFunction
{
public:
	AMethodPointer( const TCHAR* a_szName );
	AMethodPointer( const AMethodPointer& a_rOther );
	~AMethodPointer() override;

	// RETURNS_TRUE
	TBOOL IsMethod() const override { return TTRUE; } // +0x08
};

//-----------------------------------------------------------------------------
// Converts between PPropertyValue and native argument/return types.
// NOTE: The name is made up. Retail inlines every conversion into the Call/GetSignature
// instantiations, except the by-value TPString8 one (00412560).
//-----------------------------------------------------------------------------
template <class T>
struct AFunctionArg;

template <>
struct AFunctionArg<void>
{
	static const Toshi::TClass* GetType() { return PPropertyValue::TYPE_UNDEF; }
};

template <>
struct AFunctionArg<TBOOL>
{
	static const Toshi::TClass* GetType() { return PPropertyValue::TYPE_BOOL; }
	static TBOOL                Get( const PPropertyValue* a_pValue ) { return a_pValue->GetBoolean(); }
	static void                 Set( PPropertyValue* a_pValue, TBOOL a_bValue ) { a_pValue->SetBoolean( a_bValue ); }
};

template <>
struct AFunctionArg<TINT>
{
	static const Toshi::TClass* GetType() { return PPropertyValue::TYPE_INT; }
	static TINT                 Get( const PPropertyValue* a_pValue ) { return a_pValue->GetInteger(); }
	static void                 Set( PPropertyValue* a_pValue, TINT a_iValue ) { a_pValue->SetInteger( a_iValue ); }
};

// Writable int (ASoundManager::StopCue resets the script variable to -1)
// NOTE: retail passes a pointer to the value of the argument itself, so the const is cast away
template <>
struct AFunctionArg<TINT&>
{
	static const Toshi::TClass* GetType() { return PPropertyValue::TYPE_INT; }
	static TINT&                Get( const PPropertyValue* a_pValue ) { return const_cast<PPropertyValue*>( a_pValue )->GetIntegerRef(); }
};

template <>
struct AFunctionArg<TUINT32>
{
	static const Toshi::TClass* GetType() { return PPropertyValue::TYPE_UINT32; }
	static TUINT32              Get( const PPropertyValue* a_pValue ) { return a_pValue->GetUINT32(); }
	static void                 Set( PPropertyValue* a_pValue, TUINT32 a_uiValue ) { a_pValue->SetUINT32( a_uiValue ); }
};

template <>
struct AFunctionArg<TFLOAT>
{
	static const Toshi::TClass* GetType() { return PPropertyValue::TYPE_FLOAT; }
	static TFLOAT               Get( const PPropertyValue* a_pValue ) { return a_pValue->GetFloat(); }
	static void                 Set( PPropertyValue* a_pValue, TFLOAT a_fValue ) { a_pValue->SetFloat( a_fValue ); }
};

template <>
struct AFunctionArg<Toshi::TPString8>
{
	static const Toshi::TClass* GetType() { return PPropertyValue::TYPE_TPSTRING8; }

	// $Barnyard: FUNCTION 00412560
	static Toshi::TPString8 Get( const PPropertyValue* a_pValue ) { return a_pValue->GetTPString8(); }

	static void Set( PPropertyValue* a_pValue, const Toshi::TPString8& a_rValue ) { a_pValue->SetTPString8( a_rValue ); }
};

template <>
struct AFunctionArg<const Toshi::TPString8&>
{
	static const Toshi::TClass*    GetType() { return PPropertyValue::TYPE_TPSTRING8; }
	static const Toshi::TPString8& Get( const PPropertyValue* a_pValue ) { return a_pValue->GetTPString8(); }
};

// Untyped values (the type isn't checked by the FSM parser)
template <>
struct AFunctionArg<const PPropertyValue&>
{
	static const Toshi::TClass*  GetType() { return PPropertyValue::TYPE_UNDEF; }
	static const PPropertyValue& Get( const PPropertyValue* a_pValue ) { return *a_pValue; }
};

// Pointers to TObject-derived classes (AUnit* etc.)
template <class T>
struct AFunctionArg<T*>
{
	static const Toshi::TClass* GetType() { return &TGetClass( T ); }
	static T*                   Get( const PPropertyValue* a_pValue ) { return TSTATICCAST( T, a_pValue->GetTObject() ); }
	static void                 Set( PPropertyValue* a_pValue, T* a_pObject ) { a_pValue->SetTObject( a_pObject ); }
};

//-----------------------------------------------------------------------------
// Binds a member function of T to a script function name.
// NOTE: Retail has a separate class template per arity (AMethodPointer0Args<T>,
// AMethodPointer1Args<T, A1>, ..., AMethodPointer0ArgsRT<R, T>, AMethodPointer3ArgsRT<R, T, A1, A2, A3>, ...).
// They are all generated from this one variadic template; the retail names are provided as aliases below.
// Object layout matches retail (0x10 bytes) when T uses multiple inheritance, since MSVC then stores
// the method pointer as { code, this adjustor }.
//-----------------------------------------------------------------------------
template <class TRet, class T, class... TArgs>
class AMethodPointerNArgsRT
    : public AMethodPointer
{
public:
	using Method      = TRet ( T::* )( TArgs... );
	using ConstMethod = TRet ( T::* )( TArgs... ) const;

public:
	// AMethodPointer1Args<AUnitManager, AUnit*>:                              0040f830
	// AMethodPointer1ArgsRT<AUnit*, AUnitManager, const TPString8&>:          0040f8b0
	// AMethodPointer2Args<AUnitManager, AUnit*, TBOOL>:                       0040f930
	// AMethodPointer1Args<AUnitManager, const TPString8&>:                    0040f9b0
	// AMethodPointer1ArgsRT<AUnit*, AUnitNameManager, TPString8>:             00411e30
	// AMethodPointer0ArgsRT<TBOOL, AUnit>:                                    00413250
	// AMethodPointer0ArgsRT<TUINT, AUnit>:                                    004132d0
	// AMethodPointer3ArgsRT<TBOOL, AUnit, AUnit*, TFLOAT, TFLOAT>:            00413350
	// AMethodPointer1ArgsRT<TFLOAT, AUnit, AUnit*>:                           004133d0
	// AMethodPointer1ArgsRT<TBOOL, AUnit, TBOOL>:                             00413450
	// $Barnyard: FUNCTION 0040f830
	// $Barnyard: FUNCTION 0040f8b0
	// $Barnyard: FUNCTION 0040f930
	// $Barnyard: FUNCTION 0040f9b0
	// $Barnyard: FUNCTION 00411e30
	// $Barnyard: FUNCTION 00413250
	// $Barnyard: FUNCTION 004132d0
	// $Barnyard: FUNCTION 00413350
	// $Barnyard: FUNCTION 004133d0
	// $Barnyard: FUNCTION 00413450
	// AMiscFunctions instantiations (vtables 007367e0..00736920, 0x14 apart), one Create per vtable.
	// Folded instantiations share one (e.g. Or/And/Xor, Subtract/Add/Multiply/GetRandom):
	// <void, TPString8>: 006034e0, <void, TBOOL, TPString8>: 00603560, <void, AUnit*, TPString8>: 006035e0,
	// <TBOOL, TBOOL>: 00603660, <TBOOL, TBOOL, TBOOL>: 006036e0, <TFLOAT, TFLOAT, TFLOAT>: 00603760,
	// <void, TPString8, const PPropertyValue&>: 006037e0, <TINT, TINT, TINT>: 00603860, <TFLOAT>: 006038e0,
	// <void, TINT, TINT>: 00603960, <TPString8>: 006039e0, <TBOOL, TINT>: 00603a60, <void, TBOOL>: 00603ae0,
	// <TBOOL>: 00603b60, <TBOOL, AUnit*, TFLOAT>: 00603be0, <void>: 00603c60, <TINT>: 00603ce0
	// $Barnyard: FUNCTION 006034e0
	// $Barnyard: FUNCTION 00603560
	// $Barnyard: FUNCTION 006035e0
	// $Barnyard: FUNCTION 00603660
	// $Barnyard: FUNCTION 006036e0
	// $Barnyard: FUNCTION 00603760
	// $Barnyard: FUNCTION 006037e0
	// $Barnyard: FUNCTION 00603860
	// $Barnyard: FUNCTION 006038e0
	// $Barnyard: FUNCTION 00603960
	// $Barnyard: FUNCTION 006039e0
	// $Barnyard: FUNCTION 00603a60
	// $Barnyard: FUNCTION 00603ae0
	// $Barnyard: FUNCTION 00603b60
	// $Barnyard: FUNCTION 00603be0
	// $Barnyard: FUNCTION 00603c60
	// $Barnyard: FUNCTION 00603ce0
	// Create/Clone/Call/GetSignature of AAnimatableObject (<TBOOL, const TPString8&>, <TBOOL>), AModelAttachComponent
	// (AttachModel, DetachModel), AUnitBehaviour (<void, TBOOL>), AInteractionManager (CanAssignInteraction,
	// AssignInteraction, SetPersistentLatch), AInteractionComponent (GetSignature 005a1be0):
	// 0057b990/0057ba90, 0057ba10/0057bb10, 0058d220/0058d320/0058d590/0058d910, 0058d2a0/0058d3a0/0058d6d0,
	// 005930b0/00593130/005931b0, 005a2960/005a2b60, 005a29e0/005a2c10, 005a2ae0/005a2d60
	// ASteer / AInteriorSteer instantiations (Create, Clone, Call, GetSignature; Call/GetSignature are folded between them):
	// AMethodPointer1ArgsRT<TBOOL, AInteriorSteer, TPString8>:               00561d80 00562500 00565c10 00546c90
	// AMethodPointer2ArgsRT<TBOOL, AInteriorSteer, AUnit*, TFLOAT>:          00561e00 00562580 00604660 00605300
	// AMethodPointer1ArgsRT<TBOOL, AInteriorSteer, AUnit*>:                  00561e80 00562600 00562680 00566540
	// AMethodPointer3ArgsRT<TBOOL, AInteriorSteer, TFLOAT, TFLOAT, TFLOAT>:  00561f00 005626b0 00562730 00566570
	// AMethodPointer3ArgsRT<TBOOL, AInteriorSteer, AUnit*, TFLOAT, TBOOL>:   00561f80 005627a0 00562820 005665f0
	// AMethodPointer0ArgsRT<TBOOL, AInteriorSteer>:                          00562000 00562880 00562900 004130d0
	// AMethodPointer1ArgsRT<TBOOL, AInteriorSteer, TFLOAT>:                  00562080 00562920 005629a0 00572040
	// AMethodPointer2ArgsRT<TBOOL, AInteriorSteer, TPString8, TBOOL>:        00562100 005629e0 00565cb0 00566660
	// AMethodPointer0Args<AInteriorSteer>:                                   00562180 00562a60 00562ae0 00500720
	// AMethodPointer2ArgsRT<TBOOL, AInteriorSteer, TPString8, TFLOAT>:       00562200 00562af0 00565d60 005666c0
	// AMethodPointer3ArgsRT<TBOOL, AInteriorSteer, AUnit*, TFLOAT, TFLOAT>:  00562280 00562b70 00562bf0 004142c0
	// AMethodPointer2ArgsRT<TBOOL, AInteriorSteer, TPString8, AUnit*>:       00562300 00562c50 00565e10 00566720
	// AMethodPointer1Args<AInteriorSteer, TBOOL>:                            00562380 00562cd0 00571b90 00593d00
	// AMethodPointer1Args<AInteriorSteer, const TPString8&>:                 00562400 00562d50 00562dd0 005df820
	// AMethodPointer1ArgsRT<TBOOL, AInteriorSteer, const TPString8&>:        00562480 00562df0 00562e70 00546c90
	// AMethodPointer0ArgsRT<TBOOL, ASteer>:                                  005711e0 00571660 00562900 004130d0
	// AMethodPointer0Args<ASteer>:                                           00571260 005716e0 00562ae0 00500720
	// AMethodPointer0ArgsRT<AUnit*, ASteer>:                                 005712e0 00571760 005717e0 00571800
	// AMethodPointer2ArgsRT<TBOOL, ASteer, AUnit*, TFLOAT>:                  00571360 00571810 00604660 00605300
	// AMethodPointer1ArgsRT<TBOOL, ASteer, TFLOAT>:                          005713e0 00571890 005629a0 00572040
	// AMethodPointer1ArgsRT<TBOOL, ASteer, AUnit*>:                          00571460 00571910 00562680 00566540
	// AMethodPointer0ArgsRT<TFLOAT, ASteer>:                                 005714e0 00571990 00571a10 00571070
	// AMethodPointer3Args<ASteer, TFLOAT, TFLOAT, TFLOAT>:                   00571560 00571a30 00571ab0 00572080
	// AMethodPointer1Args<ASteer, TBOOL>:                                    005715e0 00571b10 00571b90 00593d00
	// ARegrowthManager / APlayerManager / ASSeqController instantiations (Create, Clone, Call, GetSignature):
	// AMethodPointer3ArgsRT<TINT, ARegrowthManager, TINT, TINT, TFLOAT>:      005e2be0 005e2ce0 005e2d60 005e6eb0
	// AMethodPointer1ArgsRT<TINT, ARegrowthManager, TINT>:                    005e2c60 005e2dc0 005d6e00 0049a010
	// AMethodPointer1ArgsRT<AUnit*, APlayerManager, TINT>:                    0061fcf0 0061fd70 0061fdf0 00620000
	// AMethodPointer5ArgsRT<TBOOL, ASSeqController, TPString8 x3, TBOOL x2>:  0062a150 0062a250 0062af60 0062b150
	// AMethodPointer0ArgsRT<TBOOL, ASSeqController>:                          0062a1d0 0062a2d0 005a2be0 004130d0
	// $Barnyard: FUNCTION 00561d80
	// $Barnyard: FUNCTION 00561e00
	// $Barnyard: FUNCTION 00561e80
	// $Barnyard: FUNCTION 00561f00
	// $Barnyard: FUNCTION 00561f80
	// $Barnyard: FUNCTION 00562000
	// $Barnyard: FUNCTION 00562080
	// $Barnyard: FUNCTION 00562100
	// $Barnyard: FUNCTION 00562180
	// $Barnyard: FUNCTION 00562200
	// $Barnyard: FUNCTION 00562280
	// $Barnyard: FUNCTION 00562300
	// $Barnyard: FUNCTION 00562380
	// $Barnyard: FUNCTION 00562400
	// $Barnyard: FUNCTION 00562480
	// $Barnyard: FUNCTION 005711e0
	// $Barnyard: FUNCTION 00571260
	// $Barnyard: FUNCTION 005712e0
	// $Barnyard: FUNCTION 00571360
	// $Barnyard: FUNCTION 005713e0
	// $Barnyard: FUNCTION 00571460
	// $Barnyard: FUNCTION 005714e0
	// $Barnyard: FUNCTION 00571560
	// $Barnyard: FUNCTION 005715e0
	// $Barnyard: FUNCTION 0057b990
	// $Barnyard: FUNCTION 0057ba10
	// $Barnyard: FUNCTION 0058d220
	// $Barnyard: FUNCTION 0058d2a0
	// $Barnyard: FUNCTION 005930b0
	// $Barnyard: FUNCTION 005a2960
	// $Barnyard: FUNCTION 005a29e0
	// $Barnyard: FUNCTION 005a2ae0
	// ANPCBehaviourComponent / ANPCMilkLauncherComponent instantiations (Create, Clone, Call):
	// AMethodPointer0ArgsRT<TPString8, ANPCBehaviourComponent>:                       00633b20 00633c20 00634560
	// AMethodPointer0Args<ANPCBehaviourComponent>:                                    00633ba0 00633ca0 00633d20
	// AMethodPointer2ArgsRT<TBOOL, ANPCMilkLauncherComponent, AUnit*, TFLOAT>:        00669b80 00669c00 00669c80
	// $Barnyard: FUNCTION 00633b20
	// $Barnyard: FUNCTION 00633ba0
	// $Barnyard: FUNCTION 00669b80
	// Unit / unit component instantiations (Create, Clone, Call, GetSignature):
	// AMethodPointer0ArgsRT<TBOOL, AVisibleUnit>:                                  00415110 00415290 005a2be0 004130d0
	// AMethodPointer1ArgsRT<TBOOL, AVisibleUnit, TFLOAT>:                          00415190 00415310 00415390 00572040
	// AMethodPointer1Args<AVisibleUnit, TBOOL>:                                    00415210 004153d0 005931b0 00593d00
	// AMethodPointer1Args<AAnimatedUnit, TBOOL>:                                   00416060 00416260 005931b0 00593d00
	// AMethodPointer2Args<AAnimatedUnit, AUnit*, const TPString8&>:                004160e0 004162e0 00416360 00604f70
	// AMethodPointer0Args<AAnimatedUnit>:                                          00416160 004163a0 00633d20 00500720
	// AMethodPointer1ArgsRT<TBOOL, AAnimatedUnit, AUnit*>:                         004161e0 00416420 004164a0 00566540
	// AMethodPointer0ArgsRT<TBOOL, ACharacterUnit>:                                00417070 00417370 005a2be0 004130d0
	// AMethodPointer0ArgsRT<ACharacterUnit*, ACharacterUnit>:                      004170f0 004173f0 00416d20 00417470
	// AMethodPointer1ArgsRT<ACharacterUnit*, ACharacterUnit, const TPString8&>:    00417170 00417480 00417500 00417ae0
	// AMethodPointer2ArgsRT<TBOOL, ACharacterUnit, AUnit*, TFLOAT>:                004171f0 00417530 00669c80 00605300
	// AMethodPointer0Args<ACharacterUnit>:                                         00417270 004175b0 00633d20 00500720
	// AMethodPointer1Args<ACharacterUnit, TFLOAT>:                                 004172f0 00417630 0058b130 005371a0
	// AMethodPointer0ArgsRT<TFLOAT, AAnimalAttributes>:                            0058ae80 0058b000 0058b080 00571070
	// AMethodPointer1Args<AAnimalAttributes, TFLOAT>:                              0058af00 0058b0b0 0058b130 005371a0
	// AMethodPointer0Args<AAnimalAttributes>:                                      0058af80 0058b160 00633d20 00500720
	// AMethodPointer1Args<AUnitFacialComponent, TFLOAT>:                           00594800 00594a00 0058b130 005371a0
	// AMethodPointer0Args<AUnitFacialComponent>:                                   00594880 00594a80 00633d20 00500720
	// AMethodPointer1Args<AUnitFacialComponent, TBOOL>:                            00594900 00594b00 005931b0 00593d00
	// AMethodPointer1Args<AUnitFacialComponent, const TPString8&>:                 00594980 00594b80 00469550 005df820
	// AMethodPointer0ArgsRT<TBOOL, AInteractionComponent>:                         005a0730 005a09b0 005a2be0 004130d0
	// AMethodPointer0Args<AInteractionComponent>:                                  005a07b0 005a0a30 00633d20 00500720
	// AMethodPointer1Args<AInteractionComponent, TBOOL>:                           005a0830 005a0ab0 005931b0 00593d00
	// AMethodPointer3Args<AInteractionComponent, const TPString8&, const TPString8&, TFLOAT>: 005a08b0 005a0b30 005a0bb0 005a1be0
	// AMethodPointer0ArgsRT<TFLOAT, AInteractionComponent>:                        005a0930 005a0c00 0058b080 00571070
	// $Barnyard: FUNCTION 00415110
	// $Barnyard: FUNCTION 00415190
	// $Barnyard: FUNCTION 00415210
	// $Barnyard: FUNCTION 00416060
	// $Barnyard: FUNCTION 004160e0
	// $Barnyard: FUNCTION 00416160
	// $Barnyard: FUNCTION 004161e0
	// $Barnyard: FUNCTION 00417070
	// $Barnyard: FUNCTION 004170f0
	// $Barnyard: FUNCTION 00417170
	// $Barnyard: FUNCTION 004171f0
	// $Barnyard: FUNCTION 00417270
	// $Barnyard: FUNCTION 004172f0
	// $Barnyard: FUNCTION 0058ae80
	// $Barnyard: FUNCTION 0058af00
	// $Barnyard: FUNCTION 0058af80
	// $Barnyard: FUNCTION 00594800
	// $Barnyard: FUNCTION 00594880
	// $Barnyard: FUNCTION 00594900
	// $Barnyard: FUNCTION 00594980
	// $Barnyard: FUNCTION 005a0730
	// $Barnyard: FUNCTION 005a07b0
	// $Barnyard: FUNCTION 005a0830
	// $Barnyard: FUNCTION 005a08b0
	// $Barnyard: FUNCTION 005a0930
	// ASoundManager / ASoundComponent instantiations (Create, Clone, Call, GetSignature):
	// AMethodPointer1ArgsRT<TINT, ASoundManager, ASoundId>:            005d6b00 005d6d80 005d6e00 0049a010
	// AMethodPointer2ArgsRT<TINT, ASoundManager, ASoundId, AUnit*>:    005d6b80 005d6e30 005d6eb0 005d89e0
	// AMethodPointer1Args<ASoundManager, TINT&>:                       005d6c00 005d6ef0 005d6f70 005d8a30
	// AMethodPointer2Args<ASoundManager, TINT, TFLOAT>:                005d6c80 005d6f90 005d7010 005d8a70
	// AMethodPointer2Args<ASoundManager, TINT, TBOOL>:                 005d6d00 005d7050 005d70d0 005d8ad0
	// AMethodPointer3Args<ASoundComponent, ASoundId, TFLOAT, TFLOAT>:  005d3040 005d30c0 005d3140 005d32d0
	// $Barnyard: FUNCTION 005d3040
	// $Barnyard: FUNCTION 005d6b00
	// $Barnyard: FUNCTION 005d6b80
	// $Barnyard: FUNCTION 005d6c00
	// $Barnyard: FUNCTION 005d6c80
	// $Barnyard: FUNCTION 005d6d00
	// AGameStateController instantiations (Create, Clone, Call, GetSignature):
	// AMethodPointer0ArgsRT<TBOOL, AGameStateController>:                      00428bb0 00428eb0 005a2be0 004130d0
	// AMethodPointer1Args<AGameStateController, const TPString8&>:             00428c30 00428f30 00469550 005df820
	// AMethodPointer1Args<AGameStateController, TPString8>:                    00428cb0 00428fb0 00429e00 005df820
	// AMethodPointer0Args<AGameStateController>:                               00428d30 00429030 00633d20 00500720
	// AMethodPointer1ArgsRT<TBOOL, AGameStateController, const TPString8&>:    00428db0 004290b0 00429130 00546c90
	// AMethodPointer3Args<AGameStateController, TBOOL, TBOOL, TBOOL>:          00428e30 00429170 004291f0 0042a6f0
	// $Barnyard: FUNCTION 00428bb0
	// $Barnyard: FUNCTION 00428c30
	// $Barnyard: FUNCTION 00428cb0
	// $Barnyard: FUNCTION 00428d30
	// $Barnyard: FUNCTION 00428db0
	// $Barnyard: FUNCTION 00428e30
	// AGateManager instantiations (Create, Clone, Call, GetSignature):
	// AMethodPointer0Args<AGateManager>:                                       005ddc00 005dde00 00633d20 00500720
	// AMethodPointer1Args<AGateManager, TPString8>:                            005ddc80 005dde80 005deaa0 005df820
	// AMethodPointer1ArgsRT<TBOOL, AGateManager, TPString8>:                   005ddd00 005ddf00 005deb30 00546c90
	// AMethodPointer2Args<AGateManager, TPString8, TBOOL>:                     005ddd80 005ddf80 005debd0 005df860
	// $Barnyard: FUNCTION 005ddc00
	// $Barnyard: FUNCTION 005ddc80
	// $Barnyard: FUNCTION 005ddd00
	// $Barnyard: FUNCTION 005ddd80
	// ABarnExploreMinigameState instantiations (Create, Clone, Call, GetSignature):
	// AMethodPointer1Args<ABarnExploreMinigameState, TBOOL>:                   0049c980 0049cb80 005931b0 00593d00
	// AMethodPointer0ArgsRT<TBOOL, ABarnExploreMinigameState>:                 0049ca00 0049cc00 005a2be0 004130d0
	// AMethodPointer0Args<ABarnExploreMinigameState>:                          0049ca80 0049cc80 00633d20 00500720
	// AMethodPointer0ArgsRT<TPString8, ABarnExploreMinigameState>:             0049cb00 0049cd00 0049f8d0 0049ba10
	// $Barnyard: FUNCTION 0049c980
	// $Barnyard: FUNCTION 0049ca00
	// $Barnyard: FUNCTION 0049ca80
	// $Barnyard: FUNCTION 0049cb00
	// AMethodPointer0ArgsRT<TFLOAT, AInteractionEvent>: 005c5800, AMethodPointer1ArgsRT<AInteractionEvent*, AInteractionEventManager, TPString8>: 005c5df0
	// $Barnyard: FUNCTION 005c5800
	// $Barnyard: FUNCTION 005c5df0
	// $Barnyard: FUNCTION 005e2be0
	// $Barnyard: FUNCTION 005e2c60
	// $Barnyard: FUNCTION 0061fcf0
	// $Barnyard: FUNCTION 0062a150
	// $Barnyard: FUNCTION 0062a1d0
	// Instantiations in 0x401000-0x55ffff (Create, Clone, Call, GetSignature; - = folded/marked elsewhere):
	// AMethodPointer0ArgsRT<AUnit*, AUnitComponent>:                                0041b240 0041b2c0 00416d20 -
	// AMethodPointer1Args<AStaticUnit, TBOOL>:                                      00418f50 00418fd0 - -
	// AMethodPointer1Args<AMiniGameManager, const TPString8&>:                      004693d0 004694d0 - -
	// AMethodPointer2Args<AMiniGameManager, const TPString8&, TINT>:                00469450 00469570 004695f0 00469f70
	// AMethodPointer0Args<APrecisionCoyote>:                                        00500900 00500980 - -
	// AMethodPointer4Args<ATreasureManager, const TPString8&, TINT, const TPString8&, TINT>: 005313c0 005314c0 00531540 -
	// AMethodPointer0Args<ATreasureManager>:                                        00531440 005315a0 - -
	// AMethodPointer0Args<ALadderBeansGame>:                                        00537940 005379c0 - -
	// AMethodPointer0Args<ABikeManager>:                                            00540910 00540990 - -
	// AMethodPointer1ArgsRT<TBOOL, AGameTimeManager, TPString8>:                    00546800 00546880 00546b60 -
	// AMethodPointer1Args<AReplayManager, TPString8>:                               00547ff0 00548070 00549f90 -
	// (ACharacterUnit Clones 00417530 004175b0 00417630 and GetSignature 00417ae0, AGameStateController Call 00429e00 /
	// GetSignature 0042a6f0 and ABarnExploreMinigameState Call 0049f8d0 / GetSignature 0049ba10 are in the lists above.)
	// $Barnyard: FUNCTION 0041b240
	// $Barnyard: FUNCTION 00418f50
	// $Barnyard: FUNCTION 004693d0
	// $Barnyard: FUNCTION 00469450
	// $Barnyard: FUNCTION 00500900
	// $Barnyard: FUNCTION 005313c0
	// $Barnyard: FUNCTION 00531440
	// $Barnyard: FUNCTION 00537940
	// $Barnyard: FUNCTION 00540910
	// $Barnyard: FUNCTION 00546800
	// $Barnyard: FUNCTION 00547ff0
	static AMethodPointerNArgsRT* Create( const TCHAR* a_szName, Method a_fnMethod )
	{
		return new AMethodPointerNArgsRT( a_szName, a_fnMethod );
	}

	// NOTE: Allows binding const-qualified methods; the retail binder only takes non-const ones.
	// MSVC uses the same representation for both, so the stored pointer is reinterpreted.
	static AMethodPointerNArgsRT* Create( const TCHAR* a_szName, ConstMethod a_fnMethod )
	{
		return new AMethodPointerNArgsRT( a_szName, reinterpret_cast<Method>( a_fnMethod ) );
	}

	// Shared (identical code folding) scalar deleting destructor of every instantiation.
	// $Barnyard: FUNCTION 0061fe20
	~AMethodPointerNArgsRT() override = default;

	// Instantiation order as in Create: 0040fa30 0040fad0 0040fb50 0040fc10 00411eb0 004134d0 00413550 004135f0 004136e0 004137a0
	// $Barnyard: FUNCTION 0040fa30
	// $Barnyard: FUNCTION 0040fad0
	// $Barnyard: FUNCTION 0040fb50
	// $Barnyard: FUNCTION 0040fc10
	// $Barnyard: FUNCTION 00411eb0
	// $Barnyard: FUNCTION 004134d0
	// $Barnyard: FUNCTION 00413550
	// $Barnyard: FUNCTION 004135f0
	// $Barnyard: FUNCTION 004136e0
	// $Barnyard: FUNCTION 004137a0
	// AMiscFunctions instantiations, same order as their Create (00603d60 + 0x80 * n, except where sizes differ)
	// $Barnyard: FUNCTION 00603d60
	// $Barnyard: FUNCTION 00603de0
	// $Barnyard: FUNCTION 00603e60
	// $Barnyard: FUNCTION 00603ee0
	// $Barnyard: FUNCTION 00603fa0
	// $Barnyard: FUNCTION 00604070
	// $Barnyard: FUNCTION 00604140
	// $Barnyard: FUNCTION 006041c0
	// $Barnyard: FUNCTION 00604280
	// $Barnyard: FUNCTION 00604300
	// $Barnyard: FUNCTION 006043b0
	// $Barnyard: FUNCTION 00604430
	// $Barnyard: FUNCTION 006044e0
	// $Barnyard: FUNCTION 00604560
	// $Barnyard: FUNCTION 006045e0
	// $Barnyard: FUNCTION 006046b0
	// $Barnyard: FUNCTION 00604730
	// $Barnyard: FUNCTION 00562500
	// $Barnyard: FUNCTION 00562580
	// $Barnyard: FUNCTION 00562600
	// $Barnyard: FUNCTION 005626b0
	// $Barnyard: FUNCTION 005627a0
	// $Barnyard: FUNCTION 00562880
	// $Barnyard: FUNCTION 00562920
	// $Barnyard: FUNCTION 005629e0
	// $Barnyard: FUNCTION 00562a60
	// $Barnyard: FUNCTION 00562af0
	// $Barnyard: FUNCTION 00562b70
	// $Barnyard: FUNCTION 00562c50
	// $Barnyard: FUNCTION 00562cd0
	// $Barnyard: FUNCTION 00562d50
	// $Barnyard: FUNCTION 00562df0
	// $Barnyard: FUNCTION 00571660
	// $Barnyard: FUNCTION 005716e0
	// $Barnyard: FUNCTION 00571760
	// $Barnyard: FUNCTION 00571810
	// $Barnyard: FUNCTION 00571890
	// $Barnyard: FUNCTION 00571910
	// $Barnyard: FUNCTION 00571990
	// $Barnyard: FUNCTION 00571a30
	// $Barnyard: FUNCTION 00571b10
	// $Barnyard: FUNCTION 0057ba90
	// $Barnyard: FUNCTION 0057bb10
	// $Barnyard: FUNCTION 0058d320
	// $Barnyard: FUNCTION 0058d3a0
	// $Barnyard: FUNCTION 00593130
	// $Barnyard: FUNCTION 005a2b60
	// $Barnyard: FUNCTION 005a2c10
	// $Barnyard: FUNCTION 005a2d60
	// $Barnyard: FUNCTION 00633c20
	// $Barnyard: FUNCTION 00633ca0
	// $Barnyard: FUNCTION 00669c00
	// Unit / unit component instantiations (see the list above Create)
	// $Barnyard: FUNCTION 00415290
	// $Barnyard: FUNCTION 00415310
	// $Barnyard: FUNCTION 004153d0
	// $Barnyard: FUNCTION 00416260
	// $Barnyard: FUNCTION 004162e0
	// $Barnyard: FUNCTION 004163a0
	// $Barnyard: FUNCTION 00416420
	// $Barnyard: FUNCTION 00417370
	// $Barnyard: FUNCTION 004173f0
	// $Barnyard: FUNCTION 00417480
	// $Barnyard: FUNCTION 0058b000
	// $Barnyard: FUNCTION 0058b0b0
	// $Barnyard: FUNCTION 0058b160
	// $Barnyard: FUNCTION 00594a00
	// $Barnyard: FUNCTION 00594a80
	// $Barnyard: FUNCTION 00594b00
	// $Barnyard: FUNCTION 00594b80
	// $Barnyard: FUNCTION 005a09b0
	// $Barnyard: FUNCTION 005a0a30
	// $Barnyard: FUNCTION 005a0ab0
	// $Barnyard: FUNCTION 005a0b30
	// $Barnyard: FUNCTION 005a0c00
	// ASoundManager instantiations (see the list above Create)
	// $Barnyard: FUNCTION 005d6d80
	// $Barnyard: FUNCTION 005d6e30
	// $Barnyard: FUNCTION 005d6ef0
	// $Barnyard: FUNCTION 005d6f90
	// $Barnyard: FUNCTION 005d7050
	// $Barnyard: FUNCTION 005d30c0
	// AGameStateController, AGateManager and ABarnExploreMinigameState instantiations, same order as their Create
	// $Barnyard: FUNCTION 00428eb0
	// $Barnyard: FUNCTION 00428f30
	// $Barnyard: FUNCTION 00428fb0
	// $Barnyard: FUNCTION 00429030
	// $Barnyard: FUNCTION 004290b0
	// $Barnyard: FUNCTION 00429170
	// $Barnyard: FUNCTION 005dde00
	// $Barnyard: FUNCTION 005dde80
	// $Barnyard: FUNCTION 005ddf00
	// $Barnyard: FUNCTION 005ddf80
	// $Barnyard: FUNCTION 0049cb80
	// $Barnyard: FUNCTION 0049cc00
	// $Barnyard: FUNCTION 0049cc80
	// $Barnyard: FUNCTION 0049cd00
	// AMethodPointer0ArgsRT<TFLOAT, AInteractionEvent>: 005c5880, AMethodPointer1ArgsRT<AInteractionEvent*, AInteractionEventManager, TPString8>: 005c5e70
	// $Barnyard: FUNCTION 005c5880
	// $Barnyard: FUNCTION 005c5e70
	// $Barnyard: FUNCTION 005e2ce0
	// $Barnyard: FUNCTION 005e2dc0
	// $Barnyard: FUNCTION 0061fd70
	// $Barnyard: FUNCTION 0062a250
	// $Barnyard: FUNCTION 0062a2d0
	// 0x401000-0x55ffff instantiations (see the list above Create)
	// $Barnyard: FUNCTION 0041b2c0
	// $Barnyard: FUNCTION 00418fd0
	// $Barnyard: FUNCTION 00417530
	// $Barnyard: FUNCTION 004175b0
	// $Barnyard: FUNCTION 00417630
	// $Barnyard: FUNCTION 004694d0
	// $Barnyard: FUNCTION 00469570
	// $Barnyard: FUNCTION 00500980
	// $Barnyard: FUNCTION 005314c0
	// $Barnyard: FUNCTION 005315a0
	// $Barnyard: FUNCTION 005379c0
	// $Barnyard: FUNCTION 00540990
	// $Barnyard: FUNCTION 00546880
	// $Barnyard: FUNCTION 00548070
	AFunction* Clone() const override // +0x04
	{
		return new AMethodPointerNArgsRT( *this );
	}

	// Instantiation order as in Create: 0040fab0 00417500 0040fbd0 00469550 004124c0 005a2be0 004135d0 00413670 00413760 00413820
	// $Barnyard: FUNCTION 0040fab0
	// $Barnyard: FUNCTION 00417500
	// $Barnyard: FUNCTION 0040fbd0
	// $Barnyard: FUNCTION 00469550
	// $Barnyard: FUNCTION 004124c0
	// $Barnyard: FUNCTION 005a2be0
	// $Barnyard: FUNCTION 004135d0
	// $Barnyard: FUNCTION 00413670
	// $Barnyard: FUNCTION 00413760
	// $Barnyard: FUNCTION 00413820
	// AMiscFunctions instantiations in Create order (the ones folded with other classes' code are marked there):
	// 00604c50 00604ce0 00604d80 00603f60 00604020 006040f0 00604e10 00604240 (00571a10) 00604380 006051f0 006044b0
	// (00571b90) (00562900) 00604660 (00562ae0) 006047b0
	// $Barnyard: FUNCTION 00604c50
	// $Barnyard: FUNCTION 00604ce0
	// $Barnyard: FUNCTION 00604d80
	// $Barnyard: FUNCTION 00603f60
	// $Barnyard: FUNCTION 00604020
	// $Barnyard: FUNCTION 006040f0
	// $Barnyard: FUNCTION 00604e10
	// $Barnyard: FUNCTION 00604240
	// $Barnyard: FUNCTION 00604380
	// $Barnyard: FUNCTION 006051f0
	// $Barnyard: FUNCTION 006044b0
	// $Barnyard: FUNCTION 00604660
	// $Barnyard: FUNCTION 006047b0
	// $Barnyard: FUNCTION 00565c10
	// $Barnyard: FUNCTION 00562680
	// $Barnyard: FUNCTION 00562730
	// $Barnyard: FUNCTION 00562820
	// $Barnyard: FUNCTION 00562900
	// $Barnyard: FUNCTION 005629a0
	// $Barnyard: FUNCTION 00565cb0
	// $Barnyard: FUNCTION 00562ae0
	// $Barnyard: FUNCTION 00565d60
	// $Barnyard: FUNCTION 00562bf0
	// $Barnyard: FUNCTION 00565e10
	// $Barnyard: FUNCTION 00571b90
	// $Barnyard: FUNCTION 00562dd0
	// $Barnyard: FUNCTION 00562e70
	// $Barnyard: FUNCTION 005717e0
	// $Barnyard: FUNCTION 00571a10
	// $Barnyard: FUNCTION 00571ab0
	// $Barnyard: FUNCTION 0058d590
	// $Barnyard: FUNCTION 0058d6d0
	// $Barnyard: FUNCTION 005931b0
	// $Barnyard: FUNCTION 00634560
	// $Barnyard: FUNCTION 00633d20
	// $Barnyard: FUNCTION 00669c80
	// Unit / unit component instantiations (see the list above Create)
	// $Barnyard: FUNCTION 00415390
	// $Barnyard: FUNCTION 00416360
	// $Barnyard: FUNCTION 004164a0
	// $Barnyard: FUNCTION 0058b080
	// $Barnyard: FUNCTION 0058b130
	// $Barnyard: FUNCTION 005a0bb0
	// ASoundManager instantiations (see the list above Create)
	// $Barnyard: FUNCTION 005d6e00
	// $Barnyard: FUNCTION 005d6eb0
	// $Barnyard: FUNCTION 005d6f70
	// $Barnyard: FUNCTION 005d7010
	// $Barnyard: FUNCTION 005d70d0
	// $Barnyard: FUNCTION 005d3140
	// AGameStateController: IsStateOnStack, SaveCurrentGameState
	// $Barnyard: FUNCTION 00429130
	// $Barnyard: FUNCTION 004291f0
	// AMethodPointer2ArgsRT<AInteractionSequence*, AInteractionManager, AUnit*, TPString8>: 005a39c0, AMethodPointer2Args<AInteractionManager, TPString8, TINT>: 005a3a70,
	// AMethodPointer1ArgsRT<AInteractionEvent*, AInteractionEventManager, TPString8>: 005c68d0
	// $Barnyard: FUNCTION 005a39c0
	// $Barnyard: FUNCTION 005a3a70
	// $Barnyard: FUNCTION 005c68d0
	// AGateManager: AMethodPointer1Args<, TPString8>: 005deaa0, AMethodPointer1ArgsRT<TBOOL, , TPString8>: 005deb30,
	// AMethodPointer2Args<, TPString8, TBOOL>: 005debd0
	// $Barnyard: FUNCTION 005deaa0
	// $Barnyard: FUNCTION 005deb30
	// $Barnyard: FUNCTION 005debd0
	// $Barnyard: FUNCTION 005e2d60
	// $Barnyard: FUNCTION 0061fdf0
	// $Barnyard: FUNCTION 0062af60
	// 0x401000-0x55ffff instantiations (see the list above Create)
	// $Barnyard: FUNCTION 00416d20
	// $Barnyard: FUNCTION 004695f0
	// $Barnyard: FUNCTION 00531540
	// $Barnyard: FUNCTION 00546b60
	// $Barnyard: FUNCTION 00549f90
	// $Barnyard: FUNCTION 0049f8d0
	// $Barnyard: FUNCTION 00429e00
	void Call( Toshi::TObject* a_pObject, PPropertyValue* a_pResult, const PPropertyValueArray* a_pArgs ) const override // +0x0C
	{
		CallImpl( a_pObject, a_pResult, a_pArgs, std::index_sequence_for<TArgs...>() );
	}

	// Instantiation order as in Create: 00410aa0 00412780 00410bb0 005df820 00412780 004130d0 004130e0 004142c0 00414330 00604fd0
	// (00412780 is shared by the two <AUnit*, ..., TPString8> instantiations)
	// $Barnyard: FUNCTION 00410aa0
	// $Barnyard: FUNCTION 00412780
	// $Barnyard: FUNCTION 00410bb0
	// $Barnyard: FUNCTION 005df820
	// $Barnyard: FUNCTION 004130d0
	// $Barnyard: FUNCTION 004130e0
	// $Barnyard: FUNCTION 004142c0
	// $Barnyard: FUNCTION 00414330
	// $Barnyard: FUNCTION 00604fd0
	// AMiscFunctions instantiations in Create order (005df820 and 00604fd0 above):
	// 005df820 00604f10 00604f70 00604fd0 00605010 00605070 006050d0 00605130 (00571070) 00605190 (0049ba10) 006052c0
	// (00593d00) 004130d0 00605300 (00500720) 00602cf0
	// $Barnyard: FUNCTION 00604f10
	// $Barnyard: FUNCTION 00604f70
	// $Barnyard: FUNCTION 00605010
	// $Barnyard: FUNCTION 00605070
	// $Barnyard: FUNCTION 006050d0
	// $Barnyard: FUNCTION 00605130
	// $Barnyard: FUNCTION 00605190
	// $Barnyard: FUNCTION 006052c0
	// $Barnyard: FUNCTION 00605300
	// $Barnyard: FUNCTION 00602cf0
	// $Barnyard: FUNCTION 00546c90
	// $Barnyard: FUNCTION 00566540
	// $Barnyard: FUNCTION 00566570
	// $Barnyard: FUNCTION 005665f0
	// $Barnyard: FUNCTION 00572040
	// $Barnyard: FUNCTION 00566660
	// $Barnyard: FUNCTION 00500720
	// $Barnyard: FUNCTION 005666c0
	// $Barnyard: FUNCTION 00566720
	// $Barnyard: FUNCTION 00571800
	// $Barnyard: FUNCTION 00571070
	// $Barnyard: FUNCTION 00572080
	// $Barnyard: FUNCTION 00532770
	// $Barnyard: FUNCTION 0058d910
	// $Barnyard: FUNCTION 005a1be0
	// Unit / unit component instantiations (see the list above Create)
	// $Barnyard: FUNCTION 00417470
	// ASoundManager instantiations (see the list above Create, 0049a010 is marked in AGopherShopInterface.cpp)
	// $Barnyard: FUNCTION 005d89e0
	// $Barnyard: FUNCTION 005d8a30
	// $Barnyard: FUNCTION 005d8a70
	// $Barnyard: FUNCTION 005d8ad0
	// $Barnyard: FUNCTION 005d32d0
	// AMethodPointer2ArgsRT<AInteractionSequence*, AInteractionManager, AUnit*, TPString8>: 005a43d0, AMethodPointer1ArgsRT<AInteractionEvent*, AInteractionEventManager, TPString8>: 005c6a90
	// $Barnyard: FUNCTION 005a43d0
	// $Barnyard: FUNCTION 005c6a90
	// $Barnyard: FUNCTION 005e6eb0
	// $Barnyard: FUNCTION 00620000
	// $Barnyard: FUNCTION 0062b150
	// AMethodPointer2Args<AGateManager, TPString8, TBOOL>: 005df860
	// $Barnyard: FUNCTION 005df860
	// 0x401000-0x55ffff instantiations (see the list above Create)
	// $Barnyard: FUNCTION 00417ae0
	// $Barnyard: FUNCTION 00469f70
	// $Barnyard: FUNCTION 0049ba10
	// $Barnyard: FUNCTION 0042a6f0
	void GetSignature( const Toshi::TClass** a_ppReturnType, Toshi::TArray<const Toshi::TClass*>& a_rArgTypes ) const override // +0x10
	{
		*a_ppReturnType = AFunctionArg<TRet>::GetType();
		( a_rArgTypes.Push( AFunctionArg<TArgs>::GetType() ), ... );
	}

private:
	AMethodPointerNArgsRT( const TCHAR* a_szName, Method a_fnMethod )
	    : AMethodPointer( a_szName )
	    , m_fnMethod( a_fnMethod )
	{}

	AMethodPointerNArgsRT( const AMethodPointerNArgsRT& a_rOther )
	    : AMethodPointer( a_rOther )
	    , m_fnMethod( a_rOther.m_fnMethod )
	{}

	template <TSIZE... Indices>
	void CallImpl( Toshi::TObject* a_pObject, PPropertyValue* a_pResult, const PPropertyValueArray* a_pArgs, std::index_sequence<Indices...> ) const
	{
		T* pThis = TSTATICCAST( T, a_pObject );

		if constexpr ( std::is_void_v<TRet> )
		{
			( pThis->*m_fnMethod )( AFunctionArg<TArgs>::Get( a_pArgs->GetValue( TINT( Indices ) ) )... );
		}
		else
		{
			AFunctionArg<TRet>::Set( a_pResult, ( pThis->*m_fnMethod )( AFunctionArg<TArgs>::Get( a_pArgs->GetValue( TINT( Indices ) ) )... ) );
		}
	}

private:
	Method m_fnMethod; // 0x08 (+ this adjustor at 0x0C)
};

template <class T>
using AMethodPointer0Args = AMethodPointerNArgsRT<void, T>;

template <class T, class A1>
using AMethodPointer1Args = AMethodPointerNArgsRT<void, T, A1>;

template <class T, class A1, class A2>
using AMethodPointer2Args = AMethodPointerNArgsRT<void, T, A1, A2>;

template <class T, class A1, class A2, class A3>
using AMethodPointer3Args = AMethodPointerNArgsRT<void, T, A1, A2, A3>;

template <class T, class A1, class A2, class A3, class A4>
using AMethodPointer4Args = AMethodPointerNArgsRT<void, T, A1, A2, A3, A4>;

template <class T, class A1, class A2, class A3, class A4, class A5>
using AMethodPointer5Args = AMethodPointerNArgsRT<void, T, A1, A2, A3, A4, A5>;

template <class R, class T>
using AMethodPointer0ArgsRT = AMethodPointerNArgsRT<R, T>;

template <class R, class T, class A1>
using AMethodPointer1ArgsRT = AMethodPointerNArgsRT<R, T, A1>;

template <class R, class T, class A1, class A2>
using AMethodPointer2ArgsRT = AMethodPointerNArgsRT<R, T, A1, A2>;

template <class R, class T, class A1, class A2, class A3>
using AMethodPointer3ArgsRT = AMethodPointerNArgsRT<R, T, A1, A2, A3>;

template <class R, class T, class A1, class A2, class A3, class A4>
using AMethodPointer4ArgsRT = AMethodPointerNArgsRT<R, T, A1, A2, A3, A4>;

template <class R, class T, class A1, class A2, class A3, class A4, class A5>
using AMethodPointer5ArgsRT = AMethodPointerNArgsRT<R, T, A1, A2, A3, A4, A5>;
