#include "pch.h"
#include "CMCheckBox.h"

namespace MFCModernUI
{
    IMPLEMENT_DYNAMIC(CMCheckBox, CMControlBase)

    BEGIN_MESSAGE_MAP(CMCheckBox, CMControlBase)
        ON_WM_LBUTTONUP()
        ON_WM_KEYDOWN()
    END_MESSAGE_MAP()

    CMCheckBox::CMCheckBox()
        : m_checked(FALSE)
        , m_indeterminate(FALSE)
        , m_textPosition(Position::Right)
        , m_checkProgress(0.0f)
        , m_checkedChangedHandler(nullptr)
        , m_checkedChangedContext(nullptr)
    {
        m_themeClass = _T("checkbox");
    }

    CMCheckBox::~CMCheckBox()
    {
    }

    BOOL CMCheckBox::Create(const CString& text, DWORD dwStyle, const CRect& rect,
        CWnd* pParentWnd, UINT nID)
    {
        m_text = text;

        CString className = AfxRegisterWndClass(
            CS_HREDRAW | CS_VREDRAW,
            LoadCursor(nullptr, IDC_HAND),
            nullptr,
            nullptr
        );

        return CWnd::Create(className, nullptr, dwStyle | WS_CHILD, rect, pParentWnd, nID);
    }

    void CMCheckBox::SetChecked(BOOL checked)
    {
        if (m_checked != checked)
        {
            m_checked = checked;
            m_indeterminate = FALSE;
            StartCheckAnimation();
        }
    }

    void CMCheckBox::SetIndeterminate(BOOL indeterminate)
    {
        if (m_indeterminate != indeterminate)
        {
            m_indeterminate = indeterminate;
            if (indeterminate)
            {
                m_checked = FALSE;
            }
            Invalidate();
        }
    }

    void CMCheckBox::SetText(const CString& text)
    {
        if (m_text != text)
        {
            m_text = text;
            Invalidate();
        }
    }

    void CMCheckBox::SetTextPosition(Position position)
    {
        if (m_textPosition != position)
        {
            m_textPosition = position;
            Invalidate();
        }
    }

    void CMCheckBox::SetCheckedChangedHandler(CheckChangedHandler handler, void* context)
    {
        m_checkedChangedHandler = handler;
        m_checkedChangedContext = context;
    }

