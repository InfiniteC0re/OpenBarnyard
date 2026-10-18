#pragma once
#include <Toshi/TPString8.h>

class PProperties;

TOSHI_NAMESPACE_START

//-----------------------------------------------------------------------------
// Purpose: periodic function used to animate the parameters of the GUI
// components: Compute( x ) = ComputeNormalised( x * frequency + phase ) * scale + offset
// (the "function" blocks of the component properties, see TGUIComponent).
//-----------------------------------------------------------------------------
class TGUIFunction : public TObject
{
public:
	TDECLARE_CLASS( TGUIFunction, TObject );

public:
	TGUIFunction();
	TGUIFunction( TFLOAT a_fFrequency, TFLOAT a_fPhase, TFLOAT a_fScale, TFLOAT a_fOffset );
	TGUIFunction( const TGUIFunction& a_rcOther );
	~TGUIFunction();

	virtual TGUIFunction* Clone() const = 0;
	virtual TBOOL         Deserialise( const PProperties* a_pProperties );
	virtual TBOOL         Serialise( PProperties* a_pProperties );

	// Value of the function at a_fX with a frequency of 1, no phase, no scale and no offset
	virtual TFLOAT ComputeNormalised( TFLOAT a_fX ) const = 0;

	TFLOAT Compute( TFLOAT a_fX ) const;

	TFLOAT GetFrequency() const { return m_fFrequency; }
	TFLOAT GetPhase() const { return m_fPhase; }
	TFLOAT GetScale() const { return m_fScale; }
	TFLOAT GetOffset() const { return m_fOffset; }

public:
	// NOTE: Nicktoons constructs these with the TClass at static init time, before
	// the TOSHI 2.0 string pool exists, so they're created by InitialiseClass
	static TPString8 ms_strFrequency;
	static TPString8 ms_strPhase;
	static TPString8 ms_strScale;
	static TPString8 ms_strOffset;

protected:
	TFLOAT m_fFrequency; // 0x04
	TFLOAT m_fPhase;     // 0x08
	TFLOAT m_fScale;     // 0x0C
	TFLOAT m_fOffset;    // 0x10
};

TOSHI_NAMESPACE_END
