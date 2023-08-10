#pragma once
#include <Toshi/File/TTSF.h>

#ifndef __TOSHI_TTRB_H__
#error Include TTRB.h to use this extension
#endif

#ifndef __TOSHI_PLUGIN_PTRB_H__
#define __TOSHI_PLUGIN_PTRB_H__
#endif

namespace PTRB
{
	class HDRX;
	class SECT;
	class RELC;
	class SYMB;

	class HDRX
	{
	public:
		HDRX(TVersion version);

		void SetVersion(TVersion version);
		void SetSectionCount(int32_t count);
		void Write(Toshi::TTSFO& ttsfo, SECT& sect);
		void Read(Toshi::TTSFI& ttsfi, SECT& sect);

	private:
		Toshi::TTRB::Header m_Header;
	};

	class RELC
	{
	public:
		RELC() = default;

		void Write(Toshi::TTSFO& ttsfo, SECT& sect);
		void Read(Toshi::TTSFI& ttsfi, SECT& sect);
	};

	class SECT
	{
	public:
		class Stack
		{
		public:
			friend class SECT;
			friend class SYMB;
			friend class RELC;

			static constexpr size_t BUFFER_GROW_SIZE = 4096;

			template <class T>
			class Ptr
			{
			public:
				Ptr() : m_Stack(TNULL), m_Offset(0) { }
				Ptr(SECT::Stack* stack, size_t offset) : m_Stack(stack), m_Offset(offset) { }
				Ptr(SECT::Stack* stack, T* ptr) : m_Stack(stack), m_Offset(stack->GetOffset(ptr)) { }

				T* get()
				{
					TASSERT(this->operator bool() == TTRUE, "Pointer is null");
					return reinterpret_cast<T*>(m_Stack->GetBuffer() + m_Offset);
				}

				const T* get() const
				{
					TASSERT(this->operator bool() == TTRUE, "Pointer is null");
					return reinterpret_cast<const T*>(m_Stack->GetBuffer() + m_Offset);
				}

				SECT::Stack* stack()
				{
					return m_Stack;
				}

				size_t offset() const
				{
					return m_Offset;
				}

				T& operator*()
				{
					TASSERT(this->operator bool() == TTRUE, "Pointer is null");
					return *reinterpret_cast<T*>(m_Stack->GetBuffer() + m_Offset);
				}

				T* operator->()
				{
					TASSERT(this->operator bool() == TTRUE, "Pointer is null");
					return reinterpret_cast<T*>(m_Stack->GetBuffer() + m_Offset);
				}

				template <typename N>
				Ptr<T> operator+(const N& a_iValue)
				{
					return Ptr<T>{m_Stack, m_Offset + sizeof(T) * a_iValue};
				}

				template <typename N>
				Ptr<T> operator-(const N& a_iValue)
				{
					return Ptr<T>{m_Stack, m_Offset - sizeof(T) * a_iValue};
				}

				operator bool() const
				{
					return m_Stack != TNULL;
				}

			private:
				SECT::Stack* m_Stack;
				size_t m_Offset;
			};

			struct RelcPtr
			{
				size_t Offset;
				size_t DataPtr;
				SECT::Stack* DataStack;
			};

		public:
			Stack(uint32_t index)
			{
				m_Index = index;
				m_Buffer = TNULL;
				m_BufferPos = TNULL;
				m_BufferSize = 0;
				m_ExpectedSize = 0;
				m_PtrList.reserve(32);
				GrowBuffer(0);
			}

			~Stack()
			{
				if (m_Buffer != TNULL)
				{
					delete[] m_Buffer;
				}
			}

			size_t Tell() { return GetUsedSize(); }

			void Seek(size_t newPos)
			{
				TASSERT(newPos < m_BufferSize, "Trying to seek out of buffer");
				m_BufferPos = m_Buffer + newPos;
			}

			void SetExpectedSize(uint32_t expectedSize) { m_ExpectedSize = expectedSize; }

			uint32_t GetExpectedSize() const { return m_ExpectedSize; }

			uint32_t GetIndex() const { return m_Index; }

