#include "stdafx.h"
#include "Scene.h"


CScene::CScene()
{
}


CScene::~CScene()
{
}

// 장면 프로세싱 상태 초기화(녹화중, 재생중, 일시정지 등)
void CScene::InitState()
{
	m_onMousePush = false;
	m_onREC = false;
	m_onRePlay = false;
	m_onPause = false;
}

// 오브젝트 생성(초기화)
void CScene::BuildObjects()
{
	//윈도우 크기
	LONG Wnd_Width = m_rcClient.right - m_rcClient.left;
	LONG Wnd_Height = m_rcClient.bottom - m_rcClient.top;

	//윈도우 크기의 n%만큼 재생바의 영역을 지정(가로: 80%, 세로: 10% - 10px)
	m_rcPlaybar.left = m_rcClient.left + Wnd_Width / 10;
	m_rcPlaybar.right = m_rcClient.right - Wnd_Width / 10;
	m_rcPlaybar.top = m_rcClient.bottom - Wnd_Height / 10 - 10;
	m_rcPlaybar.bottom = m_rcClient.bottom - 10;

	//재생바의 크기
	m_nWidth_bar = m_rcPlaybar.right - m_rcPlaybar.left;

	//윈도우 크기의 n%만큼 재생되어지는 바의 영역 초기화
	m_rcPlayedbar.left = m_rcPlayedbar.right = m_rcPlaybar.left;
	m_rcPlayedbar.top = m_rcPlaybar.top;
	m_rcPlayedbar.bottom = m_rcPlaybar.bottom;
}

// 오브젝트 해제(Vector 비우기)
void CScene::ReleaseObjects()
{
	m_Rec_data.clear();
}

// 파일 구조체 초기화
void CScene::SetOpenFileStruct(HWND hWnd)
{
	memset(&m_OFN, 0, sizeof(OPENFILENAME));
	m_OFN.lStructSize = sizeof(OPENFILENAME);
	m_OFN.hwndOwner = hWnd;//파일 오픈을 요청한 윈도우 핸들
	m_OFN.lpstrFilter = L"All Files(*.*)\0*.*\0";//오픈될 파일의 포맷(모든 파일)
	m_OFN.lpstrFile = m_szFileName;//오픈될 파일의 이름
	m_OFN.nMaxFile = m_nFileNameMaxLen;//파일 이름의 최대 길이
}

// 데이터 불러오기
void CScene::LoadData()
{
	//파일의 경로와 이름을 가져오는 에디트 컨트롤 열기
	if (0 != GetOpenFileName(&m_OFN))
	{
		//에디트 컨트롤로부터 파일이름(+파일경로) 전달받음
		SetWindowText(m_hEditFileToBeOpened, m_OFN.lpstrFile);

		//전달받은 파일이름으로부터 파일 열기
		ifstream file(m_OFN.lpstrFile, ios::in | ios::binary);

		//파일이 오픈될 경우
		if (file)
		{
			// 장면 프로세싱 상태 초기화(녹화중, 재생중, 일시정지 등)
			InitState();
			// 장면 프로세싱 상태를 "재생중"으로 변경
			m_onRePlay = true;

			//파일크기를 REC_DATA로 나누어 전체 REC_DATA의 개수를 구함.
			file.seekg(0, ios::end);
			size_t Datacnt = static_cast<size_t>(file.tellg() / sizeof(REC_DATA));
			file.seekg(0, ios::beg);

			//기존에 저장되어 있던 데이터 메모리 해제 및 Vector 크기 변경
			m_Rec_data.clear();
			m_Rec_data.resize(Datacnt);

			//파일로부터 데이터 읽어와서 Vector에 저장하기
			file.read(reinterpret_cast<char*>(m_Rec_data.data()), Datacnt * sizeof(REC_DATA));

			//파일의 데이터 중 Frame 번호가 0인 데이터가 존재할 경우
			//0이 아닌 데이터까지 현재 데이터포인터를 옮김
			m_CurrentDATA = m_Rec_data.cbegin();
			if (m_CurrentDATA != m_Rec_data.cend())
			{
				while (m_CurrentDATA->FrameCnt == 0)
				{
					++m_CurrentDATA;
					if (m_CurrentDATA == m_Rec_data.cend())
					{
						m_onRePlay = false;
						break;
					}
				}
			}
			//Frame 번호가 0이라는 것은
			//녹화가 된 시점 이전에 이미 그려져 있던 점들을 의미한다.
			//녹화가 된 시점부터 재생하기 위해서는
			//Frame 번호가 0이 아닌 지점부터 시작해야 한다.

			//프레임 카운트 변수 초기화
			m_First_Frame = m_Played_Frame = m_FrameCnt = m_CurrentDATA->FrameCnt;
		}
	}
}

