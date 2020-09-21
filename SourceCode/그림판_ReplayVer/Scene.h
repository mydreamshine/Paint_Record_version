#pragma once

// 프로그램에서 다루는 데이터 타입
class REC_DATA
{
public:
	unsigned int FrameCnt;	// 리플레이에서 쓰일 프레임 카운트
	int x;					// 그리기에 쓰일 정점좌표
	int y;					// 그리기에 쓰일 정점좌표
	bool Start;
	REC_DATA(unsigned int nFrame, int nx, int ny, bool s = false) :FrameCnt{ nFrame }, x{ nx }, y{ ny }, Start{ s } {}
	REC_DATA() = default;
	~REC_DATA() = default;

	bool operator==(unsigned int nFrame) const { return FrameCnt == nFrame; } // Find()에서 쓰일 비교(==)연산
};

class CScene
{
public:
	RECT								m_rcClient;						 // 응용프로그램 전면버퍼 영역
private:
	RECT								m_rcPlaybar;					 // 전체재생바의 영역
	RECT								m_rcPlayedbar;					 // 재생된 바의 영역
	LONG								m_nWidth_bar;					 // 재생된 바의 크기

private:
	vector<REC_DATA>					m_Rec_data;						 // 데이터를 저장하고 관리 (그리기 및 파일 입/출력)
	vector<REC_DATA>::const_iterator	m_CurrentDATA;					 // Rec_data에 저장되어 있는 정점 중 현재 프레임 수와 동일한 정점을 가르키는 iterator(Find() 이용)

private:
	HWND								m_hEditFileToBeOpened = NULL;    // 파일의 경로와 이름을 가져오는 에디트 컨트롤의 핸들
	OPENFILENAME						m_OFN;							 // 파일열기 대화상자를 초기화하기 위한 변수
	const UINT							m_nFileNameMaxLen = 512;		 // szFileName 문자열의 최대 길이
	WCHAR								m_szFileName[512];				 // 파일의 경로 및 이름을 복사하기 위한 문자열

	ofstream						    m_SaveFile;						 // 데이터를 저장할 파일의 stream

private:
	POINT								m_MousePos;						 // 마우스 좌표
	bool								m_onMousePush = false;			 // 마우스 상태

	bool								m_onREC = false;				 // 프로그램 프로세싱 상태(기록하기)
	bool								m_onRePlay = false;				 // 프로그램 프로세싱 상태(불러오기 or 리플레이)
	bool								m_onPause = false;				 // 프로그램 프로세싱 상태(일시정지)

	unsigned int						m_FrameCnt = 0;					 // 프레임 카운트 변수
	unsigned int						m_Played_Frame = 0;				 // 프레임 카운트 변수
	unsigned int						m_First_Frame = 0;				 // 프레임 카운트 변수

public:
	CScene();
	virtual ~CScene();

	virtual void InitState();											 // 장면 프로세싱 상태 초기화(녹화중, 재생중, 일시정지 등)
	virtual void BuildObjects();										 // 오브젝트 생성
	virtual void ReleaseObjects();										 // 오브젝트 해제

	virtual void SetOpenFileStruct(HWND hWnd);							 // 파일 구조체 초기화
	virtual void LoadData();											 // 데이터 불러오기

	virtual void RecordData();											 // Vector에 저장된 데이터를 파일에 저장
	virtual void StopRecording();										 // 파일 저장 중지
	virtual void ClearScene();											 // 장면 초기화
	virtual void SetPauseState();										 // 장면의 상태를 일시정지로 변경
	virtual bool CheckRePlayState();									 // 현재 장면의 상태가 "재생중"인지 확인
	virtual void PlayedFrameOffset(int n);								 // 현재 장면의 상태가 "재생중"일 때 재생되는 프레임을 Offset

	virtual void Render(HWND hWnd, HDC hDCFrameBuffer);													 // 렌더링
	virtual void OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);	 // 윈도우 마우스 메세지 처리
	virtual void OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);	 // 윈도우 키보드 메세지 처리
};

//마우스 커서가 재생바 안에 위치하고 있는지를 검사하기 위한 충돌체크 함수
inline bool PointInRect(int nx, int ny, LPRECT lpRect)
{
	if (lpRect->left <= nx && nx <= lpRect->right)
	{
		if (lpRect->top <= ny && ny <= lpRect->bottom)
			return true;
	}
	return false;
}