			size_t GetPointerCount() const { return m_PtrList.size(); }

			char* GetBuffer() const { return m_Buffer; }

			char* GetBufferPos() const { return m_BufferPos; }

			size_t GetBufferSize() const { return m_BufferSize; }

			size_t GetUsedSize() const { return (size_t)m_BufferPos - (size_t)m_Buffer; }

			size_t GetOffset(const void* ptr) const
			{
				TASSERT((size_t)ptr >= (size_t)m_Buffer && (size_t)ptr < (size_t)m_Buffer + m_BufferSize, "Pointer is out of buffer");
				return (size_t)ptr - (size_t)m_Buffer;
			}

			template <class T>
			void SetCrossPointer(T** outPtr, Ptr<T> ptr);

			template <class T>
			T* Write(size_t offset, const T& value)
			{
				TASSERT(offset >= 0 && offset < m_BufferSize, "Offset is out of buffer");
				*(T*)(&m_Buffer[offset]) = value;
				return reinterpret_cast<T*>(&m_Buffer[offset]);
			}

			void Write(size_t offset, char* value, int size)
			{
				TASSERT(offset >= 0 && offset < m_BufferSize, "Offset is out of buffer");
				Toshi::TUtil::MemCopy(&m_Buffer[offset], value, size);
			}

			template <class T>
			Ptr<T> Alloc();

			template <class T>
			Ptr<T> Alloc(size_t count);

			Ptr<char> AllocBytes(size_t Size);

			template <class T>
			void WritePointer(T** outPtr, const T* ptr);

			template <class T>
			void WritePointer(T** outPtr, const Ptr<T>& ptr);

			template <class T>
			Ptr<T> Alloc(T** outPtr, size_t count);

			template <class T, size_t Count = 1>
			Ptr<T> Alloc(T** outPtr);

			void Link();
			void Unlink();

			std::vector<RelcPtr>::iterator begin() { return m_PtrList.begin(); }
			std::vector<RelcPtr>::iterator end() { return m_PtrList.end(); }

			void GrowBuffer(size_t requiredSize);

		private:
			void ResizeBuffer(size_t size);

			void AddRelocationPtr(size_t offset, size_t dataPtr, Stack* pDataStack = TNULL)
			{
				m_PtrList.emplace_back(offset, dataPtr, pDataStack == TNULL ? this : pDataStack);
			}

			uint32_t SetIndex(uint32_t index)
			{
				uint32_t oldIndex = m_Index;
				m_Index = index;
				return oldIndex;
			}

		private:
			uint32_t m_Index;
			char* m_Buffer;
			char* m_BufferPos;
			size_t m_BufferSize;
			uint32_t m_ExpectedSize;
			std::vector<RelcPtr> m_PtrList;
			std::vector<Stack*> m_DependentStacks;
		};

	public:
		SECT() = default;

		~SECT()
		{
			Reset();
		}

		void Reset()
		{
			for (auto stack : m_Stacks)
				delete stack;

			m_Stacks.clear();
		}

		size_t GetStackCount() const
		{
			return m_Stacks.size();
		}

		SECT::Stack* CreateStack();
		bool DeleteStack(SYMB* pSymb, SECT::Stack* pStack);

		SECT::Stack* GetStack(size_t index);

		void Write(Toshi::TTSFO& ttsfo, bool compress);
		void Read(Toshi::TTSFI& ttsfi, bool compressed = false);

		std::vector<SECT::Stack*>::iterator begin() { return m_Stacks.begin(); }
		std::vector<SECT::Stack*>::iterator end() { return m_Stacks.end(); }

	private:
		std::vector<SECT::Stack*> m_Stacks;
	};

	class SYMB
	{
	public:
		SYMB()
		{
			Reset();
		}

		void Reset()
		{
			m_Symbols.clear();
			m_SymbolNames.clear();
			m_Symbols.reserve(5);
			m_SymbolNames.reserve(5);
		}

		bool Is(size_t index, const char* name)
		{
			TASSERT(index >= 0 && index < m_Symbols.size());
			auto hash = Toshi::TTRB::HashString(name);

			if (m_Symbols[index].NameHash == hash)
			{
				if (m_SymbolNames[index] == name)
				{
					return true;
				}
			}

			return false;
		}

