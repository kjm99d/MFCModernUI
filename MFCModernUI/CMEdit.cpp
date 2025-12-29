#include "pch.h"
#include "CMEdit.h"

#define IDC_INNER_EDIT 2001
#define WM_EDIT_CHANGE (WM_USER + 100)

namespace MFCModernUI
{
    IMPLEMENT_DYNAMIC(CMEdit, CMControlBase)

    BEGIN_MESSAGE_MAP(CMEdit, CMControlBase)
        ON_WM_SIZE()
        ON_WM_SETFOCUS()
        ON_WM_KILLFOCUS()
        ON_WM_LBUTTONDOWN()
        ON_MESSAGE(WM_EDIT_CHANGE, OnEditChange)
        ON_CONTROL(EN_CHANGE, IDC_INNER_EDIT, OnInnerEditChange)
    END_MESSAGE_MAP()

    CMEdit::CMEdit()
        : m_maxLength(0)
        , m_multiline(FALSE)
        , m_password(FALSE)
        , m_readOnly(FALSE)
        , m_clearButton(FALSE)
        , m_validationState(ValidationState::None)
        , m_innerEditCreated(FALSE)
        , m_textChangedHandler(nullptr)
        , m_textChangedContext(nullptr)
        , m_clearButtonHover(FALSE)
    {
        m_themeClass = _T("edit");
        m_currentBorderColor = GetColors().border.normal;
        m_targetBorderColor = m_currentBorderColor;
    }

    CMEdit::~CMEdit()
    {
        if (m_innerEdit.m_hWnd)
        {
            m_innerEdit.DestroyWindow();
        }
    }

    BOOL CMEdit::Create(DWORD dwStyle, const CRect& rect, CWnd* pParentWnd, UINT nID)
    {
        CString className = AfxRegisterWndClass(
            CS_HREDRAW | CS_VREDRAW,
            LoadCursor(nullptr, IDC_IBEAM),
            nullptr,
            nullptr
        );

        BOOL result = CWnd::Create(className, nullptr, dwStyle | WS_CHILD, rect, pParentWnd, nID);

        if (result)
        {
            CreateInnerEdit();
        }

        return result;
    }

    void CMEdit::CreateInnerEdit()
    {
        if (m_innerEditCreated)
            return;

        DWORD editStyle = WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL;

        if (m_multiline)
        {
            editStyle |= ES_MULTILINE | ES_AUTOVSCROLL | ES_WANTRETURN;
            editStyle &= ~ES_AUTOHSCROLL;
        }

        if (m_password)
        {
            editStyle |= ES_PASSWORD;
        }

        if (m_readOnly)
        {
            editStyle |= ES_READONLY;
        }

        CRect rect;
        GetClientRect(&rect);
        rect.DeflateRect(GetPadding(), (rect.Height() - GetFontSize() - 4) / 2);

        // 지우기 버튼/유효성 아이콘 공간 확보
        if (m_clearButton || m_validationState != ValidationState::None)
        {
            rect.right -= 24;
        }

        m_innerEdit.Create(editStyle, rect, this, IDC_INNER_EDIT);

        // 폰트 설정
        m_innerEdit.SetFont(GetThemeManager().GetFont(TextStyle::Body));

        // 최대 길이
        if (m_maxLength > 0)
        {
            m_innerEdit.SetLimitText(m_maxLength);
        }

        m_innerEditCreated = TRUE;
    }

    void CMEdit::UpdateInnerEditPosition()
    {
        if (!m_innerEdit.m_hWnd)
            return;

        CRect rect;
        GetClientRect(&rect);

        int padding = GetPadding();
        int fontHeight = GetFontSize() + 4;

        rect.DeflateRect(padding, (rect.Height() - fontHeight) / 2);

        // 지우기 버튼/유효성 아이콘 공간
        if (IsClearButtonVisible() || m_validationState != ValidationState::None)
        {
            rect.right -= 24;
        }

        m_innerEdit.MoveWindow(rect);
    }

    void CMEdit::SetText(const CString& text)
    {
        if (m_innerEdit.m_hWnd)
        {
            m_innerEdit.SetWindowText(text);
        }
    }

    CString CMEdit::GetText() const
    {
        CString text;
        if (m_innerEdit.m_hWnd)
        {
            m_innerEdit.GetWindowText(text);
        }
        return text;
    }

    void CMEdit::SetPlaceholder(const CString& placeholder)
    {
        m_placeholder = placeholder;
        if (m_innerEdit.m_hWnd)
        {
            m_innerEdit.SetCueBanner(placeholder);
        }
    }

    void CMEdit::SetMaxLength(int maxLength)
    {
        m_maxLength = maxLength;
        if (m_innerEdit.m_hWnd && maxLength > 0)
        {
            m_innerEdit.SetLimitText(maxLength);
        }
    }

    void CMEdit::SetMultiline(BOOL multiline)
    {
        m_multiline = multiline;
        // 멀티라인 변경은 재생성 필요
    }

    void CMEdit::SetPassword(BOOL password)
    {
        m_password = password;
        // 비밀번호 모드 변경은 재생성 필요
    }

