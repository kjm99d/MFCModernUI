#pragma once

#include "CMControlBase.h"

// MFC Modern UI - 모던 버튼 컴포넌트
// SOLID: 리스코프 치환 원칙 - CMControlBase를 대체 가능
// SOLID: 단일 책임 원칙 - 버튼 동작만 담당

namespace MFCModernUI
{
    // 버튼 클릭 이벤트 핸들러
    typedef void (*ButtonClickHandler)(void* context);

    class CMButton : public CMControlBase
    {
        DECLARE_DYNAMIC(CMButton)

    public:
        CMButton();
        virtual ~CMButton();

        // 생성
        BOOL Create(const CString& text, DWORD dwStyle, const CRect& rect,
            CWnd* pParentWnd, UINT nID);

        // 텍스트
        void SetText(const CString& text);
        const CString& GetText() const { return m_text; }

        // 버튼 스타일
        void SetButtonStyle(ButtonStyle style);
        ButtonStyle GetButtonStyle() const { return m_buttonStyle; }

        // 아이콘
        void SetIcon(UINT iconId, IconPosition position = IconPosition::Left);
        void SetIconOnly(BOOL iconOnly);
        BOOL IsIconOnly() const { return m_iconOnly; }

        // 로딩 상태
        void SetLoading(BOOL loading);
        BOOL IsLoading() const { return m_loading; }

        // 코너 라디우스 (개별 설정)
        void SetCornerRadius(int radius);
        int GetCornerRadius() const { return m_cornerRadius; }

        // 이벤트 핸들러
        void SetClickHandler(ButtonClickHandler handler, void* context);

    protected:
        // CMControlBase 오버라이드
        virtual void OnDraw(CDC* pDC) override;
        virtual void SetState(ControlState newState) override;
        virtual void HandleThemeChanged() override;

        // 색상 가져오기
        const ColorToken& GetStyleColors() const;
        COLORREF GetTextColor() const;

        // 메시지 핸들러
        afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
        afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
        afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
        afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
        afx_msg void OnTimer(UINT_PTR nIDEvent);

        DECLARE_MESSAGE_MAP()

    private:
        // 속성
        CString m_text;
        ButtonStyle m_buttonStyle;
        UINT m_iconId;
        IconPosition m_iconPosition;
        BOOL m_iconOnly;
        BOOL m_loading;
        int m_cornerRadius;

        // 애니메이션
        COLORREF m_targetBgColor;
        float m_scale;
        int m_loadingAngle;
        UINT_PTR m_loadingTimerId;

        // 이벤트 핸들러
        ButtonClickHandler m_clickHandler;
        void* m_clickContext;

        // 헬퍼
        void UpdateColors();
        void DrawLoadingSpinnerD2D(const CRect& rect);
        void StartColorAnimation(COLORREF targetColor);
    };
}
