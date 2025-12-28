#include "pch.h"
#include "CMControlBase.h"

namespace MFCModernUI
{
    IMPLEMENT_DYNAMIC(CMControlBase, CWnd)

    BEGIN_MESSAGE_MAP(CMControlBase, CWnd)
        ON_WM_PAINT()
        ON_WM_MOUSEMOVE()
        ON_WM_MOUSELEAVE()
        ON_WM_LBUTTONDOWN()
        ON_WM_LBUTTONUP()
        ON_WM_SETFOCUS()
        ON_WM_KILLFOCUS()
        ON_WM_ENABLE()
        ON_WM_ERASEBKGND()
    END_MESSAGE_MAP()

    CMControlBase::CMControlBase()
        : m_enabled(TRUE)
        , m_visible(TRUE)
        , m_tabStop(TRUE)
        , m_state(ControlState::Normal)
        , m_controlSize(ControlSize::Medium)
        , m_isEnabled(TRUE)
        , m_isHovered(FALSE)
        , m_isFocused(FALSE)
        , m_isPressed(FALSE)
        , m_mouseTracking(FALSE)
        , m_mouseOver(FALSE)
        , m_currentAnimationId(0)
        , m_animationProgress(0.0f)
        , m_currentBgColor(RGB(255, 255, 255))
        , m_currentBorderColor(RGB(200, 200, 200))
    {
        // 테마 변경 콜백 등록
        CMThemeManager::GetInstance().RegisterThemeChangedCallback(OnThemeChanged, this);
    }

    CMControlBase::~CMControlBase()
    {
        // 테마 변경 콜백 해제
        CMThemeManager::GetInstance().UnregisterThemeChangedCallback(this);

        // 애니메이션 중지
        if (m_currentAnimationId > 0)
        {
            CMAnimationManager::GetInstance().StopAnimation(m_currentAnimationId);
        }
    }

    void CMControlBase::SetEnabled(BOOL enabled)
    {
        if (m_enabled != enabled)
        {
            m_enabled = enabled;
            m_isEnabled = enabled;
            SetState(enabled ? ControlState::Normal : ControlState::Disabled);
            EnableWindow(enabled);
            Invalidate();
        }
    }

    void CMControlBase::SetVisible(BOOL visible)
    {
        if (m_visible != visible)
        {
            m_visible = visible;
            ShowWindow(visible ? SW_SHOW : SW_HIDE);
        }
    }

    void CMControlBase::SetThemeClass(const CString& themeClass)
    {
        m_themeClass = themeClass;
        Invalidate();
    }

    void CMControlBase::SetTabStop(BOOL tabStop)
    {
        m_tabStop = tabStop;
        ModifyStyle(tabStop ? 0 : WS_TABSTOP, tabStop ? WS_TABSTOP : 0);
    }

    void CMControlBase::SetTooltip(const CString& tooltip)
    {
        m_tooltip = tooltip;
        UpdateTooltip();
    }

    void CMControlBase::SetControlSize(ControlSize size)
    {
        if (m_controlSize != size)
        {
            m_controlSize = size;
            Invalidate();
        }
    }

    void CMControlBase::SetState(ControlState newState)
    {
        if (m_state != newState)
        {
            ControlState oldState = m_state;
            m_state = newState;

            // 상태 플래그 동기화
            m_isEnabled = (newState != ControlState::Disabled);
            m_isHovered = (newState == ControlState::Hover);
            m_isFocused = (newState == ControlState::Focused);
            m_isPressed = (newState == ControlState::Pressed);

            Invalidate();
        }
    }

    int CMControlBase::GetHeight() const
    {
        switch (m_controlSize)
        {
        case ControlSize::Small:
            return SizeConstants::SmallHeight;
        case ControlSize::Large:
            return SizeConstants::LargeHeight;
        default:
            return SizeConstants::MediumHeight;
        }
    }

    int CMControlBase::GetPadding() const
    {
        switch (m_controlSize)
        {
        case ControlSize::Small:
            return SizeConstants::SmallPadding;
        case ControlSize::Large:
            return SizeConstants::LargePadding;
        default:
            return SizeConstants::MediumPadding;
        }
    }

    int CMControlBase::GetFontSize() const
    {
        switch (m_controlSize)
        {
        case ControlSize::Small:
            return SizeConstants::SmallFontSize;
        case ControlSize::Large:
            return SizeConstants::LargeFontSize;
        default:
            return SizeConstants::MediumFontSize;
        }
    }

