#pragma once

#define ISZERO( X )                 ( ( X ) == 0 )
#define NOTZERO( X )                ( !ISZERO( X ) )
#define HASANYFLAG( STATE, FLAG )   ( ( ( STATE ) & ( FLAG ) ) != 0 )
#define HASALLFLAGS( STATE, FLAGS ) ( ( ( STATE ) & ( FLAGS ) ) == ( FLAGS ) )
#define STRINGIFY8( X )             #X
#define STRINGIFY16( x )            L##x
#define STRINGIFY( X )              STRINGIFY8( X )
#define BITFLAG( x )                ( 1U << ( x ) )
#define CONCATTOKEN( x, y )         x##y
#define OVERRIDE                    override

#define TVERSION( VER_MAJOR, VER_MINOR ) ( ( VER_MAJOR << 16U ) | VER_MINOR )

#define RETURN_ARG_COUNT( _1_, _2_, _3_, _4_, _5_, count, ... ) count
#define EXPAND_ARGS( args )                                     RETURN_ARG_COUNT args
#define COUNT_ARGS_MAX5( ... )                                  EXPAND_ARGS( ( __VA_ARGS__, 5, 4, 3, 2, 1, 0 ) )

#define OVERLOAD_MACRO2( name, count ) name##count
#define OVERLOAD_MACRO1( name, count ) OVERLOAD_MACRO2( name, count )
#define OVERLOAD_MACRO( name, count )  OVERLOAD_MACRO1( name, count )

#define GLUE( x, y )               x y
#define CALL_OVERLOAD( name, ... ) GLUE( OVERLOAD_MACRO( name, COUNT_ARGS_MAX5( __VA_ARGS__ ) ), ( __VA_ARGS__ ) )

#ifdef TOSHI_DYNAMIC_LINKING

#  ifdef TOSHI_USER_ENGINE

#	define TOSHI_API declspec( dllexport )

#  else // TOSHI_USER_ENGINE

#	define TOSHI_API declspec( dllimport )

#  endif // !TOSHI_USER_ENGINE

#else // TOSHI_DYNAMIC_LINKING

#  define TOSHI_API

#endif // !TOSHI_DYNAMIC_LINKING

