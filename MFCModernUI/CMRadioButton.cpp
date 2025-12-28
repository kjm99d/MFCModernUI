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
        CRect rect;
        GetClientRect(&rect);

        const ThemeColors& colors = GetColors();
        int circleSize = SizeConstants::RadioButtonSize;
        int spacing = 8;

        // 라디오 버튼 위치 계산
        CRect circleRect;
        CRect textRect = rect;

        circleRect.SetRect(
            rect.left,
            rect.top + (rect.Height() - circleSize) / 2,
            rect.left + circleSize,
            rect.top + (rect.Height() + circleSize) / 2
        );
        textRect.left = circleRect.right + spacing;

        // 배경
        pDC->FillSolidRect(rect, colors.background.normal);

        // 라디오 버튼 그리기
        DrawRadioButton(pDC, circleRect);

        // 텍스트 그리기
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

        // 외부 원 그리기
        CPen borderPen(PS_SOLID, borderWidth, borderColor);
        CBrush bgBrush(colors.surface.normal);

        CPen* pOldPen = pDC->SelectObject(&borderPen);
        CBrush* pOldBrush = pDC->SelectObject(&bgBrush);

        pDC->Ellipse(circleRect);

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

            CBrush innerBrush(innerColor);
            CPen innerPen(PS_NULL, 0, RGB(0, 0, 0));

            pDC->SelectObject(&innerPen);
            pDC->SelectObject(&innerBrush);

            pDC->Ellipse(
                cx - currentRadius,
                cy - currentRadius,
                cx + currentRadius,
                cy + currentRadius
            );
        }

        pDC->SelectObject(pOldPen);
        pDC->SelectObject(pOldBrush);
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
