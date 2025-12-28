#include "pch.h"
#include "CMButton.h"
#include <math.h>

#define LOADING_TIMER_ID 1001
#define PI 3.14159265358979323846

namespace MFCModernUI
{
    IMPLEMENT_DYNAMIC(CMButton, CMControlBase)

    BEGIN_MESSAGE_MAP(CMButton, CMControlBase)
        ON_WM_LBUTTONUP()
        ON_WM_LBUTTONDBLCLK()
        ON_WM_KEYDOWN()
        ON_WM_KEYUP()
        ON_WM_TIMER()
    END_MESSAGE_MAP()

    CMButton::CMButton()
        : m_buttonStyle(ButtonStyle::Primary)
        , m_iconId(0)
        , m_iconPosition(IconPosition::Left)
        , m_iconOnly(FALSE)
        , m_loading(FALSE)
        , m_cornerRadius(-1)  // -1 = 테마 기본값 사용
        , m_scale(1.0f)
        , m_loadingAngle(0)
        , m_loadingTimerId(0)
        , m_clickHandler(nullptr)
        , m_clickContext(nullptr)
    {
        m_themeClass = _T("button");
        UpdateColors();
    }

    CMButton::~CMButton()
    {
        if (m_loadingTimerId)
        {
            KillTimer(m_loadingTimerId);
        }
    }

    BOOL CMButton::Create(const CString& text, DWORD dwStyle, const CRect& rect,
        CWnd* pParentWnd, UINT nID)
    {
        m_text = text;

        // 윈도우 클래스 등록
        CString className = AfxRegisterWndClass(
            CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS,
            LoadCursor(nullptr, IDC_HAND),
            nullptr,
            nullptr
        );

        return CWnd::Create(className, nullptr, dwStyle | WS_CHILD, rect, pParentWnd, nID);
    }

    void CMButton::SetText(const CString& text)
    {
        if (m_text != text)
        {
            m_text = text;
            Invalidate();
        }
    }

    void CMButton::SetButtonStyle(ButtonStyle style)
    {
        if (m_buttonStyle != style)
        {
            m_buttonStyle = style;
            UpdateColors();
            Invalidate();
        }
    }

    void CMButton::SetIcon(UINT iconId, IconPosition position)
    {
        m_iconId = iconId;
        m_iconPosition = position;
        Invalidate();
    }

    void CMButton::SetIconOnly(BOOL iconOnly)
    {
        if (m_iconOnly != iconOnly)
        {
            m_iconOnly = iconOnly;
            Invalidate();
        }
    }

    void CMButton::SetLoading(BOOL loading)
    {
        if (m_loading != loading)
        {
            m_loading = loading;

            if (loading)
            {
                m_loadingTimerId = SetTimer(LOADING_TIMER_ID, 50, nullptr);
            }
            else if (m_loadingTimerId)
            {
                KillTimer(m_loadingTimerId);
                m_loadingTimerId = 0;
                m_loadingAngle = 0;
            }

            Invalidate();
        }
    }

    void CMButton::SetCornerRadius(int radius)
    {
        if (m_cornerRadius != radius)
        {
            m_cornerRadius = radius;
            Invalidate();
        }
    }

    void CMButton::SetClickHandler(ButtonClickHandler handler, void* context)
    {
        m_clickHandler = handler;
        m_clickContext = context;
    }

    const ColorToken& CMButton::GetStyleColors() const
    {
        const ThemeColors& colors = GetColors();

        switch (m_buttonStyle)
        {
        case ButtonStyle::Primary:
            return colors.primary;
        case ButtonStyle::Secondary:
            return colors.secondary;
        case ButtonStyle::Success:
            return colors.success;
        case ButtonStyle::Danger:
            return colors.danger;
        case ButtonStyle::Warning:
            return colors.warning;
        case ButtonStyle::Info:
            return colors.info;
        case ButtonStyle::Light:
            return colors.surface;
        case ButtonStyle::Dark:
            return colors.secondary;
        case ButtonStyle::Outline:
        case ButtonStyle::Ghost:
            return colors.primary;
        default:
            return colors.primary;
        }
    }

