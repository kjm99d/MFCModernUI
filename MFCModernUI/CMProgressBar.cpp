#include "pch.h"
#include "CMProgressBar.h"

#define INDETERMINATE_TIMER_ID 1002

namespace MFCModernUI
{
    IMPLEMENT_DYNAMIC(CMProgressBar, CMControlBase)

    BEGIN_MESSAGE_MAP(CMProgressBar, CMControlBase)
        ON_WM_TIMER()
        ON_WM_DESTROY()
    END_MESSAGE_MAP()

    CMProgressBar::CMProgressBar()
        : m_value(0)
        , m_minValue(0)
        , m_maxValue(100)
        , m_indeterminate(FALSE)
        , m_showText(FALSE)
        , m_barColor(RGB(0, 0, 0))
        , m_useDefaultBarColor(TRUE)
        , m_trackColor(RGB(0, 0, 0))
        , m_useDefaultTrackColor(TRUE)
        , m_displayValue(0.0f)
        , m_indeterminatePosition(0.0f)
        , m_indeterminateTimerId(0)
    {
        m_themeClass = _T("progress");
        m_tabStop = FALSE;
    }

    CMProgressBar::~CMProgressBar()
    {
    }

    BOOL CMProgressBar::Create(DWORD dwStyle, const CRect& rect, CWnd* pParentWnd, UINT nID)
    {
        CString className = AfxRegisterWndClass(
            CS_HREDRAW | CS_VREDRAW,
            LoadCursor(nullptr, IDC_ARROW),
            nullptr,
            nullptr
        );

        return CWnd::Create(className, nullptr, dwStyle | WS_CHILD, rect, pParentWnd, nID);
    }

    void CMProgressBar::SetValue(int value)
    {
        // 범위 제한
        value = max(m_minValue, min(m_maxValue, value));

        if (m_value != value)
        {
            m_value = value;
            StartValueAnimation(GetProgress());
        }
    }

    void CMProgressBar::SetRange(int minValue, int maxValue)
    {
        m_minValue = minValue;
        m_maxValue = maxValue;

        // 현재 값이 범위를 벗어나면 조정
        m_value = max(m_minValue, min(m_maxValue, m_value));
        m_displayValue = GetProgress();
        Invalidate();
    }

    void CMProgressBar::SetIndeterminate(BOOL indeterminate)
    {
        if (m_indeterminate != indeterminate)
        {
            m_indeterminate = indeterminate;

            if (indeterminate)
            {
                m_indeterminatePosition = 0.0f;
                m_indeterminateTimerId = SetTimer(INDETERMINATE_TIMER_ID, 20, nullptr);
            }
            else if (m_indeterminateTimerId)
            {
                KillTimer(m_indeterminateTimerId);
                m_indeterminateTimerId = 0;
            }

            Invalidate();
        }
    }

    void CMProgressBar::SetShowText(BOOL showText)
    {
        if (m_showText != showText)
        {
            m_showText = showText;
            Invalidate();
        }
    }

    void CMProgressBar::SetBarColor(COLORREF color)
    {
        m_barColor = color;
        m_useDefaultBarColor = FALSE;
        Invalidate();
    }

    void CMProgressBar::UseDefaultBarColor()
    {
        m_useDefaultBarColor = TRUE;
        Invalidate();
    }

    void CMProgressBar::SetTrackColor(COLORREF color)
    {
        m_trackColor = color;
        m_useDefaultTrackColor = FALSE;
        Invalidate();
    }

    void CMProgressBar::UseDefaultTrackColor()
    {
        m_useDefaultTrackColor = TRUE;
        Invalidate();
    }

    float CMProgressBar::GetProgress() const
    {
        if (m_maxValue == m_minValue)
            return 0.0f;

        return static_cast<float>(m_value - m_minValue) / static_cast<float>(m_maxValue - m_minValue);
    }

    void CMProgressBar::StartValueAnimation(float targetValue)
    {
        if (m_currentAnimationId > 0)
        {
            CMAnimationManager::GetInstance().StopAnimation(m_currentAnimationId);
        }

        float startValue = m_displayValue;

        m_currentAnimationId = CMAnimationManager::GetInstance().StartAnimation(
            m_hWnd,
            startValue,
            targetValue,
            AnimationConstants::ValueChangeDuration,
            EasingType::EaseOutQuad,
            [this](float progress) {
                m_displayValue = progress;
                Invalidate();
            },
            [this]() {
                m_currentAnimationId = 0;
            }
        );
    }

    void CMProgressBar::OnDraw(CDC* pDC)
    {
        CRect rect;
        GetClientRect(&rect);

        const ThemeColors& colors = GetColors();
        int radius = rect.Height() / 2;  // 완전 둥근 모서리

        // 트랙 색상
        COLORREF trackColor = m_useDefaultTrackColor ? colors.surface.hover : m_trackColor;

        // 바 색상
        COLORREF barColor = m_useDefaultBarColor ? colors.primary.normal : m_barColor;

        // 트랙 그리기
        DrawRoundedRect(pDC, rect, radius, trackColor);

        if (m_indeterminate)
        {
            // 불확정 모드 - 움직이는 바
            int barWidth = rect.Width() / 3;

            // 위치 계산 (왕복 운동)
            float position = m_indeterminatePosition;
            if (position > 1.0f)
                position = 2.0f - position;

            int barLeft = static_cast<int>((rect.Width() - barWidth) * position);

            CRect barRect(
                rect.left + barLeft,
                rect.top,
                rect.left + barLeft + barWidth,
                rect.bottom
            );

            DrawRoundedRect(pDC, barRect, radius, barColor);
        }
        else
        {
            // 확정 모드 - 진행률에 따른 바
            if (m_displayValue > 0.0f)
            {
                int barWidth = static_cast<int>(rect.Width() * m_displayValue);
                barWidth = max(rect.Height(), barWidth);  // 최소 높이만큼

                CRect barRect(
                    rect.left,
                    rect.top,
                    rect.left + barWidth,
                    rect.bottom
                );

                DrawRoundedRect(pDC, barRect, radius, barColor);
            }

            // 퍼센트 텍스트
            if (m_showText)
            {
                int percent = static_cast<int>(m_displayValue * 100);
                CString text;
                text.Format(_T("%d%%"), percent);

                // 텍스트 색상 (바 위에 있으면 흰색, 아니면 기본)
                COLORREF textColor;
                if (m_displayValue > 0.5f)
                {
                    textColor = colors.textOnPrimary;
                }
                else
                {
                    textColor = colors.text;
                }

                DrawText(pDC, text, rect, textColor, TextStyle::BodySmall,
                    DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            }
        }
    }

    void CMProgressBar::OnTimer(UINT_PTR nIDEvent)
    {
        if (nIDEvent == INDETERMINATE_TIMER_ID)
        {
            // 불확정 애니메이션 업데이트
            m_indeterminatePosition += 0.02f;
            if (m_indeterminatePosition > 2.0f)
            {
                m_indeterminatePosition = 0.0f;
            }
            Invalidate();
        }

        CMControlBase::OnTimer(nIDEvent);
    }

    void CMProgressBar::OnDestroy()
    {
        if (m_indeterminateTimerId)
        {
            KillTimer(m_indeterminateTimerId);
            m_indeterminateTimerId = 0;
        }

        CMControlBase::OnDestroy();
    }
}
