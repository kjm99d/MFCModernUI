#pragma once

#include "CMControlBase.h"

// MFC Modern UI - 슬라이더/트랙바
// SOLID: 단일 책임 원칙 - 값 선택만 담당

namespace MFCModernUI
{
    // 이벤트 핸들러
    typedef void (*SliderValueChangedHandler)(void* context, int oldValue, int newValue);
    typedef void (*SliderValueChangingHandler)(void* context, int value);

    // 방향
    enum class Orientation
    {
        Horizontal,
        Vertical
    };

    class CMSlider : public CMControlBase
    {
        DECLARE_DYNAMIC(CMSlider)

    public:
        CMSlider();
        virtual ~CMSlider();

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

        // 방향
        void SetOrientation(Orientation orientation);
        Orientation GetOrientation() const { return m_orientation; }

        // 눈금
        void SetShowTicks(BOOL show);
        BOOL GetShowTicks() const { return m_showTicks; }

        // 값 표시
        void SetShowValue(BOOL show);
        BOOL GetShowValue() const { return m_showValue; }

        // 이벤트 핸들러
        void SetValueChangedHandler(SliderValueChangedHandler handler, void* context);
        void SetValueChangingHandler(SliderValueChangingHandler handler, void* context);

    protected:
        virtual void OnDraw(CDC* pDC) override;

        afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
        afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
        afx_msg void OnMouseMove(UINT nFlags, CPoint point);
        afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
        afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);

        DECLARE_MESSAGE_MAP()

    private:
        // 값
        int m_value;
        int m_minValue;
        int m_maxValue;
        int m_step;

        // 옵션
        Orientation m_orientation;
        BOOL m_showTicks;
        BOOL m_showValue;

        // 드래그
        BOOL m_dragging;

        // 상수
        static const int TRACK_THICKNESS = 4;
        static const int THUMB_SIZE = 16;
        static const int THUMB_HOVER_SIZE = 20;

        // 이벤트 핸들러
        SliderValueChangedHandler m_valueChangedHandler;
        void* m_valueChangedContext;
        SliderValueChangingHandler m_valueChangingHandler;
        void* m_valueChangingContext;

        // 헬퍼
        void DrawTrack(CDC* pDC, const CRect& trackRect);
        void DrawThumb(CDC* pDC, const CRect& thumbRect);
        void DrawTicks(CDC* pDC, const CRect& trackRect);
        void DrawValueLabel(CDC* pDC, const CRect& thumbRect);

        CRect GetTrackRect();
        CRect GetThumbRect();
        int PositionToValue(int pos);
        int ValueToPosition(int value);
        void SetValueFromPosition(int pos);

        // Direct2D
        void DrawTrackD2D(const CRect& trackRect);
        void DrawThumbD2D(const CRect& thumbRect);
        void DrawTicksD2D(const CRect& trackRect);
        void DrawValueLabelD2D(const CRect& thumbRect);
    };
}
