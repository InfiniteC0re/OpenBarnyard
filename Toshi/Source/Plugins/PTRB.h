#pragma once
//-----------------------------------------------------------------------------
// This plugin helps writing your own TRB (Toshi Relocatable Binary) files
// using easy API that takes care of everything needed to make working TRB.
// Also, supports BTEC compressing algorithm when reading or writing.
// Note: does not work on x64 and was not tested on big endian systems!
//-----------------------------------------------------------------------------

#include <File/TTSF.h>
#include <Toshi/TString8.h>

// TODO: replace STL classes with native TOSHI classes
#include <vector>
#include <string>

#ifndef __TOSHI_TTRB_H__
#  error Include TTRB.h to use this extension
#endif

#ifndef __TOSHI_PLUGIN_PTRB_H__
#  define __TOSHI_PLUGIN_PTRB_H__
#endif

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

class PTRBHeader;
class PTRBSections;
class PTRBRelocations;
class PTRBSymbols;

class PTRBHeader
{
public:
	PTRBHeader( Toshi::TVersion version );

	void SetVersion( Toshi::TVersion version );
	void SetSectionCount( int32_t count );
	void Write( Toshi::TTSFO& ttsfo, PTRBSections& sect );
	void Read( Toshi::TTSFI& ttsfi, PTRBSections& sect );

private:
	Toshi::TTRB::Header m_Header;
};

class PTRBRelocations
{
public:
	PTRBRelocations() = default;

	void Write( Toshi::TTSFO& ttsfo, PTRBSections& sect );
	void Read( Toshi::TTSFI& ttsfi, PTRBSections& sect );
};

class PTRBSections
{
public:
	class MemoryStream
	{
	public:
		friend PTRBSections;
		friend PTRBSymbols;
		friend PTRBRelocations;

		static constexpr TUINT BUFFER_GROW_SIZE = 4096;

		template <class T>
		class Ptr
		{
		public:
			Ptr() :
			    m_Stack( TNULL ), m_Offset( 0 ) {}
			Ptr( PTRBSections::MemoryStream* stack, TUINT offset ) :
			    m_Stack( stack ), m_Offset( offset ) {}
			Ptr( PTRBSections::MemoryStream* stack, T* ptr ) :
			    m_Stack( stack ), m_Offset( stack->GetOffset( ptr ) ) {}

			T* get()
			{
				TASSERT( this->operator TBOOL() == TTRUE, "Pointer is null" );
				return reinterpret_cast<T*>( m_Stack->GetBuffer() + m_Offset );
			}

			const T* get() const
			{
				TASSERT( this->operator TBOOL() == TTRUE, "Pointer is null" );
				return reinterpret_cast<const T*>( m_Stack->GetBuffer() + m_Offset );
			}

			PTRBSections::MemoryStream* stack()
			{
				return m_Stack;
			}

			TUINT offset() const
			{
				return m_Offset;
			}

			T& operator*()
			{
				TASSERT( this->operator TBOOL() == TTRUE, "Pointer is null" );
				return *reinterpret_cast<T*>( m_Stack->GetBuffer() + m_Offset );
			}

			T* operator->()
			{
				TASSERT( this->operator TBOOL() == TTRUE, "Pointer is null" );
				return reinterpret_cast<T*>( m_Stack->GetBuffer() + m_Offset );
			}

			template <typename N>
			Ptr<T> operator+( const N& a_iValue )
			{
				return Ptr<T>{ m_Stack, m_Offset + sizeof( T ) * a_iValue };
			}

			template <typename N>
			Ptr<T> operator-( const N& a_iValue )
			{
				return Ptr<T>{ m_Stack, m_Offset - sizeof( T ) * a_iValue };
			}

			operator TBOOL() const
			{
				return m_Stack != TNULL;
			}

		private:
			PTRBSections::MemoryStream* m_Stack;
			TUINT                       m_Offset;
		};

		struct RelcPtr
		{
			TUINT                       Offset;
			TUINT                       DataPtr;
			PTRBSections::MemoryStream* DataStack;
		};

	public:
		MemoryStream( TUINT32 index )
		{
			m_Index        = index;
			m_Buffer       = TNULL;
			m_BufferPos    = TNULL;
			m_BufferSize   = 0;
			m_ExpectedSize = 0;
			m_PtrList.reserve( 32 );
			GrowBuffer( 0 );
		}

