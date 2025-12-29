#include "pch.h"
#include "CMListView.h"

namespace MFCModernUI
{
    IMPLEMENT_DYNAMIC(CMListView, CMControlBase)

    BEGIN_MESSAGE_MAP(CMListView, CMControlBase)
        ON_WM_LBUTTONDOWN()
        ON_WM_LBUTTONDBLCLK()
        ON_WM_MOUSEMOVE()
        ON_WM_KEYDOWN()
        ON_WM_MOUSEWHEEL()
        ON_WM_SIZE()
    END_MESSAGE_MAP()

    CMListView::CMListView()
        : m_selectionMode(SelectionMode::Single)
        , m_showCheckboxes(FALSE)
        , m_showHeaders(TRUE)
        , m_alternateRows(FALSE)
        , m_scrollY(0)
        , m_maxScrollY(0)
        , m_hoverIndex(-1)
        , m_selectionChangedHandler(nullptr)
        , m_selectionChangedContext(nullptr)
        , m_itemDoubleClickHandler(nullptr)
        , m_itemDoubleClickContext(nullptr)
        , m_columnClickHandler(nullptr)
        , m_columnClickContext(nullptr)
    {
        m_themeClass = _T("listview");
    }

    CMListView::~CMListView()
    {
    }

    BOOL CMListView::Create(DWORD dwStyle, const CRect& rect, CWnd* pParentWnd, UINT nID)
    {
        CString className = AfxRegisterWndClass(
            CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS,
            LoadCursor(nullptr, IDC_ARROW),
            nullptr,
            nullptr
        );

        return CWnd::Create(className, nullptr, dwStyle | WS_CHILD | WS_CLIPCHILDREN, rect, pParentWnd, nID);
    }

    int CMListView::AddColumn(const CString& header, int width)
    {
        ListViewColumn col(header, width);
        m_columns.push_back(col);
        Invalidate();
        return static_cast<int>(m_columns.size()) - 1;
    }

    void CMListView::RemoveColumn(int index)
    {
        if (index >= 0 && index < static_cast<int>(m_columns.size()))
        {
            m_columns.erase(m_columns.begin() + index);
            Invalidate();
        }
    }

    void CMListView::SetColumnWidth(int index, int width)
    {
        if (index >= 0 && index < static_cast<int>(m_columns.size()))
        {
            m_columns[index].width = width;
            Invalidate();
        }
    }

    int CMListView::AddItem(const CString& text)
    {
        return InsertItem(static_cast<int>(m_items.size()), text);
    }

    int CMListView::InsertItem(int index, const CString& text)
    {
        if (index < 0)
            index = 0;
        if (index > static_cast<int>(m_items.size()))
            index = static_cast<int>(m_items.size());

        ListViewItem item;
        item.texts.resize(max(1, static_cast<int>(m_columns.size())));
        item.texts[0] = text;

        m_items.insert(m_items.begin() + index, item);
        m_selection.insert(m_selection.begin() + index, FALSE);

        UpdateScrollRange();
        Invalidate();

        return index;
    }

    void CMListView::RemoveItem(int index)
    {
        if (index >= 0 && index < static_cast<int>(m_items.size()))
        {
            m_items.erase(m_items.begin() + index);
            m_selection.erase(m_selection.begin() + index);
            UpdateScrollRange();
            Invalidate();
        }
    }

    void CMListView::RemoveAllItems()
    {
        m_items.clear();
        m_selection.clear();
        m_scrollY = 0;
        UpdateScrollRange();
        Invalidate();
    }

    void CMListView::SetItemText(int item, int subItem, const CString& text)
    {
        if (item >= 0 && item < static_cast<int>(m_items.size()))
        {
            if (subItem >= static_cast<int>(m_items[item].texts.size()))
            {
                m_items[item].texts.resize(subItem + 1);
            }
            m_items[item].texts[subItem] = text;
            Invalidate();
        }
    }

    CString CMListView::GetItemText(int item, int subItem) const
    {
        if (item >= 0 && item < static_cast<int>(m_items.size()))
        {
            if (subItem >= 0 && subItem < static_cast<int>(m_items[item].texts.size()))
            {
                return m_items[item].texts[subItem];
            }
        }
        return _T("");
    }