		int FindIndex(SECT& sect, const char* name)
		{
			auto hash = Toshi::TTRB::HashString(name);

			for (size_t i = 0; i < m_Symbols.size(); i++)
			{
				if (m_Symbols[i].NameHash == hash)
				{
					if (m_SymbolNames[i] == name)
					{
						return i;
					}
				}
			}

			return -1;
		}

		int FindIndex(SECT* sect, const char* name)
		{
			return FindIndex(*sect, name);
		}

		template <class T>
		SECT::Stack::Ptr<T> Find(SECT& sect, const char* name)
		{
			int index = FindIndex(sect, name);

			if (index != -1)
			{
				auto stack = sect.GetStack(m_Symbols[index].HDRX);
				return { stack, m_Symbols[index].DataOffset };
			}

			return { TNULL, (size_t)0 };
		}

		template <class T>
		SECT::Stack::Ptr<T> Find(SECT* sect, const char* name)
		{
			return Find<T>(*sect, name);
		}

		template <class T>
		SECT::Stack::Ptr<T> GetByIndex(SECT& sect, int index)
		{
			TASSERT(index >= 0 && index < m_Symbols.size());

			return {
				sect.GetStack(m_Symbols[index].HDRX),
				m_Symbols[index].DataOffset
			};
		}

		template <class T>
		SECT::Stack::Ptr<T> GetByIndex(SECT* sect, int index)
		{
			return GetByIndex<T>(*sect, index);
		}

		size_t GetCount()
		{
			return m_Symbols.size();
		}

		void Add(SECT::Stack* pStack, const char* name, void* ptr)
		{
			m_Symbols.emplace_back(pStack->GetIndex(), 0, 0, 0, pStack->GetOffset(ptr));
			m_SymbolNames.push_back(name);
		}

		void UpdateSymbolsIndexes(SECT::Stack* pStack, uint32_t newIndex)
		{
			auto stackIndex = pStack->GetIndex();

			for (size_t i = 0; i < m_Symbols.size(); i++)
			{
				if (m_Symbols[i].HDRX == stackIndex)
				{
					m_Symbols[i].HDRX = newIndex;
				}
			}

			pStack->SetIndex(newIndex);
		}

		void Remove(size_t index)
		{
			TASSERT(index >= 0 && index < m_Symbols.size());

			m_Symbols.erase(m_Symbols.begin() + index);
			m_SymbolNames.erase(m_SymbolNames.begin() + index);
		}

		void RemoveAllWithStackIndex(int stackIndex)
		{
			for (size_t i = 0; i < m_Symbols.size();)
			{
				if (m_Symbols[i].HDRX == stackIndex)
				{
					Remove(i);
				}
				else
				{
					i++;
				}
			}
		}

		void Write(Toshi::TTSFO& ttsfo)
		{
			TASSERT(m_Symbols.size() == m_SymbolNames.size(), "");

			uint32_t nameOffset = 0;
			uint32_t symbolCount = m_Symbols.size();
			ttsfo.Write(symbolCount);

			for (size_t i = 0; i < m_Symbols.size(); i++)
			{
				m_Symbols[i].NameHash = Toshi::TTRB::HashString(m_SymbolNames[i].c_str());
				m_Symbols[i].NameOffset = nameOffset;
				nameOffset += m_SymbolNames[i].length() + 1;
				ttsfo.Write(m_Symbols[i]);
			}

			for (auto& name : m_SymbolNames)
			{
				ttsfo.WriteRaw(name.c_str(), name.length());
				ttsfo.Write((uint8_t)0);
			}
		}

