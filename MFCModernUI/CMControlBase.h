#pragma once

#include "CMTypes.h"
#include "CMColors.h"
#include "CMThemeManager.h"
#include "CMAnimationManager.h"

// Direct2D
#include <d2d1.h>
#include <d2d1_1.h>
#include <dwrite.h>
#include <wincodec.h>
#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwrite.lib")

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
        bool IsLightTheme() const { return !CMThemeManager::GetInstance().IsDarkMode(); }

        // Direct2D 유틸리티
        void DrawRoundedRectD2D(const CRect& rect, int radius, COLORREF fillColor, COLORREF borderColor = CLR_INVALID, int borderWidth = 0);
        void DrawCircleD2D(const CRect& rect, COLORREF fillColor, COLORREF borderColor = CLR_INVALID, int borderWidth = 0);
        void DrawEllipseD2D(const CRect& rect, COLORREF fillColor, COLORREF borderColor = CLR_INVALID, int borderWidth = 0);
        void DrawLineD2D(int x1, int y1, int x2, int y2, COLORREF color, int width = 1);
        void DrawTextD2D(const CString& text, const CRect& rect, COLORREF color, TextStyle style = TextStyle::Body, UINT format = DT_CENTER | DT_VCENTER | DT_SINGLELINE);

        // Direct2D 헬퍼
        static D2D1_COLOR_F ToD2DColor(COLORREF color, float alpha = 1.0f);
        ID2D1RenderTarget* GetRenderTarget();
        ID2D1SolidColorBrush* GetSolidBrush(COLORREF color, float alpha = 1.0f);
        IDWriteTextFormat* GetTextFormat(TextStyle style);

        // Direct2D 렌더링 시작/종료
        BOOL BeginD2DDraw();
        void EndD2DDraw();
        BOOL IsD2DDrawing() const { return m_isD2DDrawing; }

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

        // 상태 플래그 (파생 클래스에서 직접 접근)
        BOOL m_isEnabled;       // 활성화 상태
        BOOL m_isHovered;       // 마우스 오버 상태
        BOOL m_isFocused;       // 포커스 상태
        BOOL m_isPressed;       // 눌림 상태

        // 마우스 추적
        BOOL m_mouseTracking;
        BOOL m_mouseOver;

        // 애니메이션
        int m_currentAnimationId;
        float m_animationProgress;

        // 현재 색상 (애니메이션용)
        COLORREF m_currentBgColor;
        COLORREF m_currentBorderColor;

        // Direct2D 렌더링 상태
        BOOL m_isD2DDrawing;

        // Direct2D 리소스 (파생 클래스에서 직접 접근 가능)
        static ID2D1Factory* s_pD2DFactory;
        static IDWriteFactory* s_pDWriteFactory;
        static int s_d2dRefCount;

        ID2D1HwndRenderTarget* m_pRenderTarget;
        ID2D1SolidColorBrush* m_pSolidBrush;
        IDWriteTextFormat* m_pTextFormats[4];  // Body, Caption, Subtitle, Title

        // 테마 변경 처리 (파생 클래스에서 오버라이드 가능)
        virtual void HandleThemeChanged();

    private:
        // Direct2D 초기화/해제
        static BOOL InitD2DFactory();
        static void ReleaseD2DFactory();
        BOOL CreateDeviceResources();
        void DiscardDeviceResources();
        // 테마 변경 콜백
        static void OnThemeChanged(void* context);

        // 툴팁
        CToolTipCtrl m_tooltipCtrl;
        void UpdateTooltip();
    };
}
