#pragma once

#include "CMControlBase.h"

// MFC Modern UI - 레이블 컴포넌트
// SOLID: 단일 책임 원칙 - 텍스트 표시만 담당

namespace MFCModernUI
{
    class CMLabel : public CMControlBase
    {
        DECLARE_DYNAMIC(CMLabel)

    public:
        CMLabel();
        virtual ~CMLabel();

        // 생성
        BOOL Create(const CString& text, DWORD dwStyle, const CRect& rect,
            CWnd* pParentWnd, UINT nID = 0);

        // 텍스트
        void SetText(const CString& text);
        const CString& GetText() const { return m_text; }

        // 텍스트 스타일
        void SetTextStyle(TextStyle style);
        TextStyle GetTextStyle() const { return m_textStyle; }

        // 텍스트 정렬
        void SetTextAlign(TextAlign align);
        TextAlign GetTextAlign() const { return m_textAlign; }

        // 텍스트 색상
        void SetTextColor(COLORREF color);
        COLORREF GetTextColor() const { return m_textColor; }
        void UseDefaultColor();

        // 말줄임
        void SetEllipsis(BOOL ellipsis);
        BOOL HasEllipsis() const { return m_ellipsis; }

        // 텍스트 선택 가능
        void SetSelectable(BOOL selectable);
        BOOL IsSelectable() const { return m_selectable; }

        // 줄바꿈
        void SetWordWrap(BOOL wordWrap);
        BOOL HasWordWrap() const { return m_wordWrap; }

    protected:
        virtual void OnDraw(CDC* pDC) override;

        DECLARE_MESSAGE_MAP()

    private:
        CString m_text;
        TextStyle m_textStyle;
        TextAlign m_textAlign;
        COLORREF m_textColor;
        BOOL m_useDefaultColor;
        BOOL m_ellipsis;
        BOOL m_selectable;
        BOOL m_wordWrap;

        // Direct2D
        void OnDrawGdiPlus(CDC* pDC);
    };
}