    void CMCheckBox::Toggle()
    {
        if (!m_enabled)
            return;

        m_indeterminate = FALSE;
        m_checked = !m_checked;
        StartCheckAnimation();

        if (m_checkedChangedHandler)
        {
            m_checkedChangedHandler(m_checkedChangedContext, m_checked);
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

    void CMCheckBox::SetState(ControlState newState)
    {
        CMControlBase::SetState(newState);
    }

    void CMCheckBox::StartCheckAnimation()
    {
        if (m_currentAnimationId > 0)
        {
            CMAnimationManager::GetInstance().StopAnimation(m_currentAnimationId);
        }

        float startProgress = m_checkProgress;
        float endProgress = m_checked ? 1.0f : 0.0f;

        m_currentAnimationId = CMAnimationManager::GetInstance().StartAnimation(
            m_hWnd,
            startProgress,
            endProgress,
            AnimationConstants::NormalDuration,
            EasingType::EaseOutQuad,
            [this](float progress) {
                m_checkProgress = progress;
                Invalidate();
            },
            [this]() {
                m_currentAnimationId = 0;
            }
        );
    }

    void CMCheckBox::OnDraw(CDC* pDC)
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
        int boxSize = SizeConstants::CheckBoxSize;
        int spacing = 8;

        // 체크박스 위치 계산
        CRect boxRect;
        CRect textRect = rect;

        if (m_textPosition == Position::Right)
        {
            boxRect.SetRect(
                rect.left,
                rect.top + (rect.Height() - boxSize) / 2,
                rect.left + boxSize,
                rect.top + (rect.Height() + boxSize) / 2
            );
            textRect.left = boxRect.right + spacing;
        }
        else
        {
            boxRect.SetRect(
                rect.right - boxSize,
                rect.top + (rect.Height() - boxSize) / 2,
                rect.right,
                rect.top + (rect.Height() + boxSize) / 2
            );
            textRect.right = boxRect.left - spacing;
        }

        // 체크박스 그리기 (Direct2D)
        DrawCheckBoxD2D(boxRect);

        // 텍스트 그리기
        if (!m_text.IsEmpty())
        {
            COLORREF textColor = m_enabled ? colors.text : colors.textDisabled;
            UINT format = DT_VCENTER | DT_SINGLELINE;

            if (m_textPosition == Position::Right)
                format |= DT_LEFT;
            else
                format |= DT_RIGHT;

            DrawTextD2D(m_text, textRect, textColor, TextStyle::Body, format);
        }

        EndD2DDraw();
    }

    void CMCheckBox::OnDrawGdiPlus(CDC* pDC)
    {
        CRect rect;
        GetClientRect(&rect);

        const ThemeColors& colors = GetColors();
        int boxSize = SizeConstants::CheckBoxSize;
        int spacing = 8;

        CRect boxRect;
        CRect textRect = rect;

        if (m_textPosition == Position::Right)
        {
            boxRect.SetRect(
                rect.left,
                rect.top + (rect.Height() - boxSize) / 2,
                rect.left + boxSize,
                rect.top + (rect.Height() + boxSize) / 2
            );
            textRect.left = boxRect.right + spacing;
        }
        else
        {
            boxRect.SetRect(
                rect.right - boxSize,
                rect.top + (rect.Height() - boxSize) / 2,
                rect.right,
                rect.top + (rect.Height() + boxSize) / 2
            );
            textRect.right = boxRect.left - spacing;
        }

        pDC->FillSolidRect(rect, colors.background.normal);
        DrawCheckBox(pDC, boxRect);

        if (!m_text.IsEmpty())
        {
            COLORREF textColor = m_enabled ? colors.text : colors.textDisabled;
            UINT format = DT_VCENTER | DT_SINGLELINE;
            if (m_textPosition == Position::Right)
                format |= DT_LEFT;
            else
                format |= DT_RIGHT;
            DrawText(pDC, m_text, textRect, textColor, TextStyle::Body, format);
        }
    }

    void CMCheckBox::DrawCheckBox(CDC* pDC, const CRect& boxRect)
    {
        const ThemeColors& colors = GetColors();
        int radius = 4;

        COLORREF bgColor;
        COLORREF borderColor;

        if (!m_enabled)
        {
            bgColor = colors.surface.disabled;
            borderColor = colors.border.disabled;
        }
        else if (m_checked || m_indeterminate)
        {
            // 체크된 상태
            switch (m_state)
            {
            case ControlState::Hover:
                bgColor = colors.primary.hover;
                break;
            case ControlState::Pressed:
                bgColor = colors.primary.pressed;
                break;
            default:
                bgColor = colors.primary.normal;
                break;
            }
            borderColor = CLR_INVALID;  // 테두리 없음
        }
        else
        {
            // 체크되지 않은 상태
            bgColor = colors.surface.normal;
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

        // 배경 그리기 (GDI+ 안티앨리어싱)
        int borderWidth = (m_state == ControlState::Focused && !m_checked && !m_indeterminate) ? 2 : 1;
        DrawRoundedRectGdiPlus(pDC, boxRect, radius, bgColor, borderColor, borderWidth);

        // 체크마크 또는 불확정 마크 그리기
        if (m_indeterminate)
        {
            DrawIndeterminateMark(pDC, boxRect);
        }
        else if (m_checkProgress > 0.0f)
        {
            DrawCheckMark(pDC, boxRect);
        }
    }

    void CMCheckBox::DrawCheckMark(CDC* pDC, const CRect& boxRect)
    {
        if (m_checkProgress <= 0.0f)
            return;

        // GDI+ 안티앨리어싱으로 체크마크 그리기
        Gdiplus::Graphics graphics(pDC->GetSafeHdc());
        graphics.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);

        COLORREF markColor = GetColors().textOnPrimary;
        Gdiplus::Pen pen(ToGdiplusColor(markColor), 2.0f);
        pen.SetLineCap(Gdiplus::LineCapRound, Gdiplus::LineCapRound, Gdiplus::DashCapRound);
        pen.SetLineJoin(Gdiplus::LineJoinRound);

        int cx = boxRect.CenterPoint().x;
        int cy = boxRect.CenterPoint().y;

        // 체크마크 포인트
        Gdiplus::PointF pts[3] = {
            Gdiplus::PointF(static_cast<Gdiplus::REAL>(cx - 5), static_cast<Gdiplus::REAL>(cy)),
            Gdiplus::PointF(static_cast<Gdiplus::REAL>(cx - 1), static_cast<Gdiplus::REAL>(cy + 4)),
            Gdiplus::PointF(static_cast<Gdiplus::REAL>(cx + 5), static_cast<Gdiplus::REAL>(cy - 4))
        };

        // 애니메이션 적용 - 선의 길이를 점진적으로 그리기
        if (m_checkProgress < 0.5f)
        {
            // 첫 번째 선 (왼쪽 아래로)
            float lineProgress = m_checkProgress * 2.0f;
            Gdiplus::REAL endX = pts[0].X + (pts[1].X - pts[0].X) * lineProgress;
            Gdiplus::REAL endY = pts[0].Y + (pts[1].Y - pts[0].Y) * lineProgress;

            graphics.DrawLine(&pen, pts[0].X, pts[0].Y, endX, endY);
        }
        else
        {
            // 첫 번째 선 완성
            graphics.DrawLine(&pen, pts[0], pts[1]);

            // 두 번째 선 (오른쪽 위로)
            float lineProgress = (m_checkProgress - 0.5f) * 2.0f;
            Gdiplus::REAL endX = pts[1].X + (pts[2].X - pts[1].X) * lineProgress;
            Gdiplus::REAL endY = pts[1].Y + (pts[2].Y - pts[1].Y) * lineProgress;

            graphics.DrawLine(&pen, pts[1].X, pts[1].Y, endX, endY);
        }
    }

    void CMCheckBox::DrawIndeterminateMark(CDC* pDC, const CRect& boxRect)
    {
        // GDI+ 안티앨리어싱으로 불확정 마크 그리기
        Gdiplus::Graphics graphics(pDC->GetSafeHdc());
        graphics.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);

        COLORREF markColor = GetColors().textOnPrimary;
        Gdiplus::Pen pen(ToGdiplusColor(markColor), 2.0f);
        pen.SetLineCap(Gdiplus::LineCapRound, Gdiplus::LineCapRound, Gdiplus::DashCapRound);

        int cx = boxRect.CenterPoint().x;
        int cy = boxRect.CenterPoint().y;

        // 가로선
        graphics.DrawLine(&pen,
            static_cast<Gdiplus::REAL>(cx - 5),
            static_cast<Gdiplus::REAL>(cy),
            static_cast<Gdiplus::REAL>(cx + 5),
            static_cast<Gdiplus::REAL>(cy));
    }

