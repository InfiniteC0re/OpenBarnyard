#pragma once

namespace Toshi
{
	class TWString
	{
		unsigned short** m_pBuffer;
		unsigned int m_uilength; //guess

	public:
		int __thiscall Length();
		__thiscall operator unsigned short*() const;
	};
}


