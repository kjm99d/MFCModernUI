#include "pch.h"
#include "CMTabCtrl.h"

namespace MFCModernUI
{
    IMPLEMENT_DYNAMIC(CMTabCtrl, CMControlBase)

    BEGIN_MESSAGE_MAP(CMTabCtrl, CMControlBase)
        ON_WM_LBUTTONDOWN()
        ON_WM_MOUSEMOVE()
        ON_WM_MOUSELEAVE()
    END_MESSAGE_MAP()

    CMTabCtrl::CMTabCtrl()
        : m_selectedIndex(0)
        , m_tabPosition(Position::Top)
        , m_showCloseButton(FALSE)
        , m_scrollButtons(TRUE)
        , m_scrollOffset(0)
        , m_maxScrollOffset(0)
        , m_hoverIndex(-1)
        , m_hoverCloseIndex(-1)
        , m_tabChangedHandler(nullptr)
        , m_tabChangedContext(nullptr)
        , m_tabClosingHandler(nullptr)
        , m_tabClosingContext(nullptr)
        , m_tabClosedHandler(nullptr)
        , m_tabClosedContext(nullptr)
    {
        m_themeClass = _T("tabs");
    }

    CMTabCtrl::~CMTabCtrl()
    {
    }

    BOOL CMTabCtrl::Create(DWORD dwStyle, const CRect& rect, CWnd* pParentWnd, UINT nID)
    {
        CString className = AfxRegisterWndClass(
            CS_HREDRAW | CS_VREDRAW,
            LoadCursor(nullptr, IDC_ARROW),
            nullptr,
            nullptr
        );

        return CWnd::Create(className, nullptr, dwStyle | WS_CHILD | WS_CLIPCHILDREN, rect, pParentWnd, nID);
    }

    int CMTabCtrl::AddTab(const CString& text, BOOL closable)
    {
        return InsertTab(static_cast<int>(m_tabs.size()), text, closable);
    }

    int CMTabCtrl::InsertTab(int index, const CString& text, BOOL closable)
    {
        if (index < 0)
            index = 0;
        if (index > static_cast<int>(m_tabs.size()))
            index = static_cast<int>(m_tabs.size());

        TabItem tab(text);
        tab.closable = closable;
        m_tabs.insert(m_tabs.begin() + index, tab);

        if (m_selectedIndex >= index && m_tabs.size() > 1)
            m_selectedIndex++;

        UpdateScrollRange();
        Invalidate();

        return index;
    }

    void CMTabCtrl::RemoveTab(int index)
    {
        if (index >= 0 && index < static_cast<int>(m_tabs.size()))
        {
            m_tabs.erase(m_tabs.begin() + index);

            if (m_selectedIndex >= static_cast<int>(m_tabs.size()))
                m_selectedIndex = static_cast<int>(m_tabs.size()) - 1;
            if (m_selectedIndex < 0)
                m_selectedIndex = 0;

            UpdateScrollRange();
            Invalidate();
        }
    }

    void CMTabCtrl::RemoveAllTabs()
    {
        m_tabs.clear();
        m_selectedIndex = 0;
        m_scrollOffset = 0;
        Invalidate();
    }

    void CMTabCtrl::SetTabText(int index, const CString& text)
    {
        if (index >= 0 && index < static_cast<int>(m_tabs.size()))
        {
            m_tabs[index].text = text;
            UpdateScrollRange();
            Invalidate();
        }
    }

    CString CMTabCtrl::GetTabText(int index) const
    {
        if (index >= 0 && index < static_cast<int>(m_tabs.size()))
            return m_tabs[index].text;
        return _T("");
    }

    void CMTabCtrl::SetTabIcon(int index, UINT iconId)
    {
        if (index >= 0 && index < static_cast<int>(m_tabs.size()))
        {
            m_tabs[index].iconId = iconId;
            Invalidate();
        }
    }

    void CMTabCtrl::SetTabClosable(int index, BOOL closable)
    {
        if (index >= 0 && index < static_cast<int>(m_tabs.size()))
        {
            m_tabs[index].closable = closable;
            Invalidate();
        }
    }

    void CMTabCtrl::SetTabDisabled(int index, BOOL disabled)
    {
        if (index >= 0 && index < static_cast<int>(m_tabs.size()))
        {
            m_tabs[index].disabled = disabled;
            Invalidate();
        }
    }

