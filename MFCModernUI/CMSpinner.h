#pragma once

#include "CMControlBase.h"

// MFC Modern UI - 숫자 스피너 컴포넌트
// SOLID: 단일 책임 원칙 - 숫자 입력/조절만 담당

namespace MFCModernUI
{
    // 이벤트 핸들러
    typedef void (*SpinnerValueChangedHandler)(void* context, int oldValue, int newValue);

    class CMSpinner : public CMControlBase
    {
        DECLARE_DYNAMIC(CMSpinner)

    public:
        CMSpinner();
        virtual ~CMSpinner();

        // 생성
        BOOL Create(DWORD dwStyle, const CRect& rect, CWnd* pParentWnd, UINT nID);

        // 값
        void SetValue(int value);
        int GetValue() const { return m_value; }

        void SetRange(int minValue, int maxValue);
        int GetMin() const { return m_minValue; }
        int GetMax() const { return m_maxValue; }

        void SetStep(int step);
        int GetStep() const { return m_step; }

        // 순환 모드 (max 넘으면 min으로)
        void SetWrap(BOOL wrap);
        BOOL GetWrap() const { return m_wrap; }

        // 읽기 전용 (버튼만 사용)
        void SetReadOnly(BOOL readOnly);
        BOOL IsReadOnly() const { return m_readOnly; }

        // 이벤트 핸들러
        void SetValueChangedHandler(SpinnerValueChangedHandler handler, void* context);

    protected:
        virtual void OnDraw(CDC* pDC) override;

        afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
        afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
        afx_msg void OnMouseMove(UINT nFlags, CPoint point);
        afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
        afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
        afx_msg void OnTimer(UINT_PTR nIDEvent);
        afx_msg UINT OnGetDlgCode();

        DECLARE_MESSAGE_MAP()

    private:
        // 값
        int m_value;
        int m_minValue;
        int m_maxValue;
        int m_step;

        // 옵션
        BOOL m_wrap;
        BOOL m_readOnly;

        // 버튼 상태
        enum class ButtonArea { None, Up, Down, Input };
        ButtonArea m_hoverArea;
        ButtonArea m_pressedArea;

        // 연속 증감 타이머
        UINT_PTR m_repeatTimerId;
        static const UINT_PTR REPEAT_TIMER_ID = 2001;
        static const DWORD REPEAT_DELAY = 400;
        static const DWORD REPEAT_INTERVAL = 50;
        BOOL m_isFirstRepeat;

        // 이벤트 핸들러
        SpinnerValueChangedHandler m_valueChangedHandler;
        void* m_valueChangedContext;

        // 상수
        static const int BUTTON_WIDTH = 24;

        // 헬퍼
        void DrawInputArea(CDC* pDC, const CRect& rect);
        void DrawUpButton(CDC* pDC, const CRect& rect);
        void DrawDownButton(CDC* pDC, const CRect& rect);
        void DrawArrow(CDC* pDC, const CRect& rect, BOOL up, COLORREF color);

        CRect GetInputRect();
        CRect GetUpButtonRect();
        CRect GetDownButtonRect();
        ButtonArea HitTest(CPoint point);

        void Increment();
        void Decrement();
        void StartRepeatTimer();
        void StopRepeatTimer();

        // Direct2D
        void OnDrawGdiPlus(CDC* pDC);
        void DrawInputAreaD2D(const CRect& rect);
        void DrawUpButtonD2D(const CRect& rect);
        void DrawDownButtonD2D(const CRect& rect);
        void DrawArrowD2D(const CRect& rect, BOOL up, COLORREF color);
    };
}
