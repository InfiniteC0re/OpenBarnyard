#pragma once
#include <Toshi/TObject.h>
#include <Toshi/TRefCounted.h>
#include <Toshi/TPString8.h>

class PPropertyReader;
class PPropertyWriter;

//-----------------------------------------------------------------------------
// Purpose: handler of a "[name]" block of a text properties file (.gui files).
// Registered with PPropertyReader::AddDefinitionBlock / PPropertyWriter::AddDefinitionBlock,
// PPropertyReader::Load() calls Load for each block with its name and
// PPropertyWriter::Save() calls Save of every registered block.
//-----------------------------------------------------------------------------
class PPropertyBlock
    : public Toshi::TObject
    , public Toshi::TRefCounted
{
public:
	TDECLARE_CLASS( PPropertyBlock, Toshi::TObject );

	// NOTE: the values are unknown, the blocks of the GUI interface pass them to the constructor
	enum Priority : TINT
	{
	};

public:
	PPropertyBlock( const Toshi::TPString8& a_rcName, Priority a_ePriority );
	~PPropertyBlock();

	// Reads the content of the block (the "[name]" header has been read already)
	virtual TBOOL Load( PPropertyReader* a_pReader ) = 0;

	// Writes the block, header included
	virtual TBOOL Save( PPropertyWriter* a_pWriter ) = 0;

	// $JPOG: FUNCTION PPropertyParser.dll+00002c10
	void SetName( const Toshi::TPString8& a_rcName ) { m_strName = a_rcName; }

	// $JPOG: FUNCTION PPropertyParser.dll+00002c60
	const Toshi::TPString8& GetName() const { return m_strName; }

	// $JPOG: FUNCTION PPropertyParser.dll+00002c50
	Priority GetPriority() const { return m_ePriority; }

private:
	Toshi::TPString8 m_strName;   // 0x08
	Priority         m_ePriority; // 0x0C
};
