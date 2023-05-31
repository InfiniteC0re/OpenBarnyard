#include "ToshiPCH.h"
#include "TShader.h"

namespace Toshi
{
	TShader::TShader()
	{
		TIMPLEMENT();
		m_State = State::None;
	}

	TShader::~TShader()
	{

	}

	void TShader::OnDestroy()
	{
		m_State.Unset(State::Created);
	}

	void TShader::Flush()
	{

	}

	TBOOL TShader::Create()
	{
		m_State.Set(State::Created);
		return TTRUE;
	}

	TBOOL TShader::Validate()
	{
		m_State.Set(State::Validated);
		return TTRUE;
	}

	void TShader::Invalidate()
	{
		m_State.Unset(State::Validated);
	}
}