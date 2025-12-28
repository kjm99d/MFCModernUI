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
