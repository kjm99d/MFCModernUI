#include "pch.h"
#include "CMToggleSwitch.h"

namespace MFCModernUI
{
    IMPLEMENT_DYNAMIC(CMToggleSwitch, CMControlBase)

    BEGIN_MESSAGE_MAP(CMToggleSwitch, CMControlBase)
        ON_WM_LBUTTONUP()
        ON_WM_KEYDOWN()
    END_MESSAGE_MAP()

    CMToggleSwitch::CMToggleSwitch()
        : m_checked(FALSE)
        , m_knobPosition(0.0f)
        , m_toggleChangedHandler(nullptr)
        , m_toggleChangedContext(nullptr)
    {
        m_themeClass = _T("toggle");
    }

    CMToggleSwitch::~CMToggleSwitch()
    {
    }

    BOOL CMToggleSwitch::Create(DWORD dwStyle, const CRect& rect, CWnd* pParentWnd, UINT nID)
    {
        CString className = AfxRegisterWndClass(
            CS_HREDRAW | CS_VREDRAW,
            LoadCursor(nullptr, IDC_HAND),
            nullptr,
            nullptr
        );

        return CWnd::Create(className, nullptr, dwStyle | WS_CHILD, rect, pParentWnd, nID);
    }

    void CMToggleSwitch::SetChecked(BOOL checked)
    {
        if (m_checked != checked)
        {
            m_checked = checked;
            StartKnobAnimation();
        }
    }

    void CMToggleSwitch::SetText(const CString& text)
    {
        if (m_text != text)
        {
            m_text = text;
            Invalidate();
        }
    }

    void CMToggleSwitch::SetOnText(const CString& text)
    {
        m_onText = text;
        Invalidate();
    }

    void CMToggleSwitch::SetOffText(const CString& text)
    {
        m_offText = text;
        Invalidate();
    }

    void CMToggleSwitch::Toggle()
    {
        if (!m_enabled)
            return;

        m_checked = !m_checked;
        StartKnobAnimation();

        if (m_toggleChangedHandler)
        {
            m_toggleChangedHandler(m_toggleChangedContext, m_checked);
        }

        // 부모에게 알림
        CWnd* pParent = GetParent();
        if (pParent)
        {
            pParent->SendMessage(WM_COMMAND,
                MAKEWPARAM(GetDlgCtrlID(), BN_CLICKED),
                (LPARAM)m_hWnd);
        }
    }

    void CMToggleSwitch::SetToggleChangedHandler(ToggleChangedHandler handler, void* context)
    {
        m_toggleChangedHandler = handler;
        m_toggleChangedContext = context;
    }

    void CMToggleSwitch::StartKnobAnimation()
    {
        if (m_currentAnimationId > 0)
        {
            CMAnimationManager::GetInstance().StopAnimation(m_currentAnimationId);
        }

        float startPos = m_knobPosition;
        float endPos = m_checked ? 1.0f : 0.0f;

        m_currentAnimationId = CMAnimationManager::GetInstance().StartAnimation(
            m_hWnd,
            startPos,
            endPos,
            AnimationConstants::SlowDuration,
            EasingType::EaseOutCubic,
            [this](float progress) {
                m_knobPosition = progress;
                Invalidate();
            },
            [this]() {
                m_currentAnimationId = 0;
            }
        );
    }

    void CMToggleSwitch::OnDraw(CDC* pDC)
    {
        // Direct2D 렌더링 시작
        if (!BeginD2DDraw())
            return;

        CRect rect;
        GetClientRect(&rect);

        const ThemeColors& colors = GetColors();

        // 트랙 위치 계산
        CRect trackRect;
        trackRect.left = rect.left;
        trackRect.top = rect.top + (rect.Height() - TRACK_HEIGHT) / 2;
        trackRect.right = trackRect.left + TRACK_WIDTH;
        trackRect.bottom = trackRect.top + TRACK_HEIGHT;

        // 트랙 그리기 (Direct2D)
        DrawTrackD2D(trackRect);

        // 노브 그리기 (Direct2D)
        DrawKnobD2D(trackRect);

        // 텍스트 그리기
        if (!m_text.IsEmpty())
        {
            CRect textRect = rect;
            textRect.left = trackRect.right + 8;

            COLORREF textColor = m_enabled ? colors.text : colors.textDisabled;
            DrawTextD2D(m_text, textRect, textColor, TextStyle::Body,
                DT_LEFT | DT_VCENTER | DT_SINGLELINE);
        }

        EndD2DDraw();
    }

    void CMToggleSwitch::DrawTrackD2D(const CRect& trackRect)
    {
        if (!m_pRenderTarget || !m_isD2DDrawing)
            return;

        const ThemeColors& colors = GetColors();
        int radius = TRACK_HEIGHT / 2;

        COLORREF trackColor;
        if (!m_enabled)
        {
            trackColor = colors.surface.disabled;
        }
        else
        {
            COLORREF offColor = (m_state == ControlState::Hover) ? colors.border.hover : colors.border.normal;
            COLORREF onColor = (m_state == ControlState::Hover) ? colors.primary.hover : colors.primary.normal;
            trackColor = CMAnimationManager::InterpolateColor(offColor, onColor, m_knobPosition);
        }

        DrawRoundedRectD2D(trackRect, radius, trackColor);
    }

    void CMToggleSwitch::DrawKnobD2D(const CRect& trackRect)
    {
        if (!m_pRenderTarget || !m_isD2DDrawing)
            return;

        const ThemeColors& colors = GetColors();

        int minX = trackRect.left + KNOB_MARGIN;
        int maxX = trackRect.right - KNOB_MARGIN - KNOB_SIZE;
        int knobX = minX + static_cast<int>((maxX - minX) * m_knobPosition);
        int knobY = trackRect.top + KNOB_MARGIN;

        CRect knobRect(knobX, knobY, knobX + KNOB_SIZE, knobY + KNOB_SIZE);

        COLORREF knobColor = m_enabled ? RGB(255, 255, 255) : RGB(200, 200, 200);

        // 그림자
        CRect shadowRect = knobRect;
        shadowRect.OffsetRect(0, 1);
        m_pSolidBrush->SetColor(ToD2DColor(RGB(0, 0, 0), 0.16f));
        D2D1_ELLIPSE shadowEllipse = D2D1::Ellipse(
            D2D1::Point2F(static_cast<float>(shadowRect.CenterPoint().x), static_cast<float>(shadowRect.CenterPoint().y)),
            static_cast<float>(shadowRect.Width()) / 2.0f,
            static_cast<float>(shadowRect.Height()) / 2.0f
        );
        m_pRenderTarget->FillEllipse(shadowEllipse, m_pSolidBrush);

        // 노브
        DrawCircleD2D(knobRect, knobColor,
            m_enabled ? CLR_INVALID : colors.border.disabled,
            m_enabled ? 0 : 1);
    }

    void CMToggleSwitch::OnLButtonUp(UINT nFlags, CPoint point)
    {
        BOOL wasPressed = (m_state == ControlState::Pressed);

        CMControlBase::OnLButtonUp(nFlags, point);

        CRect rect;
        GetClientRect(&rect);

        if (wasPressed && rect.PtInRect(point) && m_enabled)
        {
            Toggle();
        }
    }

    void CMToggleSwitch::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
    {
        if (nChar == VK_SPACE && m_enabled)
        {
            Toggle();
        }

        CMControlBase::OnKeyDown(nChar, nRepCnt, nFlags);
    }
}
