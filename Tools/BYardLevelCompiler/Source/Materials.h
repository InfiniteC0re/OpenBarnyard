#pragma once

class Materials
{
public:
	struct Material_t
	{
		char MaterialName[0x68];
		char MaterialFile[0xC0];
	};

public:
	Material_t* GetMaterial(size_t index)
	{
		return reinterpret_cast<Material_t*>(this + 1) + index;
	}

public:
	int m_Zero1;
	int m_Zero2;
	size_t m_Count;
	size_t m_Size;
};