// Vector에 저장된 데이터를 파일에 저장
void CScene::RecordData()
{
	//현재 장면의 상태가 "녹화중"이 아닐 때에만 처리
	if (!m_onREC && !m_onRePlay)
	{
		//현재 장면의 상태를 "녹화중"으로 변경
		m_onREC = true;
		//파일 생성
		m_SaveFile.open("Record.bin", ios::out | ios::binary);
		//현재까지의 데이터들의 Frame 번호를 0으로 초기화
		//(녹화되기 이전의 점과 녹화되기 이후의 점으로 구분하기 위함)
		for (auto& point : m_Rec_data)
			point.FrameCnt = 0;
		//파일에 데이터 기록
		m_SaveFile.write(reinterpret_cast<const char*>(m_Rec_data.data()), m_Rec_data.size() * sizeof(REC_DATA));
	}
}

// 파일 저장 중지
void CScene::StopRecording()
{
	//현재 장면의 상태가 "녹화중"일 때에만 처리
	if (m_onREC && !m_onRePlay)
	{
		//현재 장면의 상태를 "녹화중"으로 변경
		m_onREC = false;
		m_SaveFile.close();
	}
}

// 장면 초기화(Vector 비우기, 프로그램 프로세싱 상태 초기화, 프레임 카운트 변수 초기화)
void CScene::ClearScene()
{
	ReleaseObjects();
	InitState();
	m_FrameCnt = m_Played_Frame = m_First_Frame = 0;
}

// 장면의 상태를 "일시정지"로 변경
void CScene::SetPauseState()
{
	//현재 장면의 상태가 "녹화중"일 때에만 처리
	if (m_onRePlay) m_onPause = true;
}

// 현재 장면의 상태가 "재생중"인지 확인
bool CScene::CheckRePlayState()
{
	if (m_onRePlay && !m_onREC)
	{
		m_onPause = false;
		return true;
	}
	return false;
}

// 현재 장면의 상태가 "재생중"일 때 재생되는 프레임을 Offset
void CScene::PlayedFrameOffset(int n)
{
	if (m_onRePlay)
	{
		// Offset 되는 프레임의 수가 음수나 최대 프레임 수를 넘지않게 조정.
		if (m_Played_Frame + n < 0) m_Played_Frame = 0;
		else if (m_Played_Frame + n > m_Rec_data.back().FrameCnt) m_Played_Frame = m_Rec_data.back().FrameCnt;
		else m_Played_Frame = static_cast<unsigned int>(m_Played_Frame + n);
	}
}

