#pragma once

TOSHI_NAMESPACE_START

//-----------------------------------------------------------------------------
// Purpose: This class is a helper that allows to create an unique variable
//          that can be accessed at compile time by type T and numerical ID.
//-----------------------------------------------------------------------------
template <class T, size_t ID>
struct T2UniqueValue
{
	inline static T value = T();
};

TOSHI_NAMESPACE_END
