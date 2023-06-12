#include "ToshiPCH.h"
#include "TShader.h"

namespace Toshi
{
	TShader::TShader()
	{
		m_State = State::None;
		sm_oShaderList.AddShader(this);
	}

	TShader::~TShader()
	{
		sm_oShaderList.RemoveShader(this);
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