#pragma once
#include "InterfacePointers.h"

namespace Rath
{
	//--------------------------------------------------------------------------------------
	//
	//--------------------------------------------------------------------------------------
	typedef enum
	{
		GPU_TIME_BACKGROUND_PASS,
		GPU_TIME_DEPTH_PRE_PASS,
		GPU_TIME_SHADOW_MAPS,
		GPU_TIME_SCENE,
		GPU_TIME_LIGHTSHAFTS_PASS,
		GPU_TIME_AO_PASS,
		GPU_TIME_BLOOM_PASS,
		GPU_TIME_DOF_PASS,
		GPU_TIME_AA_PASS,
		NUM_GPU_TIMES
	} RenderTimeId;

	typedef struct
	{
		float GpuTimeMS[NUM_GPU_TIMES];
		float CpuTimeUS;
	} RenderTimes;

	//--------------------------------------------------------------------------------------
	//
	//--------------------------------------------------------------------------------------
	class TimestampQueries
	{
	public:
		void CreateDevice(ID3D11Device* pD3DDevice);

		void Begin(ID3D11DeviceContext* pDeviceContext);
		void End(ID3D11DeviceContext* pDeviceContext, RenderTimes* pRenderTimes);

		void StartTimer(ID3D11DeviceContext* pDeviceContext, RenderTimeId Id);
		void StopTimer(ID3D11DeviceContext* pDeviceContext, RenderTimeId Id);

	protected:
		bool m_DisjointQueryInFlight;
		bool m_TimestampQueryInFlight[NUM_GPU_TIMES];
		ID3D11QueryPtr m_pDisjointTimestampQuery;
		ID3D11QueryPtr m_pTimestampQueriesBegin[NUM_GPU_TIMES];
		ID3D11QueryPtr m_pTimestampQueriesEnd[NUM_GPU_TIMES];
	};
	extern TimestampQueries g_TimestampQueries;
	extern RenderTimes		g_RenderTimes;

	//--------------------------------------------------------------------------------------
	//
	//--------------------------------------------------------------------------------------
	class GPUTimer
	{
	public:
		GPUTimer(TimestampQueries* pTimestampQueries, ID3D11DeviceContext* pDeviceContext, RenderTimeId Id);
		~GPUTimer();

	private:
		TimestampQueries* m_pTimestampQueries;
		ID3D11DeviceContext* m_pDeviceContext;
		RenderTimeId m_NVAORenderTimeId;
	};

#if defined(_PROFILE) | defined(_DEBUG)
	#define DEBUGTIMER(time) GPUTimer timer(&g_TimestampQueries, context, time)
#else
	#define DEBUGTIMER(time)
#endif
}