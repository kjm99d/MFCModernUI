#include "pch.h"
#include "CMSpinner.h"

namespace MFCModernUI
{
    IMPLEMENT_DYNAMIC(CMSpinner, CMControlBase)

    BEGIN_MESSAGE_MAP(CMSpinner, CMControlBase)
        ON_WM_LBUTTONDOWN()
        ON_WM_LBUTTONUP()
        ON_WM_MOUSEMOVE()
        ON_WM_KEYDOWN()
        ON_WM_MOUSEWHEEL()
        ON_WM_TIMER()
    END_MESSAGE_MAP()

    CMSpinner::CMSpinner()
        : m_value(0)
        , m_minValue(0)
        , m_maxValue(100)
        , m_step(1)
        , m_wrap(FALSE)
        , m_readOnly(FALSE)
        , m_hoverArea(ButtonArea::None)
        , m_pressedArea(ButtonArea::None)
        , m_repeatTimerId(0)
        , m_isFirstRepeat(TRUE)
        , m_valueChangedHandler(nullptr)
        , m_valueChangedContext(nullptr)
    {
        m_themeClass = _T("spinner");
        m_tabStop = TRUE;
    }

    CMSpinner::~CMSpinner()
    {
        StopRepeatTimer();
    }

    BOOL CMSpinner::Create(DWORD dwStyle, const CRect& rect, CWnd* pParentWnd, UINT nID)
    {
        CString className = AfxRegisterWndClass(
            CS_HREDRAW | CS_VREDRAW,
            LoadCursor(nullptr, IDC_ARROW),
            nullptr,
            nullptr
        );

        return CWnd::Create(className, nullptr, dwStyle | WS_CHILD | WS_TABSTOP, rect, pParentWnd, nID);
    }

    void CMSpinner::SetValue(int value)
    {
        // 범위 제한 또는 순환
        if (m_wrap)
        {
            int range = m_maxValue - m_minValue + 1;
            while (value > m_maxValue)
                value -= range;
            while (value < m_minValue)
                value += range;
        }
        else
        {
            value = max(m_minValue, min(m_maxValue, value));
        }

        if (m_value != value)
        {
            int oldValue = m_value;
            m_value = value;
            Invalidate();

            if (m_valueChangedHandler)
            {
                m_valueChangedHandler(m_valueChangedContext, oldValue, m_value);
            }
        }
    }

    void CMSpinner::SetRange(int minValue, int maxValue)
    {
        m_minValue = minValue;
        m_maxValue = maxValue;

        // 현재 값이 범위를 벗어나면 조정
        SetValue(m_value);
    }

    void CMSpinner::SetStep(int step)
    {
        m_step = max(1, step);
    }

    void CMSpinner::SetWrap(BOOL wrap)
    {
        m_wrap = wrap;
    }

    void CMSpinner::SetReadOnly(BOOL readOnly)
    {
        m_readOnly = readOnly;
        Invalidate();
    }

    void CMSpinner::SetValueChangedHandler(SpinnerValueChangedHandler handler, void* context)
    {
        m_valueChangedHandler = handler;
        m_valueChangedContext = context;
    }

    void CMSpinner::OnDraw(CDC* pDC)
    {
        CRect rect;
        GetClientRect(&rect);

        const ThemeColors& colors = GetColors();

        // 배경
        COLORREF bgColor = m_isEnabled ? colors.surface.normal : colors.surface.disabled;
        pDC->FillSolidRect(rect, bgColor);

        // 테두리
        COLORREF borderColor;
        if (!m_isEnabled)
        {
            borderColor = colors.border.disabled;
        }
        else if (m_isFocused)
        {
            borderColor = colors.primary.normal;
        }
        else if (m_isHovered)
        {
            borderColor = colors.border.hover;
        }
        else
        {
            borderColor = colors.border.normal;
        }

        DrawRoundedRect(pDC, rect, 4, bgColor);

        // 테두리 그리기
        CPen pen(PS_SOLID, m_isFocused ? 2 : 1, borderColor);
        CPen* oldPen = pDC->SelectObject(&pen);
        CBrush* oldBrush = (CBrush*)pDC->SelectStockObject(NULL_BRUSH);
        pDC->RoundRect(rect, CPoint(4, 4));
        pDC->SelectObject(oldPen);
        pDC->SelectObject(oldBrush);

        // 입력 영역
        DrawInputArea(pDC, GetInputRect());

        // 위 버튼
        DrawUpButton(pDC, GetUpButtonRect());

        // 아래 버튼
        DrawDownButton(pDC, GetDownButtonRect());
    }