    void CMEdit::SetReadOnly(BOOL readOnly)
    {
        m_readOnly = readOnly;
        if (m_innerEdit.m_hWnd)
        {
            m_innerEdit.SetReadOnly(readOnly);
        }
    }

    void CMEdit::SetClearButton(BOOL show)
    {
        if (m_clearButton != show)
        {
            m_clearButton = show;
            UpdateInnerEditPosition();
            Invalidate();
        }
    }

    void CMEdit::SetValidationState(ValidationState state)
    {
        if (m_validationState != state)
        {
            m_validationState = state;
            UpdateInnerEditPosition();
            Invalidate();

            // 테두리 색상 업데이트
            StartBorderAnimation(GetBorderColor());
        }
    }

    void CMEdit::SetTextChangedHandler(TextChangedHandler handler, void* context)
    {
        m_textChangedHandler = handler;
        m_textChangedContext = context;
    }

    COLORREF CMEdit::GetBorderColor() const
    {
        const ThemeColors& colors = GetColors();

        if (!m_enabled)
        {
            return colors.border.disabled;
        }

        switch (m_validationState)
        {
        case ValidationState::Valid:
            return colors.success.normal;
        case ValidationState::Invalid:
            return colors.danger.normal;
        case ValidationState::Warning:
            return colors.warning.normal;
        default:
            break;
        }

        switch (m_state)
        {
        case ControlState::Focused:
            return colors.borderFocus;
        case ControlState::Hover:
            return colors.border.hover;
        default:
            return colors.border.normal;
        }
    }

    void CMEdit::SetState(ControlState newState)
    {
        if (m_state != newState)
        {
            CMControlBase::SetState(newState);
            StartBorderAnimation(GetBorderColor());
        }
    }

    void CMEdit::StartBorderAnimation(COLORREF targetColor)
    {
        if (m_currentAnimationId > 0)
        {
            CMAnimationManager::GetInstance().StopAnimation(m_currentAnimationId);
        }

        COLORREF startColor = m_currentBorderColor;
        m_targetBorderColor = targetColor;

        m_currentAnimationId = CMAnimationManager::GetInstance().StartAnimation(
            m_hWnd,
            0.0f,
            1.0f,
            AnimationConstants::SlowDuration,
            EasingType::EaseOutQuad,
            [this, startColor, targetColor](float progress) {
                m_currentBorderColor = CMAnimationManager::InterpolateColor(startColor, targetColor, progress);
                Invalidate();
            },
            [this]() {
                m_currentAnimationId = 0;
            }
        );
    }

    BOOL CMEdit::IsClearButtonVisible() const
    {
        if (!m_clearButton)
            return FALSE;

        CString text = GetText();
        return !text.IsEmpty();
    }

    void CMEdit::OnDraw(CDC* pDC)
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
        int radius = GetThemeManager().GetCornerRadius();

        // 배경
        COLORREF bgColor = m_enabled ? colors.surface.normal : colors.surface.disabled;

        // 테두리 두께
        int borderWidth = (m_state == ControlState::Focused) ? 2 : 1;

        // 배경 및 테두리 그리기
        DrawRoundedRectD2D(rect, radius, bgColor, m_currentBorderColor, borderWidth);

        // 지우기 버튼
        if (IsClearButtonVisible())
        {
            DrawClearButtonD2D();
        }
        // 유효성 아이콘
        else if (m_validationState != ValidationState::None)
        {
            DrawValidationIconD2D(rect);
        }

