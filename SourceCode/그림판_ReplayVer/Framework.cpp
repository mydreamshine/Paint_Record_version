#include "stdafx.h"
#include "Framework.h"
#include "resource.h"

CFramework::CFramework()
{
}

CFramework::~CFramework()
{
}

//윈도우 핸들 지정 및 버퍼, 오브젝트 초기화
bool CFramework::OnCreate(HINSTANCE hInstance, HWND hMainWnd)
{
	//윈도우 핸들 지정
	m_hInstance = hInstance;
	m_hWnd = hMainWnd;

	//프레임 생성
	BuildFrameBuffer();

	//오브젝트 생성
	BuildObjects();

	return(true);
}

//윈도우 및 버퍼, 오브젝트 메모리 해제
void CFramework::OnDestroy()
{
	//후면버퍼 해제
	if (m_hBitmapFrameBuffer) ::DeleteObject(m_hBitmapFrameBuffer);
	if (m_hDCFrameBuffer) ::DeleteDC(m_hDCFrameBuffer);

	//윈도우 해제
	if (m_hWnd) DestroyWindow(m_hWnd);

	//오브젝트 해제
	ReleaseObjects();
}

//프레임 갱신
void CFramework::FrameAdvance()
{
	//윈도우가 활성화되어 있지 않으면 프레임을 갱신하지 않음
	if (!m_bActive) return;

	//입력처리
	ProcessInput();

	//후면버퍼 초기화
	ClearFrameBuffer(RGB(255, 255, 255));

	//렌더링
	m_pScene->Render(m_hWnd, m_hDCFrameBuffer);

	//전면버퍼와 후면버퍼 스왑
	PresentFrameBuffer();
}


// 렌더링에 쓰일 후면 버퍼 생성 및 초기화
void CFramework::BuildFrameBuffer()
{
	//현재 응용프로그램의 윈도우로부터 전면버퍼 핸들을 가져옴
	HDC hDC = ::GetDC(m_hWnd);

	//응용프로그램의 전면버퍼 영역 가져오기
	RECT rcClient;
	::GetClientRect(m_hWnd, &rcClient);

	//전면버퍼와 동일한 크기의 후면버퍼 생성
	//후면버퍼에 그릴 비트맵 형식의 텍스쳐 생성
	m_hDCFrameBuffer = ::CreateCompatibleDC(hDC);
	m_hBitmapFrameBuffer = ::CreateCompatibleBitmap(hDC, (rcClient.right - rcClient.left) + 1, (rcClient.bottom - rcClient.top) + 1);
	::SelectObject(m_hDCFrameBuffer, m_hBitmapFrameBuffer);

	//전면버퍼 핸들 해제 후 후면버퍼의 그리기 방식 지정
	::ReleaseDC(m_hWnd, hDC);
	::SetBkMode(m_hDCFrameBuffer, TRANSPARENT);
}

// 렌더링에 쓰일 후면 버퍼 초기화
void CFramework::ClearFrameBuffer(DWORD dwColor)
{
	//응용프로그램의 전면버퍼 영역 가져오기
	RECT rcClient;
	::GetClientRect(m_hWnd, &rcClient);
	m_pScene->m_rcClient = rcClient;

	//후면버퍼를 흰색으로 초기화
	::FillRect(m_hDCFrameBuffer, &rcClient, (HBRUSH)GetStockObject(WHITE_BRUSH));
}

//후면버퍼와 전면버퍼 스왑
void CFramework::PresentFrameBuffer()
{
	//현재 응용프로그램의 윈도우로부터 전면버퍼 핸들을 가져옴
	HDC hDC = ::GetDC(m_hWnd);

	//응용프로그램의 전면버퍼 영역 가져오기
	RECT rcClient;
	::GetClientRect(m_hWnd, &rcClient);
	m_pScene->m_rcClient = rcClient;

	//전면버퍼 크기
	int nWidth = rcClient.right - rcClient.left;
	int nHeight = rcClient.bottom - rcClient.top;

	//후면버퍼의 내용을 전면버퍼로 BitBlt(고속복사) 스왑
	::BitBlt(hDC, rcClient.left, rcClient.top, nWidth, nHeight, m_hDCFrameBuffer, rcClient.left, rcClient.top, SRCCOPY);
	::ReleaseDC(m_hWnd, hDC);//전면버퍼 핸들 해제
}

// 프로그램에 쓰일 객체 및 변수를 초기화
void CFramework::BuildObjects()
{
	//후면버퍼에 그릴 장면 생성
	m_pScene = new CScene();

	//응용프로그램의 전면버퍼 영역 가져오기
	RECT rcClient;
	::GetClientRect(m_hWnd, &rcClient);
	//장면의 영역은 현재 응용프로그램의 전면버퍼의 영역과 동일
	m_pScene->m_rcClient = rcClient;

	//장면에 연출될 오브젝트들 생성 및 초기화
	m_pScene->BuildObjects();
	//장면에서 쓰일 파일 입/출력 구조체 초기화
	m_pScene->SetOpenFileStruct(m_hWnd);

}