    void CMSpinner::DrawInputArea(CDC* pDC, const CRect& rect)
    {
        const ThemeColors& colors = GetColors();

        // 값 텍스트
        CString text;
        text.Format(_T("%d"), m_value);

        COLORREF textColor = m_isEnabled ? colors.text : colors.textDisabled;
        DrawText(pDC, text, rect, textColor, TextStyle::Body,
            DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    }

    void CMSpinner::DrawUpButton(CDC* pDC, const CRect& rect)
    {
        const ThemeColors& colors = GetColors();

        // 버튼 배경
        COLORREF bgColor;
        if (!m_isEnabled)
        {
            bgColor = colors.surface.disabled;
        }
        else if (m_pressedArea == ButtonArea::Up)
        {
            bgColor = colors.surface.pressed;
        }
        else if (m_hoverArea == ButtonArea::Up)
        {
            bgColor = colors.surface.hover;
        }
        else
        {
            bgColor = colors.surface.normal;
        }

        pDC->FillSolidRect(rect, bgColor);

        // 구분선
        CPen pen(PS_SOLID, 1, colors.border.normal);
        CPen* oldPen = pDC->SelectObject(&pen);
        pDC->MoveTo(rect.left, rect.top);
        pDC->LineTo(rect.left, rect.bottom);
        pDC->MoveTo(rect.left, rect.bottom - 1);
        pDC->LineTo(rect.right, rect.bottom - 1);
        pDC->SelectObject(oldPen);

        // 화살표
        COLORREF arrowColor = m_isEnabled ? colors.text : colors.textDisabled;

        // 최대값이면 비활성 색상
        if (!m_wrap && m_value >= m_maxValue)
        {
            arrowColor = colors.textDisabled;
        }

        DrawArrow(pDC, rect, TRUE, arrowColor);
    }

    void CMSpinner::DrawDownButton(CDC* pDC, const CRect& rect)
    {
        const ThemeColors& colors = GetColors();

        // 버튼 배경
        COLORREF bgColor;
        if (!m_isEnabled)
        {
            bgColor = colors.surface.disabled;
        }
        else if (m_pressedArea == ButtonArea::Down)
        {
            bgColor = colors.surface.pressed;
        }
        else if (m_hoverArea == ButtonArea::Down)
        {
            bgColor = colors.surface.hover;
        }
        else
        {
            bgColor = colors.surface.normal;
        }

        pDC->FillSolidRect(rect, bgColor);

        // 구분선
        CPen pen(PS_SOLID, 1, colors.border.normal);
        CPen* oldPen = pDC->SelectObject(&pen);
        pDC->MoveTo(rect.left, rect.top);
        pDC->LineTo(rect.left, rect.bottom);
        pDC->SelectObject(oldPen);

        // 화살표
        COLORREF arrowColor = m_isEnabled ? colors.text : colors.textDisabled;

        // 최소값이면 비활성 색상
        if (!m_wrap && m_value <= m_minValue)
        {
            arrowColor = colors.textDisabled;
        }

        DrawArrow(pDC, rect, FALSE, arrowColor);
    }

    void CMSpinner::DrawArrow(CDC* pDC, const CRect& rect, BOOL up, COLORREF color)
    {
        int centerX = rect.CenterPoint().x;
        int centerY = rect.CenterPoint().y;
        int size = 4;

        POINT points[3];

        if (up)
        {
            points[0] = { centerX, centerY - size };
            points[1] = { centerX - size, centerY + size / 2 };
            points[2] = { centerX + size, centerY + size / 2 };
        }
        else
        {
            points[0] = { centerX, centerY + size };
            points[1] = { centerX - size, centerY - size / 2 };
            points[2] = { centerX + size, centerY - size / 2 };
        }

        CBrush brush(color);
        CPen pen(PS_SOLID, 1, color);
        CPen* oldPen = pDC->SelectObject(&pen);
        CBrush* oldBrush = pDC->SelectObject(&brush);
        pDC->Polygon(points, 3);
        pDC->SelectObject(oldPen);
        pDC->SelectObject(oldBrush);
    }

    CRect CMSpinner::GetInputRect()
    {
        CRect rect;
        GetClientRect(&rect);

        rect.right -= BUTTON_WIDTH;
        rect.DeflateRect(2, 2);

        return rect;
    }

    CRect CMSpinner::GetUpButtonRect()
    {
        CRect rect;
        GetClientRect(&rect);

        rect.left = rect.right - BUTTON_WIDTH;
        rect.bottom = rect.top + rect.Height() / 2;

        return rect;
    }

    CRect CMSpinner::GetDownButtonRect()
    {
        CRect rect;
        GetClientRect(&rect);

        rect.left = rect.right - BUTTON_WIDTH;
        rect.top = rect.top + rect.Height() / 2;

        return rect;
    }

    CMSpinner::ButtonArea CMSpinner::HitTest(CPoint point)
    {
        if (GetUpButtonRect().PtInRect(point))
            return ButtonArea::Up;

        if (GetDownButtonRect().PtInRect(point))
            return ButtonArea::Down;

        if (GetInputRect().PtInRect(point))
            return ButtonArea::Input;

        return ButtonArea::None;
    }

    void CMSpinner::Increment()
    {
        SetValue(m_value + m_step);
    }

    void CMSpinner::Decrement()
    {
        SetValue(m_value - m_step);
    }

    void CMSpinner::StartRepeatTimer()
    {
        if (m_repeatTimerId == 0)
        {
            m_isFirstRepeat = TRUE;
            m_repeatTimerId = SetTimer(REPEAT_TIMER_ID, REPEAT_DELAY, nullptr);
        }
    }

    void CMSpinner::StopRepeatTimer()
    {
        if (m_repeatTimerId != 0)
        {
            KillTimer(m_repeatTimerId);
            m_repeatTimerId = 0;
        }
    }

    void CMSpinner::OnLButtonDown(UINT nFlags, CPoint point)
    {
        if (!m_isEnabled)
        {
            CMControlBase::OnLButtonDown(nFlags, point);
            return;
        }

        SetFocus();
        SetCapture();

        m_pressedArea = HitTest(point);

        if (m_pressedArea == ButtonArea::Up)
        {
            Increment();
            StartRepeatTimer();
        }
        else if (m_pressedArea == ButtonArea::Down)
        {
            Decrement();
            StartRepeatTimer();
        }

        Invalidate();
        CMControlBase::OnLButtonDown(nFlags, point);
    }

    void CMSpinner::OnLButtonUp(UINT nFlags, CPoint point)
    {
        ReleaseCapture();
        StopRepeatTimer();
        m_pressedArea = ButtonArea::None;
        Invalidate();

        CMControlBase::OnLButtonUp(nFlags, point);
    }

    void CMSpinner::OnMouseMove(UINT nFlags, CPoint point)
    {
        ButtonArea newHover = HitTest(point);
        if (newHover != m_hoverArea)
        {
            m_hoverArea = newHover;
            Invalidate();
        }

        CMControlBase::OnMouseMove(nFlags, point);
    }

    void CMSpinner::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
    {
        if (!m_isEnabled)
        {
            CMControlBase::OnKeyDown(nChar, nRepCnt, nFlags);
            return;
        }

        switch (nChar)
        {
        case VK_UP:
            Increment();
            break;

        case VK_DOWN:
            Decrement();
            break;

        case VK_PRIOR:  // Page Up
            SetValue(m_value + m_step * 10);
            break;

        case VK_NEXT:   // Page Down
            SetValue(m_value - m_step * 10);
            break;

        case VK_HOME:
            SetValue(m_minValue);
            break;

        case VK_END:
            SetValue(m_maxValue);
            break;
        }

        CMControlBase::OnKeyDown(nChar, nRepCnt, nFlags);
    }

    BOOL CMSpinner::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
    {
        if (!m_isEnabled)
            return CMControlBase::OnMouseWheel(nFlags, zDelta, pt);

        if (zDelta > 0)
            Increment();
        else
            Decrement();

        return TRUE;
    }

    void CMSpinner::OnTimer(UINT_PTR nIDEvent)
    {
        if (nIDEvent == REPEAT_TIMER_ID)
        {
            // 첫 번째 반복 후 간격 줄이기
            if (m_isFirstRepeat)
            {
                m_isFirstRepeat = FALSE;
                KillTimer(m_repeatTimerId);
                m_repeatTimerId = SetTimer(REPEAT_TIMER_ID, REPEAT_INTERVAL, nullptr);
            }

            // 버튼에 따라 증감
            if (m_pressedArea == ButtonArea::Up)
            {
                Increment();
            }
            else if (m_pressedArea == ButtonArea::Down)
            {
                Decrement();
            }
        }

        CMControlBase::OnTimer(nIDEvent);
    }

    UINT CMSpinner::OnGetDlgCode()
    {
        return DLGC_WANTARROWS;
    }
}
