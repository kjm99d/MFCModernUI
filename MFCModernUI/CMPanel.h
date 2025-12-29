#pragma once

#include "CMControlBase.h"

// MFC Modern UI - 패널/컨테이너 컴포넌트
// SOLID: 단일 책임 원칙 - 컨테이너 역할만 담당

namespace MFCModernUI
{
    class CMPanel : public CMControlBase
    {
        DECLARE_DYNAMIC(CMPanel)

    public:
        CMPanel();
        virtual ~CMPanel();

        // 생성
        BOOL Create(DWORD dwStyle, const CRect& rect, CWnd* pParentWnd, UINT nID = 0);

        // 배경색
        void SetBackgroundColor(COLORREF color);
        void UseDefaultBackgroundColor();

        // 테두리
        void SetBorderColor(COLORREF color);
        void SetBorderWidth(int width);
        int GetBorderWidth() const { return m_borderWidth; }

        // 코너 라디우스
        void SetCornerRadius(int radius);
        int GetCornerRadius() const { return m_cornerRadius; }

        // 그림자
        void SetShadow(ShadowLevel level);
        ShadowLevel GetShadow() const { return m_shadowLevel; }

        // 패딩
        void SetPadding(int padding);
        void SetPadding(int left, int top, int right, int bottom);
        CRect GetPadding() const { return m_padding; }

        // 변형 (프리셋)
        void SetVariant(PanelVariant variant);
        PanelVariant GetVariant() const { return m_variant; }

    protected:
        virtual void OnDraw(CDC* pDC) override;

        // 그림자 그리기
        void DrawShadow(CDC* pDC, const CRect& rect);

        DECLARE_MESSAGE_MAP()

    private:
        // 색상
        COLORREF m_backgroundColor;
        BOOL m_useDefaultBgColor;
        COLORREF m_borderColor;

        // 스타일
        int m_borderWidth;
        int m_cornerRadius;
        ShadowLevel m_shadowLevel;
        CRect m_padding;
        PanelVariant m_variant;

        // 헬퍼
        void ApplyVariant();

        // Direct2D
        void OnDrawGdiPlus(CDC* pDC);
        void DrawShadowD2D(const CRect& rect);
    };
}