    void CMListView::SetItemData(int item, DWORD_PTR data)
    {
        if (item >= 0 && item < static_cast<int>(m_items.size()))
        {
            m_items[item].data = data;
        }
    }

    DWORD_PTR CMListView::GetItemData(int item) const
    {
        if (item >= 0 && item < static_cast<int>(m_items.size()))
        {
            return m_items[item].data;
        }
        return 0;
    }

    void CMListView::SetSelectionMode(SelectionMode mode)
    {
        m_selectionMode = mode;
        if (mode == SelectionMode::Single || mode == SelectionMode::None)
        {
            ClearSelection();
        }
    }

    void CMListView::SelectItem(int index, BOOL select)
    {
        if (m_selectionMode == SelectionMode::None)
            return;

        if (index >= 0 && index < static_cast<int>(m_selection.size()))
        {
            if (m_selectionMode == SelectionMode::Single && select)
            {
                ClearSelection();
            }

            m_selection[index] = select;

            if (m_selectionChangedHandler)
            {
                m_selectionChangedHandler(m_selectionChangedContext);
            }

            Invalidate();
        }
    }

    void CMListView::SelectAll()
    {
        if (m_selectionMode == SelectionMode::Multiple || m_selectionMode == SelectionMode::Extended)
        {
            for (size_t i = 0; i < m_selection.size(); i++)
            {
                m_selection[i] = TRUE;
            }
            Invalidate();
        }
    }

    void CMListView::ClearSelection()
    {
        for (size_t i = 0; i < m_selection.size(); i++)
        {
            m_selection[i] = FALSE;
        }
        Invalidate();
    }

    BOOL CMListView::IsSelected(int index) const
    {
        if (index >= 0 && index < static_cast<int>(m_selection.size()))
        {
            return m_selection[index];
        }
        return FALSE;
    }

    int CMListView::GetSelectedIndex() const
    {
        for (size_t i = 0; i < m_selection.size(); i++)
        {
            if (m_selection[i])
                return static_cast<int>(i);
        }
        return -1;
    }

    std::vector<int> CMListView::GetSelectedIndices() const
    {
        std::vector<int> indices;
        for (size_t i = 0; i < m_selection.size(); i++)
        {
            if (m_selection[i])
                indices.push_back(static_cast<int>(i));
        }
        return indices;
    }

    void CMListView::SetShowCheckboxes(BOOL show)
    {
        m_showCheckboxes = show;
        Invalidate();
    }

    void CMListView::SetItemChecked(int index, BOOL checked)
    {
        if (index >= 0 && index < static_cast<int>(m_items.size()))
        {
            m_items[index].checked = checked;
            Invalidate();
        }
    }

    BOOL CMListView::IsItemChecked(int index) const
    {
        if (index >= 0 && index < static_cast<int>(m_items.size()))
        {
            return m_items[index].checked;
        }
        return FALSE;
    }

    void CMListView::SetShowHeaders(BOOL show)
    {
        m_showHeaders = show;
        UpdateScrollRange();
        Invalidate();
    }

    void CMListView::SetAlternateRows(BOOL alternate)
    {
        m_alternateRows = alternate;
        Invalidate();
    }

    void CMListView::SetSelectionChangedHandler(ListSelectionChangedHandler handler, void* context)
    {
        m_selectionChangedHandler = handler;
        m_selectionChangedContext = context;
    }

    void CMListView::SetItemDoubleClickHandler(ListItemDoubleClickHandler handler, void* context)
    {
        m_itemDoubleClickHandler = handler;
        m_itemDoubleClickContext = context;
    }

    void CMListView::SetColumnClickHandler(ListColumnClickHandler handler, void* context)
    {
        m_columnClickHandler = handler;
        m_columnClickContext = context;
    }