        EndD2DDraw();
    }

    void CMEdit::OnDrawGdiPlus(CDC* pDC)
    {
        CRect rect;
        GetClientRect(&rect);

        const ThemeColors& colors = GetColors();
        int radius = GetThemeManager().GetCornerRadius();

        COLORREF bgColor = m_enabled ? colors.surface.normal : colors.surface.disabled;
        int borderWidth = (m_state == ControlState::Focused) ? 2 : 1;

        DrawRoundedRect(pDC, rect, radius, bgColor, m_currentBorderColor, borderWidth);

        if (IsClearButtonVisible())
        {
            DrawClearButton(pDC);
        }
        else if (m_validationState != ValidationState::None)
        {
            DrawValidationIcon(pDC, rect);
        }
    }

    void CMEdit::DrawClearButton(CDC* pDC)
    {
        CRect rect;
        GetClientRect(&rect);

        // 버튼 위치 계산
        m_clearButtonRect.SetRect(
            rect.right - 28,
            rect.top + (rect.Height() - 20) / 2,
            rect.right - 8,
            rect.top + (rect.Height() + 20) / 2
        );

        // X 아이콘 그리기
        COLORREF iconColor = m_clearButtonHover
            ? GetColors().text
            : GetColors().textSecondary;

        CPen pen(PS_SOLID, 2, iconColor);
        CPen* pOldPen = pDC->SelectObject(&pen);

        int cx = m_clearButtonRect.CenterPoint().x;
        int cy = m_clearButtonRect.CenterPoint().y;
        int size = 5;

        pDC->MoveTo(cx - size, cy - size);
        pDC->LineTo(cx + size, cy + size);
        pDC->MoveTo(cx + size, cy - size);
        pDC->LineTo(cx - size, cy + size);

        pDC->SelectObject(pOldPen);
    }

    void CMEdit::DrawValidationIcon(CDC* pDC, const CRect& rect)
    {
        const ThemeColors& colors = GetColors();
        COLORREF iconColor;

        CString iconText;

        switch (m_validationState)
        {
        case ValidationState::Valid:
            iconColor = colors.success.normal;
            iconText = _T("\x2713");  // 체크 마크
            break;
        case ValidationState::Invalid:
            iconColor = colors.danger.normal;
            iconText = _T("\x2717");  // X 마크
            break;
        case ValidationState::Warning:
            iconColor = colors.warning.normal;
            iconText = _T("!");
            break;
        default:
            return;
        }

        CRect iconRect(
            rect.right - 28,
            rect.top,
            rect.right - 4,
            rect.bottom
        );

        DrawText(pDC, iconText, iconRect, iconColor, TextStyle::Body,
            DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    }

    void CMEdit::OnSize(UINT nType, int cx, int cy)
    {
        CMControlBase::OnSize(nType, cx, cy);
        UpdateInnerEditPosition();
    }

    void CMEdit::OnSetFocus(CWnd* pOldWnd)
    {
        CMControlBase::OnSetFocus(pOldWnd);

        // 내부 에디트에 포커스 전달
        if (m_innerEdit.m_hWnd)
        {
            m_innerEdit.SetFocus();
        }
    }

    void CMEdit::OnKillFocus(CWnd* pNewWnd)
    {
        // 내부 에디트로 포커스가 이동한 경우는 무시
        if (pNewWnd == &m_innerEdit)
        {
            return;
        }

        CMControlBase::OnKillFocus(pNewWnd);
    }

    LRESULT CMEdit::OnEditChange(WPARAM wParam, LPARAM lParam)
    {
        if (m_textChangedHandler)
        {
            m_textChangedHandler(m_textChangedContext, GetText());
        }

        // 지우기 버튼 표시 업데이트
        if (m_clearButton)
        {
            UpdateInnerEditPosition();
            Invalidate();
        }

        return 0;
    }

    void CMEdit::OnInnerEditChange()
    {
        // EN_CHANGE 메시지를 WM_EDIT_CHANGE로 전달
        OnEditChange(0, 0);
    }

    void CMEdit::OnLButtonDown(UINT nFlags, CPoint point)
    {
        // 지우기 버튼 클릭 확인
        if (IsClearButtonVisible() && m_clearButtonRect.PtInRect(point))
        {
            SetText(_T(""));

            if (m_textChangedHandler)
            {
                m_textChangedHandler(m_textChangedContext, _T(""));
            }

            Invalidate();
            return;
        }

        CMControlBase::OnLButtonDown(nFlags, point);
    }

    void CMEdit::DrawClearButtonD2D()
    {
        if (!m_pRenderTarget || !m_isD2DDrawing)
            return;

        CRect rect;
        GetClientRect(&rect);

        // 버튼 위치 계산
        m_clearButtonRect.SetRect(
            rect.right - 28,
            rect.top + (rect.Height() - 20) / 2,
            rect.right - 8,
            rect.top + (rect.Height() + 20) / 2
        );

        // X 아이콘 그리기
        COLORREF iconColor = m_clearButtonHover
            ? GetColors().text
            : GetColors().textSecondary;

        m_pSolidBrush->SetColor(ToD2DColor(iconColor));

        int cx = m_clearButtonRect.CenterPoint().x;
        int cy = m_clearButtonRect.CenterPoint().y;
        int size = 5;

        // X 라인 그리기
        m_pRenderTarget->DrawLine(
            D2D1::Point2F((float)(cx - size), (float)(cy - size)),
            D2D1::Point2F((float)(cx + size), (float)(cy + size)),
            m_pSolidBrush,
            2.0f
        );
        m_pRenderTarget->DrawLine(
            D2D1::Point2F((float)(cx + size), (float)(cy - size)),
            D2D1::Point2F((float)(cx - size), (float)(cy + size)),
            m_pSolidBrush,
            2.0f
        );
    }

    void CMEdit::DrawValidationIconD2D(const CRect& rect)
    {
        if (!m_pRenderTarget || !m_isD2DDrawing)
            return;

        const ThemeColors& colors = GetColors();
        COLORREF iconColor;
        CString iconText;

        switch (m_validationState)
        {
        case ValidationState::Valid:
            iconColor = colors.success.normal;
            iconText = _T("\x2713");  // 체크 마크
            break;
        case ValidationState::Invalid:
            iconColor = colors.danger.normal;
            iconText = _T("\x2717");  // X 마크
            break;
        case ValidationState::Warning:
            iconColor = colors.warning.normal;
            iconText = _T("!");
            break;
        default:
            return;
        }

        CRect iconRect(
            rect.right - 28,
            rect.top,
            rect.right - 4,
            rect.bottom
        );

        DrawTextD2D(iconText, iconRect, iconColor, TextStyle::Body,
            DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    }
}
