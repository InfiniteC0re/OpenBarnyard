#pragma once
#include "Hunk.h"

namespace TLib
{
	namespace TRBF
	{
		class SYMB;

		class SECT : public Hunk
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
					Ptr(SECT::Stack* stack, size_t offset) : m_Stack(stack), m_Offset(offset) { }
					Ptr(SECT::Stack* stack, T* ptr) : m_Stack(stack), m_Offset(stack->GetOffset(ptr)) { }

					T* get()
					{
						TASSERT(this->operator bool() == TTRUE, "Pointer is null");
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
						TASSERT(this->operator bool() == TTRUE, "Pointer is null");
						return *reinterpret_cast<T*>(m_Stack->GetBuffer() + m_Offset);
					}

					T* operator->()
					{
						TASSERT(this->operator bool() == TTRUE, "Pointer is null");
						return reinterpret_cast<T*>(m_Stack->GetBuffer() + m_Offset);
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

				size_t Tell()
				{
					return GetUsedSize();
				}

				void Seek(size_t newPos)
				{
					TASSERT(newPos < m_BufferSize, "Trying to seek out of buffer");
					m_BufferPos = m_Buffer + newPos;
				}

				void SetExpectedSize(uint32_t expectedSize)
				{
					m_ExpectedSize = expectedSize;
				}

				uint32_t GetExpectedSize() const
				{
					return m_ExpectedSize;
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
				void SetCrossPointer(T** outPtr, Ptr<T> ptr)
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

					return { this, allocated };
				}

				Ptr<char> AllocBytes(size_t Size)
				{
					size_t TSize = Size;
					GrowBuffer(m_BufferSize + TSize);

					char* allocated = reinterpret_cast<char*>(m_BufferPos);
					m_BufferPos += TSize;

					return { this, allocated };
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
					AddRelocationPtr(outPtrOffset, GetOffset(allocated));

					return { this, allocated };
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
					AddRelocationPtr(outPtrOffset, GetOffset(allocated));

					return { this, allocated };
				}

				void Link()
				{
					for (auto& ptr : m_PtrList)
					{
						Write<void*>(ptr.Offset, ptr.DataStack->GetBuffer() + ptr.DataPtr);
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

				void GrowBuffer(size_t requiredSize)
				{
					size_t newSize = ((requiredSize / BUFFER_GROW_SIZE) + 1) * BUFFER_GROW_SIZE;

					if (newSize != m_BufferSize)
					{
						ResizeBuffer(newSize);
					}
				}

			private:
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
				{
					delete stack;
				}

				m_Stacks.clear();
			}

			size_t GetStackCount() const
			{
				return m_Stacks.size();
			}

			SECT::Stack* CreateStack()
			{
				SECT::Stack* stack = new SECT::Stack(m_Stacks.size());
				m_Stacks.push_back(stack);
				return stack;
			}

			bool DeleteStack(SYMB* pSymb, SECT::Stack* pStack);

			SECT::Stack* GetStack(size_t index)
			{
				TASSERT(index >= 0 && index < GetStackCount(), "Index is out of bounds");
				return m_Stacks[index];
			}

			void Write(Toshi::TTSFO& ttsfo, bool compress)
			{
				size_t ready = 0;
				size_t count = m_Stacks.size();
				
				TOSHI_CORE_TRACE("Compressing progress: 0%");

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

			void Read(Toshi::TTSFI& ttsfi, bool compressed = false)
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

			std::vector<SECT::Stack*>::iterator begin()
			{
				return m_Stacks.begin();
			}

			std::vector<SECT::Stack*>::iterator end()
			{
				return m_Stacks.end();
			}

		private:
			std::vector<SECT::Stack*> m_Stacks;
		};
	}
}