    void CMListView::UpdateScrollRange()
    {
        CRect rect;
        GetClientRect(&rect);

        int headerHeight = m_showHeaders ? HEADER_HEIGHT : 0;
        int contentHeight = static_cast<int>(m_items.size()) * ROW_HEIGHT;
        int viewHeight = rect.Height() - headerHeight;

        m_maxScrollY = max(0, contentHeight - viewHeight);
        m_scrollY = min(m_scrollY, m_maxScrollY);
    }

    int CMListView::HitTest(CPoint point)
    {
        CRect rect;
        GetClientRect(&rect);

        int headerHeight = m_showHeaders ? HEADER_HEIGHT : 0;

        if (point.y < headerHeight)
            return -1;

        int y = headerHeight - m_scrollY;

        for (int i = 0; i < static_cast<int>(m_items.size()); i++)
        {
            if (point.y >= y && point.y < y + ROW_HEIGHT)
                return i;
            y += ROW_HEIGHT;
        }

        return -1;
    }

    CRect CMListView::GetItemRect(int index)
    {
        CRect rect;
        GetClientRect(&rect);

        int headerHeight = m_showHeaders ? HEADER_HEIGHT : 0;
        int y = headerHeight + index * ROW_HEIGHT - m_scrollY;

        return CRect(rect.left, y, rect.right, y + ROW_HEIGHT);
    }

    void CMListView::OnDraw(CDC* pDC)
    {
        // Direct2D 렌더링 시작
        if (!BeginD2DDraw())
            return;

        CRect rect;
        GetClientRect(&rect);

        const ThemeColors& colors = GetColors();
        int radius = GetThemeManager().GetCornerRadius();

        // 배경
        D2D1_RECT_F bgRect = D2D1::RectF(
            (float)rect.left, (float)rect.top,
            (float)rect.right, (float)rect.bottom
        );
        m_pSolidBrush->SetColor(ToD2DColor(colors.surface.normal));
        m_pRenderTarget->FillRectangle(bgRect, m_pSolidBrush);

        // 헤더
        if (m_showHeaders)
        {
            CRect headerRect = rect;
            headerRect.bottom = HEADER_HEIGHT;
            DrawHeaderD2D(headerRect);
        }

        // 항목
        CRect itemsRect = rect;
        if (m_showHeaders)
        {
            itemsRect.top = HEADER_HEIGHT;
        }
        DrawItemsD2D(itemsRect);

        // 테두리
        DrawRoundedRectD2D(rect, radius, 0, colors.border.normal, 1);

        EndD2DDraw();
    }

    void CMListView::OnLButtonDown(UINT nFlags, CPoint point)
    {
        SetFocus();

        int index = HitTest(point);

        if (index >= 0)
        {
            // 체크박스 클릭 체크
            if (m_showCheckboxes)
            {
                CRect itemRect = GetItemRect(index);
                CRect checkRect(itemRect.left + 8, itemRect.top + (ROW_HEIGHT - CHECKBOX_SIZE) / 2,
                               itemRect.left + 8 + CHECKBOX_SIZE, itemRect.top + (ROW_HEIGHT + CHECKBOX_SIZE) / 2);

                if (checkRect.PtInRect(point))
                {
                    SetItemChecked(index, !IsItemChecked(index));
                    return;
                }
            }

            // 선택 처리
            BOOL ctrlPressed = (nFlags & MK_CONTROL) != 0;
            BOOL shiftPressed = (nFlags & MK_SHIFT) != 0;

            if (m_selectionMode == SelectionMode::Multiple && ctrlPressed)
            {
                SelectItem(index, !IsSelected(index));
            }
            else if (m_selectionMode == SelectionMode::Extended && shiftPressed)
            {
                int anchor = GetSelectedIndex();
                if (anchor < 0) anchor = 0;

                ClearSelection();
                int start = min(anchor, index);
                int end = max(anchor, index);
                for (int i = start; i <= end; i++)
                {
                    m_selection[i] = TRUE;
                }
                Invalidate();
            }
            else
            {
                SelectItem(index, TRUE);
            }
        }

        CMControlBase::OnLButtonDown(nFlags, point);
    }

