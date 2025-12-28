#pragma once

#include "CMControlBase.h"

// MFC Modern UI - 날짜 선택 컴포넌트
// SOLID: 단일 책임 원칙 - 날짜 선택만 담당

namespace MFCModernUI
{
    // 날짜 변경 핸들러
    typedef void (*DateChangedHandler)(void* context, const COleDateTime& oldDate, const COleDateTime& newDate);

    // 드롭다운 캘린더 (내부 클래스)
    class CMCalendarPopup;

    class CMDatePicker : public CMControlBase
    {
        DECLARE_DYNAMIC(CMDatePicker)

    public:
        CMDatePicker();
        virtual ~CMDatePicker();

        // 생성
        BOOL Create(DWORD dwStyle, const CRect& rect, CWnd* pParentWnd, UINT nID);

        // 날짜
        void SetDate(const COleDateTime& date);
        COleDateTime GetDate() const { return m_date; }

        void SetMinDate(const COleDateTime& date);
        void SetMaxDate(const COleDateTime& date);

        // 포맷
        void SetFormat(LPCTSTR format);
        CString GetFormat() const { return m_format; }

        // 오늘 버튼
        void SetShowTodayButton(BOOL show);
        BOOL GetShowTodayButton() const { return m_showTodayButton; }

        // 주 시작 요일 (0=일요일, 1=월요일)
        void SetFirstDayOfWeek(int day);
        int GetFirstDayOfWeek() const { return m_firstDayOfWeek; }

        // 이벤트 핸들러
        void SetDateChangedHandler(DateChangedHandler handler, void* context);

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
        // 날짜
        COleDateTime m_date;
        COleDateTime m_minDate;
        COleDateTime m_maxDate;

        // 옵션
        CString m_format;
        BOOL m_showTodayButton;
        int m_firstDayOfWeek;

        // 드롭다운
        CMCalendarPopup* m_popup;

        // 이벤트
        DateChangedHandler m_dateChangedHandler;
        void* m_dateChangedContext;

        // 상수
        static const int BUTTON_WIDTH = 24;

        // 헬퍼
        void DrawButton(CDC* pDC, const CRect& rect);
        CString FormatDate(const COleDateTime& date);
        void NotifyDateChanged(const COleDateTime& oldDate);

        friend class CMCalendarPopup;
    };

    // 캘린더 팝업 창
    class CMCalendarPopup : public CWnd
    {
    public:
        CMCalendarPopup(CMDatePicker* pOwner);
        virtual ~CMCalendarPopup();

        BOOL Create();
        void Show(CPoint pt);
        void Hide();

    protected:
        afx_msg void OnPaint();
        afx_msg BOOL OnEraseBkgnd(CDC* pDC);
        afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
        afx_msg void OnMouseMove(UINT nFlags, CPoint point);
        afx_msg void OnKillFocus(CWnd* pNewWnd);
        afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);

        DECLARE_MESSAGE_MAP()

    private:
        CMDatePicker* m_pOwner;
        COleDateTime m_viewDate;  // 현재 보여지는 월
        int m_hoverDay;
        int m_hoverMonth;  // -1: 이전, 0: 현재, 1: 다음

        static const int CELL_WIDTH = 32;
        static const int CELL_HEIGHT = 28;
        static const int HEADER_HEIGHT = 36;
        static const int WEEKDAY_HEIGHT = 24;
        static const int FOOTER_HEIGHT = 32;

        void OnDraw(CDC* pDC);
        void DrawHeader(CDC* pDC, const CRect& rect);
        void DrawWeekDays(CDC* pDC, const CRect& rect);
        void DrawDays(CDC* pDC, const CRect& rect);
        void DrawFooter(CDC* pDC, const CRect& rect);

        int HitTestDay(CPoint point);
        CRect GetDayRect(int day);
        int GetDaysInMonth(int year, int month);
        int GetFirstDayOfMonth(int year, int month);
        BOOL IsDateValid(const COleDateTime& date);

        const ThemeColors& GetColors() const;
    };
}