		void Read(Toshi::TTSFI& ttsfi, SECT& sect)
		{
			uint32_t symbolCount = 0;
			ttsfi.Read(&symbolCount);

			// Read symbols
			size_t symbolsSize = sizeof(Toshi::TTRB::TTRBSymbol) * symbolCount;
			m_Symbols.resize(symbolCount);
			ttsfi.ReadRaw(m_Symbols.data(), symbolsSize);

			// Read symbol names
			size_t namesSize = ttsfi.GetCurrentHunk().Size - symbolsSize;
			m_SymbolNames.reserve(symbolCount);
			char* namesBuffer = new char[namesSize];
			ttsfi.ReadRaw(namesBuffer, namesSize);

			for (auto& symbol : m_Symbols)
			{
				const char* symbolName = &namesBuffer[symbol.NameOffset];
				m_SymbolNames.push_back(symbolName);
			}

			delete[] namesBuffer;
		}

		std::vector<std::string>::iterator begin() { return m_SymbolNames.begin(); }
		std::vector<std::string>::iterator end() { return m_SymbolNames.end(); }

	private:
		std::vector<Toshi::TTRB::TTRBSymbol> m_Symbols;
		std::vector<std::string> m_SymbolNames;
	};

	class TRBF
	{
	public:
		static constexpr TVersion VERSION = { TMAKEVERSION(1, 1) };

	public:
		TRBF() : m_HDRX(VERSION) { }
		TRBF(const std::string& filepath) : m_HDRX(VERSION) { ReadFromFile(filepath); }

		void Reset()
		{
			m_SECT.Reset();
			m_SYMB.Reset();
		}

		bool ReadFromFile(const std::string& filepath)
		{
			Reset();

			Toshi::TTSFI ttsfi;
			auto pFile = Toshi::TFile::Create(filepath.c_str());

			if (pFile && ttsfi.Open(pFile) == Toshi::TTRB::ERROR_OK)
			{
				int32_t leftSize = ttsfi.GetCurrentHunk().Size - 4;

				while (leftSize > sizeof(Toshi::TTSF::Hunk))
				{
					if (ttsfi.ReadHunk() != Toshi::TTRB::ERROR_OK) break;
					leftSize -= ttsfi.GetCurrentHunk().Size + sizeof(Toshi::TTSF::Hunk);

					switch (ttsfi.GetCurrentHunk().Name)
					{
					case TMAKEFOUR("HDRX"):
						m_HDRX.Read(ttsfi, m_SECT);
						break;
					case TMAKEFOUR("SECT"):
						m_SECT.Read(ttsfi);
						break;
					case TMAKEFOUR("SECC"):
						m_SECT.Read(ttsfi, true);
						break;
					case TMAKEFOUR("RELC"):
						m_RELC.Read(ttsfi, m_SECT);
						break;
					case TMAKEFOUR("SYMB"):
						m_SYMB.Read(ttsfi, m_SECT);
						break;
					}

					ttsfi.SkipHunk();
				}

				ttsfi.Close(TFALSE);
				pFile->Destroy();
				return true;
			}

			if (pFile != TNULL)
			{
				ttsfi.Close(TFALSE);
				pFile->Destroy();
			}

			return false;
		}

		void WriteToFile(const std::string& filepath, bool compress = false, Toshi::TTSF::Endianess endianess = Toshi::TTSF::Endianess_Little)
		{
			Toshi::TTSFO ttsfo;
			Toshi::TTSFO::HunkMark mark;
			ttsfo.Create(filepath.c_str(), "TRBF", endianess);

			// HDRX
			ttsfo.OpenHunk(&mark, "HDRX");
			m_HDRX.SetSectionCount(m_SECT.GetStackCount());
			m_HDRX.Write(ttsfo, m_SECT);
			ttsfo.CloseHunk(&mark);

			// SECT
			ttsfo.OpenHunk(&mark, compress ? "SECC" : "SECT");
			m_SECT.Write(ttsfo, compress);
			ttsfo.CloseHunk(&mark);

			// RELC
			ttsfo.OpenHunk(&mark, "RELC");
			m_RELC.Write(ttsfo, m_SECT);
			ttsfo.CloseHunk(&mark);

			// SYMB
			ttsfo.OpenHunk(&mark, "SYMB");
			m_SYMB.Write(ttsfo);
			ttsfo.CloseHunk(&mark);

			ttsfo.Close();
		}

		SYMB* GetSYMB()
		{
			return &m_SYMB;
		}

		SECT* GetSECT()
		{
			return &m_SECT;
		}

