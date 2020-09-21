#include "stdafx.h"
#include "Scene.h"


CScene::CScene()
{
}


CScene::~CScene()
{
}

// ��� ���μ��� ���� �ʱ�ȭ(��ȭ��, �����, �Ͻ����� ��)
void CScene::InitState()
{
	m_onMousePush = false;
	m_onREC = false;
	m_onRePlay = false;
	m_onPause = false;
}

// ������Ʈ ����(�ʱ�ȭ)
void CScene::BuildObjects()
{
	//������ ũ��
	LONG Wnd_Width = m_rcClient.right - m_rcClient.left;
	LONG Wnd_Height = m_rcClient.bottom - m_rcClient.top;

	//������ ũ���� n%��ŭ ������� ������ ����(����: 80%, ����: 10% - 10px)
	m_rcPlaybar.left = m_rcClient.left + Wnd_Width / 10;
	m_rcPlaybar.right = m_rcClient.right - Wnd_Width / 10;
	m_rcPlaybar.top = m_rcClient.bottom - Wnd_Height / 10 - 10;
	m_rcPlaybar.bottom = m_rcClient.bottom - 10;

	//������� ũ��
	m_nWidth_bar = m_rcPlaybar.right - m_rcPlaybar.left;

	//������ ũ���� n%��ŭ ����Ǿ����� ���� ���� �ʱ�ȭ
	m_rcPlayedbar.left = m_rcPlayedbar.right = m_rcPlaybar.left;
	m_rcPlayedbar.top = m_rcPlaybar.top;
	m_rcPlayedbar.bottom = m_rcPlaybar.bottom;
}

// ������Ʈ ����(Vector ����)
void CScene::ReleaseObjects()
{
	m_Rec_data.clear();
}

// ���� ����ü �ʱ�ȭ
void CScene::SetOpenFileStruct(HWND hWnd)
{
	memset(&m_OFN, 0, sizeof(OPENFILENAME));
	m_OFN.lStructSize = sizeof(OPENFILENAME);
	m_OFN.hwndOwner = hWnd;//���� ������ ��û�� ������ �ڵ�
	m_OFN.lpstrFilter = L"All Files(*.*)\0*.*\0";//���µ� ������ ����(��� ����)
	m_OFN.lpstrFile = m_szFileName;//���µ� ������ �̸�
	m_OFN.nMaxFile = m_nFileNameMaxLen;//���� �̸��� �ִ� ����
}

// ������ �ҷ�����
void CScene::LoadData()
{
	//������ ��ο� �̸��� �������� ����Ʈ ��Ʈ�� ����
	if (0 != GetOpenFileName(&m_OFN))
	{
		//����Ʈ ��Ʈ�ѷκ��� �����̸�(+���ϰ��) ���޹���
		SetWindowText(m_hEditFileToBeOpened, m_OFN.lpstrFile);

		//���޹��� �����̸����κ��� ���� ����
		ifstream file(m_OFN.lpstrFile, ios::in | ios::binary);

		//������ ���µ� ���
		if (file)
		{
			// ��� ���μ��� ���� �ʱ�ȭ(��ȭ��, �����, �Ͻ����� ��)
			InitState();
			// ��� ���μ��� ���¸� "�����"���� ����
			m_onRePlay = true;

			//����ũ�⸦ REC_DATA�� ������ ��ü REC_DATA�� ������ ����.
			file.seekg(0, ios::end);
			size_t Datacnt = static_cast<size_t>(file.tellg() / sizeof(REC_DATA));
			file.seekg(0, ios::beg);

			//������ ����Ǿ� �ִ� ������ �޸� ���� �� Vector ũ�� ����
			m_Rec_data.clear();
			m_Rec_data.resize(Datacnt);

			//���Ϸκ��� ������ �о�ͼ� Vector�� �����ϱ�
			file.read(reinterpret_cast<char*>(m_Rec_data.data()), Datacnt * sizeof(REC_DATA));

			//������ ������ �� Frame ��ȣ�� 0�� �����Ͱ� ������ ���
			//0�� �ƴ� �����ͱ��� ���� �����������͸� �ű�
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
			//Frame ��ȣ�� 0�̶�� ����
			//��ȭ�� �� ���� ������ �̹� �׷��� �ִ� ������ �ǹ��Ѵ�.
			//��ȭ�� �� �������� ����ϱ� ���ؼ���
			//Frame ��ȣ�� 0�� �ƴ� �������� �����ؾ� �Ѵ�.

			//������ ī��Ʈ ���� �ʱ�ȭ
			m_First_Frame = m_Played_Frame = m_FrameCnt = m_CurrentDATA->FrameCnt;
		}
	}
}

