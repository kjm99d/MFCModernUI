#include "pch.h"
#include "CMSlider.h"

namespace MFCModernUI
{
    IMPLEMENT_DYNAMIC(CMSlider, CMControlBase)

    BEGIN_MESSAGE_MAP(CMSlider, CMControlBase)
        ON_WM_LBUTTONDOWN()
        ON_WM_LBUTTONUP()
        ON_WM_MOUSEMOVE()
        ON_WM_KEYDOWN()
        ON_WM_MOUSEWHEEL()
    END_MESSAGE_MAP()

    CMSlider::CMSlider()
        : m_value(0)
        , m_minValue(0)
        , m_maxValue(100)
        , m_step(1)
        , m_orientation(Orientation::Horizontal)
        , m_showTicks(FALSE)
        , m_showValue(FALSE)
        , m_dragging(FALSE)
        , m_valueChangedHandler(nullptr)
        , m_valueChangedContext(nullptr)
        , m_valueChangingHandler(nullptr)
        , m_valueChangingContext(nullptr)
    {
        m_themeClass = _T("slider");
        m_tabStop = TRUE;
    }

    CMSlider::~CMSlider()
    {
    }

    BOOL CMSlider::Create(DWORD dwStyle, const CRect& rect, CWnd* pParentWnd, UINT nID)
    {
        CString className = AfxRegisterWndClass(
            CS_HREDRAW | CS_VREDRAW,
            LoadCursor(nullptr, IDC_HAND),
            nullptr,
            nullptr
        );

        return CWnd::Create(className, nullptr, dwStyle | WS_CHILD | WS_TABSTOP, rect, pParentWnd, nID);
    }

    void CMSlider::SetValue(int value)
    {
        // 범위 및 스텝 제한
        value = max(m_minValue, min(m_maxValue, value));

        // 스텝에 맞게 조정
        if (m_step > 1)
        {
            int steps = (value - m_minValue) / m_step;
            value = m_minValue + steps * m_step;
        }

        if (m_value != value)
        {
            int oldValue = m_value;
            m_value = value;
            Invalidate();

            // 이벤트 발생
            if (m_valueChangedHandler)
            {
                m_valueChangedHandler(m_valueChangedContext, oldValue, m_value);
            }
        }
    }

    void CMSlider::SetRange(int minValue, int maxValue)
    {
        m_minValue = minValue;
        m_maxValue = maxValue;

        // 현재 값이 범위를 벗어나면 조정
        m_value = max(m_minValue, min(m_maxValue, m_value));
        Invalidate();
    }

    void CMSlider::SetStep(int step)
    {
        m_step = max(1, step);
    }

    void CMSlider::SetOrientation(Orientation orientation)
    {
        if (m_orientation != orientation)
        {
            m_orientation = orientation;
            Invalidate();
        }
    }

    void CMSlider::SetShowTicks(BOOL show)
    {
        if (m_showTicks != show)
        {
            m_showTicks = show;
            Invalidate();
        }
    }

    void CMSlider::SetShowValue(BOOL show)
    {
        if (m_showValue != show)
        {
            m_showValue = show;
            Invalidate();
        }
    }

    void CMSlider::SetValueChangedHandler(SliderValueChangedHandler handler, void* context)
    {
        m_valueChangedHandler = handler;
        m_valueChangedContext = context;
    }

    void CMSlider::SetValueChangingHandler(SliderValueChangingHandler handler, void* context)
    {
        m_valueChangingHandler = handler;
        m_valueChangingContext = context;
    }