    void CMTabCtrl::SetTabBadge(int index, const CString& badge)
    {
        if (index >= 0 && index < static_cast<int>(m_tabs.size()))
        {
            m_tabs[index].badge = badge;
            Invalidate();
        }
    }

    void CMTabCtrl::SetSelectedIndex(int index)
    {
        if (index >= 0 && index < static_cast<int>(m_tabs.size()) && !m_tabs[index].disabled)
        {
            if (m_selectedIndex != index)
            {
                int oldIndex = m_selectedIndex;
                m_selectedIndex = index;

                if (m_tabChangedHandler)
                {
                    m_tabChangedHandler(m_tabChangedContext, oldIndex, index);
                }

                Invalidate();
            }
        }
    }

    void CMTabCtrl::SetTabPosition(Position position)
    {
        m_tabPosition = position;
        Invalidate();
    }

    void CMTabCtrl::SetShowCloseButton(BOOL show)
    {
        m_showCloseButton = show;
        UpdateScrollRange();
        Invalidate();
    }

    void CMTabCtrl::SetScrollButtons(BOOL show)
    {
        m_scrollButtons = show;
        Invalidate();
    }

    void CMTabCtrl::SetTabChangedHandler(TabChangedHandler handler, void* context)
    {
        m_tabChangedHandler = handler;
        m_tabChangedContext = context;
    }

    void CMTabCtrl::SetTabClosingHandler(TabClosingHandler handler, void* context)
    {
        m_tabClosingHandler = handler;
        m_tabClosingContext = context;
    }

    void CMTabCtrl::SetTabClosedHandler(TabClosedHandler handler, void* context)
    {
        m_tabClosedHandler = handler;
        m_tabClosedContext = context;
    }

    int CMTabCtrl::GetTabAreaSize() const
    {
        return TAB_HEIGHT;
    }

    int CMTabCtrl::CalculateTabWidth(int index)
    {
        if (index < 0 || index >= static_cast<int>(m_tabs.size()))
            return TAB_MIN_WIDTH;

        CDC* pDC = GetDC();
        CFont* pFont = GetThemeManager().GetFont(TextStyle::Body);
        CFont* pOldFont = pDC->SelectObject(pFont);

        CSize textSize = pDC->GetTextExtent(m_tabs[index].text);

        pDC->SelectObject(pOldFont);
        ReleaseDC(pDC);

        int width = textSize.cx + TAB_PADDING * 2;

        if (m_tabs[index].closable || m_showCloseButton)
            width += CLOSE_BUTTON_SIZE + 8;

        if (!m_tabs[index].badge.IsEmpty())
            width += 24;

        return max(TAB_MIN_WIDTH, min(TAB_MAX_WIDTH, width));
    }

    void CMTabCtrl::UpdateScrollRange()
    {
        CRect rect;
        GetClientRect(&rect);

        int totalWidth = 0;
        for (int i = 0; i < static_cast<int>(m_tabs.size()); i++)
        {
            totalWidth += CalculateTabWidth(i);
        }

        m_maxScrollOffset = max(0, totalWidth - rect.Width());
        m_scrollOffset = min(m_scrollOffset, m_maxScrollOffset);
    }

    CRect CMTabCtrl::GetTabRect(int index)
    {
        CRect rect;
        GetClientRect(&rect);

        int x = -m_scrollOffset;

        for (int i = 0; i < index; i++)
        {
            x += CalculateTabWidth(i);
        }

        int width = CalculateTabWidth(index);

        if (m_tabPosition == Position::Top || m_tabPosition == Position::Bottom)
        {
            int y = (m_tabPosition == Position::Top) ? 0 : rect.Height() - TAB_HEIGHT;
            return CRect(x, y, x + width, y + TAB_HEIGHT);
        }
        else
        {
            // 세로 탭 (Left/Right)
            int xPos = (m_tabPosition == Position::Left) ? 0 : rect.Width() - TAB_HEIGHT;
            return CRect(xPos, x, xPos + TAB_HEIGHT, x + width);
        }
    }