    void CMCheckBox::DrawCheckBoxD2D(const CRect& boxRect)
    {
        if (!m_pRenderTarget || !m_isD2DDrawing)
            return;

        const ThemeColors& colors = GetColors();
        int radius = 4;

        COLORREF bgColor;
        COLORREF borderColor;

        if (!m_enabled)
        {
            bgColor = colors.surface.disabled;
            borderColor = colors.border.disabled;
        }
        else if (m_checked || m_indeterminate)
        {
            switch (m_state)
            {
            case ControlState::Hover:
                bgColor = colors.primary.hover;
                break;
            case ControlState::Pressed:
                bgColor = colors.primary.pressed;
                break;
            default:
                bgColor = colors.primary.normal;
                break;
            }
            borderColor = CLR_INVALID;
        }
        else
        {
            bgColor = colors.surface.normal;
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

        int borderWidth = (m_state == ControlState::Focused && !m_checked && !m_indeterminate) ? 2 : 1;
        DrawRoundedRectD2D(boxRect, radius, bgColor, borderColor, borderWidth);

        if (m_indeterminate)
        {
            DrawIndeterminateMarkD2D(boxRect);
        }
        else if (m_checkProgress > 0.0f)
        {
            DrawCheckMarkD2D(boxRect);
        }
    }

    void CMCheckBox::DrawCheckMarkD2D(const CRect& boxRect)
    {
        if (!m_pRenderTarget || !m_isD2DDrawing || m_checkProgress <= 0.0f)
            return;

        COLORREF markColor = GetColors().textOnPrimary;
        m_pSolidBrush->SetColor(ToD2DColor(markColor));

        float cx = static_cast<float>(boxRect.CenterPoint().x);
        float cy = static_cast<float>(boxRect.CenterPoint().y);

        // 체크마크 포인트
        D2D1_POINT_2F pts[3] = {
            D2D1::Point2F(cx - 5, cy),
            D2D1::Point2F(cx - 1, cy + 4),
            D2D1::Point2F(cx + 5, cy - 4)
        };

        // 스트로크 스타일 (둥근 끝)
        ID2D1Factory* pFactory = nullptr;
        m_pRenderTarget->GetFactory(&pFactory);

        D2D1_STROKE_STYLE_PROPERTIES strokeProps = D2D1::StrokeStyleProperties();
        strokeProps.startCap = D2D1_CAP_STYLE_ROUND;
        strokeProps.endCap = D2D1_CAP_STYLE_ROUND;
        strokeProps.lineJoin = D2D1_LINE_JOIN_ROUND;

        ID2D1StrokeStyle* pStrokeStyle = nullptr;
        pFactory->CreateStrokeStyle(strokeProps, nullptr, 0, &pStrokeStyle);

        if (m_checkProgress < 0.5f)
        {
            float lineProgress = m_checkProgress * 2.0f;
            float endX = pts[0].x + (pts[1].x - pts[0].x) * lineProgress;
            float endY = pts[0].y + (pts[1].y - pts[0].y) * lineProgress;

            m_pRenderTarget->DrawLine(pts[0], D2D1::Point2F(endX, endY), m_pSolidBrush, 2.0f, pStrokeStyle);
        }
        else
        {
            m_pRenderTarget->DrawLine(pts[0], pts[1], m_pSolidBrush, 2.0f, pStrokeStyle);

            float lineProgress = (m_checkProgress - 0.5f) * 2.0f;
            float endX = pts[1].x + (pts[2].x - pts[1].x) * lineProgress;
            float endY = pts[1].y + (pts[2].y - pts[1].y) * lineProgress;

            m_pRenderTarget->DrawLine(pts[1], D2D1::Point2F(endX, endY), m_pSolidBrush, 2.0f, pStrokeStyle);
        }

        if (pStrokeStyle)
            pStrokeStyle->Release();
    }

    void CMCheckBox::DrawIndeterminateMarkD2D(const CRect& boxRect)
    {
        if (!m_pRenderTarget || !m_isD2DDrawing)
            return;

        COLORREF markColor = GetColors().textOnPrimary;
        m_pSolidBrush->SetColor(ToD2DColor(markColor));

        float cx = static_cast<float>(boxRect.CenterPoint().x);
        float cy = static_cast<float>(boxRect.CenterPoint().y);

        ID2D1Factory* pFactory = nullptr;
        m_pRenderTarget->GetFactory(&pFactory);

        D2D1_STROKE_STYLE_PROPERTIES strokeProps = D2D1::StrokeStyleProperties();
        strokeProps.startCap = D2D1_CAP_STYLE_ROUND;
        strokeProps.endCap = D2D1_CAP_STYLE_ROUND;

        ID2D1StrokeStyle* pStrokeStyle = nullptr;
        pFactory->CreateStrokeStyle(strokeProps, nullptr, 0, &pStrokeStyle);

        m_pRenderTarget->DrawLine(
            D2D1::Point2F(cx - 5, cy),
            D2D1::Point2F(cx + 5, cy),
            m_pSolidBrush, 2.0f, pStrokeStyle
        );

        if (pStrokeStyle)
            pStrokeStyle->Release();
    }

    void CMCheckBox::OnLButtonUp(UINT nFlags, CPoint point)
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

    void CMCheckBox::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
    {
        if (nChar == VK_SPACE && m_enabled)
        {
            Toggle();
        }

        CMControlBase::OnKeyDown(nChar, nRepCnt, nFlags);
    }
}
