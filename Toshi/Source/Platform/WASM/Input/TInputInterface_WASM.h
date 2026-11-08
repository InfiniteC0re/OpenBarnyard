#pragma once
#include "Input/TInputInterface.h"

TOSHI_NAMESPACE_START

class TInputWASMInterface : public TInputInterface
{
public:
	TDECLARE_CLASS( TInputWASMInterface, TInputInterface );

public:
	TInputWASMInterface();
	virtual ~TInputWASMInterface() override;

	virtual TBOOL Initialise() override;
	virtual TBOOL Deinitialise() override;
};

TOSHI_NAMESPACE_END
