#include "pch.h"
#include "CMPanel.h"

namespace MFCModernUI
{
    IMPLEMENT_DYNAMIC(CMPanel, CMControlBase)

    BEGIN_MESSAGE_MAP(CMPanel, CMControlBase)
    END_MESSAGE_MAP()

    CMPanel::CMPanel()
        : m_backgroundColor(RGB(255, 255, 255))
        , m_useDefaultBgColor(TRUE)
        , m_borderColor(RGB(200, 200, 200))
        , m_borderWidth(0)
        , m_cornerRadius(-1)  // -1 = 테마 기본값
        , m_shadowLevel(ShadowLevel::None)
        , m_variant(PanelVariant::Flat)
    {
        m_themeClass = _T("panel");
        m_tabStop = FALSE;  // 패널은 기본적으로 탭 정지하지 않음
        m_padding.SetRect(16, 16, 16, 16);
    }

    CMPanel::~CMPanel()
    {
    }

    BOOL CMPanel::Create(DWORD dwStyle, const CRect& rect, CWnd* pParentWnd, UINT nID)
    {
        CString className = AfxRegisterWndClass(
            CS_HREDRAW | CS_VREDRAW,
            LoadCursor(nullptr, IDC_ARROW),
            nullptr,
            nullptr
        );

        return CWnd::Create(className, nullptr, dwStyle | WS_CHILD | WS_CLIPCHILDREN, rect, pParentWnd, nID);
    }

    void CMPanel::SetBackgroundColor(COLORREF color)
    {
        m_backgroundColor = color;
        m_useDefaultBgColor = FALSE;
        Invalidate();
    }

    void CMPanel::UseDefaultBackgroundColor()
    {
        m_useDefaultBgColor = TRUE;
        Invalidate();
    }

    void CMPanel::SetBorderColor(COLORREF color)
    {
        m_borderColor = color;
        Invalidate();
    }

    void CMPanel::SetBorderWidth(int width)
    {
        if (m_borderWidth != width)
        {
            m_borderWidth = width;
            Invalidate();
        }
    }

    void CMPanel::SetCornerRadius(int radius)
    {
        if (m_cornerRadius != radius)
        {
            m_cornerRadius = radius;
            Invalidate();
        }
    }

    void CMPanel::SetShadow(ShadowLevel level)
    {
        if (m_shadowLevel != level)
        {
            m_shadowLevel = level;
            Invalidate();
        }
    }

    void CMPanel::SetPadding(int padding)
    {
        m_padding.SetRect(padding, padding, padding, padding);
    }

    void CMPanel::SetPadding(int left, int top, int right, int bottom)
    {
        m_padding.SetRect(left, top, right, bottom);
    }

    void CMPanel::SetVariant(PanelVariant variant)
    {
        if (m_variant != variant)
        {
            m_variant = variant;
            ApplyVariant();
            Invalidate();
        }
    }

    void CMPanel::ApplyVariant()
    {
        const ThemeColors& colors = GetColors();

        switch (m_variant)
        {
        case PanelVariant::Flat:
            m_useDefaultBgColor = TRUE;
            m_borderWidth = 0;
            m_shadowLevel = ShadowLevel::None;
            break;

        case PanelVariant::Outlined:
            m_backgroundColor = colors.surface.normal;
            m_useDefaultBgColor = FALSE;
            m_borderColor = colors.border.normal;
            m_borderWidth = 1;
            m_shadowLevel = ShadowLevel::None;
            break;

        case PanelVariant::Elevated:
            m_useDefaultBgColor = TRUE;
            m_borderWidth = 0;
            m_shadowLevel = ShadowLevel::Medium;
            break;

        case PanelVariant::Filled:
            m_useDefaultBgColor = TRUE;
            m_borderWidth = 0;
            m_shadowLevel = ShadowLevel::None;
            break;
        }
    }

    void CMPanel::OnDraw(CDC* pDC)
    {
        // Direct2D 렌더링 시작
        if (!BeginD2DDraw())
        {
            OnDrawGdiPlus(pDC);
            return;
        }

        CRect rect;
        GetClientRect(&rect);

        const ThemeColors& colors = GetColors();
        int radius = (m_cornerRadius >= 0) ? m_cornerRadius : GetThemeManager().GetCornerRadius();

        // 그림자 그리기 (메인 패널 전에)
        if (m_shadowLevel != ShadowLevel::None)
        {
            DrawShadowD2D(rect);
        }

        // 배경색
        COLORREF bgColor = m_useDefaultBgColor ? colors.surface.normal : m_backgroundColor;

        // 테두리
        COLORREF borderColor = CLR_INVALID;
        if (m_borderWidth > 0)
        {
            borderColor = m_borderColor;
        }

        // 패널 그리기
        DrawRoundedRectD2D(rect, radius, bgColor, borderColor, m_borderWidth);

        EndD2DDraw();
    }

