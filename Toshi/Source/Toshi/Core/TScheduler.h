#pragma once
#include "TKernelInterface.h"
#include "TNodeTree.h"

namespace Toshi
{
	class TTask;

	class TScheduler : public TObject
	{
		TOSHI_CLASS_NO_CREATE_DEFINE(TScheduler)

	public:
		TScheduler(TKernelInterface* kernelInterface);
		TTask* CreateTask(TClass* toshiClass, TTask* task);

	public:
		inline TKernelInterface* GetKernelInterface() const { return m_KernelInterface; }
		inline float GetCurrentTimeDelta() const { return m_CurrentTimeDelta; }
		inline float GetTotalTime() const { return m_TotalTime; }
		inline uint32_t GetFrameCount() const { return m_FrameCount; }
		inline TNodeTree<TTask> GetTree() const { return m_TaskTree; }
		
		inline uint32_t TaskCount() const { return m_TaskCount; }
		inline void TaskCount(uint32_t count) { m_TaskCount = count; }

		inline void DeleteTask()
		{
			uint32_t oldTaskCount = TaskCount();
			TaskCount(oldTaskCount - 1);

			if (oldTaskCount == 1) { Delete(); }
		}
			
	private:
		uint32_t m_TaskCount;                   // 0x04
		TKernelInterface* m_KernelInterface;    // 0x08
		TNodeTree<TTask> m_TaskTree;            // 0x0C
		void* m_Unk1;                           // 0x10
		void* m_Unk2;                           // 0x14
		uint32_t m_Unk3;                        // 0x18
		TTask* m_SomeTask;                      // 0x1C
		void* m_Unk4;                           // 0x20
		float m_CurrentTimeDelta;               // 0x24
		float m_TotalTime;                      // 0x28
		float m_TimeDeltaLimit;                 // 0x2C
		uint32_t m_FrameCount;                  // 0x30
	};
}