// Vector�� ����� �����͸� ���Ͽ� ����
void CScene::RecordData()
{
	//���� ����� ���°� "��ȭ��"�� �ƴ� ������ ó��
	if (!m_onREC && !m_onRePlay)
	{
		//���� ����� ���¸� "��ȭ��"���� ����
		m_onREC = true;
		//���� ����
		m_SaveFile.open("Record.bin", ios::out | ios::binary);
		//��������� �����͵��� Frame ��ȣ�� 0���� �ʱ�ȭ
		//(��ȭ�Ǳ� ������ ���� ��ȭ�Ǳ� ������ ������ �����ϱ� ����)
		for (auto& point : m_Rec_data)
			point.FrameCnt = 0;
		//���Ͽ� ������ ���
		m_SaveFile.write(reinterpret_cast<const char*>(m_Rec_data.data()), m_Rec_data.size() * sizeof(REC_DATA));
	}
}

// ���� ���� ����
void CScene::StopRecording()
{
	//���� ����� ���°� "��ȭ��"�� ������ ó��
	if (m_onREC && !m_onRePlay)
	{
		//���� ����� ���¸� "��ȭ��"���� ����
		m_onREC = false;
		m_SaveFile.close();
	}
}

// ��� �ʱ�ȭ(Vector ����, ���α׷� ���μ��� ���� �ʱ�ȭ, ������ ī��Ʈ ���� �ʱ�ȭ)
void CScene::ClearScene()
{
	ReleaseObjects();
	InitState();
	m_FrameCnt = m_Played_Frame = m_First_Frame = 0;
}

// ����� ���¸� "�Ͻ�����"�� ����
void CScene::SetPauseState()
{
	//���� ����� ���°� "��ȭ��"�� ������ ó��
	if (m_onRePlay) m_onPause = true;
}

// ���� ����� ���°� "�����"���� Ȯ��
bool CScene::CheckRePlayState()
{
	if (m_onRePlay && !m_onREC)
	{
		m_onPause = false;
		return true;
	}
	return false;
}

// ���� ����� ���°� "�����"�� �� ����Ǵ� �������� Offset
void CScene::PlayedFrameOffset(int n)
{
	if (m_onRePlay)
	{
		// Offset �Ǵ� �������� ���� ������ �ִ� ������ ���� �����ʰ� ����.
		if (m_Played_Frame + n < 0) m_Played_Frame = 0;
		else if (m_Played_Frame + n > m_Rec_data.back().FrameCnt) m_Played_Frame = m_Rec_data.back().FrameCnt;
		else m_Played_Frame = static_cast<unsigned int>(m_Played_Frame + n);
	}
}