    void CMControlBase::DrawRoundedRect(CDC* pDC, const CRect& rect, int radius,
        COLORREF fillColor, COLORREF borderColor, int borderWidth)
    {
        // GDI+를 사용하지 않고 순수 GDI로 둥근 사각형 그리기
        CPen pen;
        CBrush brush;

        if (borderColor != CLR_INVALID && borderWidth > 0)
        {
            pen.CreatePen(PS_SOLID, borderWidth, borderColor);
        }
        else
        {
            pen.CreatePen(PS_NULL, 0, RGB(0, 0, 0));
        }

        brush.CreateSolidBrush(fillColor);

        CPen* pOldPen = pDC->SelectObject(&pen);
        CBrush* pOldBrush = pDC->SelectObject(&brush);

        pDC->RoundRect(rect, CPoint(radius * 2, radius * 2));

        pDC->SelectObject(pOldPen);
        pDC->SelectObject(pOldBrush);
    }

    void CMControlBase::DrawText(CDC* pDC, const CString& text, const CRect& rect,
        COLORREF color, TextStyle style, UINT format)
    {
        CFont* pFont = GetThemeManager().GetFont(style);
        CFont* pOldFont = pDC->SelectObject(pFont);
        COLORREF oldColor = pDC->SetTextColor(color);
        int oldBkMode = pDC->SetBkMode(TRANSPARENT);

        CRect textRect = rect;
        pDC->DrawText(text, &textRect, format);

        pDC->SetBkMode(oldBkMode);
        pDC->SetTextColor(oldColor);
        pDC->SelectObject(pOldFont);
    }

    // GDI+ 헬퍼 함수들
    Gdiplus::Color CMControlBase::ToGdiplusColor(COLORREF color, BYTE alpha)
    {
        return Gdiplus::Color(alpha, GetRValue(color), GetGValue(color), GetBValue(color));
    }

    void CMControlBase::CreateRoundedRectPath(Gdiplus::GraphicsPath& path, const Gdiplus::RectF& rect, float radius)
    {
        path.Reset();

        if (radius <= 0)
        {
            path.AddRectangle(rect);
            return;
        }

        float diameter = radius * 2.0f;

        // 왼쪽 상단 모서리
        path.AddArc(rect.X, rect.Y, diameter, diameter, 180.0f, 90.0f);
        // 오른쪽 상단 모서리
        path.AddArc(rect.X + rect.Width - diameter, rect.Y, diameter, diameter, 270.0f, 90.0f);
        // 오른쪽 하단 모서리
        path.AddArc(rect.X + rect.Width - diameter, rect.Y + rect.Height - diameter, diameter, diameter, 0.0f, 90.0f);
        // 왼쪽 하단 모서리
        path.AddArc(rect.X, rect.Y + rect.Height - diameter, diameter, diameter, 90.0f, 90.0f);

        path.CloseFigure();
    }

    void CMControlBase::DrawRoundedRectGdiPlus(CDC* pDC, const CRect& rect, int radius,
        COLORREF fillColor, COLORREF borderColor, int borderWidth)
    {
        Gdiplus::Graphics graphics(pDC->GetSafeHdc());
        graphics.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
        graphics.SetPixelOffsetMode(Gdiplus::PixelOffsetModeHighQuality);

        Gdiplus::RectF gdipRect(
            static_cast<Gdiplus::REAL>(rect.left),
            static_cast<Gdiplus::REAL>(rect.top),
            static_cast<Gdiplus::REAL>(rect.Width()),
            static_cast<Gdiplus::REAL>(rect.Height())
        );

        // 테두리가 있으면 영역 조정
        if (borderColor != CLR_INVALID && borderWidth > 0)
        {
            float halfBorder = borderWidth / 2.0f;
            gdipRect.X += halfBorder;
            gdipRect.Y += halfBorder;
            gdipRect.Width -= borderWidth;
            gdipRect.Height -= borderWidth;
        }

        Gdiplus::GraphicsPath path;
        CreateRoundedRectPath(path, gdipRect, static_cast<float>(radius));

        // 채우기
        Gdiplus::SolidBrush brush(ToGdiplusColor(fillColor));
        graphics.FillPath(&brush, &path);

        // 테두리
        if (borderColor != CLR_INVALID && borderWidth > 0)
        {
            Gdiplus::Pen pen(ToGdiplusColor(borderColor), static_cast<Gdiplus::REAL>(borderWidth));
            graphics.DrawPath(&pen, &path);
        }
    }