    COLORREF CMButton::GetTextColor() const
    {
        const ThemeColors& colors = GetColors();

        if (!m_enabled)
        {
            return colors.textDisabled;
        }

        switch (m_buttonStyle)
        {
        case ButtonStyle::Light:
            return colors.text;
        case ButtonStyle::Outline:
        case ButtonStyle::Ghost:
            return GetStyleColors().normal;
        default:
            return colors.textOnPrimary;
        }
    }

    void CMButton::UpdateColors()
    {
        const ColorToken& styleColors = GetStyleColors();

        switch (m_state)
        {
        case ControlState::Normal:
            m_targetBgColor = (m_buttonStyle == ButtonStyle::Outline || m_buttonStyle == ButtonStyle::Ghost)
                ? GetColors().surface.normal : styleColors.normal;
            break;
        case ControlState::Hover:
            m_targetBgColor = (m_buttonStyle == ButtonStyle::Outline || m_buttonStyle == ButtonStyle::Ghost)
                ? GetColors().surface.hover : styleColors.hover;
            break;
        case ControlState::Pressed:
            m_targetBgColor = (m_buttonStyle == ButtonStyle::Outline || m_buttonStyle == ButtonStyle::Ghost)
                ? GetColors().surface.pressed : styleColors.pressed;
            break;
        case ControlState::Disabled:
            m_targetBgColor = styleColors.disabled;
            break;
        default:
            m_targetBgColor = styleColors.normal;
            break;
        }

        m_currentBgColor = m_targetBgColor;
    }

    void CMButton::SetState(ControlState newState)
    {
        if (m_state != newState)
        {
            COLORREF oldColor = m_currentBgColor;
            CMControlBase::SetState(newState);
            UpdateColors();

            // 애니메이션 시작
            StartColorAnimation(m_targetBgColor);

            // 스케일 애니메이션
            if (newState == ControlState::Pressed)
            {
                m_scale = 0.98f;
            }
            else
            {
                m_scale = 1.0f;
            }
        }
    }

    void CMButton::StartColorAnimation(COLORREF targetColor)
    {
        if (m_currentAnimationId > 0)
        {
            CMAnimationManager::GetInstance().StopAnimation(m_currentAnimationId);
        }

        COLORREF startColor = m_currentBgColor;

        m_currentAnimationId = CMAnimationManager::GetInstance().StartAnimation(
            m_hWnd,
            0.0f,
            1.0f,
            AnimationConstants::NormalDuration,
            EasingType::EaseOutQuad,
            [this, startColor, targetColor](float progress) {
                m_currentBgColor = CMAnimationManager::InterpolateColor(startColor, targetColor, progress);
                Invalidate();
            },
            [this]() {
                m_currentAnimationId = 0;
            }
        );
    }