// 렌더링
void CScene::Render(HWND hWnd, HDC hDCFrameBuffer)
{
	// 현재 장면의 상태가 "재생중"일 때,
	if (m_onRePlay)
	{
		//현재 재생되어지고 있는 프레임 번호가 최대 프레임 번호를 넘었을 때
		//현재 데이터포인터를 마지막의 이전 데이터를 가르키기 함
		if (m_Played_Frame >= m_Rec_data.back().FrameCnt)
		{
			m_CurrentDATA = --m_Rec_data.cend();
			--m_CurrentDATA;
			m_Played_Frame = m_CurrentDATA->FrameCnt;
		}
		else
		{
			//현재 재생되어지고 있는 프레임 번호와 동일한 프레임 번호를 갖는 데이터를 검색
			//그 위치를 현재 데이터포인터가 가르키게 함
			auto findData = find(m_Rec_data.cbegin(), m_Rec_data.cend(), m_Played_Frame);
			if (findData != m_Rec_data.cend()) m_CurrentDATA = findData;
		}

		//현재 데이터의 프레임번호와 동일한 프레임번호를 갖는 데이터가 존재할 수 있다.
		if (next(m_CurrentDATA, 1) != m_Rec_data.cend())
		{
			//현재 데이터포인터를 프레임번호가 동일하지 않은 데이터의 이전위치로 가르키게 함
			while (m_Played_Frame == m_CurrentDATA->FrameCnt)
			{
				++m_CurrentDATA;

				//현재 장면의 상태가 "재생중"이면서 현재 데이터포인터가 마지막 데이터를 가르킬 때
				if (next(m_CurrentDATA, 1) == m_Rec_data.cend())
				{
					//장면의 상태를 "재생중"이 아닌 상태로 변경하고
					//사용자에게 리플레이 여부를 물어보는 메세지 박스를 출력한다.
					m_onRePlay = false;
					static bool bDisplay = false;
					if (!bDisplay)
					{
						bDisplay = true;
						if (MessageBox(hWnd, L"다시 플레이 하시겠습니까?", L"질문", MB_YESNO) == IDYES)
						{
							m_Played_Frame = m_First_Frame;
							m_onRePlay = true;
							m_onMousePush = false;
						}
						bDisplay = false;
					}
					break;
				}
			}
		}

		//현재 데이터포인터가 가르키고 있는 데이터까지 순회하면서
		//각 데이터의 점을 잇는 직선을 그린다.
		auto DataEnd = next(m_CurrentDATA, 1);
		for (auto DrawStart = m_Rec_data.cbegin(); DrawStart != DataEnd; ++DrawStart)
		{
			auto Draw_End = next(DrawStart, 1);
			if (Draw_End == DataEnd) break;
			//다음 점이 선의 시작점이라면
			if (Draw_End->Start)
			{
				//현재 그리기 시작점을 다음 점으로 바꾸고
				DrawStart = Draw_End;
				//그리기 끝점을 다다음점으로 바꾼다.
				++Draw_End;
			}
			if (Draw_End == DataEnd) break;
			//두 점을 잇는 직선을 그린다.
			MoveToEx(hDCFrameBuffer, DrawStart->x, DrawStart->y, NULL);
			LineTo(hDCFrameBuffer, Draw_End->x, Draw_End->y);
		}
		//현재 장면의 상태가 "재생중"일 때, 재생되어지는 프레임 번호를 1증가 한다.
		if (!m_onPause && m_Played_Frame < m_Rec_data.back().FrameCnt)
			m_Played_Frame++;

		//재생되어지는 프레임 번호에 따라,
		//재생되어지는 재생바의 영역 너비를 변경한다.
		//재생되어지는 재생바의 오른쪽 = 재생바의 왼쪽 + 재생바 크기 * (현재 프레임 번호 / 전체 재생해야할 프레임 수)
		m_rcPlayedbar.right = m_rcPlayedbar.left + static_cast<LONG>(m_nWidth_bar * (static_cast<float>(m_Played_Frame - m_First_Frame) / (m_Rec_data.back().FrameCnt - m_First_Frame)));

		//재생바를 그린다.
		FillRect(hDCFrameBuffer, &m_rcPlaybar, (HBRUSH)GetStockObject(GRAY_BRUSH));
		FillRect(hDCFrameBuffer, &m_rcPlayedbar, (HBRUSH)GetStockObject(BLACK_BRUSH));
	}
	else
	{
		//현재 데이터포인터가 가르키고 있는 데이터까지 순회하면서
		//각 데이터의 점을 잇는 직선을 그린다.
		auto DataEnd = m_Rec_data.cend();
		for (auto DrawStart = m_Rec_data.cbegin(); DrawStart != DataEnd; ++DrawStart)
		{
			auto Draw_End = next(DrawStart, 1);
			if (Draw_End == DataEnd) break;
			//다음 점이 선의 시작점이라면
			if (Draw_End->Start)
			{
				//현재 그리기 시작점을 다음 점으로 바꾸고
				DrawStart = Draw_End;
				//그리기 끝점을 다다음점으로 바꾼다.
				++Draw_End;
			}
			if (Draw_End == DataEnd) break;
			MoveToEx(hDCFrameBuffer, DrawStart->x, DrawStart->y, NULL);
			LineTo(hDCFrameBuffer, Draw_End->x, Draw_End->y);
		}
	}

	//프레임 번호 증가
	m_FrameCnt++;
}