    void CMPanel::OnDrawGdiPlus(CDC* pDC)
    {
        CRect rect;
        GetClientRect(&rect);

        const ThemeColors& colors = GetColors();
        int radius = (m_cornerRadius >= 0) ? m_cornerRadius : GetThemeManager().GetCornerRadius();

        if (m_shadowLevel != ShadowLevel::None)
        {
            DrawShadow(pDC, rect);
        }

        COLORREF bgColor = m_useDefaultBgColor ? colors.surface.normal : m_backgroundColor;

        COLORREF borderColor = CLR_INVALID;
        if (m_borderWidth > 0)
        {
            borderColor = m_borderColor;
        }

        DrawRoundedRect(pDC, rect, radius, bgColor, borderColor, m_borderWidth);
    }

    void CMPanel::DrawShadowD2D(const CRect& rect)
    {
        if (!m_pRenderTarget || !m_isD2DDrawing)
            return;

        int offsetY = 0;
        int blur = 0;
        int spread = 0;
        float alpha = 0.0f;

        switch (m_shadowLevel)
        {
        case ShadowLevel::Small:
            offsetY = 1;
            blur = 2;
            spread = 0;
            alpha = 0.08f;
            break;
        case ShadowLevel::Medium:
            offsetY = 4;
            blur = 6;
            spread = -1;
            alpha = 0.12f;
            break;
        case ShadowLevel::Large:
            offsetY = 10;
            blur = 15;
            spread = -3;
            alpha = 0.16f;
            break;
        default:
            return;
        }

        int radius = (m_cornerRadius >= 0) ? m_cornerRadius : GetThemeManager().GetCornerRadius();

        // 그림자 레이어 그리기
        for (int i = blur; i >= 1; i -= 2)
        {
            float currentAlpha = alpha * static_cast<float>(blur - i + 1) / blur;

            CRect shadowRect = rect;
            shadowRect.OffsetRect(0, offsetY);
            shadowRect.InflateRect(i + spread, i + spread);

            m_pSolidBrush->SetColor(ToD2DColor(RGB(0, 0, 0), currentAlpha));

            D2D1_ROUNDED_RECT roundedRect = D2D1::RoundedRect(
                D2D1::RectF(
                    static_cast<float>(shadowRect.left),
                    static_cast<float>(shadowRect.top),
                    static_cast<float>(shadowRect.right),
                    static_cast<float>(shadowRect.bottom)
                ),
                static_cast<float>(radius + i),
                static_cast<float>(radius + i)
            );

            m_pRenderTarget->FillRoundedRectangle(roundedRect, m_pSolidBrush);
        }
    }

    void CMPanel::DrawShadow(CDC* pDC, const CRect& rect)
    {
        // 그림자 파라미터
        int offsetY = 0;
        int blur = 0;
        int spread = 0;
        BYTE alpha = 0;

        switch (m_shadowLevel)
        {
        case ShadowLevel::Small:
            offsetY = 1;
            blur = 2;
            spread = 0;
            alpha = 20;
            break;
        case ShadowLevel::Medium:
            offsetY = 4;
            blur = 6;
            spread = -1;
            alpha = 30;
            break;
        case ShadowLevel::Large:
            offsetY = 10;
            blur = 15;
            spread = -3;
            alpha = 40;
            break;
        default:
            return;
        }

        int radius = (m_cornerRadius >= 0) ? m_cornerRadius : GetThemeManager().GetCornerRadius();

        // 간단한 그림자 시뮬레이션 (여러 레이어)
        for (int i = blur; i >= 1; i -= 2)
        {
            BYTE currentAlpha = static_cast<BYTE>(alpha * (blur - i + 1) / blur);
            COLORREF shadowColor = RGB(0, 0, 0);

            // 그림자 영역
            CRect shadowRect = rect;
            shadowRect.OffsetRect(0, offsetY);
            shadowRect.InflateRect(i + spread, i + spread);

            // 반투명 그림자 (단순화된 버전)
            int grayLevel = 255 - currentAlpha;
            COLORREF grayColor = RGB(grayLevel, grayLevel, grayLevel);

            CPen pen(PS_NULL, 0, RGB(0, 0, 0));
            CBrush brush(grayColor);

            CPen* pOldPen = pDC->SelectObject(&pen);
            CBrush* pOldBrush = pDC->SelectObject(&brush);

            pDC->RoundRect(shadowRect, CPoint(radius + i, radius + i));

            pDC->SelectObject(pOldPen);
            pDC->SelectObject(pOldBrush);
        }
    }
}
