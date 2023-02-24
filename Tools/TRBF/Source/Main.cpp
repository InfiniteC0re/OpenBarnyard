#include "pch.h"
#define _CRT_SECURE_NO_WARNINGS

// TOSHI_TMEMORY_FLAGS allows you to change the mode TMemory initializes with
// Flags_NativeMethods stands for native memory management methods instead of using dlmalloc
#define TOSHI_TMEMORY_FLAGS Toshi::TMemory::Flags_NativeMethods

// TOSHI_TMEMORY_SIZE allows you to change size of the global memory block
// The value is not used when TMemory initialized with native methods
#define TOSHI_TMEMORY_SIZE 0

// This file includes the entrypoint so set all the settings before including it
#include "Toshi.h"
#include "TRBF/TRBF.h"
#include <filesystem>

class PProperties
{
public:
	class PProperty
	{
	public:
		typedef uint32_t Type;
		enum Type_ : Type
		{
			Type_UInt,
			Type_Unknown,
			Type_Float,
			Type_Bool,
			Type_PProperty,
			Type_Unknown2,
			Type_Array,
			Type_String,
			Type_Int,
		};

		class Array
		{
		public:
			class Item
			{
			public:
				template<class T>
				T* GetData()
				{
					return reinterpret_cast<T*>(Data);
				}

			private:
				Type Type;
				void* Data;
			};

		public:
			uint32_t GetSize()
			{
				return m_Size;
			}

			Item* GetItem(size_t index)
			{
				TASSERT(index >= 0 && index <= m_Size);
				return &m_Items[index];
			}

		private:
			Item* m_Items;
			uint32_t m_Size;
		};

	public:
		const char* GetName() const
		{
			return *m_Name;
		}

		Type GetType() const
		{
			return *m_Type;
		}

		template<class T>
		T* GetData()
		{
			return reinterpret_cast<T*>(m_Type + 1);
		}

	private:
		const char** m_Name;
		Type* m_Type;
	};

	struct Main
	{
		uint32_t m_Zero;
		PProperty* m_Properties;
		uint32_t m_PropertyCount;
	};

public:
	PProperties(Main* header)
	{
		m_Header = header;
	}

	uint32_t GetPropertyCount()
	{
		return m_Header->m_PropertyCount;
	}

	PProperty* GetProperty(size_t index)
	{
		TASSERT(index >= 0 && index <= m_Header->m_PropertyCount);
		return &m_Header->m_Properties[index];
	}

private:
	Main* m_Header;
};

int TMain(int argc, char** argv)
{
	TLib::TRBF::TRBF trbf;
	trbf.ReadFromFile(argv[1]);

	Toshi::TFile* file = Toshi::TFile::Create(argv[2], Toshi::TFile::FileMode_Read);
	int size = file->GetSize();
	char* buffer = (char*)TMalloc(size);
	file->Read(buffer, size);
	file->Destroy();

	auto pSect = trbf.GetSECT();
	auto pSymb = trbf.GetSYMB();

	for (size_t i = 0; i < pSect->GetSectionCount(); i++)
	{
		auto curSect = pSect->GetSection(i);
		auto buff = curSect->GetBuffer();
		buff += 4;
		char* fileName = *(char**)buff;
		if (fileName != std::filesystem::path(argv[2]).filename())
		{
			continue;
		}
		buff += 4;

		auto xur = pSect->GetSection(i+1);
		auto xurBuffer = xur->GetBuffer();

		if (size > xur->GetBufferSize())
		{
			xur->GrowBuffer(size);
			xur->Write(0, buffer, size);
		}
		else
		{
			xur->SetExpectedSize(size);
			xur->Write(0, buffer, size);
		}
		
		break;
	}

	trbf.WriteToFile("C:\\Users\\Leon\\Desktop\\test.trb");

	/*PProperties properties(pSymb->Find<PProperties::Main>(pSect, "Main").get());
	TOSHI_INFO("PProperty count: {0}", properties.GetPropertyCount());

	for (size_t i = 0; i < properties.GetPropertyCount(); i++)
	{
		auto property = properties.GetProperty(i);
		
		TOSHI_INFO("PProperty name: {0}", property->GetName());

		switch (property->GetType())
		{
			case PProperties::PProperty::Type_String:
				TOSHI_INFO("PProperty value: {0}", *property->GetData<char*>());
				break;
			case PProperties::PProperty::Type_Array:
			{
				auto array = *property->GetData<PProperties::PProperty::Array*>();
				TOSHI_INFO("PProperty array size: {0}", array->GetSize());

				for (size_t k = 0; k < array->GetSize(); k++)
				{
					TOSHI_INFO("{0}) {1}", k + 1, array->GetItem(k)->GetData<char>());
				}
				break;
			}
		}
	}*/

	return 0;
}