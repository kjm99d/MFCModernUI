#pragma once

#include "CMTypes.h"
#include "CMColors.h"
#include "CMThemeManager.h"
#include "CMAnimationManager.h"

// MFC Modern UI - 컨트롤 기본 클래스
// SOLID: 개방-폐쇄 원칙 - 확장에 열려있고 수정에 닫혀있음
// SOLID: 리스코프 치환 원칙 - 파생 클래스가 기본 클래스를 대체 가능

namespace MFCModernUI
{
    class CMControlBase : public CWnd
    {
        DECLARE_DYNAMIC(CMControlBase)

    public:
        CMControlBase();
        virtual ~CMControlBase();

        // 공통 속성
        void SetEnabled(BOOL enabled);
        BOOL IsEnabled() const { return m_enabled; }

        void SetVisible(BOOL visible);
        BOOL IsVisible() const { return m_visible; }

        void SetThemeClass(const CString& themeClass);
        const CString& GetThemeClass() const { return m_themeClass; }

        void SetTabStop(BOOL tabStop);
        BOOL IsTabStop() const { return m_tabStop; }

        void SetTooltip(const CString& tooltip);
        const CString& GetTooltip() const { return m_tooltip; }

        // 상태 관리
        ControlState GetState() const { return m_state; }

        // 크기 설정
        void SetControlSize(ControlSize size);
        ControlSize GetControlSize() const { return m_controlSize; }

    protected:
        // 상태 변경 (파생 클래스에서 호출)
        virtual void SetState(ControlState newState);

        // 그리기 (파생 클래스에서 구현)
        virtual void OnDraw(CDC* pDC) = 0;

        // 테마 관련
        CMThemeManager& GetThemeManager() { return CMThemeManager::GetInstance(); }
        const ThemeColors& GetColors() const { return CMThemeManager::GetInstance().GetColors(); }

        // 유틸리티
        void DrawRoundedRect(CDC* pDC, const CRect& rect, int radius, COLORREF fillColor, COLORREF borderColor = CLR_INVALID, int borderWidth = 0);
        void DrawText(CDC* pDC, const CString& text, const CRect& rect, COLORREF color, TextStyle style = TextStyle::Body, UINT format = DT_CENTER | DT_VCENTER | DT_SINGLELINE);

        // 크기 헬퍼
        int GetHeight() const;
        int GetPadding() const;
        int GetFontSize() const;

        // 메시지 핸들러
        afx_msg void OnPaint();
        afx_msg void OnMouseMove(UINT nFlags, CPoint point);
        afx_msg void OnMouseLeave();
        afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
        afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
        afx_msg void OnSetFocus(CWnd* pOldWnd);
        afx_msg void OnKillFocus(CWnd* pNewWnd);
        afx_msg void OnEnable(BOOL bEnable);
        afx_msg BOOL OnEraseBkgnd(CDC* pDC);

        DECLARE_MESSAGE_MAP()

    protected:
        // 공통 속성
        BOOL m_enabled;
        BOOL m_visible;
        CString m_themeClass;
        BOOL m_tabStop;
        CString m_tooltip;

        // 상태
        ControlState m_state;
        ControlSize m_controlSize;

        // 마우스 추적
        BOOL m_mouseTracking;
        BOOL m_mouseOver;

        // 애니메이션
        int m_currentAnimationId;
        float m_animationProgress;

        // 현재 색상 (애니메이션용)
        COLORREF m_currentBgColor;
        COLORREF m_currentBorderColor;

    private:
        // 테마 변경 콜백
        static void OnThemeChanged(void* context);
        void HandleThemeChanged();

        // 툴팁
        CToolTipCtrl m_tooltipCtrl;
        void UpdateTooltip();
    };
}