    void CMControlBase::DrawCircleGdiPlus(CDC* pDC, const CRect& rect, COLORREF fillColor,
        COLORREF borderColor, int borderWidth)
    {
        DrawEllipseGdiPlus(pDC, rect, fillColor, borderColor, borderWidth);
    }

    void CMControlBase::DrawEllipseGdiPlus(CDC* pDC, const CRect& rect, COLORREF fillColor,
        COLORREF borderColor, int borderWidth)
    {
        Gdiplus::Graphics graphics(pDC->GetSafeHdc());
        graphics.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
        graphics.SetPixelOffsetMode(Gdiplus::PixelOffsetModeHighQuality);

        Gdiplus::RectF gdipRect(
            static_cast<Gdiplus::REAL>(rect.left),
            static_cast<Gdiplus::REAL>(rect.top),
            static_cast<Gdiplus::REAL>(rect.Width()),
            static_cast<Gdiplus::REAL>(rect.Height())
        );

        // 테두리가 있으면 영역 조정
        if (borderColor != CLR_INVALID && borderWidth > 0)
        {
            float halfBorder = borderWidth / 2.0f;
            gdipRect.X += halfBorder;
            gdipRect.Y += halfBorder;
            gdipRect.Width -= borderWidth;
            gdipRect.Height -= borderWidth;
        }

        // 채우기
        Gdiplus::SolidBrush brush(ToGdiplusColor(fillColor));
        graphics.FillEllipse(&brush, gdipRect);

        // 테두리
        if (borderColor != CLR_INVALID && borderWidth > 0)
        {
            Gdiplus::Pen pen(ToGdiplusColor(borderColor), static_cast<Gdiplus::REAL>(borderWidth));
            graphics.DrawEllipse(&pen, gdipRect);
        }
    }

    void CMControlBase::DrawLineGdiPlus(CDC* pDC, int x1, int y1, int x2, int y2, COLORREF color, int width)
    {
        Gdiplus::Graphics graphics(pDC->GetSafeHdc());
        graphics.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);

        Gdiplus::Pen pen(ToGdiplusColor(color), static_cast<Gdiplus::REAL>(width));
        pen.SetLineCap(Gdiplus::LineCapRound, Gdiplus::LineCapRound, Gdiplus::DashCapRound);

