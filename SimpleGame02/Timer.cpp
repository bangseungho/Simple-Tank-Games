#include "stdafx.h"
#include "Timer.h"

CGameTimer::CGameTimer()
{
	::QueryPerformanceFrequency((LARGE_INTEGER*)&m_PerformanceFrequencyPerSec);
	::QueryPerformanceCounter((LARGE_INTEGER*)&m_nLastPerformanceCounter);
	m_fTimeScale = 1.0 / (double)m_PerformanceFrequencyPerSec;

	m_nBasePerformanceCounter = m_nLastPerformanceCounter;
	m_nPausedPerformanceCounter = 0;
	m_nStopPerformanceCounter = 0;

	m_nSampleCount = 0;
	m_nCurrentFrameRate = 0;
	m_FramePerSecond = 0;
	m_fFPSTimeElapsed = 0.0f;
}

CGameTimer::~CGameTimer()
{
}

void CGameTimer::Tick(float fLockFPS)
{
	// stop 플래그라면 경과 시간을 0으로 설정 후 리턴한다.
	if (m_bStopped)
	{
		m_fTimeElapsed = 0.0f;
		return;
	}

	float fTimeElapsed;
	// 경과 시간을 성능 카운터의 현재 값과 이전 값을 뺄셈 하여 타임 스케일을 곱한 뒤 저장한다.
	::QueryPerformanceCounter((LARGE_INTEGER*)&m_nCurrentPerformanceCounter);
	fTimeElapsed = float((m_nCurrentPerformanceCounter - m_nLastPerformanceCounter) * m_fTimeScale);

	// fLockFPS가 0보다 크다면 이 시간만큼 호출한 함수를 기다리게 한다.
	// 예를 들어 60프레임을 맞추고 싶다면 Tick(60.0f)를 호출하며 60프레임을 맞춘다.
	if (fLockFPS > 0.0f)
	{
		// 설정한 프레임보다 경과시간이 작다면 경과 시간을 계속 갱신한다
		while (fTimeElapsed < (1.0f / fLockFPS))
		{
			::QueryPerformanceCounter((LARGE_INTEGER*)&m_nCurrentPerformanceCounter);
			fTimeElapsed = float((m_nCurrentPerformanceCounter - m_nLastPerformanceCounter) * m_fTimeScale);
		}
	}

	// 만약 경과 시간이 저장 되었다면 마지막 카운터에 현재 카운터를 저장한다.
	m_nLastPerformanceCounter = m_nCurrentPerformanceCounter;

	// 만약 마지막 프레임 처리 시간과 현재 프레임 처리 시간의 차이가 1초보다 작다면 현재 프레임 처리 시간을 저장한다.
	if (fabsf(fTimeElapsed - m_fTimeElapsed) < 1.0f)
	{
		::memmove(&m_fFrameTime[1], m_fFrameTime, (MAX_SAMPLE_COUNT - 1) * sizeof(float));
		m_fFrameTime[0] = fTimeElapsed;
		if (m_nSampleCount < MAX_SAMPLE_COUNT) m_nSampleCount++;
	}

	// 초당 프레임 수를 1 증가시키고 현재 프레임 처리 시간을 누적하여 저장한다.
	m_FramePerSecond++;
	m_fFPSTimeElapsed += fTimeElapsed;
	if (m_fFPSTimeElapsed > 1.0f)
	{
		m_nCurrentFrameRate = m_FramePerSecond;
		m_FramePerSecond = 0;
		m_fFPSTimeElapsed = 0.0f;
	}

	// 누적된 프레임 처리 시간의 평균을 구하여 프레임 처리 시간을 구한다.
	m_fTimeElapsed = 0.0f;
	for (ULONG i = 0; i < m_nSampleCount; i++) m_fTimeElapsed += m_fFrameTime[i];
	if (m_nSampleCount > 0) m_fTimeElapsed /= m_nSampleCount;
}

unsigned long CGameTimer::GetFrameRate(LPTSTR lpszString, int nCharacters)
{
	// 현재 프레임 레이트를 문자열로 반환하여 lpszString 버퍼에 쓰고 " FPS"와 결합한다.
	if (lpszString)
	{
		_itow_s(m_nCurrentFrameRate, lpszString, nCharacters, 10);
		wcscat_s(lpszString, nCharacters, _T(" FPS)"));
	}

	return(m_nCurrentFrameRate);
}

float CGameTimer::GetTimeElapsed()
{
	return(m_fTimeElapsed);
}

float CGameTimer::GetTotalTime()
{
	if (m_bStopped) return(float(((m_nStopPerformanceCounter - m_nPausedPerformanceCounter) - m_nBasePerformanceCounter) * m_fTimeScale));
	return(float(((m_nCurrentPerformanceCounter - m_nPausedPerformanceCounter) - m_nBasePerformanceCounter) * m_fTimeScale));
}

void CGameTimer::Reset()
{
	__int64 nPerformanceCounter;
	::QueryPerformanceCounter((LARGE_INTEGER*)&nPerformanceCounter);

	m_nBasePerformanceCounter = nPerformanceCounter;
	m_nLastPerformanceCounter = nPerformanceCounter;
	m_nStopPerformanceCounter = 0;
	m_bStopped = false;
}

void CGameTimer::Start()
{
	__int64 nPerformanceCounter;
	::QueryPerformanceCounter((LARGE_INTEGER*)&nPerformanceCounter);
	if (m_bStopped)
	{
		m_nPausedPerformanceCounter += (nPerformanceCounter - m_nStopPerformanceCounter);
		m_nLastPerformanceCounter = nPerformanceCounter;
		m_nStopPerformanceCounter = 0;
		m_bStopped = false;
	}
}

void CGameTimer::Stop()
{
	if (!m_bStopped)
	{
		::QueryPerformanceCounter((LARGE_INTEGER*)&m_nStopPerformanceCounter);
		m_bStopped = true;
	}
}
