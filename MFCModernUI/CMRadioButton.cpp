#include "pch.h"
#include "CMRadioButton.h"
#include <algorithm>

namespace MFCModernUI
{
    // CMRadioGroup 구현
    CMRadioGroup& CMRadioGroup::GetInstance()
    {
        static CMRadioGroup instance;
        return instance;
    }

    void CMRadioGroup::RegisterRadio(int groupId, CMRadioButton* radio)
    {
        m_radios.push_back(std::make_pair(groupId, radio));
    }

    void CMRadioGroup::UnregisterRadio(CMRadioButton* radio)
    {
        m_radios.erase(
            std::remove_if(m_radios.begin(), m_radios.end(),
                [radio](const std::pair<int, CMRadioButton*>& pair) {
                    return pair.second == radio;
                }),
            m_radios.end()
        );
    }

    void CMRadioGroup::SelectRadio(int groupId, CMRadioButton* selected)
    {
        for (auto& pair : m_radios)
        {
            if (pair.first == groupId && pair.second != selected)
            {
                pair.second->SetSelected(FALSE);
            }
        }
    }

    CMRadioButton* CMRadioGroup::GetSelectedRadio(int groupId)
    {
        for (auto& pair : m_radios)
        {
            if (pair.first == groupId && pair.second->IsSelected())
            {
                return pair.second;
            }
        }
        return nullptr;
    }

    // CMRadioButton 구현
    IMPLEMENT_DYNAMIC(CMRadioButton, CMControlBase)

    BEGIN_MESSAGE_MAP(CMRadioButton, CMControlBase)
        ON_WM_LBUTTONUP()
        ON_WM_KEYDOWN()
    END_MESSAGE_MAP()

    CMRadioButton::CMRadioButton()
        : m_selected(FALSE)
        , m_groupId(0)
        , m_innerCircleScale(0.0f)
        , m_selectionChangedHandler(nullptr)
        , m_selectionChangedContext(nullptr)
    {
        m_themeClass = _T("radio");
    }

    CMRadioButton::~CMRadioButton()
    {
        CMRadioGroup::GetInstance().UnregisterRadio(this);
    }

    BOOL CMRadioButton::Create(const CString& text, DWORD dwStyle, const CRect& rect,
        CWnd* pParentWnd, UINT nID, int groupId)
    {
        m_text = text;
        m_groupId = groupId;

        CString className = AfxRegisterWndClass(
            CS_HREDRAW | CS_VREDRAW,
            LoadCursor(nullptr, IDC_HAND),
            nullptr,
            nullptr
        );

        BOOL result = CWnd::Create(className, nullptr, dwStyle | WS_CHILD, rect, pParentWnd, nID);

        if (result)
        {
            CMRadioGroup::GetInstance().RegisterRadio(groupId, this);
        }

        return result;
    }

    void CMRadioButton::SetSelected(BOOL selected)
    {
        if (m_selected != selected)
        {
            m_selected = selected;

            if (selected)
            {
                // 같은 그룹의 다른 라디오 버튼 해제
                CMRadioGroup::GetInstance().SelectRadio(m_groupId, this);

                if (m_selectionChangedHandler)
                {
                    m_selectionChangedHandler(m_selectionChangedContext, this);
                }
            }

            StartSelectionAnimation();
        }
    }

    void CMRadioButton::SetText(const CString& text)
    {
        if (m_text != text)
        {
            m_text = text;
            Invalidate();
        }
    }

    void CMRadioButton::SetSelectionChangedHandler(RadioSelectionChangedHandler handler, void* context)
    {
        m_selectionChangedHandler = handler;
        m_selectionChangedContext = context;
    }

    void CMRadioButton::SetState(ControlState newState)
    {
        CMControlBase::SetState(newState);
    }

    void CMRadioButton::StartSelectionAnimation()
    {
        if (m_currentAnimationId > 0)
        {
            CMAnimationManager::GetInstance().StopAnimation(m_currentAnimationId);
        }

        float startScale = m_innerCircleScale;
        float endScale = m_selected ? 1.0f : 0.0f;

        m_currentAnimationId = CMAnimationManager::GetInstance().StartAnimation(
            m_hWnd,
            startScale,
            endScale,
            m_selected ? AnimationConstants::NormalDuration : AnimationConstants::FastDuration,
            EasingType::EaseOutQuad,
            [this](float progress) {
                m_innerCircleScale = progress;
                Invalidate();
            },
            [this]() {
                m_currentAnimationId = 0;
            }
        );
    }

    void CMRadioButton::OnDraw(CDC* pDC)
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
        int circleSize = SizeConstants::RadioButtonSize;
        int spacing = 8;

        CRect circleRect;
        CRect textRect = rect;

        circleRect.SetRect(
            rect.left,
            rect.top + (rect.Height() - circleSize) / 2,
            rect.left + circleSize,
            rect.top + (rect.Height() + circleSize) / 2
        );
        textRect.left = circleRect.right + spacing;

        // 라디오 버튼 그리기 (Direct2D)
        DrawRadioButtonD2D(circleRect);

        // 텍스트 그리기
        if (!m_text.IsEmpty())
        {
            COLORREF textColor = m_enabled ? colors.text : colors.textDisabled;
            DrawTextD2D(m_text, textRect, textColor, TextStyle::Body,
                DT_LEFT | DT_VCENTER | DT_SINGLELINE);
        }

