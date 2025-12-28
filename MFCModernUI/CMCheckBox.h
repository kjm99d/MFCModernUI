#pragma once

#include "CMControlBase.h"

// MFC Modern UI - 체크박스 컴포넌트
// SOLID: 단일 책임 원칙 - 체크박스 동작만 담당

namespace MFCModernUI
{
    // 체크 변경 이벤트 핸들러
    typedef void (*CheckChangedHandler)(void* context, BOOL checked);

    class CMCheckBox : public CMControlBase
    {
        DECLARE_DYNAMIC(CMCheckBox)

    public:
        CMCheckBox();
        virtual ~CMCheckBox();

        // 생성
        BOOL Create(const CString& text, DWORD dwStyle, const CRect& rect,
            CWnd* pParentWnd, UINT nID);

        // 체크 상태
        void SetChecked(BOOL checked);
        BOOL IsChecked() const { return m_checked; }

        // 불확정 상태
        void SetIndeterminate(BOOL indeterminate);
        BOOL IsIndeterminate() const { return m_indeterminate; }

        // 텍스트
        void SetText(const CString& text);
        const CString& GetText() const { return m_text; }

        // 텍스트 위치
        void SetTextPosition(Position position);
        Position GetTextPosition() const { return m_textPosition; }

        // 이벤트 핸들러
        void SetCheckedChangedHandler(CheckChangedHandler handler, void* context);

        // 토글
        void Toggle();

    protected:
        virtual void OnDraw(CDC* pDC) override;
        virtual void SetState(ControlState newState) override;

        // 메시지 핸들러
        afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
        afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);

        DECLARE_MESSAGE_MAP()

    private:
        // 속성
        BOOL m_checked;
        BOOL m_indeterminate;
        CString m_text;
        Position m_textPosition;

        // 애니메이션
        float m_checkProgress;  // 0.0 ~ 1.0, 체크마크 그리기용

        // 이벤트 핸들러
        CheckChangedHandler m_checkedChangedHandler;
        void* m_checkedChangedContext;

        // 헬퍼
        void DrawCheckBox(CDC* pDC, const CRect& boxRect);
        void DrawCheckMark(CDC* pDC, const CRect& boxRect);
        void DrawIndeterminateMark(CDC* pDC, const CRect& boxRect);
        void StartCheckAnimation();
    };
}
