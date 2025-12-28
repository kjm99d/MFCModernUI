#pragma once

#include "CMControlBase.h"

// MFC Modern UI - 색상 선택 컴포넌트
// SOLID: 단일 책임 원칙 - 색상 선택만 담당

namespace MFCModernUI
{
    // 색상 변경 핸들러
    typedef void (*ColorChangedHandler)(void* context, COLORREF oldColor, COLORREF newColor);

    // 색상 팝업 (내부 클래스)
    class CMColorPopup;

    class CMColorPicker : public CMControlBase
    {
        DECLARE_DYNAMIC(CMColorPicker)

    public:
        CMColorPicker();
        virtual ~CMColorPicker();

        // 생성
        BOOL Create(DWORD dwStyle, const CRect& rect, CWnd* pParentWnd, UINT nID);

        // 색상
        void SetColor(COLORREF color);
        COLORREF GetColor() const { return m_color; }

        // 알파 지원
        void SetAlphaEnabled(BOOL enable);
        BOOL IsAlphaEnabled() const { return m_alphaEnabled; }
        void SetAlpha(BYTE alpha);
        BYTE GetAlpha() const { return m_alpha; }

        // 사용자 정의 색상 목록
        void SetCustomColors(const COLORREF* colors, int count);
        void AddCustomColor(COLORREF color);

        // 이벤트 핸들러
        void SetColorChangedHandler(ColorChangedHandler handler, void* context);

        // 드롭다운
        void ShowDropDown();
        void HideDropDown();
        BOOL IsDroppedDown() const;

    protected:
        virtual void OnDraw(CDC* pDC) override;

        afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
        afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);

        DECLARE_MESSAGE_MAP()

    private:
        // 색상
        COLORREF m_color;
        BYTE m_alpha;
        BOOL m_alphaEnabled;

        // 사용자 정의 색상
        CArray<COLORREF> m_customColors;

        // 드롭다운
        CMColorPopup* m_popup;

        // 이벤트
        ColorChangedHandler m_colorChangedHandler;
        void* m_colorChangedContext;

        // 상수
        static const int BUTTON_WIDTH = 24;

        // 헬퍼
        void DrawColorPreview(CDC* pDC, const CRect& rect);
        void DrawButton(CDC* pDC, const CRect& rect);
        CString ColorToHex(COLORREF color);
        void NotifyColorChanged(COLORREF oldColor);

        friend class CMColorPopup;
    };

    // 색상 선택 팝업
    class CMColorPopup : public CWnd
    {
    public:
        CMColorPopup(CMColorPicker* pOwner);
        virtual ~CMColorPopup();

        BOOL Create();
        void Show(CPoint pt);
        void Hide();

    protected:
        afx_msg void OnPaint();
        afx_msg BOOL OnEraseBkgnd(CDC* pDC);
        afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
        afx_msg void OnMouseMove(UINT nFlags, CPoint point);
        afx_msg void OnKillFocus(CWnd* pNewWnd);

        DECLARE_MESSAGE_MAP()

    private:
        CMColorPicker* m_pOwner;

        COLORREF m_tempColor;
        int m_hoverPaletteIndex;
        int m_hoverCustomIndex;
        BOOL m_hoverMoreButton;

        // HSV 값
        float m_hue;
        float m_saturation;
        float m_value;

        // 기본 팔레트 색상
        static const COLORREF s_paletteColors[];
        static const int PALETTE_ROWS = 5;
        static const int PALETTE_COLS = 10;

        static const int CELL_SIZE = 20;
        static const int PADDING = 8;
        static const int SECTION_GAP = 8;

        void OnDraw(CDC* pDC);
        void DrawPalette(CDC* pDC, const CRect& rect);
        void DrawCustomColors(CDC* pDC, const CRect& rect);
        void DrawPreview(CDC* pDC, const CRect& rect);
        void DrawMoreButton(CDC* pDC, const CRect& rect);

        int HitTestPalette(CPoint point);
        int HitTestCustom(CPoint point);
        BOOL HitTestMoreButton(CPoint point);
        CRect GetPaletteRect();
        CRect GetCustomRect();
        CRect GetPreviewRect();
        CRect GetMoreButtonRect();

        void RGBToHSV(COLORREF color, float& h, float& s, float& v);
        COLORREF HSVToRGB(float h, float s, float v);

        const ThemeColors& GetColors() const;
    };
}