    void CMButton::OnDraw(CDC* pDC)
    {
        CRect rect;
        GetClientRect(&rect);

        // 코너 라디우스
        int radius = (m_cornerRadius >= 0) ? m_cornerRadius : GetThemeManager().GetCornerRadius();

        // 스케일 적용 (중앙 기준)
        if (m_scale < 1.0f)
        {
            int dx = static_cast<int>(rect.Width() * (1.0f - m_scale) / 2);
            int dy = static_cast<int>(rect.Height() * (1.0f - m_scale) / 2);
            rect.DeflateRect(dx, dy);
        }

        // 배경색
        COLORREF bgColor = m_currentBgColor;

        // 테두리 (Outline 스타일)
        COLORREF borderColor = CLR_INVALID;
        int borderWidth = 0;

        if (m_buttonStyle == ButtonStyle::Outline)
        {
            borderColor = GetStyleColors().normal;
            borderWidth = 1;

            if (m_state == ControlState::Focused)
            {
                borderColor = GetColors().borderFocus;
                borderWidth = 2;
            }
        }
        else if (m_state == ControlState::Focused)
        {
            borderColor = GetColors().borderFocus;
            borderWidth = 2;
        }

        // 배경 그리기 (GDI+ 안티앨리어싱)
        DrawRoundedRectGdiPlus(pDC, rect, radius, bgColor, borderColor, borderWidth);

        // 로딩 상태
        if (m_loading)
        {
            DrawLoadingSpinner(pDC, rect);
            return;
        }

        // 텍스트/아이콘 그리기
        COLORREF textColor = GetTextColor();

        if (!m_iconOnly && !m_text.IsEmpty())
        {
            CRect textRect = rect;
            textRect.DeflateRect(GetPadding(), 0);

            // 아이콘이 있는 경우 위치 조정
            if (m_iconId != 0)
            {
                // TODO: 아이콘 그리기 구현
            }

            // 텍스트 스타일 결정
            TextStyle textStyle = TextStyle::Body;
            switch (m_controlSize)
            {
            case ControlSize::Small:
                textStyle = TextStyle::BodySmall;
                break;
            case ControlSize::Large:
                textStyle = TextStyle::H6;
                break;
            }

            DrawText(pDC, m_text, textRect, textColor, textStyle,
                DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        }
    }

    void CMButton::DrawLoadingSpinner(CDC* pDC, const CRect& rect)
    {
        Gdiplus::Graphics graphics(pDC->GetSafeHdc());
        graphics.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);

        int centerX = rect.CenterPoint().x;
        int centerY = rect.CenterPoint().y;
        int radius = min(rect.Width(), rect.Height()) / 4;

        // 스피너 펜 (GDI+)
        COLORREF textColor = GetTextColor();
        Gdiplus::Pen pen(ToGdiplusColor(textColor), 2.0f);
        pen.SetLineCap(Gdiplus::LineCapRound, Gdiplus::LineCapRound, Gdiplus::DashCapRound);

        // 스피너 그리기 (부분 원호 - 270도)
        Gdiplus::RectF arcRect(
            static_cast<Gdiplus::REAL>(centerX - radius),
            static_cast<Gdiplus::REAL>(centerY - radius),
            static_cast<Gdiplus::REAL>(radius * 2),
            static_cast<Gdiplus::REAL>(radius * 2)
        );

        // GDI+에서 각도는 3시 방향이 0도, 시계방향
        graphics.DrawArc(&pen, arcRect,
            static_cast<Gdiplus::REAL>(m_loadingAngle),
            270.0f);
    }

    void CMButton::OnLButtonUp(UINT nFlags, CPoint point)
    {
        BOOL wasPressed = (m_state == ControlState::Pressed);

        CMControlBase::OnLButtonUp(nFlags, point);

        CRect rect;
        GetClientRect(&rect);

        if (wasPressed && rect.PtInRect(point) && m_enabled && !m_loading)
        {
            // 클릭 이벤트 발생
            if (m_clickHandler)
            {
                m_clickHandler(m_clickContext);
            }

            // 부모에게 BN_CLICKED 알림
            CWnd* pParent = GetParent();
            if (pParent)
            {
                pParent->SendMessage(WM_COMMAND,
                    MAKEWPARAM(GetDlgCtrlID(), BN_CLICKED),
                    (LPARAM)m_hWnd);
            }
        }
    }

    void CMButton::OnLButtonDblClk(UINT nFlags, CPoint point)
    {
        // 더블클릭을 단일 클릭처럼 처리
        OnLButtonDown(nFlags, point);
    }

    void CMButton::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
    {
        if ((nChar == VK_SPACE || nChar == VK_RETURN) && m_enabled && !m_loading)
        {
            SetState(ControlState::Pressed);
        }

        CMControlBase::OnKeyDown(nChar, nRepCnt, nFlags);
    }

    void CMButton::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
    {
        if ((nChar == VK_SPACE || nChar == VK_RETURN) && m_state == ControlState::Pressed)
        {
            SetState(ControlState::Focused);

            // 클릭 이벤트 발생
            if (m_clickHandler)
            {
                m_clickHandler(m_clickContext);
            }

            // 부모에게 BN_CLICKED 알림
            CWnd* pParent = GetParent();
            if (pParent)
            {
                pParent->SendMessage(WM_COMMAND,
                    MAKEWPARAM(GetDlgCtrlID(), BN_CLICKED),
                    (LPARAM)m_hWnd);
            }
        }

        CMControlBase::OnKeyUp(nChar, nRepCnt, nFlags);
    }

    void CMButton::OnTimer(UINT_PTR nIDEvent)
    {
        if (nIDEvent == LOADING_TIMER_ID)
        {
            m_loadingAngle = (m_loadingAngle + 15) % 360;
            Invalidate();
        }

        CMControlBase::OnTimer(nIDEvent);
    }
}
