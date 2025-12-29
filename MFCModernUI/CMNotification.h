#pragma once

#include "CMTypes.h"
#include "CMThemeManager.h"

// Direct2D
#include <d2d1.h>
#include <d2d1_1.h>
#include <dwrite.h>
#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwrite.lib")

// MFC Modern UI - 알림/토스트 컴포넌트
// SOLID: 단일 책임 원칙 - 알림 메시지 표시만 담당

namespace MFCModernUI
{
    // 알림 타입
    enum class NotificationType
    {
        Info,
        Success,
        Warning,
        Error
    };

    // 알림 위치
    enum class NotificationPosition
    {
        TopLeft,
        TopCenter,
        TopRight,
        BottomLeft,
        BottomCenter,
        BottomRight
    };

    // 알림 클릭 핸들러
    typedef void (*NotificationClickHandler)(void* context, int notificationId);
    typedef void (*NotificationCloseHandler)(void* context, int notificationId);

    // 단일 알림 창
    class CMNotificationWindow : public CWnd
    {
    public:
        CMNotificationWindow();
        virtual ~CMNotificationWindow();

        BOOL Create(CWnd* pParentWnd);

        void Show(int id, NotificationType type, LPCTSTR title, LPCTSTR message,
            DWORD duration, BOOL showClose);
        void Hide();

        int GetNotificationId() const { return m_id; }
        int GetHeight() const;

        void SetClickHandler(NotificationClickHandler handler, void* context);
        void SetCloseHandler(NotificationCloseHandler handler, void* context);

    protected:
        afx_msg void OnPaint();
        afx_msg BOOL OnEraseBkgnd(CDC* pDC);
        afx_msg void OnTimer(UINT_PTR nIDEvent);
        afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
        afx_msg void OnMouseMove(UINT nFlags, CPoint point);
        afx_msg void OnMouseLeave();

        DECLARE_MESSAGE_MAP()

    private:
        int m_id;
        NotificationType m_type;
        CString m_title;
        CString m_message;
        DWORD m_duration;
        BOOL m_showClose;
        BOOL m_isHovered;
        BOOL m_closeHovered;
        UINT_PTR m_timerId;

        static const UINT_PTR HIDE_TIMER_ID = 5001;
        static const int WIDTH = 320;
        static const int MIN_HEIGHT = 60;
        static const int ICON_SIZE = 24;
        static const int CLOSE_SIZE = 16;
        static const int PADDING = 12;

        NotificationClickHandler m_clickHandler;
        void* m_clickContext;
        NotificationCloseHandler m_closeHandler;
        void* m_closeContext;

        void OnDraw(CDC* pDC);
        void DrawIcon(CDC* pDC, const CRect& rect);
        void DrawCloseButton(CDC* pDC, const CRect& rect);
        CRect GetCloseButtonRect();
        COLORREF GetTypeColor() const;
        const ThemeColors& GetColors() const;

        // Direct2D
        void InitD2D();
        void ReleaseD2D();
        BOOL BeginD2DDraw();
        void EndD2DDraw();
        static D2D1_COLOR_F ToD2DColor(COLORREF color, float alpha = 1.0f);

        void OnDrawD2D();
        void DrawIconD2D(const CRect& rect);
        void DrawCloseButtonD2D(const CRect& rect);

        // Direct2D 리소스
        static ID2D1Factory* s_pD2DFactory;
        static IDWriteFactory* s_pDWriteFactory;
        static int s_d2dRefCount;

        ID2D1HwndRenderTarget* m_pRenderTarget;
        ID2D1SolidColorBrush* m_pSolidBrush;
        IDWriteTextFormat* m_pTextFormat;
        BOOL m_isD2DDrawing;
    };

    // 알림 매니저 (싱글톤)
    class CMNotificationManager
    {
    public:
        static CMNotificationManager& GetInstance();

        // 알림 표시
        int ShowNotification(NotificationType type, LPCTSTR title, LPCTSTR message,
            DWORD duration = 3000, BOOL showClose = TRUE);

        // 간편 메서드
        int ShowInfo(LPCTSTR message, DWORD duration = 3000);
        int ShowSuccess(LPCTSTR message, DWORD duration = 3000);
        int ShowWarning(LPCTSTR message, DWORD duration = 5000);
        int ShowError(LPCTSTR message, DWORD duration = 0);  // 0 = 수동 닫기

        // 알림 숨기기
        void HideNotification(int id);
        void HideAllNotifications();

        // 설정
        void SetPosition(NotificationPosition position);
        NotificationPosition GetPosition() const { return m_position; }

        void SetMaxVisible(int max);
        int GetMaxVisible() const { return m_maxVisible; }

        void SetSpacing(int spacing);
        int GetSpacing() const { return m_spacing; }

        // 이벤트 핸들러
        void SetClickHandler(NotificationClickHandler handler, void* context);
        void SetCloseHandler(NotificationCloseHandler handler, void* context);

    private:
        CMNotificationManager();
        ~CMNotificationManager();

        CMNotificationManager(const CMNotificationManager&) = delete;
        CMNotificationManager& operator=(const CMNotificationManager&) = delete;

        // 알림 창 풀
        CArray<CMNotificationWindow*> m_notifications;
        int m_nextId;

        // 설정
        NotificationPosition m_position;
        int m_maxVisible;
        int m_spacing;

        // 이벤트
        NotificationClickHandler m_clickHandler;
        void* m_clickContext;
        NotificationCloseHandler m_closeHandler;
        void* m_closeContext;

        // 헬퍼
        void RepositionNotifications();
        CMNotificationWindow* CreateNotificationWindow();
        void OnNotificationClosed(int id);

        static void StaticCloseHandler(void* context, int id);
    };
}