        graphics.DrawLine(&pen,
            static_cast<Gdiplus::REAL>(x1),
            static_cast<Gdiplus::REAL>(y1),
            static_cast<Gdiplus::REAL>(x2),
            static_cast<Gdiplus::REAL>(y2)
        );
    }

    void CMControlBase::DrawTextGdiPlus(CDC* pDC, const CString& text, const CRect& rect,
        COLORREF color, TextStyle style, UINT format)
    {
        Gdiplus::Graphics graphics(pDC->GetSafeHdc());
        graphics.SetTextRenderingHint(Gdiplus::TextRenderingHintClearTypeGridFit);

        // 폰트 설정
        CFont* pFont = GetThemeManager().GetFont(style);
        LOGFONT lf;
        pFont->GetLogFont(&lf);

        Gdiplus::Font font(pDC->GetSafeHdc(), &lf);

        // 정렬 설정
        Gdiplus::StringFormat stringFormat;
        if (format & DT_CENTER)
            stringFormat.SetAlignment(Gdiplus::StringAlignmentCenter);
        else if (format & DT_RIGHT)
            stringFormat.SetAlignment(Gdiplus::StringAlignmentFar);
        else
            stringFormat.SetAlignment(Gdiplus::StringAlignmentNear);

        if (format & DT_VCENTER)
            stringFormat.SetLineAlignment(Gdiplus::StringAlignmentCenter);
        else if (format & DT_BOTTOM)
            stringFormat.SetLineAlignment(Gdiplus::StringAlignmentFar);
        else
            stringFormat.SetLineAlignment(Gdiplus::StringAlignmentNear);

        if (format & DT_SINGLELINE)
            stringFormat.SetFormatFlags(Gdiplus::StringFormatFlagsNoWrap);

        Gdiplus::RectF layoutRect(
            static_cast<Gdiplus::REAL>(rect.left),
            static_cast<Gdiplus::REAL>(rect.top),
            static_cast<Gdiplus::REAL>(rect.Width()),
            static_cast<Gdiplus::REAL>(rect.Height())
        );

        Gdiplus::SolidBrush brush(ToGdiplusColor(color));
        graphics.DrawString(text, -1, &font, layoutRect, &stringFormat, &brush);
    }

    void CMControlBase::OnPaint()
    {
        CPaintDC dc(this);

        CRect rect;
        GetClientRect(&rect);

        // 더블 버퍼링
        CDC memDC;
        memDC.CreateCompatibleDC(&dc);

        CBitmap bitmap;
        bitmap.CreateCompatibleBitmap(&dc, rect.Width(), rect.Height());
        CBitmap* pOldBitmap = memDC.SelectObject(&bitmap);

        // 배경 지우기
        memDC.FillSolidRect(rect, GetColors().background.normal);

        // 파생 클래스의 그리기 호출
        OnDraw(&memDC);

        // 화면에 복사
        dc.BitBlt(0, 0, rect.Width(), rect.Height(), &memDC, 0, 0, SRCCOPY);

        memDC.SelectObject(pOldBitmap);
    }

    void CMControlBase::OnMouseMove(UINT nFlags, CPoint point)
    {
        if (!m_mouseTracking)
        {
            TRACKMOUSEEVENT tme;
            tme.cbSize = sizeof(TRACKMOUSEEVENT);
            tme.dwFlags = TME_LEAVE;
            tme.hwndTrack = m_hWnd;
            TrackMouseEvent(&tme);
            m_mouseTracking = TRUE;
        }

        if (!m_mouseOver && m_enabled)
        {
            m_mouseOver = TRUE;
            if (m_state != ControlState::Pressed)
            {
                SetState(ControlState::Hover);
            }
        }

        CWnd::OnMouseMove(nFlags, point);
    }

    void CMControlBase::OnMouseLeave()
    {
        m_mouseTracking = FALSE;
        m_mouseOver = FALSE;

        if (m_enabled && m_state != ControlState::Focused)
        {
            SetState(ControlState::Normal);
        }
        else if (m_state == ControlState::Focused)
        {
            Invalidate();
        }

        CWnd::OnMouseLeave();
    }

    void CMControlBase::OnLButtonDown(UINT nFlags, CPoint point)
    {
        if (m_enabled)
        {
            SetFocus();
            SetCapture();
            SetState(ControlState::Pressed);
        }

        CWnd::OnLButtonDown(nFlags, point);
    }

    void CMControlBase::OnLButtonUp(UINT nFlags, CPoint point)
    {
        if (GetCapture() == this)
        {
            ReleaseCapture();

            CRect rect;
            GetClientRect(&rect);

            if (rect.PtInRect(point) && m_enabled)
            {
                SetState(ControlState::Hover);
            }
            else if (m_enabled)
            {
                SetState(ControlState::Normal);
            }
        }

        CWnd::OnLButtonUp(nFlags, point);
    }

    void CMControlBase::OnSetFocus(CWnd* pOldWnd)
    {
        CWnd::OnSetFocus(pOldWnd);

        if (m_enabled && m_state != ControlState::Pressed)
        {
            SetState(ControlState::Focused);
        }
    }

    void CMControlBase::OnKillFocus(CWnd* pNewWnd)
    {
        CWnd::OnKillFocus(pNewWnd);

        if (m_enabled)
        {
            SetState(m_mouseOver ? ControlState::Hover : ControlState::Normal);
        }
    }

    void CMControlBase::OnEnable(BOOL bEnable)
    {
        CWnd::OnEnable(bEnable);
        m_enabled = bEnable;
        SetState(bEnable ? ControlState::Normal : ControlState::Disabled);
    }

    BOOL CMControlBase::OnEraseBkgnd(CDC* pDC)
    {
        // 깜빡임 방지를 위해 배경 지우기 무시
        return TRUE;
    }

    void CMControlBase::OnThemeChanged(void* context)
    {
        CMControlBase* pThis = static_cast<CMControlBase*>(context);
        if (pThis)
        {
            pThis->HandleThemeChanged();
        }
    }

    void CMControlBase::HandleThemeChanged()
    {
        Invalidate();
    }

    void CMControlBase::UpdateTooltip()
    {
        if (m_hWnd && !m_tooltip.IsEmpty())
        {
            if (!m_tooltipCtrl.m_hWnd)
            {
                m_tooltipCtrl.Create(this, TTS_ALWAYSTIP);
                m_tooltipCtrl.Activate(TRUE);
            }

            CRect rect;
            GetClientRect(&rect);
            m_tooltipCtrl.AddTool(this, m_tooltip, &rect, 1);
        }
    }
}
