#pragma once

class Materials
{
public:
	struct Material_t
	{
		char MaterialName[104];
		char MaterialFile[192];
	};

	friend class MaterialList;

public:
	Materials()
	{
		m_Zero1 = 0;
		m_Zero2 = 0;
		m_Count = 0;
		m_Size = 0;
	}

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

class MaterialList
{
public:
	void AddMaterial(const char name[104], const char file[192])
	{
		for (auto& material : m_List)
		{
			if (strcmp(name, material.MaterialName) == 0)
				return;
		}

		auto& material = m_List.emplace_back();
		strcpy(material.MaterialName, name);
		strcpy(material.MaterialFile, file);
		m_Data.m_Count += 1;
		m_Data.m_Size  += sizeof(Materials::Material_t);
	}

	void SetData(const Materials& materialsData, Materials::Material_t* list)
	{
		m_Data = materialsData;
		m_List.clear();
		m_List.reserve(materialsData.m_Count);

		for (size_t i = 0; i < materialsData.m_Count; i++)
			m_List[i] = list[i];
	}

	const Materials& GetData() const
	{
		return m_Data;
	}

	size_t GetMaterialCount()
	{
		return m_Data.m_Count;
	}

	Materials::Material_t* operator[](size_t index)
	{
		return &m_List[index];
	}

	std::vector<Materials::Material_t>::iterator begin()
	{
		return m_List.begin();
	}

	std::vector<Materials::Material_t>::iterator end()
	{
		return m_List.end();
	}

private:
	Materials m_Data;
	std::vector<Materials::Material_t> m_List;
};