	private:
		HDRX m_HDRX;
		SECT m_SECT;
		RELC m_RELC;
		SYMB m_SYMB;
	};

	inline HDRX::HDRX(TVersion version)
	{
		m_Header.m_ui32Version = version;
		m_Header.m_i32SectionCount = 0;
	}

	inline void HDRX::SetVersion(TVersion version)
	{
		m_Header.m_ui32Version = version;
	}

	inline void HDRX::SetSectionCount(int32_t count)
	{
		m_Header.m_i32SectionCount = count;
	}

	inline void HDRX::Write(Toshi::TTSFO& ttsfo, SECT& sect)
	{
		ttsfo.Write(m_Header);

		for (SECT::Stack* stack : sect)
		{
			Toshi::TTRB::SecInfo sectionInfo = {};
			sectionInfo.m_Size = Toshi::TMath::AlignNumUp(stack->GetUsedSize());
			ttsfo.Write(sectionInfo);
		}
	}

	inline void HDRX::Read(Toshi::TTSFI& ttsfi, SECT& sect)
	{
		ttsfi.Read(&m_Header);

		for (int i = 0; i < m_Header.m_i32SectionCount; i++)
		{
			Toshi::TTRB::SecInfo sectionInfo;
			ttsfi.Read(&sectionInfo);

			auto stack = sect.CreateStack();
			stack->SetExpectedSize(sectionInfo.m_Size);
		}
	}

	inline void RELC::Write(Toshi::TTSFO& ttsfo, SECT& sect)
	{
		uint32_t ptrCount = 0;
		for (auto section : sect)
		{
			ptrCount += section->GetPointerCount();
		}

		ttsfo.Write(ptrCount);

		for (size_t i = 0; i < sect.GetStackCount(); i++)
		{
			auto section = sect.GetStack(i);

			for (auto& ptr : *section)
			{
				Toshi::TTRB::RELCEntry entry = {};
				entry.HDRX1 = (short)i;
				entry.HDRX2 = ptr.DataStack->GetIndex();
				entry.Offset = ptr.Offset;
				ttsfo.Write(entry);
			}
		}
	}

	inline void RELC::Read(Toshi::TTSFI& ttsfi, SECT& sect)
	{
		uint32_t ptrCount = 0;
		ttsfi.Read(&ptrCount);

		Toshi::TTRB::RELCEntry entry;
		for (uint32_t i = 0; i < ptrCount; i++)
		{
			ttsfi.Read(&entry);
			auto stack = sect.GetStack(entry.HDRX1);
			auto dataStack = sect.GetStack(entry.HDRX2);
			uint32_t dataPtr = *(uint32_t*)(&stack->GetBuffer()[entry.Offset]);
			stack->AddRelocationPtr(entry.Offset, dataPtr, dataStack);
		}

		for (auto stack : sect)
		{
			stack->Link();
		}
	}

	template<class T>
	inline void SECT::Stack::SetCrossPointer(T** outPtr, Ptr<T> ptr)
	{
		TASSERT((size_t)outPtr >= (size_t)m_Buffer && (size_t)outPtr < (size_t)m_Buffer + (size_t)m_BufferSize, "Out pointer is out of buffer");
		TASSERT(TNULL != ptr.stack());
		TASSERT(this != ptr.stack());

		size_t outPtrOffset = GetOffset(outPtr);
		Write<T*>(outPtrOffset, ptr.get());

		AddRelocationPtr(outPtrOffset, ptr.offset(), ptr.stack());

		// Set current stack as dependent from ptr.stack()
		auto crossStack = ptr.stack();
		auto it = std::find(crossStack->m_DependentStacks.begin(), crossStack->m_DependentStacks.end(), this);

		if (it == crossStack->m_DependentStacks.end())
		{
			// Add this stack
			crossStack->m_DependentStacks.push_back(this);
		}
	}

	template<class T>
	inline SECT::Stack::Ptr<T> SECT::Stack::Alloc()
	{
		m_BufferPos = TREINTERPRETCAST(char*, Toshi::TMath::AlignPointer(m_BufferPos));

		constexpr size_t TSize = sizeof(T);
		GrowBuffer(GetUsedSize() + TSize);

		T* allocated = reinterpret_cast<T*>(m_BufferPos);
		m_BufferPos += TSize;

		return { this, allocated };
	}