    void CMListView::OnLButtonDblClk(UINT nFlags, CPoint point)
    {
        int index = HitTest(point);

        if (index >= 0 && m_itemDoubleClickHandler)
        {
            m_itemDoubleClickHandler(m_itemDoubleClickContext, index);
        }

        CMControlBase::OnLButtonDblClk(nFlags, point);
    }

    void CMListView::OnMouseMove(UINT nFlags, CPoint point)
    {
        int newHover = HitTest(point);
        if (newHover != m_hoverIndex)
        {
            m_hoverIndex = newHover;
            Invalidate();
        }

        CMControlBase::OnMouseMove(nFlags, point);
    }

    void CMListView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
    {
        int selected = GetSelectedIndex();

        switch (nChar)
        {
        case VK_UP:
            if (selected > 0)
                SelectItem(selected - 1, TRUE);
            break;

        case VK_DOWN:
            if (selected < static_cast<int>(m_items.size()) - 1)
                SelectItem(selected + 1, TRUE);
            break;

        case VK_HOME:
            if (!m_items.empty())
                SelectItem(0, TRUE);
            break;

        case VK_END:
            if (!m_items.empty())
                SelectItem(static_cast<int>(m_items.size()) - 1, TRUE);
            break;

        case VK_SPACE:
            if (m_showCheckboxes && selected >= 0)
                SetItemChecked(selected, !IsItemChecked(selected));
            break;
        }

        // 선택된 항목이 보이도록 스크롤
        selected = GetSelectedIndex();
        if (selected >= 0)
        {
            CRect itemRect = GetItemRect(selected);
            CRect clientRect;
            GetClientRect(&clientRect);

            int headerHeight = m_showHeaders ? HEADER_HEIGHT : 0;

            if (itemRect.top < headerHeight)
            {
                m_scrollY -= (headerHeight - itemRect.top);
            }
            else if (itemRect.bottom > clientRect.bottom)
            {
                m_scrollY += (itemRect.bottom - clientRect.bottom);
            }

            m_scrollY = max(0, min(m_scrollY, m_maxScrollY));
            Invalidate();
        }

        CMControlBase::OnKeyDown(nChar, nRepCnt, nFlags);
    }

    BOOL CMListView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
    {
        int delta = -zDelta / WHEEL_DELTA * ROW_HEIGHT * 3;
        m_scrollY = max(0, min(m_scrollY + delta, m_maxScrollY));
        Invalidate();

        return TRUE;
    }

    void CMListView::OnSize(UINT nType, int cx, int cy)
    {
        CMControlBase::OnSize(nType, cx, cy);
        UpdateScrollRange();
    }

