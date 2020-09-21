#include "stdafx.h"
#include "Framework.h"
#include "resource.h"

CFramework::CFramework()
{
}

CFramework::~CFramework()
{
}

//������ �ڵ� ���� �� ����, ������Ʈ �ʱ�ȭ
bool CFramework::OnCreate(HINSTANCE hInstance, HWND hMainWnd)
{
	//������ �ڵ� ����
	m_hInstance = hInstance;
	m_hWnd = hMainWnd;

	//������ ����
	BuildFrameBuffer();

	//������Ʈ ����
	BuildObjects();

	return(true);
}

//������ �� ����, ������Ʈ �޸� ����
void CFramework::OnDestroy()
{
	//�ĸ���� ����
	if (m_hBitmapFrameBuffer) ::DeleteObject(m_hBitmapFrameBuffer);
	if (m_hDCFrameBuffer) ::DeleteDC(m_hDCFrameBuffer);

	//������ ����
	if (m_hWnd) DestroyWindow(m_hWnd);

	//������Ʈ ����
	ReleaseObjects();
}

//������ ����
void CFramework::FrameAdvance()
{
	//�����찡 Ȱ��ȭ�Ǿ� ���� ������ �������� �������� ����
	if (!m_bActive) return;

	//�Է�ó��
	ProcessInput();

	//�ĸ���� �ʱ�ȭ
	ClearFrameBuffer(RGB(255, 255, 255));

	//������
	m_pScene->Render(m_hWnd, m_hDCFrameBuffer);

	//������ۿ� �ĸ���� ����
	PresentFrameBuffer();
}


// �������� ���� �ĸ� ���� ���� �� �ʱ�ȭ
void CFramework::BuildFrameBuffer()
{
	//���� �������α׷��� ������κ��� ������� �ڵ��� ������
	HDC hDC = ::GetDC(m_hWnd);

	//�������α׷��� ������� ���� ��������
	RECT rcClient;
	::GetClientRect(m_hWnd, &rcClient);

	//������ۿ� ������ ũ���� �ĸ���� ����
	//�ĸ���ۿ� �׸� ��Ʈ�� ������ �ؽ��� ����
	m_hDCFrameBuffer = ::CreateCompatibleDC(hDC);
	m_hBitmapFrameBuffer = ::CreateCompatibleBitmap(hDC, (rcClient.right - rcClient.left) + 1, (rcClient.bottom - rcClient.top) + 1);
	::SelectObject(m_hDCFrameBuffer, m_hBitmapFrameBuffer);

	//������� �ڵ� ���� �� �ĸ������ �׸��� ��� ����
	::ReleaseDC(m_hWnd, hDC);
	::SetBkMode(m_hDCFrameBuffer, TRANSPARENT);
}

// �������� ���� �ĸ� ���� �ʱ�ȭ
void CFramework::ClearFrameBuffer(DWORD dwColor)
{
	//�������α׷��� ������� ���� ��������
	RECT rcClient;
	::GetClientRect(m_hWnd, &rcClient);
	m_pScene->m_rcClient = rcClient;

	//�ĸ���۸� ������� �ʱ�ȭ
	::FillRect(m_hDCFrameBuffer, &rcClient, (HBRUSH)GetStockObject(WHITE_BRUSH));
}

//�ĸ���ۿ� ������� ����
void CFramework::PresentFrameBuffer()
{
	//���� �������α׷��� ������κ��� ������� �ڵ��� ������
	HDC hDC = ::GetDC(m_hWnd);

	//�������α׷��� ������� ���� ��������
	RECT rcClient;
	::GetClientRect(m_hWnd, &rcClient);
	m_pScene->m_rcClient = rcClient;

	//������� ũ��
	int nWidth = rcClient.right - rcClient.left;
	int nHeight = rcClient.bottom - rcClient.top;

	//�ĸ������ ������ ������۷� BitBlt(��Ӻ���) ����
	::BitBlt(hDC, rcClient.left, rcClient.top, nWidth, nHeight, m_hDCFrameBuffer, rcClient.left, rcClient.top, SRCCOPY);
	::ReleaseDC(m_hWnd, hDC);//������� �ڵ� ����
}