    void CMSlider::OnDraw(CDC* pDC)
    {
        // Direct2D 렌더링 시작
        if (!BeginD2DDraw())
            return;

        CRect rect;
        GetClientRect(&rect);

        const ThemeColors& colors = GetColors();

        // 트랙 그리기
        CRect trackRect = GetTrackRect();
        DrawTrackD2D(trackRect);

        // 눈금 그리기
        if (m_showTicks)
        {
            DrawTicksD2D(trackRect);
        }

        // 썸 그리기
        CRect thumbRect = GetThumbRect();
        DrawThumbD2D(thumbRect);

        // 값 레이블 그리기
        if (m_showValue && (m_dragging || m_isHovered))
        {
            DrawValueLabelD2D(thumbRect);
        }

        // 포커스 표시
        if (m_isFocused)
        {
            CRect focusRect = thumbRect;
            focusRect.InflateRect(2, 2);
            DrawCircleD2D(focusRect, CLR_INVALID, colors.primary.normal, 1);
        }

        EndD2DDraw();
    }

    void CMSlider::DrawTrackD2D(const CRect& trackRect)
    {
        if (!m_pRenderTarget || !m_isD2DDrawing)
            return;

        const ThemeColors& colors = GetColors();
        int radius = TRACK_THICKNESS / 2;

        COLORREF trackColor = m_isEnabled ? colors.surface.hover : colors.surface.disabled;
        DrawRoundedRectD2D(trackRect, radius, trackColor);

        if (m_value > m_minValue)
        {
            CRect filledRect = trackRect;
            int thumbPos = ValueToPosition(m_value);

            if (m_orientation == Orientation::Horizontal)
                filledRect.right = thumbPos;
            else
                filledRect.top = thumbPos;

            COLORREF fillColor = m_isEnabled ? colors.primary.normal : colors.primary.disabled;
            DrawRoundedRectD2D(filledRect, radius, fillColor);
        }
    }

    void CMSlider::DrawThumbD2D(const CRect& thumbRect)
    {
        if (!m_pRenderTarget || !m_isD2DDrawing)
            return;

        const ThemeColors& colors = GetColors();
        int size = (m_isHovered || m_dragging) ? THUMB_HOVER_SIZE : THUMB_SIZE;

        CRect drawRect = thumbRect;
        int diff = (THUMB_HOVER_SIZE - size) / 2;
        drawRect.InflateRect(-diff, -diff);

        COLORREF thumbColor;
        if (!m_isEnabled)
            thumbColor = colors.surface.disabled;
        else if (m_dragging)
            thumbColor = colors.primary.pressed;
        else if (m_isHovered)
            thumbColor = colors.primary.hover;
        else
            thumbColor = colors.primary.normal;

        // 그림자
        if (m_isEnabled)
        {
            CRect shadowRect = drawRect;
            shadowRect.OffsetRect(1, 2);
            m_pSolidBrush->SetColor(ToD2DColor(RGB(0, 0, 0), 0.2f));
            D2D1_ELLIPSE shadowEllipse = D2D1::Ellipse(
                D2D1::Point2F(static_cast<float>(shadowRect.CenterPoint().x), static_cast<float>(shadowRect.CenterPoint().y)),
                static_cast<float>(shadowRect.Width()) / 2.0f,
                static_cast<float>(shadowRect.Height()) / 2.0f
            );
            m_pRenderTarget->FillEllipse(shadowEllipse, m_pSolidBrush);
        }

        DrawCircleD2D(drawRect, thumbColor);
    }

    void CMSlider::DrawTicksD2D(const CRect& trackRect)
    {
        if (!m_pRenderTarget || !m_isD2DDrawing)
            return;

        const ThemeColors& colors = GetColors();
        m_pSolidBrush->SetColor(ToD2DColor(colors.textSecondary));

        int tickCount = (m_maxValue - m_minValue) / m_step;
        if (tickCount > 20) tickCount = 20;

        for (int i = 0; i <= tickCount; i++)
        {
            int value = m_minValue + (i * (m_maxValue - m_minValue)) / tickCount;
            int pos = ValueToPosition(value);

            if (m_orientation == Orientation::Horizontal)
            {
                m_pRenderTarget->DrawLine(
                    D2D1::Point2F(static_cast<float>(pos), static_cast<float>(trackRect.bottom + 4)),
                    D2D1::Point2F(static_cast<float>(pos), static_cast<float>(trackRect.bottom + 8)),
                    m_pSolidBrush, 1.0f
                );
            }
            else
            {
                m_pRenderTarget->DrawLine(
                    D2D1::Point2F(static_cast<float>(trackRect.right + 4), static_cast<float>(pos)),
                    D2D1::Point2F(static_cast<float>(trackRect.right + 8), static_cast<float>(pos)),
                    m_pSolidBrush, 1.0f
                );
            }
        }
    }

