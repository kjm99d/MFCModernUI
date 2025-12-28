#pragma once

#include "CMControlBase.h"

// MFC Modern UI - 진행률 바 컴포넌트
// SOLID: 단일 책임 원칙 - 진행률 표시만 담당

namespace MFCModernUI
{
    class CMProgressBar : public CMControlBase
    {
        DECLARE_DYNAMIC(CMProgressBar)

    public:
        CMProgressBar();
        virtual ~CMProgressBar();

        // 생성
        BOOL Create(DWORD dwStyle, const CRect& rect, CWnd* pParentWnd, UINT nID = 0);

        // 값
        void SetValue(int value);
        int GetValue() const { return m_value; }

        void SetRange(int minValue, int maxValue);
        int GetMin() const { return m_minValue; }
        int GetMax() const { return m_maxValue; }

        // 불확정 모드
        void SetIndeterminate(BOOL indeterminate);
        BOOL IsIndeterminate() const { return m_indeterminate; }

        // 퍼센트 텍스트 표시
        void SetShowText(BOOL showText);
        BOOL IsShowText() const { return m_showText; }

        // 바 색상
        void SetBarColor(COLORREF color);
        void UseDefaultBarColor();

        // 트랙 색상
        void SetTrackColor(COLORREF color);
        void UseDefaultTrackColor();

    protected:
        virtual void OnDraw(CDC* pDC) override;

        afx_msg void OnTimer(UINT_PTR nIDEvent);
        afx_msg void OnDestroy();

        DECLARE_MESSAGE_MAP()

    private:
        // 값
        int m_value;
        int m_minValue;
        int m_maxValue;

        // 옵션
        BOOL m_indeterminate;
        BOOL m_showText;

        // 색상
        COLORREF m_barColor;
        BOOL m_useDefaultBarColor;
        COLORREF m_trackColor;
        BOOL m_useDefaultTrackColor;

        // 애니메이션
        float m_displayValue;  // 표시용 값 (애니메이션)
        float m_indeterminatePosition;  // 불확정 모드 위치 (0.0 ~ 1.0)
        UINT_PTR m_indeterminateTimerId;

        // 헬퍼
        void StartValueAnimation(float targetValue);
        float GetProgress() const;
    };
}
