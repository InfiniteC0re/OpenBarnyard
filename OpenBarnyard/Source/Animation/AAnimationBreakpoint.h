#pragma once

class AAnimationBreakpoint
    : public Toshi::TObject
{
public:
	TDECLARE_CLASS( AAnimationBreakpoint, Toshi::TObject );

public:
	AAnimationBreakpoint();
	~AAnimationBreakpoint();

	// TODO: virtual methods

	TFLOAT GetTime() const { return m_fTime; }

private:
	TFLOAT m_fTime; // ?
};
