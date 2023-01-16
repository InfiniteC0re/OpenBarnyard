#include "ToshiPCH.h"
#include "TShader.h"

namespace Toshi
{
	TShader::~TShader()
	{

	}

	void TShader::OnDestroy()
	{
		m_State &= ~State_Created;
	}

	void TShader::Flush()
	{

	}

	void TShader::Create()
	{
		m_State |= State_Created;
	}

	void TShader::Validate()
	{
		m_State |= State_Validated;
	}

	void TShader::Invalidate()
	{
		m_State &= ~State_Validated;
	}
}