//프로그램에 쓰였던 객체 및 변수를 초기화
void CFramework::ReleaseObjects()
{
	//후면 버퍼에 그려졌던 장면 해제
	if (m_pScene)
	{
		//장면에 연출될 오브젝트들 해제
		m_pScene->ReleaseObjects();
		delete m_pScene;
	}
}

//한 프레임에서 처리할 입력처리
void CFramework::ProcessInput()
{
	//키입력 버퍼
	static UCHAR pKeyBuffer[256];
	//윈도우의 불투명도
	static int nAlphaBlend = 255;

	//키 입력을 키입력 버퍼에 저장
	if (::GetKeyboardState(pKeyBuffer))
	{
		//화살표 키에 대해,
		//현재 프로그램의 상태가 "재생중"일 때 재생되는 프레임을 Offset 시킨다.
		if (pKeyBuffer[VK_LEFT] & 0xF0)
			m_pScene->PlayedFrameOffset(-5);
		if (pKeyBuffer[VK_RIGHT] & 0xF0)
			m_pScene->PlayedFrameOffset(4);

		//PageUp/PageDown 키에 대해,
		//현재 윈도우의 불투명도를 조절한다.
		if (pKeyBuffer[VK_PRIOR] & 0xF0)
		{
			//불투명하게
			if (nAlphaBlend < 255) nAlphaBlend++;
			SetWindowLong(m_hWnd, GWL_EXSTYLE, GetWindowLong(m_hWnd, GWL_EXSTYLE) | WS_EX_LAYERED);
			::SetLayeredWindowAttributes(m_hWnd, 0, nAlphaBlend, LWA_ALPHA);
		}
		if (pKeyBuffer[VK_NEXT] & 0xF0)
		{
			//투명하게
			if (nAlphaBlend > 0) nAlphaBlend--;
			SetWindowLong(m_hWnd, GWL_EXSTYLE, GetWindowLong(m_hWnd, GWL_EXSTYLE) | WS_EX_LAYERED);
			::SetLayeredWindowAttributes(m_hWnd, 0, nAlphaBlend, LWA_ALPHA);
		}
	}
}

//윈도우 마우스 메세지 처리
void CFramework::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	if(m_pScene) m_pScene->OnProcessingMouseMessage(hWnd, nMessageID, wParam, lParam);
}

//윈도우 키보드 메세지 처리
void CFramework::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	if(m_pScene) m_pScene->OnProcessingKeyboardMessage(hWnd, nMessageID, wParam, lParam);
	switch (nMessageID)
	{
	case WM_KEYDOWN:
		switch (wParam)
		{
		case VK_ESCAPE:
			::PostQuitMessage(0);
			break;
		default:
			break;
		}
		break;
	default:
		break;
	}
}

//윈도우 메세지 처리
LRESULT CFramework::OnProcessingWindowMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	//윈도우 마우스 및 키보드 입력 메세지 처리
	OnProcessingMouseMessage(hWnd, nMessageID, wParam, lParam);
	OnProcessingKeyboardMessage(hWnd, nMessageID, wParam, lParam);

	switch (nMessageID)
	{
	case WM_COMMAND:
	{
		int wmId = LOWORD(wParam);
		// 메뉴 선택에 따른 처리
		switch (wmId)
		{
		case IDM_LOAD:
			m_pScene->LoadData();
			break;
		case IDM_REC:
			m_pScene->RecordData();
			break;
		case IDM_STOP_REC:
			m_pScene->StopRecording();
			break;
		case IDM_CLEAR:
			m_pScene->ClearScene();
			break;
		case IDM_PAUSE:
			m_pScene->SetPauseState();
			break;
		case IDM_PLAYx1:
			if (m_pScene->CheckRePlayState())
				SetTimer(hWnd, 1, 1000 / FPS, (TIMERPROC)TimeProc);
			break;
		case IDM_PLAYx4:
			if (m_pScene->CheckRePlayState())
				SetTimer(hWnd, 1, 1000 / (FPS * 4), (TIMERPROC)TimeProc);
			break;
		case IDM_PLAYx8:
			if (m_pScene->CheckRePlayState())
				SetTimer(hWnd, 1, 1000 / (FPS * 8), (TIMERPROC)TimeProc);
			break;
		case IDM_PLAYx16:
			if (m_pScene->CheckRePlayState())
				SetTimer(hWnd, 1, 1000 / (FPS * 16), (TIMERPROC)TimeProc);
			break;
		}
	}
	default:
		break;
	}
	return(0);
}