		MemoryStream( const MemoryStream& other )
		{
			TASSERT( other.m_DependentStacks.size() == 0, "Cross pointers are not supported" );

			m_Index        = other.m_Index;
			m_Buffer       = TNULL;
			m_BufferSize   = 0;
			m_ExpectedSize = other.m_ExpectedSize;
			Resize( other.m_BufferSize );
			m_BufferPos = m_Buffer + other.GetUsedSize();
			Toshi::TUtil::MemCopy( m_Buffer, other.m_Buffer, other.GetUsedSize() );

			m_PtrList.reserve( other.m_PtrList.size() );
			for ( TUINT i = 0; i < other.m_PtrList.size(); i++ )
			{
				m_PtrList.emplace_back(
				    other.m_PtrList[ i ].Offset,
				    other.m_PtrList[ i ].DataPtr,
				    this // TODO: support cross pointers
				);
			}
		}

		~MemoryStream()
		{
			if ( m_Buffer != TNULL )
			{
				delete[] m_Buffer;
			}
		}

		TUINT Tell() { return GetUsedSize(); }

		void Seek( TUINT newPos )
		{
			TASSERT( newPos < m_BufferSize, "Trying to seek out of buffer" );
			m_BufferPos = m_Buffer + newPos;
		}

		TUINT32 SetIndex( TUINT32 index )
		{
			TUINT32 oldIndex = m_Index;
			m_Index          = index;
			return oldIndex;
		}

		void SetExpectedSize( TUINT32 expectedSize ) { m_ExpectedSize = expectedSize; }

		TUINT32 GetExpectedSize() const { return m_ExpectedSize; }

		TUINT32 GetIndex() const { return m_Index; }

		TUINT GetPointerCount() const { return m_PtrList.size(); }

		TCHAR* GetBuffer() const { return m_Buffer; }

		TCHAR* GetBufferPos() const { return m_BufferPos; }

		TUINT GetBufferSize() const { return m_BufferSize; }

		TUINT GetUsedSize() const { return (TUINT)m_BufferPos - (TUINT)m_Buffer; }

		TUINT GetOffset( const void* ptr ) const
		{
			TASSERT( (TUINT)ptr >= (TUINT)m_Buffer && (TUINT)ptr < (TUINT)m_Buffer + m_BufferSize, "Pointer is out of buffer" );
			return (TUINT)ptr - (TUINT)m_Buffer;
		}

		template <class T>
		void SetCrossPointer( T** outPtr, Ptr<T> ptr );

		template <class T>
		T* Write( TUINT offset, const T& value )
		{
			TASSERT( offset >= 0 && offset < m_BufferSize, "Offset is out of buffer" );
			*(T*)( &m_Buffer[ offset ] ) = value;
			return reinterpret_cast<T*>( &m_Buffer[ offset ] );
		}

		void Write( TUINT offset, TCHAR* value, TINT size )
		{
			TASSERT( offset >= 0 && offset < m_BufferSize, "Offset is out of buffer" );
			Toshi::TUtil::MemCopy( &m_Buffer[ offset ], value, size );
		}

		template <class T>
		Ptr<T> Alloc();

		template <class T>
		Ptr<T> Alloc( TUINT count );

		Ptr<TCHAR> AllocBytes( TUINT Size );

		template <class T>
		void WritePointer( T** outPtr, const T* ptr );

		template <class T>
		void WritePointer( T** outPtr, const Ptr<T>& ptr );

		template <class T>
		Ptr<T> Alloc( T** outPtr, TUINT count );

		template <class T, TUINT Count = 1>
		Ptr<T> Alloc( T** outPtr );

		void Link();
		void Unlink();

		std::vector<RelcPtr>::iterator begin() { return m_PtrList.begin(); }
		std::vector<RelcPtr>::iterator end() { return m_PtrList.end(); }

		void GrowBuffer( TUINT requiredSize );

	private:
		void Resize( TUINT size );

		void AddRelocationPtr( TUINT offset, TUINT dataPtr, MemoryStream* pDataStack = TNULL )
		{
			m_PtrList.emplace_back( offset, dataPtr, pDataStack == TNULL ? this : pDataStack );
		}

