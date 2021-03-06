#pragma once
#include "Scene.h"

void CALLBACK TimeProc(HWND hWnd, UINT uMsg, UINT idEvent, DWORD dwTime);

class CFramework
{
public:
	CFramework();
	~CFramework();

	bool OnCreate(HINSTANCE hInstance, HWND hMainWnd);		    //윈도우 핸들 지정 및 버퍼, 오브젝트 초기화
	void OnDestroy();										    //윈도우 및 버퍼, 오브젝트 메모리 해제

	void FrameAdvance();										//프레임 갱신

	void SetActive(bool bActive) { m_bActive = bActive; }		//응용프로그램 활성화 상태 지정

private:
	HINSTANCE					m_hInstance = NULL;				//응용프로그램 윈도우 객체 핸들
	HWND						m_hWnd = NULL;					//응용프로그램 메인 윈도우 핸들
	
	bool						m_bActive = true;				//응용프로그램 활성화 상태
	
	HDC							m_hDCFrameBuffer = NULL;		//후면버퍼 핸들
	HBITMAP						m_hBitmapFrameBuffer = NULL;	//후면버퍼 텍스쳐 핸들
	
	CScene		   			   *m_pScene = nullptr;				//후면버퍼에 렌더링할 장면

public:
	void BuildFrameBuffer();									//후면버퍼 생성
	void ClearFrameBuffer(DWORD dwColor);						//후면버퍼 초기화
	void PresentFrameBuffer();									//후면버퍼와 전면버퍼 스왑

	void BuildObjects();										//프로그램에 쓰일 객체 및 변수를 초기화
	void ReleaseObjects();										//프로그램에 쓰였던 객체 및 변수를 초기화
	void ProcessInput();										//한 프레임에서 처리할 입력처리

	void OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);	  //윈도우 마우스 메세지 처리
	void OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);	  //윈도우 키보드 메세지 처리
	LRESULT OnProcessingWindowMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);  //윈도우 메세지 처리
};