    int CMTabCtrl::HitTest(CPoint point)
    {
        for (int i = 0; i < static_cast<int>(m_tabs.size()); i++)
        {
            CRect tabRect = GetTabRect(i);
            if (tabRect.PtInRect(point))
                return i;
        }
        return -1;
    }

    int CMTabCtrl::HitTestClose(CPoint point)
    {
        for (int i = 0; i < static_cast<int>(m_tabs.size()); i++)
        {
            if (!m_tabs[i].closable && !m_showCloseButton)
                continue;

            CRect tabRect = GetTabRect(i);
            CRect closeRect(
                tabRect.right - CLOSE_BUTTON_SIZE - 8,
                tabRect.top + (TAB_HEIGHT - CLOSE_BUTTON_SIZE) / 2,
                tabRect.right - 8,
                tabRect.top + (TAB_HEIGHT + CLOSE_BUTTON_SIZE) / 2
            );

            if (closeRect.PtInRect(point))
                return i;
        }
        return -1;
    }

    void CMTabCtrl::CloseTab(int index)
    {
        if (index < 0 || index >= static_cast<int>(m_tabs.size()))
            return;

        // 닫기 전 이벤트
        if (m_tabClosingHandler)
        {
            if (!m_tabClosingHandler(m_tabClosingContext, index))
                return;  // 취소됨
        }

        RemoveTab(index);

        // 닫힌 후 이벤트
        if (m_tabClosedHandler)
        {
            m_tabClosedHandler(m_tabClosedContext, index);
        }
    }

    void CMTabCtrl::OnDraw(CDC* pDC)
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

        // 배경
        D2D1_RECT_F bgRect = D2D1::RectF(
            (float)rect.left, (float)rect.top,
            (float)rect.right, (float)rect.bottom
        );
        m_pSolidBrush->SetColor(ToD2DColor(colors.surface.normal));
        m_pRenderTarget->FillRectangle(bgRect, m_pSolidBrush);

        // 탭 영역 배경
        CRect tabAreaRect = rect;
        if (m_tabPosition == Position::Top)
        {
            tabAreaRect.bottom = TAB_HEIGHT;
        }
        else if (m_tabPosition == Position::Bottom)
        {
            tabAreaRect.top = rect.Height() - TAB_HEIGHT;
        }

        D2D1_RECT_F tabAreaD2D = D2D1::RectF(
            (float)tabAreaRect.left, (float)tabAreaRect.top,
            (float)tabAreaRect.right, (float)tabAreaRect.bottom
        );
        m_pSolidBrush->SetColor(ToD2DColor(colors.surface.hover));
        m_pRenderTarget->FillRectangle(tabAreaD2D, m_pSolidBrush);

        // 구분선
        m_pSolidBrush->SetColor(ToD2DColor(colors.border.normal));
        if (m_tabPosition == Position::Top)
        {
            m_pRenderTarget->DrawLine(
                D2D1::Point2F((float)rect.left, (float)(TAB_HEIGHT - 1)),
                D2D1::Point2F((float)rect.right, (float)(TAB_HEIGHT - 1)),
                m_pSolidBrush, 1.0f
            );
        }
        else if (m_tabPosition == Position::Bottom)
        {
            m_pRenderTarget->DrawLine(
                D2D1::Point2F((float)rect.left, (float)(rect.Height() - TAB_HEIGHT)),
                D2D1::Point2F((float)rect.right, (float)(rect.Height() - TAB_HEIGHT)),
                m_pSolidBrush, 1.0f
            );
        }

        // 탭 그리기
        for (int i = 0; i < static_cast<int>(m_tabs.size()); i++)
        {
            CRect tabRect = GetTabRect(i);

            // 화면에 보이는 탭만 그리기
            if (tabRect.right > 0 && tabRect.left < rect.Width())
            {
                DrawTabD2D(i, tabRect);
            }
        }