    void CMSlider::DrawValueLabelD2D(const CRect& thumbRect)
    {
        if (!m_pRenderTarget || !m_isD2DDrawing)
            return;

        const ThemeColors& colors = GetColors();

        CString text;
        text.Format(_T("%d"), m_value);

        CRect labelRect;
        if (m_orientation == Orientation::Horizontal)
        {
            labelRect.left = thumbRect.CenterPoint().x - 20;
            labelRect.right = thumbRect.CenterPoint().x + 20;
            labelRect.bottom = thumbRect.top - 4;
            labelRect.top = labelRect.bottom - 20;
        }
        else
        {
            labelRect.left = thumbRect.right + 4;
            labelRect.right = labelRect.left + 40;
            labelRect.top = thumbRect.CenterPoint().y - 10;
            labelRect.bottom = labelRect.top + 20;
        }

        DrawRoundedRectD2D(labelRect, 4, colors.surface.normal);
        DrawTextD2D(text, labelRect, colors.text, TextStyle::Caption,
            DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    }

    CRect CMSlider::GetTrackRect()
    {
        CRect rect;
        GetClientRect(&rect);

        CRect trackRect;

        if (m_orientation == Orientation::Horizontal)
        {
            int padding = THUMB_HOVER_SIZE / 2;
            trackRect.left = rect.left + padding;
            trackRect.right = rect.right - padding;
            trackRect.top = rect.CenterPoint().y - TRACK_THICKNESS / 2;
            trackRect.bottom = trackRect.top + TRACK_THICKNESS;
        }
        else
        {
            int padding = THUMB_HOVER_SIZE / 2;
            trackRect.top = rect.top + padding;
            trackRect.bottom = rect.bottom - padding;
            trackRect.left = rect.CenterPoint().x - TRACK_THICKNESS / 2;
            trackRect.right = trackRect.left + TRACK_THICKNESS;
        }

        return trackRect;
    }

    CRect CMSlider::GetThumbRect()
    {
        CRect rect;
        GetClientRect(&rect);

        int pos = ValueToPosition(m_value);
        int halfSize = THUMB_HOVER_SIZE / 2;

        CRect thumbRect;

        if (m_orientation == Orientation::Horizontal)
        {
            thumbRect.left = pos - halfSize;
            thumbRect.right = pos + halfSize;
            thumbRect.top = rect.CenterPoint().y - halfSize;
            thumbRect.bottom = rect.CenterPoint().y + halfSize;
        }
        else
        {
            thumbRect.left = rect.CenterPoint().x - halfSize;
            thumbRect.right = rect.CenterPoint().x + halfSize;
            thumbRect.top = pos - halfSize;
            thumbRect.bottom = pos + halfSize;
        }

        return thumbRect;
    }

    int CMSlider::PositionToValue(int pos)
    {
        CRect trackRect = GetTrackRect();

        int trackStart, trackEnd;
        if (m_orientation == Orientation::Horizontal)
        {
            trackStart = trackRect.left;
            trackEnd = trackRect.right;
        }
        else
        {
            // 수직: 위가 max, 아래가 min
            trackStart = trackRect.bottom;
            trackEnd = trackRect.top;
        }

        if (trackEnd == trackStart)
            return m_minValue;

        float ratio = static_cast<float>(pos - trackStart) / static_cast<float>(trackEnd - trackStart);
        ratio = max(0.0f, min(1.0f, ratio));

        int value = m_minValue + static_cast<int>(ratio * (m_maxValue - m_minValue));

        // 스텝에 맞게 조정
        if (m_step > 1)
        {
            int steps = (value - m_minValue + m_step / 2) / m_step;
            value = m_minValue + steps * m_step;
        }

        return max(m_minValue, min(m_maxValue, value));
    }

    int CMSlider::ValueToPosition(int value)
    {
        CRect trackRect = GetTrackRect();

        if (m_maxValue == m_minValue)
        {
            return (m_orientation == Orientation::Horizontal) ? trackRect.left : trackRect.bottom;
        }

        float ratio = static_cast<float>(value - m_minValue) / static_cast<float>(m_maxValue - m_minValue);

        int trackStart, trackEnd;
        if (m_orientation == Orientation::Horizontal)
        {
            trackStart = trackRect.left;
            trackEnd = trackRect.right;
        }
        else
        {
            // 수직: 위가 max, 아래가 min
            trackStart = trackRect.bottom;
            trackEnd = trackRect.top;
        }

        return trackStart + static_cast<int>(ratio * (trackEnd - trackStart));
    }

    void CMSlider::SetValueFromPosition(int pos)
    {
        int newValue = PositionToValue(pos);

        if (newValue != m_value)
        {
            int oldValue = m_value;
            m_value = newValue;
            Invalidate();

            // ValueChanging 이벤트 (드래그 중)
            if (m_valueChangingHandler)
            {
                m_valueChangingHandler(m_valueChangingContext, m_value);
            }
        }
    }

    void CMSlider::OnLButtonDown(UINT nFlags, CPoint point)
    {
        if (!m_isEnabled)
        {
            CMControlBase::OnLButtonDown(nFlags, point);
            return;
        }

        SetFocus();
        SetCapture();
        m_dragging = TRUE;

        // 클릭 위치에 따라 값 설정
        int pos = (m_orientation == Orientation::Horizontal) ? point.x : point.y;
        SetValueFromPosition(pos);

        CMControlBase::OnLButtonDown(nFlags, point);
    }

    void CMSlider::OnLButtonUp(UINT nFlags, CPoint point)
    {
        if (m_dragging)
        {
            ReleaseCapture();
            m_dragging = FALSE;

            // ValueChanged 이벤트 (드래그 완료)
            // SetValueFromPosition에서 이미 처리됨

            Invalidate();
        }

        CMControlBase::OnLButtonUp(nFlags, point);
    }

    void CMSlider::OnMouseMove(UINT nFlags, CPoint point)
    {
        if (m_dragging)
        {
            int pos = (m_orientation == Orientation::Horizontal) ? point.x : point.y;
            SetValueFromPosition(pos);
        }

        CMControlBase::OnMouseMove(nFlags, point);
    }

    void CMSlider::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
    {
        if (!m_isEnabled)
        {
            CMControlBase::OnKeyDown(nChar, nRepCnt, nFlags);
            return;
        }

        int delta = 0;
        BOOL isEnd = FALSE;

        switch (nChar)
        {
        case VK_LEFT:
        case VK_DOWN:
            delta = -m_step;
            break;

        case VK_RIGHT:
        case VK_UP:
            delta = m_step;
            break;

        case VK_PRIOR:  // Page Up
            delta = m_step * 10;
            break;

        case VK_NEXT:   // Page Down
            delta = -m_step * 10;
            break;

        case VK_HOME:
            SetValue(m_minValue);
            return;

        case VK_END:
            SetValue(m_maxValue);
            return;
        }

        if (delta != 0)
        {
            SetValue(m_value + delta);
        }

        CMControlBase::OnKeyDown(nChar, nRepCnt, nFlags);
    }

    BOOL CMSlider::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
    {
        if (!m_isEnabled)
            return CMControlBase::OnMouseWheel(nFlags, zDelta, pt);

        int delta = (zDelta > 0) ? m_step : -m_step;
        SetValue(m_value + delta);

        return TRUE;
    }
}