	private:
		TUINT32                    m_Index;
		TCHAR*                     m_Buffer;
		TCHAR*                     m_BufferPos;
		TUINT                      m_BufferSize;
		TUINT32                    m_ExpectedSize;
		std::vector<RelcPtr>       m_PtrList;
		std::vector<MemoryStream*> m_DependentStacks;
	};

public:
	PTRBSections() = default;

	~PTRBSections()
	{
		Reset();
	}

	void Reset()
	{
		for ( auto stack : m_Stacks )
			delete stack;

		m_Stacks.clear();
	}

	TUINT GetStackCount() const
	{
		return m_Stacks.size();
	}

	PTRBSections::MemoryStream* CreateStream();
	PTRBSections::MemoryStream* CreateStream( const PTRBSections::MemoryStream* pStream );
	TBOOL                       DeleteStack( PTRBSymbols* pSymb, PTRBSections::MemoryStream* pStream );

	PTRBSections::MemoryStream* GetStack( TUINT index );

	void Write( Toshi::TTSFO& ttsfo, TBOOL compress );
	void Read( Toshi::TTSFI& ttsfi, TBOOL compressed = false );

	std::vector<PTRBSections::MemoryStream*>::iterator begin() { return m_Stacks.begin(); }
	std::vector<PTRBSections::MemoryStream*>::iterator end() { return m_Stacks.end(); }

private:
	std::vector<PTRBSections::MemoryStream*> m_Stacks;
};

class PTRBSymbols
{
public:
	PTRBSymbols()
	{
		Reset();
	}

	void Reset()
	{
		m_Symbols.clear();
		m_SymbolNames.clear();
		m_Symbols.reserve( 5 );
		m_SymbolNames.reserve( 5 );
	}

	TBOOL Is( TUINT index, const TCHAR* name )
	{
		TASSERT( index >= 0 && index < m_Symbols.size() );
		auto hash = Toshi::TTRB::HashString( name );

		if ( m_Symbols[ index ].NameHash == hash )
		{
			if ( m_SymbolNames[ index ] == name )
			{
				return true;
			}
		}

		return false;
	}

	TINT FindIndex( PTRBSections& sect, const TCHAR* name )
	{
		auto hash = Toshi::TTRB::HashString( name );

		for ( TUINT i = 0; i < m_Symbols.size(); i++ )
		{
			if ( m_Symbols[ i ].NameHash == hash )
			{
				if ( m_SymbolNames[ i ] == name )
				{
					return i;
				}
			}
		}

		return -1;
	}

	TINT FindIndex( PTRBSections* sect, const TCHAR* name )
	{
		return FindIndex( *sect, name );
	}

	template <class T>
	PTRBSections::MemoryStream::Ptr<T> Find( PTRBSections& sect, const TCHAR* name )
	{
		TINT index = FindIndex( sect, name );

		if ( index != -1 )
		{
			auto stack = sect.GetStack( m_Symbols[ index ].HDRX );
			return { stack, m_Symbols[ index ].DataOffset };
		}

		return { TNULL, (TUINT)0 };
	}

	template <class T>
	PTRBSections::MemoryStream::Ptr<T> Find( PTRBSections* sect, const TCHAR* name )
	{
		return Find<T>( *sect, name );
	}

	PTRBSections::MemoryStream* FindStack( PTRBSections& sect, const TCHAR* name )
	{
		TINT index = FindIndex( sect, name );

		if ( index != -1 )
		{
			auto stack = sect.GetStack( m_Symbols[ index ].HDRX );
			return stack;
		}

		return TNULL;
	}

	PTRBSections::MemoryStream* FindStack( PTRBSections* sect, const TCHAR* name )
	{
		return FindStack( *sect, name );
	}

	template <class T>
	PTRBSections::MemoryStream::Ptr<T> GetByIndex( PTRBSections& sect, TUINT index )
	{
		TASSERT( index < m_Symbols.size() );

		return {
			sect.GetStack( m_Symbols[ index ].HDRX ),
			m_Symbols[ index ].DataOffset
		};
	}

	template <class T>
	PTRBSections::MemoryStream::Ptr<T> GetByIndex( PTRBSections* sect, TUINT index )
	{
		return GetByIndex<T>( *sect, index );
	}