        EndD2DDraw();
    }

    void CMRadioButton::OnDrawGdiPlus(CDC* pDC)
    {
        CRect rect;
        GetClientRect(&rect);

        const ThemeColors& colors = GetColors();
        int circleSize = SizeConstants::RadioButtonSize;
        int spacing = 8;

        CRect circleRect;
        CRect textRect = rect;

        circleRect.SetRect(
            rect.left,
            rect.top + (rect.Height() - circleSize) / 2,
            rect.left + circleSize,
            rect.top + (rect.Height() + circleSize) / 2
        );
        textRect.left = circleRect.right + spacing;

        pDC->FillSolidRect(rect, colors.background.normal);
        DrawRadioButton(pDC, circleRect);

        if (!m_text.IsEmpty())
        {
            COLORREF textColor = m_enabled ? colors.text : colors.textDisabled;
            DrawText(pDC, m_text, textRect, textColor, TextStyle::Body,
                DT_LEFT | DT_VCENTER | DT_SINGLELINE);
        }
    }

    void CMRadioButton::DrawRadioButton(CDC* pDC, const CRect& circleRect)
    {
        const ThemeColors& colors = GetColors();
        int cx = circleRect.CenterPoint().x;
        int cy = circleRect.CenterPoint().y;
        int outerRadius = circleRect.Width() / 2;
        int innerRadius = SizeConstants::RadioInnerSize / 2;

        // 외부 원
        COLORREF borderColor;
        int borderWidth = 2;

        if (!m_enabled)
        {
            borderColor = colors.border.disabled;
        }
        else if (m_selected)
        {
            switch (m_state)
            {
            case ControlState::Hover:
                borderColor = colors.primary.hover;
                break;
            default:
                borderColor = colors.primary.normal;
                break;
            }
        }
        else
        {
            switch (m_state)
            {
            case ControlState::Hover:
                borderColor = colors.border.hover;
                break;
            case ControlState::Focused:
                borderColor = colors.borderFocus;
                break;
            default:
                borderColor = colors.border.normal;
                break;
            }
        }

        // GDI+로 외부 원 그리기 (안티앨리어싱)
        DrawCircleGdiPlus(pDC, circleRect, colors.surface.normal, borderColor, borderWidth);

        // 내부 원 (선택됨)
        if (m_innerCircleScale > 0.0f)
        {
            COLORREF innerColor;

            if (!m_enabled)
            {
                innerColor = colors.textDisabled;
            }
            else
            {
                switch (m_state)
                {
                case ControlState::Hover:
                    innerColor = colors.primary.hover;
                    break;
                default:
                    innerColor = colors.primary.normal;
                    break;
                }
            }

            int currentRadius = static_cast<int>(innerRadius * m_innerCircleScale);

            // GDI+로 내부 원 그리기
            CRect innerRect(
                cx - currentRadius,
                cy - currentRadius,
                cx + currentRadius,
                cy + currentRadius
            );
            DrawCircleGdiPlus(pDC, innerRect, innerColor);
        }
    }

    void CMRadioButton::DrawRadioButtonD2D(const CRect& circleRect)
    {
        if (!m_pRenderTarget || !m_isD2DDrawing)
            return;

        const ThemeColors& colors = GetColors();
        int cx = circleRect.CenterPoint().x;
        int cy = circleRect.CenterPoint().y;
        int innerRadius = SizeConstants::RadioInnerSize / 2;

        COLORREF borderColor;
        int borderWidth = 2;

        if (!m_enabled)
        {
            borderColor = colors.border.disabled;
        }
        else if (m_selected)
        {
            borderColor = (m_state == ControlState::Hover) ? colors.primary.hover : colors.primary.normal;
        }
        else
        {
            switch (m_state)
            {
            case ControlState::Hover:
                borderColor = colors.border.hover;
                break;
            case ControlState::Focused:
                borderColor = colors.borderFocus;
                break;
            default:
                borderColor = colors.border.normal;
                break;
            }
        }

        DrawCircleD2D(circleRect, colors.surface.normal, borderColor, borderWidth);

        if (m_innerCircleScale > 0.0f)
        {
            COLORREF innerColor;
            if (!m_enabled)
            {
                innerColor = colors.textDisabled;
            }
            else
            {
                innerColor = (m_state == ControlState::Hover) ? colors.primary.hover : colors.primary.normal;
            }

            int currentRadius = static_cast<int>(innerRadius * m_innerCircleScale);
            CRect innerRect(
                cx - currentRadius,
                cy - currentRadius,
                cx + currentRadius,
                cy + currentRadius
            );
            DrawCircleD2D(innerRect, innerColor);
        }
    }

    void CMRadioButton::OnLButtonUp(UINT nFlags, CPoint point)
    {
        BOOL wasPressed = (m_state == ControlState::Pressed);

        CMControlBase::OnLButtonUp(nFlags, point);

        CRect rect;
        GetClientRect(&rect);

        if (wasPressed && rect.PtInRect(point) && m_enabled && !m_selected)
        {
            SetSelected(TRUE);

            // 부모에게 알림
            CWnd* pParent = GetParent();
            if (pParent)
            {
                pParent->SendMessage(WM_COMMAND,
                    MAKEWPARAM(GetDlgCtrlID(), BN_CLICKED),
                    (LPARAM)m_hWnd);
            }
        }
    }

    void CMRadioButton::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
    {
        if (nChar == VK_SPACE && m_enabled && !m_selected)
        {
            SetSelected(TRUE);

            // 부모에게 알림
            CWnd* pParent = GetParent();
            if (pParent)
            {
                pParent->SendMessage(WM_COMMAND,
                    MAKEWPARAM(GetDlgCtrlID(), BN_CLICKED),
                    (LPARAM)m_hWnd);
            }
        }

        CMControlBase::OnKeyDown(nChar, nRepCnt, nFlags);
    }
}