#define TOSHI_NAMESPACE       Toshi
#define TOSHI_NAMESPACE_USING using namespace TOSHI_NAMESPACE;
#define TOSHI_NAMESPACE_START \
	namespace TOSHI_NAMESPACE \
	{
#define TOSHI_NAMESPACE_END }

#ifdef TOSHI_FINAL
#  define TOSHI_NO_LOGS
#endif

#ifdef TOSHI_DEBUG
#  ifdef TOSHI_SKU_WINDOWS
#	define TBREAK() __debugbreak()
#	define TOSHI_ENABLE_ASSERTS
#  endif // TOSHI_SKU_WINDOWS
#endif   // TOSHI_DEBUG

#ifndef TBREAK
#  define TBREAK()
#endif // TBREAK

#define TSTATICASSERT( ... )              static_assert( __VA_ARGS__, "Compile time assert failed: " #__VA_ARGS__ )
#define TREINTERPRETCAST( TYPE, VALUE )   ( reinterpret_cast<TYPE>( VALUE ) )
#define TSTATICCAST( POINTERTYPE, VALUE ) ( static_cast<POINTERTYPE*>( VALUE ) )
#define TARRAYSIZE( ARRAY )               ( sizeof( ARRAY ) / sizeof( *ARRAY ) )
#define TOFFSETOF( CLASS, MEMBER )        offsetof( CLASS, MEMBER )
#define TALIGNAS( VALUE )                 alignas( VALUE )
#define TINLINE                           inline
#define TFORCEINLINE                      __forceinline

#define TANONYMOUSVAR_1( Y )                CONCATTOKEN( _anonymous, Y )
#define TANONYMOUSVAR( TYPE )               TYPE TANONYMOUSVAR_1( __COUNTER__ )
#define TANONYMOUSVARC( TYPE, COUNTER )     TYPE TANONYMOUSVAR_1( COUNTER )
#define TANONYMOUSVARC_GET( TYPE, COUNTER ) TANONYMOUSVAR_1( COUNTER )

#define TDEPRECATED                  [[deprecated]]
#define TDEPRECATED_REASON( REASON ) [[deprecated( REASON )]]

#define TDECLARE_POINTER_HANDLE( NAME ) typedef void* NAME

#ifdef TOSHI_ENABLE_ASSERTS
#  define TCALL_ONCE_IF( CONDITION ) \
	  static TBOOL s_bFlag;          \
	  if ( !s_bFlag && ( CONDITION ) && ( s_bFlag = TTRUE ) )
#  define TWIP()                                                                       \
	  {                                                                                \
		  TCALL_ONCE_IF( TTRUE )                                                       \
		  {                                                                            \
			  TWARN( "Work in progress: '%s', at line %d\n", __FUNCTION__, __LINE__ ); \
		  }                                                                            \
	  }
#  define TTODO( DESC )                                                               \
	  {                                                                               \
		  TCALL_ONCE_IF( TTRUE )                                                      \
		  {                                                                           \
			  TWARN( "TODO: %s ('%s', at line %d)\n", DESC, __FUNCTION__, __LINE__ ); \
		  }                                                                           \
	  }
#  define TFIXME( DESC )                                                               \
	  {                                                                                \
		  TCALL_ONCE_IF( TTRUE )                                                       \
		  {                                                                            \
			  TWARN( "FIXME: %s ('%s', at line %d)\n", DESC, __FUNCTION__, __LINE__ ); \
		  }                                                                            \
	  }
#  define TIMPLEMENT()                                          \
	  {                                                         \
		  TCALL_ONCE_IF( TTRUE )                                \
		  {                                                     \
			  TWARN( "%s is not implemented\n", __FUNCTION__ ); \
		  }                                                     \
	  }
#  define TIMPLEMENT_D( DESC )                                            \
	  {                                                                   \
		  TCALL_ONCE_IF( TTRUE )                                          \
		  {                                                               \
			  TWARN( "%s is not implemented: %s\n", __FUNCTION__, DESC ); \
		  }                                                               \
	  }
#  define TASSERT( X, ... )                                                       \
	  {                                                                           \
		  TCALL_ONCE_IF( !( X ) )                                                 \
		  if ( 1 == TDebug_AssertHandler( #X, __FILE__, __LINE__, __VA_ARGS__ ) ) \
			  TBREAK();                                                           \
	  }
#  define TVALIDPTR( PTR ) TASSERT( TDebug_IsValidAddress( PTR ) )
#else // TOSHI_ENABLE_ASSERTS
#  define TWIP()
#  define TTODO( DESC )
#  define TFIXME( DESC )
#  define TIMPLEMENT()
#  define TIMPLEMENT_D( DESC )
#  define TASSERT( X, ... ) ( TFALSE )
#  define TVALIDPTR( PTR )
#endif // TOSHI_ENABLE_ASSERTS

#define TUNREACHABLE()                                     \
	{                                                      \
		TASSERT( TFALSE, "Unreachable code is reached!" ); \
		__assume( 0 );                                     \
	}

#ifndef TOSHI_NO_LOGS
#  define TLOGUP()         Toshi::TUtil::LogUp();
#  define TLOGDOWN()       Toshi::TUtil::LogDown();
#  define TINFO( ... )     Toshi::TUtil::Log( Toshi::TUtil::LogType_Info, __VA_ARGS__ )
#  define TWARN( ... )     Toshi::TUtil::Log( Toshi::TUtil::LogType_Warning, __VA_ARGS__ )
#  define TERROR( ... )    Toshi::TUtil::Log( Toshi::TUtil::LogType_Error, __VA_ARGS__ )
#  define TCRITICAL( ... ) Toshi::TUtil::Log( Toshi::TUtil::LogType_Critical, __VA_ARGS__ )
#  define TTRACE( ... )    TDebug_FinalPrintf( __VA_ARGS__ )
#else // TOSHI_NO_LOGS
#  define TLOGUP()
#  define TLOGDOWN()
#  define TINFO( ... )
#  define TWARN( ... )
#  define TERROR( ... )
#  define TCRITICAL( ... )
#  define TTRACE( ... )
#endif // TOSHI_NO_LOGS

#ifndef TOSHI_USER_ENGINE
#  define TOSHI_USER_CLIENT
#endif // !TOSHI_USER_ENGINE