	PTRBSections::MemoryStream* GetStack( PTRBSections& sect, TUINT index )
	{
		TASSERT( index < m_Symbols.size() );
		return sect.GetStack( m_Symbols[ index ].HDRX );
	}

	PTRBSections::MemoryStream* GetStack( PTRBSections* sect, TUINT index )
	{
		return GetStack( *sect, index );
	}

	Toshi::TString8 GetName( TUINT index )
	{
		return m_SymbolNames[ index ].c_str();
	}

	TUINT GetCount()
	{
		return m_Symbols.size();
	}

	void Add( PTRBSections::MemoryStream* pStream, const TCHAR* name, void* ptr )
	{
		m_Symbols.emplace_back( pStream->GetIndex(), 0, 0, 0, pStream->GetOffset( ptr ) );
		m_SymbolNames.push_back( name );
	}

	void UpdateSymbolsIndexes( PTRBSections::MemoryStream* pStream, TUINT32 newIndex )
	{
		auto stackIndex = pStream->GetIndex();

		for ( TUINT i = 0; i < m_Symbols.size(); i++ )
		{
			if ( m_Symbols[ i ].HDRX == stackIndex )
			{
				m_Symbols[ i ].HDRX = newIndex;
			}
		}

		pStream->SetIndex( newIndex );
	}

	void Remove( TUINT index )
	{
		TASSERT( index >= 0 && index < m_Symbols.size() );

		m_Symbols.erase( m_Symbols.begin() + index );
		m_SymbolNames.erase( m_SymbolNames.begin() + index );
	}

	void RemoveAllWithStackIndex( TINT stackIndex )
	{
		for ( TUINT i = 0; i < m_Symbols.size(); )
		{
			if ( m_Symbols[ i ].HDRX == stackIndex )
			{
				Remove( i );
			}
			else
			{
				i++;
			}
		}
	}

	void Write( Toshi::TTSFO& ttsfo )
	{
		TASSERT( m_Symbols.size() == m_SymbolNames.size(), "" );

		TUINT32 nameOffset  = 0;
		TUINT32 symbolCount = m_Symbols.size();
		ttsfo.Write( symbolCount );

		for ( TUINT i = 0; i < m_Symbols.size(); i++ )
		{
			m_Symbols[ i ].NameHash   = Toshi::TTRB::HashString( m_SymbolNames[ i ].c_str() );
			m_Symbols[ i ].NameOffset = nameOffset;
			nameOffset += m_SymbolNames[ i ].length() + 1;
			ttsfo.Write( m_Symbols[ i ] );
		}

		for ( auto& name : m_SymbolNames )
		{
			ttsfo.WriteRaw( name.c_str(), name.length() );
			ttsfo.Write( (uint8_t)0 );
		}
	}

	void Read( Toshi::TTSFI& ttsfi, PTRBSections& sect )
	{
		TUINT32 symbolCount = 0;
		ttsfi.Read( &symbolCount );

		// Read symbols
		TUINT symbolsSize = sizeof( Toshi::TTRB::TTRBSymbol ) * symbolCount;
		m_Symbols.resize( symbolCount );
		ttsfi.ReadRaw( m_Symbols.data(), symbolsSize );

		// Read symbol names
		TUINT namesSize = ttsfi.GetCurrentHunk().Size - symbolsSize;
		m_SymbolNames.reserve( symbolCount );
		TCHAR* namesBuffer = new TCHAR[ namesSize ];
		ttsfi.ReadRaw( namesBuffer, namesSize );

		for ( auto& symbol : m_Symbols )
		{
			const TCHAR* symbolName = &namesBuffer[ symbol.NameOffset ];
			m_SymbolNames.push_back( symbolName );
		}

		delete[] namesBuffer;
	}

	std::vector<std::string>::iterator begin() { return m_SymbolNames.begin(); }
	std::vector<std::string>::iterator end() { return m_SymbolNames.end(); }

private:
	std::vector<Toshi::TTRB::TTRBSymbol> m_Symbols;
	std::vector<std::string>             m_SymbolNames;
};

