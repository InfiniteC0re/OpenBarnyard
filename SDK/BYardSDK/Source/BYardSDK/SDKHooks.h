#pragma once

// General utility macro
#define PP_CAT( A, B )   A##B
#define PP_EXPAND( ... ) __VA_ARGS__

// Macro overloading feature support
#define PP_VA_ARG_SIZE( ... ) PP_EXPAND( PP_APPLY_ARG_N( ( PP_ZERO_ARGS_DETECT( __VA_ARGS__ ), PP_RSEQ_N ) ) )

#define PP_ZERO_ARGS_DETECT( ... )                          PP_EXPAND( PP_ZERO_ARGS_DETECT_PREFIX_##__VA_ARGS__##_ZERO_ARGS_DETECT_SUFFIX )
#define PP_ZERO_ARGS_DETECT_PREFIX__ZERO_ARGS_DETECT_SUFFIX , , , , , , , , , , , 0

#define PP_APPLY_ARG_N( ARGS )                                                         PP_EXPAND( PP_ARG_N ARGS )
#define PP_ARG_N( _0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, N, ... ) N
#define PP_RSEQ_N                                                                      14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0

#define PP_OVERLOAD_SELECT( NAME, NUM ) PP_CAT( NAME##_, NUM )
#define PP_MACRO_OVERLOAD( NAME, ... )  PP_OVERLOAD_SELECT( NAME, PP_VA_ARG_SIZE( __VA_ARGS__ ) )( __VA_ARGS__ )

#define CALL_THIS_4( ADDR, TYPE, RET_TYPE, THIS )                                                                             ( ( RET_TYPE( __thiscall* )( TYPE pThis ) )( ADDR ) )( THIS )
#define CALL_THIS_6( ADDR, TYPE, RET_TYPE, THIS, TYPE1, VALUE1 )                                                              ( ( RET_TYPE( __thiscall* )( TYPE, TYPE1 ) )( ADDR ) )( THIS, VALUE1 )
#define CALL_THIS_8( ADDR, TYPE, RET_TYPE, THIS, TYPE1, VALUE1, TYPE2, VALUE2 )                                               ( ( RET_TYPE( __thiscall* )( TYPE, TYPE1, TYPE2 ) )( ADDR ) )( THIS, VALUE1, VALUE2 )
#define CALL_THIS_10( ADDR, TYPE, RET_TYPE, THIS, TYPE1, VALUE1, TYPE2, VALUE2, TYPE3, VALUE3 )                               ( ( RET_TYPE( __thiscall* )( TYPE, TYPE1, TYPE2, TYPE3 ) )( ADDR ) )( THIS, VALUE1, VALUE2, VALUE3 )
#define CALL_THIS_12( ADDR, TYPE, RET_TYPE, THIS, TYPE1, VALUE1, TYPE2, VALUE2, TYPE3, VALUE3, TYPE4, VALUE4 )                ( ( RET_TYPE( __thiscall* )( TYPE, TYPE1, TYPE2, TYPE3, TYPE4 ) )( ADDR ) )( THIS, VALUE1, VALUE2, VALUE3, VALUE4 )
#define CALL_THIS_14( ADDR, TYPE, RET_TYPE, THIS, TYPE1, VALUE1, TYPE2, VALUE2, TYPE3, VALUE3, TYPE4, VALUE4, TYPE5, VALUE5 ) ( ( RET_TYPE( __thiscall* )( TYPE, TYPE1, TYPE2, TYPE3, TYPE4, TYPE5 ) )( ADDR ) )( THIS, VALUE1, VALUE2, VALUE3, VALUE4, VALUE5 )
#define CALL_THIS( ... )                                                                                                      PP_MACRO_OVERLOAD( CALL_THIS, __VA_ARGS__ )

#define CALL_2( ADDR, RET_TYPE )                                                                             ( ( RET_TYPE( __stdcall* )() )( ADDR ) )()
#define CALL_4( ADDR, RET_TYPE, TYPE1, VALUE1 )                                                              ( ( RET_TYPE( __stdcall* )( TYPE1 ) )( ADDR ) )( VALUE1 )
#define CALL_6( ADDR, RET_TYPE, TYPE1, VALUE1, TYPE2, VALUE2 )                                               ( ( RET_TYPE( __stdcall* )( TYPE1, TYPE2 ) )( ADDR ) )( VALUE1, VALUE2 )
#define CALL_8( ADDR, RET_TYPE, TYPE1, VALUE1, TYPE2, VALUE2, TYPE3, VALUE3 )                                ( ( RET_TYPE( __stdcall* )( TYPE1, TYPE2, TYPE3 ) )( ADDR ) )( VALUE1, VALUE2, VALUE3 )
#define CALL_10( ADDR, RET_TYPE, TYPE1, VALUE1, TYPE2, VALUE2, TYPE3, VALUE3, TYPE4, VALUE4 )                ( ( RET_TYPE( __stdcall* )( TYPE1, TYPE2, TYPE3, TYPE4 ) )( ADDR ) )( VALUE1, VALUE2, VALUE3, VALUE4 )
#define CALL_12( ADDR, RET_TYPE, TYPE1, VALUE1, TYPE2, VALUE2, TYPE3, VALUE3, TYPE4, VALUE4, TYPE5, VALUE5 ) ( ( RET_TYPE( __stdcall* )( TYPE1, TYPE2, TYPE3, TYPE4, TYPE5 ) )( ADDR ) )( VALUE1, VALUE2, VALUE3, VALUE4, VALUE5 )
#define CALL( ... )                                                                                          PP_MACRO_OVERLOAD( CALL, __VA_ARGS__ )
