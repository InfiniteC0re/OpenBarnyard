#pragma once
#include "Hunk.h"

namespace TLib
{
	namespace TRBF
	{
		class SECT : public Hunk
		{
		public:
			class Stack
			{
			public:
				static constexpr size_t BUFFER_GROW_SIZE = 4096;

				template <class T>
				class Ptr
				{
				public:
					Ptr(SECT::Stack* stack, size_t offset) : m_Stack(stack), m_Offset(offset) { }

					T* get()
					{
						return reinterpret_cast<T*>(m_Stack->GetBuffer() + m_Offset);
					}

					SECT::Stack* stack()
					{
						return m_Stack;
					}

					size_t offset()
					{
						return m_Offset;
					}

					T& operator*()
					{
						return *reinterpret_cast<T*>(m_Stack->GetBuffer() + m_Offset);
					}

					T* operator->()
					{
						return reinterpret_cast<T*>(m_Stack->GetBuffer() + m_Offset);
					}

				private:
					SECT::Stack* m_Stack;
					size_t m_Offset;
				};

				struct RelcPtr
				{
					size_t Offset;
					size_t DataPtr;
				};

			public:
				Stack(uint32_t index)
				{
					m_Index = index;
					m_Buffer = TNULL;
					m_BufferPos = TNULL;
					m_BufferSize = 0;
					GrowBuffer(0);
				}

				~Stack()
				{
					if (m_Buffer != TNULL)
					{
						delete[] m_Buffer;
					}
				}

				uint32_t GetIndex() const
				{
					return m_Index;
				}

				size_t GetPointerCount() const
				{
					return m_PtrList.size();
				}

				char* GetBuffer() const
				{
					return m_Buffer;
				}

				char* GetBufferPos() const
				{
					return m_BufferPos;
				}

				size_t GetUsedSize() const
				{
					return (size_t)m_BufferPos - (size_t)m_Buffer;
				}

				size_t GetOffset(void* ptr) const
				{
					TASSERT((size_t)ptr >= (size_t)m_Buffer && (size_t)ptr < (size_t)m_Buffer + m_BufferSize, "Pointer is out of buffer");
					return (size_t)ptr - (size_t)m_Buffer;
				}

				template <class T>
				T* Write(size_t offset, const T& value)
				{
					TASSERT(offset >= 0 && offset < m_BufferSize, "Offset is out of buffer");
					*(T*)(&m_Buffer[offset]) = value;
					return reinterpret_cast<T*>(&m_Buffer[offset]);
				}

				template <class T>
				Ptr<T> Alloc()
				{
					constexpr size_t TSize = sizeof(T);
					GrowBuffer(m_BufferSize + TSize);

					T* allocated = reinterpret_cast<T*>(m_BufferPos);
					m_BufferPos += TSize;

					return { this, (size_t)allocated - (size_t)m_Buffer };
				}

				template <class T>
				Ptr<T> Alloc(T** outPtr, size_t count)
				{
					TASSERT((size_t)outPtr >= (size_t)m_Buffer && (size_t)outPtr < (size_t)m_Buffer + (size_t)m_BufferSize, "Out pointer is out of buffer");

					const size_t TSize = sizeof(T) * count;
					size_t outPtrOffset = GetOffset(outPtr);
					GrowBuffer(m_BufferSize + TSize);

					T* allocated = reinterpret_cast<T*>(m_BufferPos);
					m_BufferPos += TSize;
					Write<T*>(outPtrOffset, allocated);

					size_t absolutePos = (size_t)allocated - (size_t)m_Buffer;
					m_PtrList.emplace_back(outPtrOffset, absolutePos);

					return { this, absolutePos };
				}

				template <class T, size_t Count = 1>
				Ptr<T> Alloc(T** outPtr)
				{
					TASSERT((size_t)outPtr >= (size_t)m_Buffer && (size_t)outPtr < (size_t)m_Buffer + (size_t)m_BufferSize, "Out pointer is out of buffer");

					constexpr size_t TSize = sizeof(T) * Count;
					size_t outPtrOffset = GetOffset(outPtr);
					GrowBuffer(m_BufferSize + TSize);

					T* allocated = reinterpret_cast<T*>(m_BufferPos);
					m_BufferPos += TSize;
					Write<T*>(outPtrOffset, allocated);

					size_t absolutePos = (size_t)allocated - (size_t)m_Buffer;
					m_PtrList.emplace_back(outPtrOffset, absolutePos);

					return { this, absolutePos };
				}

				void Link()
				{
					for (auto& ptr : m_PtrList)
					{
						Write<void*>(ptr.Offset, GetBuffer() + ptr.DataPtr);
					}
				}

				void Unlink()
				{
					for (auto& ptr : m_PtrList)
					{
						Write<void*>(ptr.Offset, (void*)ptr.DataPtr);
					}
				}

				std::vector<RelcPtr>::iterator begin()
				{
					return m_PtrList.begin();
				}

				std::vector<RelcPtr>::iterator end()
				{
					return m_PtrList.end();
				}

			private:
				void GrowBuffer(size_t requiredSize)
				{
					size_t newSize = ((requiredSize / BUFFER_GROW_SIZE) + 1) * BUFFER_GROW_SIZE;

					if (newSize != m_BufferSize)
					{
						ResizeBuffer(newSize);
					}
				}

				void ResizeBuffer(size_t size)
				{
					TASSERT(size > 0, "Size should be positive");
					TASSERT(size != m_BufferSize, "Size is the same");
					TASSERT(size > m_BufferSize, "Buffer can't shrink");

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

			private:
				uint32_t m_Index;
				char* m_Buffer;
				char* m_BufferPos;
				size_t m_BufferSize;
				std::vector<RelcPtr> m_PtrList;
			};

		public:
			SECT() = default;

			~SECT()
			{
				for (auto stack : m_Sections)
				{
					delete stack;
				}
			}

			size_t GetSectionCount() const
			{
				return m_Sections.size();
			}

			SECT::Stack* CreateSection()
			{
				SECT::Stack* stack = new SECT::Stack(m_Sections.size());
				m_Sections.push_back(stack);
				return stack;
			}

			SECT::Stack* GetSection(size_t index)
			{
				TASSERT(index >= 0 && index < GetSectionCount(), "Index is out of bounds");
				return m_Sections[index];
			}

			void Write(Toshi::TTSFO& ttsfo)
			{
				for (auto stack : m_Sections)
				{
					stack->Unlink();
					ttsfo.WriteRaw(stack->GetBuffer(), stack->GetUsedSize());
					stack->Link();
				}
			}

			std::vector<SECT::Stack*>::iterator begin()
			{
				return m_Sections.begin();
			}

			std::vector<SECT::Stack*>::iterator end()
			{
				return m_Sections.end();
			}

		private:
			std::vector<SECT::Stack*> m_Sections;
		};
	}
}