class PTRBWriter
{
public:
	static constexpr Toshi::TVersion VERSION = { TVERSION( 1, 1 ) };

public:
	PTRBWriter() :
	    m_HDRX( VERSION ) {}
	PTRBWriter( const std::string& filepath ) :
	    m_HDRX( VERSION ) { ReadFromFile( filepath ); }

	void Reset()
	{
		m_SECT.Reset();
		m_SYMB.Reset();
	}

	TBOOL ReadFromFile( const std::string& filepath )
	{
		Reset();

		Toshi::TTSFI ttsfi;
		auto         pFile = Toshi::TFile::Create( filepath.c_str() );

		if ( pFile && ttsfi.Open( pFile ) == Toshi::TTRB::ERROR_OK )
		{
			int32_t leftSize = ttsfi.GetCurrentHunk().Size - 4;

			while ( leftSize > sizeof( Toshi::TTSF::Hunk ) )
			{
				if ( ttsfi.ReadHunk() != Toshi::TTRB::ERROR_OK ) break;
				leftSize -= ttsfi.GetCurrentHunk().Size + sizeof( Toshi::TTSF::Hunk );

				switch ( ttsfi.GetCurrentHunk().Name )
				{
					case TFourCC( "HDRX" ):
						m_HDRX.Read( ttsfi, m_SECT );
						break;
					case TFourCC( "SECT" ):
						m_SECT.Read( ttsfi );
						break;
					case TFourCC( "SECC" ):
						m_SECT.Read( ttsfi, true );
						break;
					case TFourCC( "RELC" ):
						m_RELC.Read( ttsfi, m_SECT );
						break;
					case TFourCC( "SYMB" ):
						m_SYMB.Read( ttsfi, m_SECT );
						break;
				}

				ttsfi.SkipHunk();
			}

			ttsfi.Close( TFALSE );
			pFile->Destroy();
			return true;
		}

		if ( pFile != TNULL )
		{
			ttsfi.Close( TFALSE );
			pFile->Destroy();
		}

		return false;
	}

	void WriteToFile( const std::string& filepath, TBOOL compress = false, Toshi::TTSF::Endianess endianess = Toshi::TTSF::Endianess_Little )
	{
		Toshi::TTSFO           ttsfo;
		Toshi::TTSFO::HunkMark mark;
		ttsfo.Create( filepath.c_str(), "TRBF", endianess );

		// HDRX
		ttsfo.OpenHunk( &mark, "HDRX" );
		m_HDRX.SetSectionCount( m_SECT.GetStackCount() );
		m_HDRX.Write( ttsfo, m_SECT );
		ttsfo.CloseHunk( &mark );

		// SECT
		ttsfo.OpenHunk( &mark, compress ? "SECC" : "SECT" );
		m_SECT.Write( ttsfo, compress );
		ttsfo.CloseHunk( &mark );

		// RELC
		ttsfo.OpenHunk( &mark, "RELC" );
		m_RELC.Write( ttsfo, m_SECT );
		ttsfo.CloseHunk( &mark );

		// SYMB
		ttsfo.OpenHunk( &mark, "SYMB" );
		m_SYMB.Write( ttsfo );
		ttsfo.CloseHunk( &mark );

		ttsfo.Close();
	}

	PTRBSymbols* GetSymbols()
	{
		return &m_SYMB;
	}

	PTRBSections* GetSections()
	{
		return &m_SECT;
	}

private:
	PTRBHeader      m_HDRX;
	PTRBSections    m_SECT;
	PTRBRelocations m_RELC;
	PTRBSymbols     m_SYMB;
};

inline PTRBHeader::PTRBHeader( Toshi::TVersion version )
{
	m_Header.m_ui32Version     = version;
	m_Header.m_i32SectionCount = 0;
}

inline void PTRBHeader::SetVersion( Toshi::TVersion version )
{
	m_Header.m_ui32Version = version;
}

inline void PTRBHeader::SetSectionCount( int32_t count )
{
	m_Header.m_i32SectionCount = count;
}

inline void PTRBHeader::Write( Toshi::TTSFO& ttsfo, PTRBSections& sect )
{
	ttsfo.Write( m_Header );

	for ( PTRBSections::MemoryStream* stack : sect )
	{
		Toshi::TTRB::SecInfo sectionInfo = {};
		sectionInfo.m_Size               = TAlignNumUp( stack->GetUsedSize() );
		ttsfo.Write( sectionInfo );
	}
}