// ���α׷��� ���� ��ü �� ������ �ʱ�ȭ
void CFramework::BuildObjects()
{
	//�ĸ���ۿ� �׸� ��� ����
	m_pScene = new CScene();

	//�������α׷��� ������� ���� ��������
	RECT rcClient;
	::GetClientRect(m_hWnd, &rcClient);
	//����� ������ ���� �������α׷��� ��������� ������ ����
	m_pScene->m_rcClient = rcClient;

	//��鿡 ����� ������Ʈ�� ���� �� �ʱ�ȭ
	m_pScene->BuildObjects();
	//��鿡�� ���� ���� ��/��� ����ü �ʱ�ȭ
	m_pScene->SetOpenFileStruct(m_hWnd);

}

//���α׷��� ������ ��ü �� ������ �ʱ�ȭ
void CFramework::ReleaseObjects()
{
	//�ĸ� ���ۿ� �׷����� ��� ����
	if (m_pScene)
	{
		//��鿡 ����� ������Ʈ�� ����
		m_pScene->ReleaseObjects();
		delete m_pScene;
	}
}

//�� �����ӿ��� ó���� �Է�ó��
void CFramework::ProcessInput()
{
	//Ű�Է� ����
	static UCHAR pKeyBuffer[256];
	//�������� ������
	static int nAlphaBlend = 255;

	//Ű �Է��� Ű�Է� ���ۿ� ����
	if (::GetKeyboardState(pKeyBuffer))
	{
		//ȭ��ǥ Ű�� ����,
		//���� ���α׷��� ���°� "�����"�� �� ����Ǵ� �������� Offset ��Ų��.
		if (pKeyBuffer[VK_LEFT] & 0xF0)
			m_pScene->PlayedFrameOffset(-5);
		if (pKeyBuffer[VK_RIGHT] & 0xF0)
			m_pScene->PlayedFrameOffset(4);

		//PageUp/PageDown Ű�� ����,
		//���� �������� �������� �����Ѵ�.
		if (pKeyBuffer[VK_PRIOR] & 0xF0)
		{
			//�������ϰ�
			if (nAlphaBlend < 255) nAlphaBlend++;
			SetWindowLong(m_hWnd, GWL_EXSTYLE, GetWindowLong(m_hWnd, GWL_EXSTYLE) | WS_EX_LAYERED);
			::SetLayeredWindowAttributes(m_hWnd, 0, nAlphaBlend, LWA_ALPHA);
		}
		if (pKeyBuffer[VK_NEXT] & 0xF0)
		{
			//�����ϰ�
			if (nAlphaBlend > 0) nAlphaBlend--;
			SetWindowLong(m_hWnd, GWL_EXSTYLE, GetWindowLong(m_hWnd, GWL_EXSTYLE) | WS_EX_LAYERED);
			::SetLayeredWindowAttributes(m_hWnd, 0, nAlphaBlend, LWA_ALPHA);
		}
	}
}

//������ ���콺 �޼��� ó��
void CFramework::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	if(m_pScene) m_pScene->OnProcessingMouseMessage(hWnd, nMessageID, wParam, lParam);
}

//������ Ű���� �޼��� ó��
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

//������ �޼��� ó��
LRESULT CFramework::OnProcessingWindowMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	//������ ���콺 �� Ű���� �Է� �޼��� ó��
	OnProcessingMouseMessage(hWnd, nMessageID, wParam, lParam);
	OnProcessingKeyboardMessage(hWnd, nMessageID, wParam, lParam);

	switch (nMessageID)
	{
	case WM_COMMAND:
	{
		int wmId = LOWORD(wParam);
		// �޴� ���ÿ� ���� ó��
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
