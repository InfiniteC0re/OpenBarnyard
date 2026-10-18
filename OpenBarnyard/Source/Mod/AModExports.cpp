#include "pch.h"

// HACK:
#if defined( BARNYARD_COMMUNITY_PATCH ) && defined( TOSHI_SKU_WINDOWS ) && defined( _M_IX86 )

// File/TCompress_Compress.cpp: TCompress::Compress
#  pragma comment( linker, "/INCLUDE:?Compress@TCompress@Toshi@@SAIPAVTFile@2@PAEII_N@Z" )

// Platform/Windows/T2NamedPipeServer_Win.cpp: T2NamedPipeServer::T2NamedPipeServer()
#  pragma comment( linker, "/INCLUDE:??0T2NamedPipeServer@Toshi@@QAE@XZ" )

#endif