// 윈도우 마우스 메세지 처리
void CScene::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	switch (nMessageID)
	{
	case WM_LBUTTONDOWN://마우스 왼쪽버튼을 누를 때,
		m_MousePos.x = LOWORD(lParam);
		m_MousePos.y = HIWORD(lParam);
		m_onMousePush = true;

		//현재 장면의 상태가 "재생중"이 아닐 때 처리
		if (!m_onRePlay)
		{
			//현재 마우스 좌표를 Vector에 저장
			m_Rec_data.emplace_back(m_FrameCnt, m_MousePos.x, m_MousePos.y, true);
			//현재 장면의 상태가 "녹화중"일 때에는,
			//Vector에 가장 최근에 저장된 데이터를 파일에 기록한다.
			if (m_onREC)
				m_SaveFile.write(reinterpret_cast<const char*>(&m_Rec_data.back()), sizeof(REC_DATA));
		}
		//현재 장면의 상태가 "재생중"이면서,
		//마우스의 좌표가 재생바 내부에 위치할 경우
		//마우스의 x값에 따라 재생되어지는 프레임 번호를 변경한다.
		//재생되어지는 재생바의 오른쪽 = 재생바의 왼쪽 + 재생바 크기 * (현재 프레임 번호 / 전체 재생해야할 프레임 수)
		//마우스 x값 = 재생바의 왼쪽 + 재생바 크기 * (현재 프레임 번호 / 전체 재생해야할 프레임 수)
		//마우스 x값 - 재생바의 왼쪽 = 재생바 크기 * (현재 프레임 번호 / 전체 재생해야할 프레임 수)
		//(마우스 x값 - 재생바의 왼쪽) / 재생바 크기 = 현재 프레임 번호 / 전체 재생해야할 프레임 수
		//현재 프레임 번호 = 전체 재생해야할 프레임 수 * ((마우스 x값 - 재생바의 왼쪽) / 재생바 크기)
		else if (PointInRect(m_MousePos.x, m_MousePos.y, &m_rcPlaybar))
			m_Played_Frame = static_cast<unsigned int>(static_cast<float>(m_MousePos.x - m_rcPlayedbar.left) / m_nWidth_bar * (m_Rec_data.back().FrameCnt - m_First_Frame) + m_First_Frame);
		break;
	case WM_LBUTTONUP:
		m_onMousePush = false;
		break;
	case WM_MOUSEMOVE://마우스를 움직일 때
		m_MousePos.x = LOWORD(lParam);
		m_MousePos.y = HIWORD(lParam);
		//드래그 할 때,
		//현재 장면의 상태가 "재생중"이 아닐 때 처리
		if (m_onMousePush && !m_onRePlay)
		{
			//현재 마우스 좌표를 Vector에 저장
			m_Rec_data.emplace_back(m_FrameCnt, m_MousePos.x, m_MousePos.y);
			//현재 장면의 상태가 "녹화중"일 때에는,
			//Vector에 가장 최근에 저장된 데이터를 파일에 기록한다.
			if (m_onREC)
				m_SaveFile.write(reinterpret_cast<const char*>(&m_Rec_data.back()), sizeof(REC_DATA));
		}
		//현재 장면의 상태가 "재생중"이면서,
		//마우스의 좌표가 재생바 내부에 위치할 경우
		//마우스의 x값에 따라 재생되어지는 프레임 번호를 변경한다.
		//현재 프레임 번호 = 전체 재생해야할 프레임 수 * ((마우스 x값 - 재생바의 왼쪽) / 재생바 크기)
		else if (m_onMousePush && m_onRePlay && PointInRect(m_MousePos.x, m_MousePos.y, &m_rcPlaybar))
			m_Played_Frame = static_cast<unsigned int>(static_cast<float>(m_MousePos.x - m_rcPlayedbar.left) / m_nWidth_bar * (m_Rec_data.back().FrameCnt - m_First_Frame) + m_First_Frame);
		break;
	default:
		break;
	}
}

// 윈도우 키보드 메세지 처리
void CScene::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
}