inline void PTRBHeader::Read( Toshi::TTSFI& ttsfi, PTRBSections& sect )
{
	ttsfi.Read( &m_Header );

	for ( TINT i = 0; i < m_Header.m_i32SectionCount; i++ )
	{
		Toshi::TTRB::SecInfo sectionInfo;
		ttsfi.Read( &sectionInfo );

		auto stack = sect.CreateStream();
		stack->SetExpectedSize( sectionInfo.m_Size );
	}
}

inline void PTRBRelocations::Write( Toshi::TTSFO& ttsfo, PTRBSections& sect )
{
	TUINT32 ptrCount = 0;
	for ( auto section : sect )
	{
		ptrCount += section->GetPointerCount();
	}

	ttsfo.Write( ptrCount );

	for ( TUINT i = 0; i < sect.GetStackCount(); i++ )
	{
		auto section = sect.GetStack( i );

		for ( auto& ptr : *section )
		{
			Toshi::TTRB::RELCEntry entry = {};
			entry.HDRX1                  = (short)i;
			entry.HDRX2                  = ptr.DataStack->GetIndex();
			entry.Offset                 = ptr.Offset;
			ttsfo.Write( entry );
		}
	}
}

inline void PTRBRelocations::Read( Toshi::TTSFI& ttsfi, PTRBSections& sect )
{
	TUINT32 ptrCount = 0;
	ttsfi.Read( &ptrCount );

	Toshi::TTRB::RELCEntry entry;
	for ( TUINT32 i = 0; i < ptrCount; i++ )
	{
		ttsfi.Read( &entry );
		auto    stack     = sect.GetStack( entry.HDRX1 );
		auto    dataStack = sect.GetStack( entry.HDRX2 );
		TUINT32 dataPtr   = *(TUINT32*)( &stack->GetBuffer()[ entry.Offset ] );
		stack->AddRelocationPtr( entry.Offset, dataPtr, dataStack );
	}

	for ( auto stack : sect )
	{
		stack->Link();
	}
}

template <class T>
inline void PTRBSections::MemoryStream::SetCrossPointer( T** outPtr, Ptr<T> ptr )
{
	TASSERT( (TUINT)outPtr >= (TUINT)m_Buffer && (TUINT)outPtr < (TUINT)m_Buffer + (TUINT)m_BufferSize, "Out pointer is out of buffer" );
	TASSERT( TNULL != ptr.stack() );
	TASSERT( this != ptr.stack() );

	TUINT outPtrOffset = GetOffset( outPtr );
	Write<T*>( outPtrOffset, ptr.get() );

	AddRelocationPtr( outPtrOffset, ptr.offset(), ptr.stack() );

	// Set current stack as dependent from ptr.stack()
	auto crossStack = ptr.stack();
	auto it         = std::find( crossStack->m_DependentStacks.begin(), crossStack->m_DependentStacks.end(), this );

	if ( it == crossStack->m_DependentStacks.end() )
	{
		// Add this stack
		crossStack->m_DependentStacks.push_back( this );
	}
}

template <class T>
inline PTRBSections::MemoryStream::Ptr<T> PTRBSections::MemoryStream::Alloc()
{
	m_BufferPos = TREINTERPRETCAST( TCHAR*, TAlignPointerUp( m_BufferPos ) );

	constexpr TUINT TSize = sizeof( T );
	GrowBuffer( GetUsedSize() + TSize );

	T* allocated = reinterpret_cast<T*>( m_BufferPos );
	m_BufferPos += TSize;

	return { this, allocated };
}

template <class T>
inline PTRBSections::MemoryStream::Ptr<T> PTRBSections::MemoryStream::Alloc( TUINT count )
{
	m_BufferPos = TREINTERPRETCAST( TCHAR*, TAlignPointerUp( m_BufferPos ) );

	const TUINT TSize = sizeof( T ) * count;
	GrowBuffer( GetUsedSize() + TSize );

	T* allocated = reinterpret_cast<T*>( m_BufferPos );
	m_BufferPos += TSize;

	return { this, allocated };
}

