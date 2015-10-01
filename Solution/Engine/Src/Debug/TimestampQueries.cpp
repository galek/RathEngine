#include "pch.h"
#include "TimestampQueries.h"

#include "Debug/Exceptions.h"

namespace Rath
{
	TimestampQueries g_TimestampQueries;
	RenderTimes		 g_RenderTimes;

	void TimestampQueries::CreateDevice(ID3D11Device* pD3DDevice)
	{
		D3D11_QUERY_DESC queryDesc;
		queryDesc.MiscFlags = 0;

		queryDesc.Query = D3D11_QUERY_TIMESTAMP_DISJOINT;
		DXCall(pD3DDevice->CreateQuery(&queryDesc, &m_pDisjointTimestampQuery));
		m_DisjointQueryInFlight = false;

		queryDesc.Query = D3D11_QUERY_TIMESTAMP;
		for (UINT i = 0; i < NUM_GPU_TIMES; ++i)
		{
			DXCall(pD3DDevice->CreateQuery(&queryDesc, &m_pTimestampQueriesBegin[i]));
			DXCall(pD3DDevice->CreateQuery(&queryDesc, &m_pTimestampQueriesEnd[i]));
			m_TimestampQueryInFlight[i] = false;
		}
	}

	void TimestampQueries::Begin(ID3D11DeviceContext* pDeviceContext)
	{
		if (!m_DisjointQueryInFlight)
		{
			pDeviceContext->Begin(m_pDisjointTimestampQuery);
		}
	}

	void TimestampQueries::End(ID3D11DeviceContext* pDeviceContext, RenderTimes* pRenderTimes)
	{
		if (!m_DisjointQueryInFlight)
		{
			pDeviceContext->End(m_pDisjointTimestampQuery);
		}
		m_DisjointQueryInFlight = true;

		D3D11_QUERY_DATA_TIMESTAMP_DISJOINT disjointTimestampValue;
		if (pDeviceContext->GetData(m_pDisjointTimestampQuery, &disjointTimestampValue, sizeof(disjointTimestampValue), D3D11_ASYNC_GETDATA_DONOTFLUSH) == S_OK)
		{
			m_DisjointQueryInFlight = false;

			if (!disjointTimestampValue.Disjoint)
			{
				double InvFrequencyMS = 1000.0 / disjointTimestampValue.Frequency;
				for (UINT i = 0; i < NUM_GPU_TIMES; ++i)
				{
					if (m_TimestampQueryInFlight[i])
					{
						UINT64 TimestampValueBegin;
						UINT64 TimestampValueEnd;
						if ((pDeviceContext->GetData(m_pTimestampQueriesBegin[i], &TimestampValueBegin, sizeof(UINT64), D3D11_ASYNC_GETDATA_DONOTFLUSH) == S_OK) &&
							(pDeviceContext->GetData(m_pTimestampQueriesEnd[i], &TimestampValueEnd, sizeof(UINT64), D3D11_ASYNC_GETDATA_DONOTFLUSH) == S_OK))
						{
							m_TimestampQueryInFlight[i] = false;
							pRenderTimes->GpuTimeMS[i] = float(double(TimestampValueEnd - TimestampValueBegin) * InvFrequencyMS);
						}
					}
					else
					{
						pRenderTimes->GpuTimeMS[i] = 0.f;
					}
				}
			}
		}
	}

	void TimestampQueries::StartTimer(ID3D11DeviceContext* pDeviceContext, RenderTimeId Id)
	{
		if (!m_TimestampQueryInFlight[Id])
		{
			pDeviceContext->End(m_pTimestampQueriesBegin[Id]);
		}
	}

	void TimestampQueries::StopTimer(ID3D11DeviceContext* pDeviceContext, RenderTimeId Id)
	{
		if (!m_TimestampQueryInFlight[Id])
		{
			pDeviceContext->End(m_pTimestampQueriesEnd[Id]);
		}
		m_TimestampQueryInFlight[Id] = true;
	}

	GPUTimer::GPUTimer(TimestampQueries* pTimestampQueries, ID3D11DeviceContext* pDeviceContext, RenderTimeId Id)
		: m_pTimestampQueries(pTimestampQueries)
		, m_pDeviceContext(pDeviceContext)
		, m_NVAORenderTimeId(Id)
	{
		m_pTimestampQueries->StartTimer(m_pDeviceContext, m_NVAORenderTimeId);
	}

	GPUTimer::~GPUTimer()
	{
		m_pTimestampQueries->StopTimer(m_pDeviceContext, m_NVAORenderTimeId);
	}
}