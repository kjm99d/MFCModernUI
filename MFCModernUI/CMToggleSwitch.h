#pragma once

#include "CMControlBase.h"

// MFC Modern UI - iOS 스타일 토글 스위치
// SOLID: 단일 책임 원칙 - 토글 동작만 담당

namespace MFCModernUI
{
    // 토글 변경 이벤트 핸들러
    typedef void (*ToggleChangedHandler)(void* context, BOOL checked);

    class CMToggleSwitch : public CMControlBase
    {
        DECLARE_DYNAMIC(CMToggleSwitch)

    public:
        CMToggleSwitch();
        virtual ~CMToggleSwitch();

        // 생성
        BOOL Create(DWORD dwStyle, const CRect& rect, CWnd* pParentWnd, UINT nID);

        // 체크 상태
        void SetChecked(BOOL checked);
        BOOL IsChecked() const { return m_checked; }

        // 텍스트
        void SetText(const CString& text);
        const CString& GetText() const { return m_text; }

        // On/Off 텍스트
        void SetOnText(const CString& text);
        void SetOffText(const CString& text);
        const CString& GetOnText() const { return m_onText; }
        const CString& GetOffText() const { return m_offText; }

        // 토글
        void Toggle();

        // 이벤트 핸들러
        void SetToggleChangedHandler(ToggleChangedHandler handler, void* context);

    protected:
        virtual void OnDraw(CDC* pDC) override;

        afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
        afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);

        DECLARE_MESSAGE_MAP()

    private:
        // 속성
        BOOL m_checked;
        CString m_text;
        CString m_onText;
        CString m_offText;

        // 애니메이션
        float m_knobPosition;  // 0.0 (Off) ~ 1.0 (On)

        // 이벤트 핸들러
        ToggleChangedHandler m_toggleChangedHandler;
        void* m_toggleChangedContext;

        // 크기 상수
        static const int TRACK_WIDTH = 44;
        static const int TRACK_HEIGHT = 24;
        static const int KNOB_SIZE = 20;
        static const int KNOB_MARGIN = 2;

        // 헬퍼
        void StartKnobAnimation();
        void DrawTrack(CDC* pDC, const CRect& trackRect);
        void DrawKnob(CDC* pDC, const CRect& trackRect);

        // Direct2D
        void DrawTrackD2D(const CRect& trackRect);
        void DrawKnobD2D(const CRect& trackRect);
    };
}
