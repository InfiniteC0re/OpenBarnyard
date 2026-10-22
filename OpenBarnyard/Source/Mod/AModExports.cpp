#include "pch.h"

#include <ToshiTools/tinyxml2.h>

// HACK:
#if defined( BARNYARD_COMMUNITY_PATCH ) && defined( TOSHI_SKU_WINDOWS ) && defined( _M_IX86 )

// File/TCompress_Compress.cpp: TCompress::Compress
#  pragma comment( linker, "/INCLUDE:?Compress@TCompress@Toshi@@SAIPAVTFile@2@PAEII_N@Z" )

// Platform/Windows/T2NamedPipeServer_Win.cpp: T2NamedPipeServer::T2NamedPipeServer()
#  pragma comment( linker, "/INCLUDE:??0T2NamedPipeServer@Toshi@@QAE@XZ" )

// ToshiTools/tinyxml2.cpp: unused by the game, referencing it keeps it for the mods
void AModExports_KeepTinyXML2()
{
	tinyxml2::XMLDocument oDocument;
}

#  ifdef TRENDERINTERFACE_DX11
// Platform/DX11/TShaderCombo_DX11.cpp: TShaderCombo::TShaderCombo()
#    pragma comment( linker, "/INCLUDE:??0TShaderCombo@Toshi@@QAE@XZ" )
#  endif

#endif