// ������
void CScene::Render(HWND hWnd, HDC hDCFrameBuffer)
{
	// ���� ����� ���°� "�����"�� ��,
	if (m_onRePlay)
	{
		//���� ����Ǿ����� �ִ� ������ ��ȣ�� �ִ� ������ ��ȣ�� �Ѿ��� ��
		//���� �����������͸� �������� ���� �����͸� ����Ű�� ��
		if (m_Played_Frame >= m_Rec_data.back().FrameCnt)
		{
			m_CurrentDATA = --m_Rec_data.cend();
			--m_CurrentDATA;
			m_Played_Frame = m_CurrentDATA->FrameCnt;
		}
		else
		{
			//���� ����Ǿ����� �ִ� ������ ��ȣ�� ������ ������ ��ȣ�� ���� �����͸� �˻�
			//�� ��ġ�� ���� �����������Ͱ� ����Ű�� ��
			auto findData = find(m_Rec_data.cbegin(), m_Rec_data.cend(), m_Played_Frame);
			if (findData != m_Rec_data.cend()) m_CurrentDATA = findData;
		}

		//���� �������� �����ӹ�ȣ�� ������ �����ӹ�ȣ�� ���� �����Ͱ� ������ �� �ִ�.
		if (next(m_CurrentDATA, 1) != m_Rec_data.cend())
		{
			//���� �����������͸� �����ӹ�ȣ�� �������� ���� �������� ������ġ�� ����Ű�� ��
			while (m_Played_Frame == m_CurrentDATA->FrameCnt)
			{
				++m_CurrentDATA;

				//���� ����� ���°� "�����"�̸鼭 ���� �����������Ͱ� ������ �����͸� ����ų ��
				if (next(m_CurrentDATA, 1) == m_Rec_data.cend())
				{
					//����� ���¸� "�����"�� �ƴ� ���·� �����ϰ�
					//����ڿ��� ���÷��� ���θ� ����� �޼��� �ڽ��� ����Ѵ�.
					m_onRePlay = false;
					static bool bDisplay = false;
					if (!bDisplay)
					{
						bDisplay = true;
						if (MessageBox(hWnd, L"�ٽ� �÷��� �Ͻðڽ��ϱ�?", L"����", MB_YESNO) == IDYES)
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

		//���� �����������Ͱ� ����Ű�� �ִ� �����ͱ��� ��ȸ�ϸ鼭
		//�� �������� ���� �մ� ������ �׸���.
		auto DataEnd = next(m_CurrentDATA, 1);
		for (auto DrawStart = m_Rec_data.cbegin(); DrawStart != DataEnd; ++DrawStart)
		{
			auto Draw_End = next(DrawStart, 1);
			if (Draw_End == DataEnd) break;
			//���� ���� ���� �������̶��
			if (Draw_End->Start)
			{
				//���� �׸��� �������� ���� ������ �ٲٰ�
				DrawStart = Draw_End;
				//�׸��� ������ �ٴ��������� �ٲ۴�.
				++Draw_End;
			}
			if (Draw_End == DataEnd) break;
			//�� ���� �մ� ������ �׸���.
			MoveToEx(hDCFrameBuffer, DrawStart->x, DrawStart->y, NULL);
			LineTo(hDCFrameBuffer, Draw_End->x, Draw_End->y);
		}
		//���� ����� ���°� "�����"�� ��, ����Ǿ����� ������ ��ȣ�� 1���� �Ѵ�.
		if (!m_onPause && m_Played_Frame < m_Rec_data.back().FrameCnt)
			m_Played_Frame++;

		//����Ǿ����� ������ ��ȣ�� ����,
		//����Ǿ����� ������� ���� �ʺ� �����Ѵ�.
		//����Ǿ����� ������� ������ = ������� ���� + ����� ũ�� * (���� ������ ��ȣ / ��ü ����ؾ��� ������ ��)
		m_rcPlayedbar.right = m_rcPlayedbar.left + static_cast<LONG>(m_nWidth_bar * (static_cast<float>(m_Played_Frame - m_First_Frame) / (m_Rec_data.back().FrameCnt - m_First_Frame)));

		//����ٸ� �׸���.
		FillRect(hDCFrameBuffer, &m_rcPlaybar, (HBRUSH)GetStockObject(GRAY_BRUSH));
		FillRect(hDCFrameBuffer, &m_rcPlayedbar, (HBRUSH)GetStockObject(BLACK_BRUSH));
	}
	else
	{
		//���� �����������Ͱ� ����Ű�� �ִ� �����ͱ��� ��ȸ�ϸ鼭
		//�� �������� ���� �մ� ������ �׸���.
		auto DataEnd = m_Rec_data.cend();
		for (auto DrawStart = m_Rec_data.cbegin(); DrawStart != DataEnd; ++DrawStart)
		{
			auto Draw_End = next(DrawStart, 1);
			if (Draw_End == DataEnd) break;
			//���� ���� ���� �������̶��
			if (Draw_End->Start)
			{
				//���� �׸��� �������� ���� ������ �ٲٰ�
				DrawStart = Draw_End;
				//�׸��� ������ �ٴ��������� �ٲ۴�.
				++Draw_End;
			}
			if (Draw_End == DataEnd) break;
			MoveToEx(hDCFrameBuffer, DrawStart->x, DrawStart->y, NULL);
			LineTo(hDCFrameBuffer, Draw_End->x, Draw_End->y);
		}
	}

	//������ ��ȣ ����
	m_FrameCnt++;
}

// ������ ���콺 �޼��� ó��
void CScene::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	switch (nMessageID)
	{
	case WM_LBUTTONDOWN://���콺 ���ʹ�ư�� ���� ��,
		m_MousePos.x = LOWORD(lParam);
		m_MousePos.y = HIWORD(lParam);
		m_onMousePush = true;

		//���� ����� ���°� "�����"�� �ƴ� �� ó��
		if (!m_onRePlay)
		{
			//���� ���콺 ��ǥ�� Vector�� ����
			m_Rec_data.emplace_back(m_FrameCnt, m_MousePos.x, m_MousePos.y, true);
			//���� ����� ���°� "��ȭ��"�� ������,
			//Vector�� ���� �ֱٿ� ����� �����͸� ���Ͽ� ����Ѵ�.
			if (m_onREC)
				m_SaveFile.write(reinterpret_cast<const char*>(&m_Rec_data.back()), sizeof(REC_DATA));
		}
		//���� ����� ���°� "�����"�̸鼭,
		//���콺�� ��ǥ�� ����� ���ο� ��ġ�� ���
		//���콺�� x���� ���� ����Ǿ����� ������ ��ȣ�� �����Ѵ�.
		//����Ǿ����� ������� ������ = ������� ���� + ����� ũ�� * (���� ������ ��ȣ / ��ü ����ؾ��� ������ ��)
		//���콺 x�� = ������� ���� + ����� ũ�� * (���� ������ ��ȣ / ��ü ����ؾ��� ������ ��)
		//���콺 x�� - ������� ���� = ����� ũ�� * (���� ������ ��ȣ / ��ü ����ؾ��� ������ ��)
		//(���콺 x�� - ������� ����) / ����� ũ�� = ���� ������ ��ȣ / ��ü ����ؾ��� ������ ��
		//���� ������ ��ȣ = ��ü ����ؾ��� ������ �� * ((���콺 x�� - ������� ����) / ����� ũ��)
		else if (PointInRect(m_MousePos.x, m_MousePos.y, &m_rcPlaybar))
			m_Played_Frame = static_cast<unsigned int>(static_cast<float>(m_MousePos.x - m_rcPlayedbar.left) / m_nWidth_bar * (m_Rec_data.back().FrameCnt - m_First_Frame) + m_First_Frame);
		break;
	case WM_LBUTTONUP:
		m_onMousePush = false;
		break;
	case WM_MOUSEMOVE://���콺�� ������ ��
		m_MousePos.x = LOWORD(lParam);
		m_MousePos.y = HIWORD(lParam);
		//�巡�� �� ��,
		//���� ����� ���°� "�����"�� �ƴ� �� ó��
		if (m_onMousePush && !m_onRePlay)
		{
			//���� ���콺 ��ǥ�� Vector�� ����
			m_Rec_data.emplace_back(m_FrameCnt, m_MousePos.x, m_MousePos.y);
			//���� ����� ���°� "��ȭ��"�� ������,
			//Vector�� ���� �ֱٿ� ����� �����͸� ���Ͽ� ����Ѵ�.
			if (m_onREC)
				m_SaveFile.write(reinterpret_cast<const char*>(&m_Rec_data.back()), sizeof(REC_DATA));
		}
		//���� ����� ���°� "�����"�̸鼭,
		//���콺�� ��ǥ�� ����� ���ο� ��ġ�� ���
		//���콺�� x���� ���� ����Ǿ����� ������ ��ȣ�� �����Ѵ�.
		//���� ������ ��ȣ = ��ü ����ؾ��� ������ �� * ((���콺 x�� - ������� ����) / ����� ũ��)
		else if (m_onMousePush && m_onRePlay && PointInRect(m_MousePos.x, m_MousePos.y, &m_rcPlaybar))
			m_Played_Frame = static_cast<unsigned int>(static_cast<float>(m_MousePos.x - m_rcPlayedbar.left) / m_nWidth_bar * (m_Rec_data.back().FrameCnt - m_First_Frame) + m_First_Frame);
		break;
	default:
		break;
	}
}

// ������ Ű���� �޼��� ó��
void CScene::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
}