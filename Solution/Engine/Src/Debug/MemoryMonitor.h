#pragma once
#include "nvapi.h"

class MemoryMonitor
{
public:
	struct MemInfo : PROCESS_MEMORY_COUNTERS
	{
		unsigned int DedicatedVideoMemoryInMB;
		unsigned int AvailableDedicatedVideoMemoryInMB;
		unsigned int CurrentAvailableDedicatedVideoMemoryInMB;
	};

	MemoryMonitor();

	void Init();
	void GetMemoryInfo(MemInfo* pInfo);
private:
	NvPhysicalGpuHandle m_GpuHandle;
};
extern MemoryMonitor g_MemoryMonitor;
