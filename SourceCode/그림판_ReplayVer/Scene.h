#pragma once

// ���α׷����� �ٷ�� ������ Ÿ��
class REC_DATA
{
public:
	unsigned int FrameCnt;	// ���÷��̿��� ���� ������ ī��Ʈ
	int x;					// �׸��⿡ ���� ������ǥ
	int y;					// �׸��⿡ ���� ������ǥ
	bool Start;
	REC_DATA(unsigned int nFrame, int nx, int ny, bool s = false) :FrameCnt{ nFrame }, x{ nx }, y{ ny }, Start{ s } {}
	REC_DATA() = default;
	~REC_DATA() = default;

	bool operator==(unsigned int nFrame) const { return FrameCnt == nFrame; } // Find()���� ���� ��(==)����
};

class CScene
{
public:
	RECT								m_rcClient;						 // �������α׷� ������� ����
private:
	RECT								m_rcPlaybar;					 // ��ü������� ����
	RECT								m_rcPlayedbar;					 // ����� ���� ����
	LONG								m_nWidth_bar;					 // ����� ���� ũ��

private:
	vector<REC_DATA>					m_Rec_data;						 // �����͸� �����ϰ� ���� (�׸��� �� ���� ��/���)
	vector<REC_DATA>::const_iterator	m_CurrentDATA;					 // Rec_data�� ����Ǿ� �ִ� ���� �� ���� ������ ���� ������ ������ ����Ű�� iterator(Find() �̿�)

private:
	HWND								m_hEditFileToBeOpened = NULL;    // ������ ��ο� �̸��� �������� ����Ʈ ��Ʈ���� �ڵ�
	OPENFILENAME						m_OFN;							 // ���Ͽ��� ��ȭ���ڸ� �ʱ�ȭ�ϱ� ���� ����
	const UINT							m_nFileNameMaxLen = 512;		 // szFileName ���ڿ��� �ִ� ����
	WCHAR								m_szFileName[512];				 // ������ ��� �� �̸��� �����ϱ� ���� ���ڿ�

	ofstream						    m_SaveFile;						 // �����͸� ������ ������ stream

private:
	POINT								m_MousePos;						 // ���콺 ��ǥ
	bool								m_onMousePush = false;			 // ���콺 ����

	bool								m_onREC = false;				 // ���α׷� ���μ��� ����(����ϱ�)
	bool								m_onRePlay = false;				 // ���α׷� ���μ��� ����(�ҷ����� or ���÷���)
	bool								m_onPause = false;				 // ���α׷� ���μ��� ����(�Ͻ�����)

	unsigned int						m_FrameCnt = 0;					 // ������ ī��Ʈ ����
	unsigned int						m_Played_Frame = 0;				 // ������ ī��Ʈ ����
	unsigned int						m_First_Frame = 0;				 // ������ ī��Ʈ ����

public:
	CScene();
	virtual ~CScene();

	virtual void InitState();											 // ��� ���μ��� ���� �ʱ�ȭ(��ȭ��, �����, �Ͻ����� ��)
	virtual void BuildObjects();										 // ������Ʈ ����
	virtual void ReleaseObjects();										 // ������Ʈ ����

	virtual void SetOpenFileStruct(HWND hWnd);							 // ���� ����ü �ʱ�ȭ
	virtual void LoadData();											 // ������ �ҷ�����

	virtual void RecordData();											 // Vector�� ����� �����͸� ���Ͽ� ����
	virtual void StopRecording();										 // ���� ���� ����
	virtual void ClearScene();											 // ��� �ʱ�ȭ
	virtual void SetPauseState();										 // ����� ���¸� �Ͻ������� ����
	virtual bool CheckRePlayState();									 // ���� ����� ���°� "�����"���� Ȯ��
	virtual void PlayedFrameOffset(int n);								 // ���� ����� ���°� "�����"�� �� ����Ǵ� �������� Offset

	virtual void Render(HWND hWnd, HDC hDCFrameBuffer);													 // ������
	virtual void OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);	 // ������ ���콺 �޼��� ó��
	virtual void OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);	 // ������ Ű���� �޼��� ó��
};

//���콺 Ŀ���� ����� �ȿ� ��ġ�ϰ� �ִ����� �˻��ϱ� ���� �浹üũ �Լ�
inline bool PointInRect(int nx, int ny, LPRECT lpRect)
{
	if (lpRect->left <= nx && nx <= lpRect->right)
	{
		if (lpRect->top <= ny && ny <= lpRect->bottom)
			return true;
	}
	return false;
}