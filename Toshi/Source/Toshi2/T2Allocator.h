#pragma once

namespace Toshi
{
	class T2Allocator
	{
	public:
		virtual void* Malloc(size_t size) = 0;
		virtual void* Malloc(size_t size, size_t alignment) = 0;
		virtual void Free(void* ptr) = 0;
		virtual bool CanAllocate(int unk) = 0;
		virtual void* TryMalloc(size_t size) = 0;
		virtual void* TryMalloc(size_t size, size_t alignment) = 0;
		virtual size_t Size() = 0;
		virtual size_t Capacity() = 0;

		template<class T>
		T* New()
		{
			void* mem = Malloc(sizeof(T));
			T* object = new (mem) T();
			return object;
		}

		template<class T>
		T* New(const T& value)
		{
			void* mem = Malloc(sizeof(T));
			T* object = new (mem) T(value);
			return object;
		}

		template<class T, class I>
		T* New(const I& value)
		{
			void* mem = Malloc(sizeof(T));
			T* object = new (mem) T(value);
			return object;
		}

	public:
		static class T2GlobalAllocator s_GlobalAllocator;
	};

	class T2GlobalAllocator : public T2Allocator
	{
	public:
		virtual void* Malloc(size_t size) override;
		virtual void* Malloc(size_t size, size_t alignment) override;
		virtual void Free(void* ptr) override;
		virtual bool CanAllocate(int unk) override;
		virtual void* TryMalloc(size_t size) override;
		virtual void* TryMalloc(size_t size, size_t alignment) override;
		virtual size_t Size() override;
		virtual size_t Capacity() override;
	};
}