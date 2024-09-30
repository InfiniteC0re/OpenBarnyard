#include "pch.h"
#include "AAssetPack.h"

TOSHI_NAMESPACE_USING

TBOOL AAssetPack::Load( const char* a_szFileName )
{
	m_FileName    = a_szFileName;
	TBOOL bReaded = m_TRBFile.ReadFromFile( a_szFileName );

	m_bLoaded = TFALSE;

	if ( bReaded )
	{
		auto pInSECT = m_TRBFile.GetSections();
		auto pInSYMB = m_TRBFile.GetSymbols();

		for ( TUINT i = 0; i < pInSYMB->GetCount(); i++ )
		{
			TString8 name   = pInSYMB->GetName( i );
			auto     pStack = pInSYMB->GetStack( pInSECT, i );

			char szAssetFileName[ MAX_PATH ];
			auto iUnderlinePos = name.FindReverse( '_', -1 );

			TStringManager::String8Copy( szAssetFileName, name, iUnderlinePos );

			if ( TINT32 iLODPos = name.Find( "LOD" ) )
			{
				szAssetFileName[ iLODPos >= 0 ? iLODPos - 1 : iUnderlinePos ] = 0;
			}

			Asset_t* pFound = TNULL;

			for ( TINT k = m_Assets.Size() - 1; k >= 0; k-- )
			{
				if ( m_Assets[ k ].pStack == pStack )
				{
					pFound = &m_Assets[ k ];
					break;
				}
			}

			if ( !pFound )
			{
				m_Assets.PushBack( { .Name = TPS8D( szAssetFileName ), .Format = "trb", .pTRB = &m_TRBFile, .pStack = pStack } );
				auto pAsset = m_Assets.Back();

				TINT iAssetFileNameLength = TStringManager::String8Length( szAssetFileName );

				for ( TUINT k = i; k < pInSYMB->GetCount(); k++ )
				{
					TString8 inSymbolName     = pInSYMB->GetName( k );
					auto     pInSymbolAddress = pInSYMB->GetByIndex<char>( pInSECT, k );

					if ( pInSymbolAddress.stack() == pAsset->pStack )
					{
						if ( inSymbolName.Find( "_keylib" ) >= 0 )
						{
							pAsset->Format[ 0 ] = 't';
							pAsset->Format[ 1 ] = 'k';
							pAsset->Format[ 2 ] = 'l';
						}
						else if ( inSymbolName.Find( "_TTL" ) >= 0 )
						{
							pAsset->Format[ 0 ] = 't';
							pAsset->Format[ 1 ] = 't';
							pAsset->Format[ 2 ] = 'l';
						}

						if ( inSymbolName.Compare( szAssetFileName, iAssetFileNameLength ) == 0 )
						{
							auto name = TPString8( inSymbolName );
							pAsset->RelatedSymbols.Push( name );
							m_SymbolToAsset.Insert( name, pAsset );
						}
					}
					else
					{
						break;
					}
				}
			}
		}

		for ( auto it = m_SymbolToAsset.Begin(); it != m_SymbolToAsset.End(); it++ )
		{
			continue;
		}

		m_bLoaded = TTRUE;
	}

	return m_bLoaded;
}

void AAssetPack::Save( const char* a_szFileName, TBOOL a_bCompress )
{
	PTRBWriter outFile;

	auto pSECT = outFile.GetSections();
	auto pSYMB = outFile.GetSymbols();

	for ( auto asset = m_Assets.Begin(); asset != m_Assets.End(); asset++ )
	{
		auto pInStack  = asset->pStack;
		auto pOutStack = pSECT->CreateStream( pInStack );

		for ( auto symbol = asset->RelatedSymbols.Begin(); !symbol.IsOver(); symbol++ )
		{
			auto  pInPtr  = asset->pTRB->GetSymbols()->Find<char>( asset->pTRB->GetSections(), symbol->GetString8() );
			void* pOutPtr = pOutStack->GetBuffer() + ( pInPtr.get() - pInStack->GetBuffer() );

			if ( symbol->GetString8().Find( asset->Name ) == 0 )
			{
				pSYMB->Add( pOutStack, symbol->GetString8(), pOutPtr );
			}
			else
			{
				auto symbolName = TString8::VarArgs( "%s_%s", asset->Name.GetString8().GetString(), symbol->GetString8().GetString() );
				pSYMB->Add( pOutStack, symbolName, pOutPtr );
			}
		}
	}

	outFile.WriteToFile( a_szFileName, a_bCompress );
}

AAssetPack::Asset_t* AAssetPack::GetAssetFromSymbol( const Toshi::TPString8& a_rSymbolName )
{
	auto ppResult = m_SymbolToAsset.Find( a_rSymbolName );

	if ( ppResult != m_SymbolToAsset.End() )
	{
		return ppResult->GetSecond();
	}

	return TNULL;
}