	template<class T>
	inline SECT::Stack::Ptr<T> SECT::Stack::Alloc(size_t count)
	{
		m_BufferPos = TREINTERPRETCAST(char*, Toshi::TMath::AlignPointer(m_BufferPos));

		const size_t TSize = sizeof(T) * count;
		GrowBuffer(GetUsedSize() + TSize);

		T* allocated = reinterpret_cast<T*>(m_BufferPos);
		m_BufferPos += TSize;

		return { this, allocated };
	}

	template<class T>
	inline SECT::Stack::Ptr<T> SECT::Stack::Alloc(T** outPtr, size_t count)
	{
		TASSERT((size_t)outPtr >= (size_t)m_Buffer && (size_t)outPtr < (size_t)m_Buffer + (size_t)m_BufferSize, "Out pointer is out of buffer");
		m_BufferPos = TREINTERPRETCAST(char*, Toshi::TMath::AlignPointer(m_BufferPos));

		const size_t TSize = sizeof(T) * count;
		size_t outPtrOffset = GetOffset(outPtr);
		GrowBuffer(GetUsedSize() + TSize);

		T* allocated = reinterpret_cast<T*>(m_BufferPos);
		m_BufferPos += TSize;

		Write<T*>(outPtrOffset, allocated);
		AddRelocationPtr(outPtrOffset, GetOffset(allocated));

		return { this, allocated };
	}

	template<class T, size_t Count>
	inline SECT::Stack::Ptr<T> SECT::Stack::Alloc(T** outPtr)
	{
		TASSERT((size_t)outPtr >= (size_t)m_Buffer && (size_t)outPtr < (size_t)m_Buffer + (size_t)m_BufferSize, "Out pointer is out of buffer");
		m_BufferPos = TREINTERPRETCAST(char*, Toshi::TMath::AlignPointer(m_BufferPos));

		constexpr size_t TSize = sizeof(T) * Count;
		size_t outPtrOffset = GetOffset(outPtr);
		GrowBuffer(GetUsedSize() + TSize);

		T* allocated = reinterpret_cast<T*>(m_BufferPos);
		m_BufferPos += TSize;

		Write<T*>(outPtrOffset, allocated);
		AddRelocationPtr(outPtrOffset, GetOffset(allocated));

		return { this, allocated };
	}

	template<class T>
	inline void SECT::Stack::WritePointer(T** outPtr, const T* ptr)
	{
		TASSERT((size_t)outPtr >= (size_t)m_Buffer && (size_t)outPtr < (size_t)m_Buffer + (size_t)m_BufferSize, "Out pointer is out of buffer");
		TASSERT((size_t)ptr >= (size_t)m_Buffer && (size_t)ptr < (size_t)m_Buffer + (size_t)m_BufferSize, "Pointer is out of buffer");

		size_t outPtrOffset = GetOffset(outPtr);
		Write<uint32_t>(outPtrOffset, TREINTERPRETCAST(uint32_t, ptr));
		AddRelocationPtr(outPtrOffset, GetOffset(ptr));
	}

	template<class T>
	inline void SECT::Stack::WritePointer(T** outPtr, const SECT::Stack::Ptr<T>& ptr)
	{
		TASSERT((size_t)outPtr >= (size_t)m_Buffer && (size_t)outPtr < (size_t)m_Buffer + (size_t)m_BufferSize, "Out pointer is out of buffer");

		size_t outPtrOffset = GetOffset(outPtr);
		Write<uint32_t>(outPtrOffset, TREINTERPRETCAST(uint32_t, ptr.get()));
		AddRelocationPtr(outPtrOffset, ptr.offset());
	}

	inline SECT::Stack::Ptr<char> SECT::Stack::AllocBytes(size_t Size)
	{
		GrowBuffer(GetUsedSize() + Size);

		char* allocated = reinterpret_cast<char*>(m_BufferPos);
		m_BufferPos += Size;

		return { this, allocated };
	}