template <class T>
inline PTRBSections::MemoryStream::Ptr<T> PTRBSections::MemoryStream::Alloc( T** outPtr, TUINT count )
{
	TASSERT( (TUINT)outPtr >= (TUINT)m_Buffer && (TUINT)outPtr < (TUINT)m_Buffer + (TUINT)m_BufferSize, "Out pointer is out of buffer" );
	m_BufferPos = TREINTERPRETCAST( TCHAR*, TAlignPointerUp( m_BufferPos ) );

	const TUINT TSize        = sizeof( T ) * count;
	TUINT       outPtrOffset = GetOffset( outPtr );
	GrowBuffer( GetUsedSize() + TSize );

	T* allocated = reinterpret_cast<T*>( m_BufferPos );
	m_BufferPos += TSize;

	Write<T*>( outPtrOffset, allocated );
	AddRelocationPtr( outPtrOffset, GetOffset( allocated ) );

	return { this, allocated };
}

template <class T, TUINT Count>
inline PTRBSections::MemoryStream::Ptr<T> PTRBSections::MemoryStream::Alloc( T** outPtr )
{
	TASSERT( (TUINT)outPtr >= (TUINT)m_Buffer && (TUINT)outPtr < (TUINT)m_Buffer + (TUINT)m_BufferSize, "Out pointer is out of buffer" );
	m_BufferPos = TREINTERPRETCAST( TCHAR*, TAlignPointerUp( m_BufferPos ) );

	constexpr TUINT TSize        = sizeof( T ) * Count;
	TUINT           outPtrOffset = GetOffset( outPtr );
	GrowBuffer( GetUsedSize() + TSize );

	T* allocated = reinterpret_cast<T*>( m_BufferPos );
	m_BufferPos += TSize;

	Write<T*>( outPtrOffset, allocated );
	AddRelocationPtr( outPtrOffset, GetOffset( allocated ) );

	return { this, allocated };
}

template <class T>
inline void PTRBSections::MemoryStream::WritePointer( T** outPtr, const T* ptr )
{
	TASSERT( (TUINT)outPtr >= (TUINT)m_Buffer && (TUINT)outPtr < (TUINT)m_Buffer + (TUINT)m_BufferSize, "Out pointer is out of buffer" );
	TASSERT( (TUINT)ptr >= (TUINT)m_Buffer && (TUINT)ptr < (TUINT)m_Buffer + (TUINT)m_BufferSize, "Pointer is out of buffer" );

	TUINT outPtrOffset = GetOffset( outPtr );
	Write<TUINT32>( outPtrOffset, TREINTERPRETCAST( TUINT32, ptr ) );
	AddRelocationPtr( outPtrOffset, GetOffset( ptr ) );
}

template <class T>
inline void PTRBSections::MemoryStream::WritePointer( T** outPtr, const PTRBSections::MemoryStream::Ptr<T>& ptr )
{
	TASSERT( (TUINT)outPtr >= (TUINT)m_Buffer && (TUINT)outPtr < (TUINT)m_Buffer + (TUINT)m_BufferSize, "Out pointer is out of buffer" );

	TUINT outPtrOffset = GetOffset( outPtr );
	Write<TUINT32>( outPtrOffset, TREINTERPRETCAST( TUINT32, ptr.get() ) );
	AddRelocationPtr( outPtrOffset, ptr.offset() );
}

inline PTRBSections::MemoryStream::Ptr<TCHAR> PTRBSections::MemoryStream::AllocBytes( TUINT Size )
{
	GrowBuffer( GetUsedSize() + Size );

	TCHAR* allocated = reinterpret_cast<TCHAR*>( m_BufferPos );
	m_BufferPos += Size;

	return { this, allocated };
}

inline void PTRBSections::MemoryStream::Link()
{
	for ( auto& ptr : m_PtrList )
	{
		Write<void*>( ptr.Offset, ptr.DataStack->GetBuffer() + ptr.DataPtr );
	}
}

inline void PTRBSections::MemoryStream::Unlink()
{
	for ( auto& ptr : m_PtrList )
	{
		Write<void*>( ptr.Offset, (void*)ptr.DataPtr );
	}
}

