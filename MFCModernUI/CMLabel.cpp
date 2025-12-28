#include "pch.h"
#include "CMLabel.h"

namespace MFCModernUI
{
    IMPLEMENT_DYNAMIC(CMLabel, CMControlBase)

    BEGIN_MESSAGE_MAP(CMLabel, CMControlBase)
    END_MESSAGE_MAP()

    CMLabel::CMLabel()
        : m_textStyle(TextStyle::Body)
        , m_textAlign(TextAlign::Left)
        , m_textColor(RGB(0, 0, 0))
        , m_useDefaultColor(TRUE)
        , m_ellipsis(FALSE)
        , m_selectable(FALSE)
        , m_wordWrap(FALSE)
    {
        m_themeClass = _T("label");
        m_tabStop = FALSE;  // 레이블은 기본적으로 탭 정지하지 않음
    }

    CMLabel::~CMLabel()
    {
    }

    BOOL CMLabel::Create(const CString& text, DWORD dwStyle, const CRect& rect,
        CWnd* pParentWnd, UINT nID)
    {
        m_text = text;

        CString className = AfxRegisterWndClass(
            CS_HREDRAW | CS_VREDRAW,
            LoadCursor(nullptr, IDC_ARROW),
            nullptr,
            nullptr
        );

        return CWnd::Create(className, nullptr, dwStyle | WS_CHILD, rect, pParentWnd, nID);
    }

    void CMLabel::SetText(const CString& text)
    {
        if (m_text != text)
        {
            m_text = text;
            Invalidate();
        }
    }

    void CMLabel::SetTextStyle(TextStyle style)
    {
        if (m_textStyle != style)
        {
            m_textStyle = style;
            Invalidate();
        }
    }

    void CMLabel::SetTextAlign(TextAlign align)
    {
        if (m_textAlign != align)
        {
            m_textAlign = align;
            Invalidate();
        }
    }

    void CMLabel::SetTextColor(COLORREF color)
    {
        m_textColor = color;
        m_useDefaultColor = FALSE;
        Invalidate();
    }

    void CMLabel::UseDefaultColor()
    {
        m_useDefaultColor = TRUE;
        Invalidate();
    }

    void CMLabel::SetEllipsis(BOOL ellipsis)
    {
        if (m_ellipsis != ellipsis)
        {
            m_ellipsis = ellipsis;
            Invalidate();
        }
    }

    void CMLabel::SetSelectable(BOOL selectable)
    {
        m_selectable = selectable;
    }

    void CMLabel::SetWordWrap(BOOL wordWrap)
    {
        if (m_wordWrap != wordWrap)
        {
            m_wordWrap = wordWrap;
            Invalidate();
        }
    }

    void CMLabel::OnDraw(CDC* pDC)
    {
        CRect rect;
        GetClientRect(&rect);

        // 배경 (투명)
        pDC->FillSolidRect(rect, GetColors().background.normal);

        // 텍스트 색상
        COLORREF textColor = m_useDefaultColor
            ? (m_enabled ? GetColors().text : GetColors().textDisabled)
            : m_textColor;

        // 정렬 플래그
        UINT format = DT_VCENTER;

        switch (m_textAlign)
        {
        case TextAlign::Left:
            format |= DT_LEFT;
            break;
        case TextAlign::Center:
            format |= DT_CENTER;
            break;
        case TextAlign::Right:
            format |= DT_RIGHT;
            break;
        }

        if (m_wordWrap)
        {
            format |= DT_WORDBREAK;
            format &= ~DT_VCENTER;  // 멀티라인에서는 세로 중앙 정렬 불가
            format |= DT_TOP;
        }
        else
        {
            format |= DT_SINGLELINE;
        }

        if (m_ellipsis)
        {
            format |= DT_END_ELLIPSIS;
        }

        // 폰트 설정
        CFont* pFont = GetThemeManager().GetFont(m_textStyle);
        CFont* pOldFont = pDC->SelectObject(pFont);
        COLORREF oldColor = pDC->SetTextColor(textColor);
        int oldBkMode = pDC->SetBkMode(TRANSPARENT);

        pDC->DrawText(m_text, rect, format);

        pDC->SetBkMode(oldBkMode);
        pDC->SetTextColor(oldColor);
        pDC->SelectObject(pOldFont);
    }
}