	inline void SECT::Stack::Link()
	{
		for (auto& ptr : m_PtrList)
		{
			Write<void*>(ptr.Offset, ptr.DataStack->GetBuffer() + ptr.DataPtr);
		}
	}

	inline void SECT::Stack::Unlink()
	{
		for (auto& ptr : m_PtrList)
		{
			Write<void*>(ptr.Offset, (void*)ptr.DataPtr);
		}
	}

	inline void SECT::Stack::GrowBuffer(size_t requiredSize)
	{
		size_t newSize = ((requiredSize / BUFFER_GROW_SIZE) + 1) * BUFFER_GROW_SIZE;

		if (newSize != m_BufferSize)
		{
			ResizeBuffer(newSize);
		}
	}

	inline void SECT::Stack::ResizeBuffer(size_t size)
	{
		TASSERT(size > 0, "Size should be positive");
		//TASSERT(size != m_BufferSize, "Size is the same");
		//TASSERT(size > m_BufferSize, "Buffer can't shrink");

		char* oldBuffer = m_Buffer;
		size_t usedSize = GetUsedSize();
		m_Buffer = new char[size];
		m_BufferSize = size;

		Toshi::TUtil::MemClear(m_Buffer, size);

		if (oldBuffer != TNULL)
		{
			Toshi::TUtil::MemCopy(m_Buffer, oldBuffer, usedSize);
			m_BufferPos = m_Buffer + usedSize;
			delete[] oldBuffer;
		}
		else
		{
			m_BufferPos = m_Buffer;
		}

		Link();
	}

	inline SECT::Stack* SECT::CreateStack()
	{
		SECT::Stack* stack = new SECT::Stack(m_Stacks.size());
		m_Stacks.push_back(stack);
		return stack;
	}

	inline bool SECT::DeleteStack(SYMB* pSymb, SECT::Stack* pStack)
	{
		auto result = std::find(m_Stacks.begin(), m_Stacks.end(), pStack);

		if (result != m_Stacks.end())
		{
			pSymb->RemoveAllWithStackIndex(pStack->GetIndex());

			m_Stacks.erase(result);
			delete pStack;

			// Update indexes
			uint32_t index = 0;
			for (auto it = m_Stacks.begin(); it != m_Stacks.end(); it++)
			{
				auto stack = *it;
				pSymb->UpdateSymbolsIndexes(stack, index++);
			}

			return true;
		}
		else
		{
			return false;
		}
	}

	inline SECT::Stack* SECT::GetStack(size_t index)
	{
		TASSERT(index >= 0 && index < GetStackCount(), "Index is out of bounds");
		return m_Stacks[index];
	}

	inline void SECT::Write(Toshi::TTSFO& ttsfo, bool compress)
	{
		size_t ready = 0;
		size_t count = m_Stacks.size();

		if (compress)
		{
			TOSHI_CORE_TRACE("Compressing progress: 0%");
		}

		for (auto stack : m_Stacks)
		{
			stack->Unlink();

			if (compress)
			{
				ttsfo.WriteCompressed(stack->GetBuffer(), stack->GetUsedSize());
				ready += 1;
				TOSHI_CORE_TRACE("Compressing progress: {0:.1f}%", (double)ready / count * 100);
			}
			else
			{
				ttsfo.WriteRaw(stack->GetBuffer(), stack->GetUsedSize());
			}

			ttsfo.WriteAlignmentPad();
			stack->Link();
		}
	}

	inline void SECT::Read(Toshi::TTSFI& ttsfi, bool compressed)
	{
		for (auto stack : m_Stacks)
		{
			size_t expectedSize = stack->GetExpectedSize();

			if (expectedSize > 0)
			{
				stack->GrowBuffer(expectedSize);

				if (compressed)
				{
					ttsfi.ReadCompressed(stack->GetBuffer(), expectedSize);
				}
				else
				{
					ttsfi.ReadRaw(stack->GetBuffer(), expectedSize);
				}

				stack->Seek(expectedSize);
				stack->SetExpectedSize(0);
			}
		}
	}
}