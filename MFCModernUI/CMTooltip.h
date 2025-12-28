#pragma once

#include "CMControlBase.h"

// MFC Modern UI - 툴팁 컴포넌트
// SOLID: 단일 책임 원칙 - 툴팁 표시만 담당

namespace MFCModernUI
{
    // 툴팁 위치
    enum class TooltipPosition
    {
        Auto,       // 자동 (공간에 따라)
        Top,
        Bottom,
        Left,
        Right
    };

    class CMTooltip : public CWnd
    {
        DECLARE_DYNAMIC(CMTooltip)

    public:
        CMTooltip();
        virtual ~CMTooltip();

        // 생성 (보통 애플리케이션에서 하나만 생성)
        BOOL Create(CWnd* pParentWnd);

        // 툴팁 등록/해제
        void AddTool(CWnd* pWnd, LPCTSTR lpszText);
        void AddTool(CWnd* pWnd, UINT nIDText);
        void DelTool(CWnd* pWnd);
        void UpdateTipText(CWnd* pWnd, LPCTSTR lpszText);

        // 표시 설정
        void SetPosition(TooltipPosition position);
        TooltipPosition GetPosition() const { return m_position; }

        void SetDelay(DWORD showDelay, DWORD hideDelay = 0);
        void SetMaxWidth(int maxWidth);

        // 수동 표시/숨김
        void Show(CWnd* pWnd, CPoint point);
        void Hide();

        // 마우스 이벤트 릴레이 (호출 필요)
        void RelayEvent(MSG* pMsg);

    protected:
        virtual void OnDraw(CDC* pDC);
        virtual void PreSubclassWindow();

        afx_msg void OnPaint();
        afx_msg void OnTimer(UINT_PTR nIDEvent);
        afx_msg BOOL OnEraseBkgnd(CDC* pDC);

        DECLARE_MESSAGE_MAP()

    private:
        // 툴 정보
        struct ToolInfo
        {
            CWnd* pWnd;
            CString text;
        };
        CArray<ToolInfo> m_tools;

        // 설정
        TooltipPosition m_position;
        DWORD m_showDelay;
        DWORD m_hideDelay;
        int m_maxWidth;

        // 상태
        BOOL m_visible;
        CWnd* m_currentTool;
        CPoint m_mousePos;
        CString m_currentText;

        // 타이머
        static const UINT_PTR SHOW_TIMER_ID = 3001;
        static const UINT_PTR HIDE_TIMER_ID = 3002;
        UINT_PTR m_showTimerId;
        UINT_PTR m_hideTimerId;

        // 테마
        BOOL IsLightTheme() const;
        const ThemeColors& GetColors() const;

        // 헬퍼
        ToolInfo* FindTool(CWnd* pWnd);
        CRect CalculateTooltipRect(const CString& text, CPoint anchor);
        CPoint CalculatePosition(CRect tooltipRect, CPoint anchor);
        void StartShowTimer();
        void StartHideTimer();
        void StopTimers();
    };
}
