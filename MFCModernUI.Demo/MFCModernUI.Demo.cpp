// MFCModernUI.Demo.cpp : MFC Modern UI Library Demo Application

#include "pch.h"
#include "framework.h"
#include "MFCModernUI.Demo.h"
#include "DemoMainFrame.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// Demo Application Class
class CDemoApp : public CWinApp
{
public:
    CDemoApp() {}

    virtual BOOL InitInstance() override
    {
        // MFC 초기화
        CWinApp::InitInstance();

        // 공용 컨트롤 초기화 (Windows XP 이상)
        INITCOMMONCONTROLSEX icc;
        icc.dwSize = sizeof(icc);
        icc.dwICC = ICC_WIN95_CLASSES | ICC_COOL_CLASSES | ICC_DATE_CLASSES;
        InitCommonControlsEx(&icc);

        // GDI+ 초기화
        Gdiplus::GdiplusStartupInput gdiplusStartupInput;
        Gdiplus::GdiplusStartup(&m_gdiplusToken, &gdiplusStartupInput, NULL);

        // 메인 프레임 생성
        CDemoMainFrame* pMainFrame = new CDemoMainFrame();
        if (!pMainFrame->Create())
        {
            delete pMainFrame;
            return FALSE;
        }

        m_pMainWnd = pMainFrame;

        // 윈도우 표시
        pMainFrame->ShowWindow(SW_SHOW);
        pMainFrame->UpdateWindow();

        return TRUE;
    }

    virtual int ExitInstance() override
    {
        // GDI+ 정리
        Gdiplus::GdiplusShutdown(m_gdiplusToken);

        return CWinApp::ExitInstance();
    }

private:
    ULONG_PTR m_gdiplusToken;
};

// 전역 애플리케이션 객체
CDemoApp theApp;