        EndD2DDraw();
    }

    void CMTabCtrl::OnDrawGdiPlus(CDC* pDC)
    {
        CRect rect;
        GetClientRect(&rect);

        const ThemeColors& colors = GetColors();

        // 배경
        pDC->FillSolidRect(rect, colors.surface.normal);

        // 탭 영역 배경
        CRect tabAreaRect = rect;
        if (m_tabPosition == Position::Top)
        {
            tabAreaRect.bottom = TAB_HEIGHT;
        }
        else if (m_tabPosition == Position::Bottom)
        {
            tabAreaRect.top = rect.Height() - TAB_HEIGHT;
        }

        pDC->FillSolidRect(tabAreaRect, colors.surface.hover);

        // 구분선
        CPen pen(PS_SOLID, 1, colors.border.normal);
        CPen* pOldPen = pDC->SelectObject(&pen);

        if (m_tabPosition == Position::Top)
        {
            pDC->MoveTo(rect.left, TAB_HEIGHT - 1);
            pDC->LineTo(rect.right, TAB_HEIGHT - 1);
        }
        else if (m_tabPosition == Position::Bottom)
        {
            pDC->MoveTo(rect.left, rect.Height() - TAB_HEIGHT);
            pDC->LineTo(rect.right, rect.Height() - TAB_HEIGHT);
        }

        pDC->SelectObject(pOldPen);

        // 탭 그리기
        for (int i = 0; i < static_cast<int>(m_tabs.size()); i++)
        {
            CRect tabRect = GetTabRect(i);

            // 화면에 보이는 탭만 그리기
            if (tabRect.right > 0 && tabRect.left < rect.Width())
            {
                DrawTab(pDC, i, tabRect);
            }
        }
    }

    void CMTabCtrl::DrawTab(CDC* pDC, int index, const CRect& tabRect)
    {
        const ThemeColors& colors = GetColors();
        const TabItem& tab = m_tabs[index];

        BOOL isSelected = (index == m_selectedIndex);
        BOOL isHover = (index == m_hoverIndex);
        BOOL isDisabled = tab.disabled;

        // 탭 배경
        COLORREF bgColor;
        if (isDisabled)
        {
            bgColor = colors.surface.disabled;
        }
        else if (isSelected)
        {
            bgColor = colors.surface.normal;
        }
        else if (isHover)
        {
            bgColor = colors.surface.pressed;
        }
        else
        {
            bgColor = colors.surface.hover;
        }

        CRect bgRect = tabRect;
        if (m_tabPosition == Position::Top && isSelected)
        {
            bgRect.bottom += 1;  // 구분선 가리기
        }

        pDC->FillSolidRect(bgRect, bgColor);

        // 선택 표시 (하단 또는 상단 바)
        if (isSelected)
        {
            CRect indicatorRect = tabRect;
            if (m_tabPosition == Position::Top)
            {
                indicatorRect.top = indicatorRect.bottom - 3;
            }
            else
            {
                indicatorRect.bottom = indicatorRect.top + 3;
            }
            pDC->FillSolidRect(indicatorRect, colors.primary.normal);
        }

        // 텍스트
        CRect textRect = tabRect;
        textRect.DeflateRect(TAB_PADDING, 0);

        if (tab.closable || m_showCloseButton)
        {
            textRect.right -= CLOSE_BUTTON_SIZE + 8;
        }

        COLORREF textColor;
        if (isDisabled)
        {
            textColor = colors.textDisabled;
        }
        else if (isSelected)
        {
            textColor = colors.primary.normal;
        }
        else
        {
            textColor = colors.text;
        }

        DrawText(pDC, tab.text, textRect, textColor, TextStyle::Body,
            DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);

        // 닫기 버튼
        if ((tab.closable || m_showCloseButton) && !isDisabled)
        {
            CRect closeRect(
                tabRect.right - CLOSE_BUTTON_SIZE - 8,
                tabRect.top + (TAB_HEIGHT - CLOSE_BUTTON_SIZE) / 2,
                tabRect.right - 8,
                tabRect.top + (TAB_HEIGHT + CLOSE_BUTTON_SIZE) / 2
            );

            DrawCloseButton(pDC, closeRect, index == m_hoverCloseIndex);
        }

        // 배지
        if (!tab.badge.IsEmpty())
        {
            DrawBadge(pDC, tabRect, tab.badge);
        }
    }

    void CMTabCtrl::DrawCloseButton(CDC* pDC, const CRect& rect, BOOL hover)
    {
        const ThemeColors& colors = GetColors();

        if (hover)
        {
            // 호버 배경
            CBrush brush(colors.surface.pressed);
            CPen nullPen(PS_NULL, 0, RGB(0, 0, 0));
            pDC->SelectObject(&brush);
            pDC->SelectObject(&nullPen);
            pDC->Ellipse(rect);
        }

        // X 아이콘
        COLORREF iconColor = hover ? colors.danger.normal : colors.textSecondary;
        CPen pen(PS_SOLID, 1, iconColor);
        CPen* pOldPen = pDC->SelectObject(&pen);

        int cx = rect.CenterPoint().x;
        int cy = rect.CenterPoint().y;
        int size = 4;

        pDC->MoveTo(cx - size, cy - size);
        pDC->LineTo(cx + size + 1, cy + size + 1);
        pDC->MoveTo(cx + size, cy - size);
        pDC->LineTo(cx - size - 1, cy + size + 1);

        pDC->SelectObject(pOldPen);
    }

    void CMTabCtrl::DrawBadge(CDC* pDC, const CRect& tabRect, const CString& badge)
    {
        const ThemeColors& colors = GetColors();

        CRect badgeRect(
            tabRect.right - 28,
            tabRect.top + 8,
            tabRect.right - 8,
            tabRect.top + 22
        );

        // 배지 배경
        CBrush brush(colors.danger.normal);
        CPen nullPen(PS_NULL, 0, RGB(0, 0, 0));
        pDC->SelectObject(&brush);
        pDC->SelectObject(&nullPen);
        pDC->RoundRect(badgeRect, CPoint(7, 7));

        // 배지 텍스트
        DrawText(pDC, badge, badgeRect, colors.textOnPrimary, TextStyle::Caption,
            DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    }

    void CMTabCtrl::OnLButtonDown(UINT nFlags, CPoint point)
    {
        SetFocus();

        // 닫기 버튼 클릭
        int closeIndex = HitTestClose(point);
        if (closeIndex >= 0)
        {
            CloseTab(closeIndex);
            return;
        }

        // 탭 클릭
        int tabIndex = HitTest(point);
        if (tabIndex >= 0)
        {
            SetSelectedIndex(tabIndex);
        }

        CMControlBase::OnLButtonDown(nFlags, point);
    }

    void CMTabCtrl::OnMouseMove(UINT nFlags, CPoint point)
    {
        int newHover = HitTest(point);
        int newCloseHover = HitTestClose(point);

        if (newHover != m_hoverIndex || newCloseHover != m_hoverCloseIndex)
        {
            m_hoverIndex = newHover;
            m_hoverCloseIndex = newCloseHover;
            Invalidate();
        }

        CMControlBase::OnMouseMove(nFlags, point);
    }

    void CMTabCtrl::OnMouseLeave()
    {
        m_hoverIndex = -1;
        m_hoverCloseIndex = -1;
        Invalidate();

        CMControlBase::OnMouseLeave();
    }

    void CMTabCtrl::DrawTabD2D(int index, const CRect& tabRect)
    {
        if (!m_pRenderTarget || !m_isD2DDrawing)
            return;

        const ThemeColors& colors = GetColors();
        const TabItem& tab = m_tabs[index];

        BOOL isSelected = (index == m_selectedIndex);
        BOOL isHover = (index == m_hoverIndex);
        BOOL isDisabled = tab.disabled;

        // 탭 배경
        COLORREF bgColor;
        if (isDisabled)
        {
            bgColor = colors.surface.disabled;
        }
        else if (isSelected)
        {
            bgColor = colors.surface.normal;
        }
        else if (isHover)
        {
            bgColor = colors.surface.pressed;
        }
        else
        {
            bgColor = colors.surface.hover;
        }

        CRect bgRect = tabRect;
        if (m_tabPosition == Position::Top && isSelected)
        {
            bgRect.bottom += 1;  // 구분선 가리기
        }

        D2D1_RECT_F d2dBgRect = D2D1::RectF(
            (float)bgRect.left, (float)bgRect.top,
            (float)bgRect.right, (float)bgRect.bottom
        );
        m_pSolidBrush->SetColor(ToD2DColor(bgColor));
        m_pRenderTarget->FillRectangle(d2dBgRect, m_pSolidBrush);

        // 선택 표시 (하단 또는 상단 바)
        if (isSelected)
        {
            CRect indicatorRect = tabRect;
            if (m_tabPosition == Position::Top)
            {
                indicatorRect.top = indicatorRect.bottom - 3;
            }
            else
            {
                indicatorRect.bottom = indicatorRect.top + 3;
            }

            D2D1_RECT_F d2dIndicator = D2D1::RectF(
                (float)indicatorRect.left, (float)indicatorRect.top,
                (float)indicatorRect.right, (float)indicatorRect.bottom
            );
            m_pSolidBrush->SetColor(ToD2DColor(colors.primary.normal));
            m_pRenderTarget->FillRectangle(d2dIndicator, m_pSolidBrush);
        }

        // 텍스트
        CRect textRect = tabRect;
        textRect.DeflateRect(TAB_PADDING, 0);

        if (tab.closable || m_showCloseButton)
        {
            textRect.right -= CLOSE_BUTTON_SIZE + 8;
        }

        COLORREF textColor;
        if (isDisabled)
        {
            textColor = colors.textDisabled;
        }
        else if (isSelected)
        {
            textColor = colors.primary.normal;
        }
        else
        {
            textColor = colors.text;
        }

        DrawTextD2D(tab.text, textRect, textColor, TextStyle::Body,
            DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);

        // 닫기 버튼
        if ((tab.closable || m_showCloseButton) && !isDisabled)
        {
            CRect closeRect(
                tabRect.right - CLOSE_BUTTON_SIZE - 8,
                tabRect.top + (TAB_HEIGHT - CLOSE_BUTTON_SIZE) / 2,
                tabRect.right - 8,
                tabRect.top + (TAB_HEIGHT + CLOSE_BUTTON_SIZE) / 2
            );

            DrawCloseButtonD2D(closeRect, index == m_hoverCloseIndex);
        }

        // 배지
        if (!tab.badge.IsEmpty())
        {
            DrawBadgeD2D(tabRect, tab.badge);
        }
    }

    void CMTabCtrl::DrawCloseButtonD2D(const CRect& rect, BOOL hover)
    {
        if (!m_pRenderTarget || !m_isD2DDrawing)
            return;

        const ThemeColors& colors = GetColors();

        if (hover)
        {
            // 호버 배경 (원)
            D2D1_ELLIPSE ellipse = D2D1::Ellipse(
                D2D1::Point2F((float)rect.CenterPoint().x, (float)rect.CenterPoint().y),
                (float)(rect.Width() / 2),
                (float)(rect.Height() / 2)
            );
            m_pSolidBrush->SetColor(ToD2DColor(colors.surface.pressed));
            m_pRenderTarget->FillEllipse(ellipse, m_pSolidBrush);
        }

        // X 아이콘
        COLORREF iconColor = hover ? colors.danger.normal : colors.textSecondary;
        m_pSolidBrush->SetColor(ToD2DColor(iconColor));

        float cx = (float)rect.CenterPoint().x;
        float cy = (float)rect.CenterPoint().y;
        float size = 4.0f;

        m_pRenderTarget->DrawLine(
            D2D1::Point2F(cx - size, cy - size),
            D2D1::Point2F(cx + size + 1, cy + size + 1),
            m_pSolidBrush, 1.0f
        );
        m_pRenderTarget->DrawLine(
            D2D1::Point2F(cx + size, cy - size),
            D2D1::Point2F(cx - size - 1, cy + size + 1),
            m_pSolidBrush, 1.0f
        );
    }

    void CMTabCtrl::DrawBadgeD2D(const CRect& tabRect, const CString& badge)
    {
        if (!m_pRenderTarget || !m_isD2DDrawing)
            return;

        const ThemeColors& colors = GetColors();

        CRect badgeRect(
            tabRect.right - 28,
            tabRect.top + 8,
            tabRect.right - 8,
            tabRect.top + 22
        );

        // 배지 배경
        D2D1_ROUNDED_RECT roundedRect = D2D1::RoundedRect(
            D2D1::RectF((float)badgeRect.left, (float)badgeRect.top,
                       (float)badgeRect.right, (float)badgeRect.bottom),
            7.0f, 7.0f
        );
        m_pSolidBrush->SetColor(ToD2DColor(colors.danger.normal));
        m_pRenderTarget->FillRoundedRectangle(roundedRect, m_pSolidBrush);

        // 배지 텍스트
        DrawTextD2D(badge, badgeRect, colors.textOnPrimary, TextStyle::Caption,
            DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    }
}
