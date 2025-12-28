#pragma once

#include "CMControlBase.h"

// MFC Modern UI - 시간 선택 컴포넌트
// SOLID: 단일 책임 원칙 - 시간 선택만 담당

namespace MFCModernUI
{
    // 시간 변경 핸들러
    typedef void (*TimeChangedHandler)(void* context, int oldHour, int oldMinute, int newHour, int newMinute);

    // 시간 팝업 (내부 클래스)
    class CMTimePopup;

    class CMTimePicker : public CMControlBase
    {
        DECLARE_DYNAMIC(CMTimePicker)

    public:
        CMTimePicker();
        virtual ~CMTimePicker();

        // 생성
        BOOL Create(DWORD dwStyle, const CRect& rect, CWnd* pParentWnd, UINT nID);

        // 시간
        void SetTime(int hour, int minute);
        int GetHour() const { return m_hour; }
        int GetMinute() const { return m_minute; }

        // 24시간 형식
        void Set24HourFormat(BOOL use24Hour);
        BOOL Is24HourFormat() const { return m_use24Hour; }

        // 분 간격
        void SetMinuteStep(int step);
        int GetMinuteStep() const { return m_minuteStep; }

        // 초 표시
        void SetShowSeconds(BOOL show);
        BOOL GetShowSeconds() const { return m_showSeconds; }
        void SetSecond(int second);
        int GetSecond() const { return m_second; }

        // 이벤트 핸들러
        void SetTimeChangedHandler(TimeChangedHandler handler, void* context);

        // 드롭다운
        void ShowDropDown();
        void HideDropDown();
        BOOL IsDroppedDown() const;

    protected:
        virtual void OnDraw(CDC* pDC) override;

        afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
        afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
        afx_msg void OnKillFocus(CWnd* pNewWnd);

        DECLARE_MESSAGE_MAP()

    private:
        // 시간
        int m_hour;
        int m_minute;
        int m_second;

        // 옵션
        BOOL m_use24Hour;
        int m_minuteStep;
        BOOL m_showSeconds;

        // 드롭다운
        CMTimePopup* m_popup;

        // 이벤트
        TimeChangedHandler m_timeChangedHandler;
        void* m_timeChangedContext;

        // 상수
        static const int BUTTON_WIDTH = 24;

        // 헬퍼
        void DrawButton(CDC* pDC, const CRect& rect);
        CString FormatTime();
        void NotifyTimeChanged(int oldHour, int oldMinute);

        friend class CMTimePopup;
    };

    // 시간 선택 팝업
    class CMTimePopup : public CWnd
    {
    public:
        CMTimePopup(CMTimePicker* pOwner);
        virtual ~CMTimePopup();

        BOOL Create();
        void Show(CPoint pt);
        void Hide();

    protected:
        afx_msg void OnPaint();
        afx_msg BOOL OnEraseBkgnd(CDC* pDC);
        afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
        afx_msg void OnMouseMove(UINT nFlags, CPoint point);
        afx_msg void OnKillFocus(CWnd* pNewWnd);
        afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
        afx_msg void OnClose();

        DECLARE_MESSAGE_MAP()

    private:
        CMTimePicker* m_pOwner;

        int m_tempHour;
        int m_tempMinute;
        int m_tempSecond;

        int m_hoverColumn;  // 0=시, 1=분, 2=초, 3=AM/PM
        int m_hoverRow;     // -1=위 버튼, 0=값, 1=아래 버튼

        static const int COLUMN_WIDTH = 50;
        static const int ROW_HEIGHT = 32;
        static const int BUTTON_HEIGHT = 28;

        void OnDraw(CDC* pDC);
        void DrawColumn(CDC* pDC, int col, const CRect& rect);
        void DrawArrowButton(CDC* pDC, const CRect& rect, BOOL up, BOOL hovered);

        int HitTestColumn(CPoint point);
        int HitTestRow(CPoint point);
        void IncrementValue(int col);
        void DecrementValue(int col);
        void Apply();

        const ThemeColors& GetColors() const;
    };
}