inline void PTRBSections::MemoryStream::GrowBuffer( TUINT requiredSize )
{
	TUINT newSize = ( ( requiredSize / BUFFER_GROW_SIZE ) + 1 ) * BUFFER_GROW_SIZE;

	if ( newSize != m_BufferSize )
	{
		Resize( newSize );
	}
}

inline void PTRBSections::MemoryStream::Resize( TUINT size )
{
	TASSERT( size > 0, "Size should be positive" );
	//TASSERT(size != m_BufferSize, "Size is the same");
	//TASSERT(size > m_BufferSize, "Buffer can't shrink");

	TCHAR* oldBuffer = m_Buffer;
	TUINT  usedSize  = GetUsedSize();
	m_Buffer         = new TCHAR[ size ];
	m_BufferSize     = size;

	Toshi::TUtil::MemClear( m_Buffer, size );

	if ( oldBuffer != TNULL )
	{
		Toshi::TUtil::MemCopy( m_Buffer, oldBuffer, usedSize );
		m_BufferPos = m_Buffer + usedSize;
		delete[] oldBuffer;
	}
	else
	{
		m_BufferPos = m_Buffer;
	}

	Link();
}

inline PTRBSections::MemoryStream* PTRBSections::CreateStream()
{
	PTRBSections::MemoryStream* stack = new PTRBSections::MemoryStream( m_Stacks.size() );
	m_Stacks.push_back( stack );
	return stack;
}

inline PTRBSections::MemoryStream* PTRBSections::CreateStream( const PTRBSections::MemoryStream* pStream )
{
	PTRBSections::MemoryStream* stack = new PTRBSections::MemoryStream( *pStream );
	stack->SetIndex( m_Stacks.size() );
	m_Stacks.push_back( stack );
	return stack;
}

inline TBOOL PTRBSections::DeleteStack( PTRBSymbols* pSymb, PTRBSections::MemoryStream* pStream )
{
	auto result = std::find( m_Stacks.begin(), m_Stacks.end(), pStream );

	if ( result != m_Stacks.end() )
	{
		pSymb->RemoveAllWithStackIndex( pStream->GetIndex() );

		m_Stacks.erase( result );
		delete pStream;

		// Update indexes
		TUINT32 index = 0;
		for ( auto it = m_Stacks.begin(); it != m_Stacks.end(); it++ )
		{
			auto stack = *it;
			pSymb->UpdateSymbolsIndexes( stack, index++ );
		}

		return true;
	}
	else
	{
		return false;
	}
}

inline PTRBSections::MemoryStream* PTRBSections::GetStack( TUINT index )
{
	TASSERT( index >= 0 && index < GetStackCount(), "Index is out of bounds" );
	return m_Stacks[ index ];
}

inline void PTRBSections::Write( Toshi::TTSFO& ttsfo, TBOOL compress )
{
	TUINT ready = 0;
	TUINT count = m_Stacks.size();

	if ( compress )
	{
		if ( count > 1 )
		{
			TTRACE( "Compressing progress: 0%\n" );
		}
		else
		{
			TTRACE( "Started BTEC compression...\n" );
		}
	}

	for ( auto stack : m_Stacks )
	{
		stack->Unlink();

		if ( compress )
		{
			ttsfo.WriteCompressed( stack->GetBuffer(), stack->GetUsedSize() );
			ready += 1;

			if ( count > 1 )
			{
				TTRACE( "Compressing progress: %.1f\n", (double)ready / count * 100 );
			}
			else
			{
				TTRACE( "BTEC compression completed...\n" );
			}
		}
		else
		{
			ttsfo.WriteRaw( stack->GetBuffer(), stack->GetUsedSize() );
		}

		ttsfo.WriteAlignmentPad();
		stack->Link();
	}
}

inline void PTRBSections::Read( Toshi::TTSFI& ttsfi, TBOOL compressed )
{
	for ( auto stack : m_Stacks )
	{
		TUINT expectedSize = stack->GetExpectedSize();

		if ( expectedSize > 0 )
		{
			stack->GrowBuffer( expectedSize );

			if ( compressed )
			{
				ttsfi.ReadCompressed( stack->GetBuffer(), expectedSize );
			}
			else
			{
				ttsfi.ReadRaw( stack->GetBuffer(), expectedSize );
			}

			stack->Seek( expectedSize );
			stack->SetExpectedSize( 0 );
		}
	}
}