    void CMListView::DrawHeaderD2D(const CRect& rect)
    {
        if (!m_pRenderTarget || !m_isD2DDrawing)
            return;

        const ThemeColors& colors = GetColors();

        // 헤더 배경
        D2D1_RECT_F headerRect = D2D1::RectF(
            (float)rect.left, (float)rect.top,
            (float)rect.right, (float)rect.bottom
        );
        m_pSolidBrush->SetColor(ToD2DColor(colors.surface.hover));
        m_pRenderTarget->FillRectangle(headerRect, m_pSolidBrush);

        // 하단 구분선
        m_pSolidBrush->SetColor(ToD2DColor(colors.border.normal));
        m_pRenderTarget->DrawLine(
            D2D1::Point2F((float)rect.left, (float)(rect.bottom - 1)),
            D2D1::Point2F((float)rect.right, (float)(rect.bottom - 1)),
            m_pSolidBrush, 1.0f
        );

        // 열 헤더 텍스트
        int x = rect.left + (m_showCheckboxes ? 40 : 8);

        for (const auto& col : m_columns)
        {
            CRect colRect(x, rect.top, x + col.width, rect.bottom);
            DrawTextD2D(col.header, colRect, colors.text, TextStyle::Body,
                DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
            x += col.width;
        }
    }

    void CMListView::DrawItemsD2D(const CRect& rect)
    {
        if (!m_pRenderTarget || !m_isD2DDrawing)
            return;

        // 클리핑 영역 설정
        m_pRenderTarget->PushAxisAlignedClip(
            D2D1::RectF((float)rect.left, (float)rect.top, (float)rect.right, (float)rect.bottom),
            D2D1_ANTIALIAS_MODE_PER_PRIMITIVE
        );

        int y = rect.top - m_scrollY;

        for (int i = 0; i < static_cast<int>(m_items.size()); i++)
        {
            CRect itemRect(rect.left, y, rect.right, y + ROW_HEIGHT);

            // 화면에 보이는 항목만 그리기
            if (itemRect.bottom > rect.top && itemRect.top < rect.bottom)
            {
                DrawItemD2D(i, itemRect);
            }

            y += ROW_HEIGHT;
        }

        m_pRenderTarget->PopAxisAlignedClip();
    }

    void CMListView::DrawItemD2D(int index, const CRect& itemRect)
    {
        if (!m_pRenderTarget || !m_isD2DDrawing)
            return;

        const ThemeColors& colors = GetColors();
        const ListViewItem& item = m_items[index];

        // 배경
        COLORREF bgColor = colors.surface.normal;

        if (m_selection[index])
        {
            bgColor = colors.primary.normal;
        }
        else if (index == m_hoverIndex)
        {
            bgColor = colors.surface.hover;
        }
        else if (m_alternateRows && (index % 2 == 1))
        {
            bgColor = colors.surface.hover;
        }

        D2D1_RECT_F d2dRect = D2D1::RectF(
            (float)itemRect.left, (float)itemRect.top,
            (float)itemRect.right, (float)itemRect.bottom
        );
        m_pSolidBrush->SetColor(ToD2DColor(bgColor));
        m_pRenderTarget->FillRectangle(d2dRect, m_pSolidBrush);

        // 체크박스
        int x = itemRect.left + 8;

        if (m_showCheckboxes)
        {
            CRect checkRect(x, itemRect.top + (ROW_HEIGHT - CHECKBOX_SIZE) / 2,
                           x + CHECKBOX_SIZE, itemRect.top + (ROW_HEIGHT + CHECKBOX_SIZE) / 2);

            if (item.checked)
            {
                // 체크된 배경
                D2D1_RECT_F d2dCheckRect = D2D1::RectF(
                    (float)checkRect.left, (float)checkRect.top,
                    (float)checkRect.right, (float)checkRect.bottom
                );
                m_pSolidBrush->SetColor(ToD2DColor(colors.primary.normal));
                m_pRenderTarget->FillRectangle(d2dCheckRect, m_pSolidBrush);

                // 체크마크
                m_pSolidBrush->SetColor(ToD2DColor(colors.textOnPrimary));
                float cx = (float)checkRect.CenterPoint().x;
                float cy = (float)checkRect.CenterPoint().y;

                m_pRenderTarget->DrawLine(
                    D2D1::Point2F(cx - 4, cy),
                    D2D1::Point2F(cx - 1, cy + 3),
                    m_pSolidBrush, 2.0f
                );
                m_pRenderTarget->DrawLine(
                    D2D1::Point2F(cx - 1, cy + 3),
                    D2D1::Point2F(cx + 4, cy - 3),
                    m_pSolidBrush, 2.0f
                );
            }
            else
            {
                // 빈 체크박스
                D2D1_ROUNDED_RECT roundedRect = D2D1::RoundedRect(
                    D2D1::RectF((float)checkRect.left, (float)checkRect.top,
                               (float)checkRect.right, (float)checkRect.bottom),
                    3.0f, 3.0f
                );
                m_pSolidBrush->SetColor(ToD2DColor(colors.border.normal));
                m_pRenderTarget->DrawRoundedRectangle(roundedRect, m_pSolidBrush, 1.0f);
            }

            x += 32;
        }

        // 텍스트
        COLORREF textColor = m_selection[index] ? colors.textOnPrimary : colors.text;

        for (size_t col = 0; col < m_columns.size() && col < item.texts.size(); col++)
        {
            CRect textRect(x, itemRect.top, x + m_columns[col].width - 8, itemRect.bottom);
            DrawTextD2D(item.texts[col], textRect, textColor, TextStyle::Body,
                DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
            x += m_columns[col].width;
        }
    }
}
