#include "pch.h"
#include "CMDataGrid.h"

namespace MFCModernUI
{
    IMPLEMENT_DYNAMIC(CMDataGrid, CMControlBase)

    BEGIN_MESSAGE_MAP(CMDataGrid, CMControlBase)
        ON_WM_LBUTTONDOWN()
        ON_WM_LBUTTONDBLCLK()
        ON_WM_KEYDOWN()
        ON_WM_MOUSEWHEEL()
        ON_WM_SIZE()
        ON_WM_HSCROLL()
        ON_WM_VSCROLL()
    END_MESSAGE_MAP()

    CMDataGrid::CMDataGrid()
        : m_selectedRow(-1)
        , m_selectedCol(-1)
        , m_multiSelect(FALSE)
        , m_sortColumn(-1)
        , m_sortOrder(ColumnSortOrder::None)
        , m_showHeader(TRUE)
        , m_showGridLines(TRUE)
        , m_rowHeight(28)
        , m_headerHeight(32)
        , m_alternateRowColors(TRUE)
        , m_scrollX(0)
        , m_scrollY(0)
        , m_totalWidth(0)
        , m_totalHeight(0)
        , m_selectionChangedHandler(nullptr)
        , m_selectionChangedContext(nullptr)
        , m_cellClickHandler(nullptr)
        , m_cellClickContext(nullptr)
        , m_cellDoubleClickHandler(nullptr)
        , m_cellDoubleClickContext(nullptr)
        , m_columnSortHandler(nullptr)
        , m_columnSortContext(nullptr)
    {
        m_themeClass = _T("datagrid");
        m_tabStop = TRUE;
    }

    CMDataGrid::~CMDataGrid()
    {
        RemoveAllRows();
        RemoveAllColumns();
    }

    BOOL CMDataGrid::Create(DWORD dwStyle, const CRect& rect, CWnd* pParentWnd, UINT nID)
    {
        CString className = AfxRegisterWndClass(
            CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS,
            LoadCursor(nullptr, IDC_ARROW),
            nullptr,
            nullptr
        );

        return CWnd::Create(className, nullptr,
            dwStyle | WS_CHILD | WS_TABSTOP | WS_VSCROLL | WS_HSCROLL,
            rect, pParentWnd, nID);
    }

    int CMDataGrid::AddColumn(LPCTSTR header, int width, CellType type)
    {
        CMGridColumn col;
        col.header = header;
        col.width = width;
        col.type = type;

        int index = (int)m_columns.Add(col);

        // 기존 행에 셀 추가
        for (INT_PTR i = 0; i < m_rows.GetSize(); i++)
        {
            m_rows[i].cells.Add(CMGridCell());
        }

        CalculateTotalSize();
        UpdateScrollBars();
        Invalidate();

        return index;
    }

    void CMDataGrid::SetColumn(int col, const CMGridColumn& column)
    {
        if (col >= 0 && col < m_columns.GetSize())
        {
            m_columns[col] = column;
            CalculateTotalSize();
            UpdateScrollBars();
            Invalidate();
        }
    }

    CMGridColumn* CMDataGrid::GetColumn(int col)
    {
        if (col >= 0 && col < m_columns.GetSize())
            return &m_columns[col];
        return nullptr;
    }

    void CMDataGrid::RemoveColumn(int col)
    {
        if (col >= 0 && col < m_columns.GetSize())
        {
            m_columns.RemoveAt(col);

            // 행에서도 제거
            for (INT_PTR i = 0; i < m_rows.GetSize(); i++)
            {
                if (col < m_rows[i].cells.GetSize())
                    m_rows[i].cells.RemoveAt(col);
            }

            CalculateTotalSize();
            UpdateScrollBars();
            Invalidate();
        }
    }

    void CMDataGrid::RemoveAllColumns()
    {
        m_columns.RemoveAll();

        for (INT_PTR i = 0; i < m_rows.GetSize(); i++)
        {
            m_rows[i].cells.RemoveAll();
        }

        CalculateTotalSize();
        UpdateScrollBars();
        Invalidate();
    }

    int CMDataGrid::AddRow()
    {
        CMGridRow row;

        // 컬럼 수만큼 셀 추가
        for (INT_PTR i = 0; i < m_columns.GetSize(); i++)
        {
            row.cells.Add(CMGridCell());
        }

        int index = (int)m_rows.Add(row);

        CalculateTotalSize();
        UpdateScrollBars();
        Invalidate();

        return index;
    }

    int CMDataGrid::InsertRow(int rowIndex)
    {
        CMGridRow row;

        for (INT_PTR i = 0; i < m_columns.GetSize(); i++)
        {
            row.cells.Add(CMGridCell());
        }

        if (rowIndex < 0 || rowIndex >= m_rows.GetSize())
        {
            return (int)m_rows.Add(row);
        }

        m_rows.InsertAt(rowIndex, row);

        CalculateTotalSize();
        UpdateScrollBars();
        Invalidate();

        return rowIndex;
    }

    void CMDataGrid::RemoveRow(int row)
    {
        if (row >= 0 && row < m_rows.GetSize())
        {
            m_rows.RemoveAt(row);

            if (m_selectedRow == row)
                m_selectedRow = -1;
            else if (m_selectedRow > row)
                m_selectedRow--;

            CalculateTotalSize();
            UpdateScrollBars();
            Invalidate();
        }
    }

    void CMDataGrid::RemoveAllRows()
    {
        m_rows.RemoveAll();
        m_selectedRow = -1;
        m_selectedCol = -1;

        CalculateTotalSize();
        UpdateScrollBars();
        Invalidate();
    }

    void CMDataGrid::SetCellText(int row, int col, LPCTSTR text)
    {
        if (row >= 0 && row < m_rows.GetSize() &&
            col >= 0 && col < m_rows[row].cells.GetSize())
        {
            m_rows[row].cells[col].text = text;
            Invalidate();
        }
    }

    CString CMDataGrid::GetCellText(int row, int col)
    {
        if (row >= 0 && row < m_rows.GetSize() &&
            col >= 0 && col < m_rows[row].cells.GetSize())
        {
            return m_rows[row].cells[col].text;
        }
        return _T("");
    }

    void CMDataGrid::SetCellData(int row, int col, LPARAM data)
    {
        if (row >= 0 && row < m_rows.GetSize() &&
            col >= 0 && col < m_rows[row].cells.GetSize())
        {
            m_rows[row].cells[col].data = data;
        }
    }

    LPARAM CMDataGrid::GetCellData(int row, int col)
    {
        if (row >= 0 && row < m_rows.GetSize() &&
            col >= 0 && col < m_rows[row].cells.GetSize())
        {
            return m_rows[row].cells[col].data;
        }
        return 0;
    }

    void CMDataGrid::SetCellChecked(int row, int col, BOOL checked)
    {
        if (row >= 0 && row < m_rows.GetSize() &&
            col >= 0 && col < m_rows[row].cells.GetSize())
        {
            m_rows[row].cells[col].checked = checked;
            Invalidate();
        }
    }

    BOOL CMDataGrid::GetCellChecked(int row, int col)
    {
        if (row >= 0 && row < m_rows.GetSize() &&
            col >= 0 && col < m_rows[row].cells.GetSize())
        {
            return m_rows[row].cells[col].checked;
        }
        return FALSE;
    }

    void CMDataGrid::SetCellColors(int row, int col, COLORREF textColor, COLORREF bgColor)
    {
        if (row >= 0 && row < m_rows.GetSize() &&
            col >= 0 && col < m_rows[row].cells.GetSize())
        {
            m_rows[row].cells[col].textColor = textColor;
            m_rows[row].cells[col].bgColor = bgColor;
            m_rows[row].cells[col].useCustomColors = TRUE;
            Invalidate();
        }
    }

    void CMDataGrid::SetRowData(int row, LPARAM data)
    {
        if (row >= 0 && row < m_rows.GetSize())
        {
            m_rows[row].data = data;
        }
    }

    LPARAM CMDataGrid::GetRowData(int row)
    {
        if (row >= 0 && row < m_rows.GetSize())
        {
            return m_rows[row].data;
        }
        return 0;
    }

    void CMDataGrid::SetSelectedCell(int row, int col)
    {
        if (m_selectedRow != row || m_selectedCol != col)
        {
            // 이전 선택 해제
            if (m_selectedRow >= 0 && m_selectedRow < m_rows.GetSize())
            {
                m_rows[m_selectedRow].selected = FALSE;
            }

            m_selectedRow = row;
            m_selectedCol = col;

            // 새 선택
            if (m_selectedRow >= 0 && m_selectedRow < m_rows.GetSize())
            {
                m_rows[m_selectedRow].selected = TRUE;
            }

            if (m_selectionChangedHandler)
            {
                m_selectionChangedHandler(m_selectionChangedContext, row, col);
            }

            Invalidate();
        }
    }

    void CMDataGrid::SetMultiSelect(BOOL multi)
    {
        m_multiSelect = multi;
    }

    void CMDataGrid::SetShowHeader(BOOL show)
    {
        if (m_showHeader != show)
        {
            m_showHeader = show;
            CalculateTotalSize();
            UpdateScrollBars();
            Invalidate();
        }
    }

    void CMDataGrid::SetShowGridLines(BOOL show)
    {
        if (m_showGridLines != show)
        {
            m_showGridLines = show;
            Invalidate();
        }
    }

    void CMDataGrid::SetRowHeight(int height)
    {
        if (m_rowHeight != height)
        {
            m_rowHeight = height;
            CalculateTotalSize();
            UpdateScrollBars();
            Invalidate();
        }
    }

    void CMDataGrid::SetHeaderHeight(int height)
    {
        if (m_headerHeight != height)
        {
            m_headerHeight = height;
            CalculateTotalSize();
            UpdateScrollBars();
            Invalidate();
        }
    }

    void CMDataGrid::SetAlternateRowColors(BOOL alternate)
    {
        if (m_alternateRowColors != alternate)
        {
            m_alternateRowColors = alternate;
            Invalidate();
        }
    }

    void CMDataGrid::SortByColumn(int col, ColumnSortOrder order)
    {
        if (col < 0 || col >= m_columns.GetSize())
            return;

        m_sortColumn = col;
        m_sortOrder = order;

        // 실제 정렬 로직 (간단한 버블 정렬)
        if (order != ColumnSortOrder::None)
        {
            for (INT_PTR i = 0; i < m_rows.GetSize() - 1; i++)
            {
                for (INT_PTR j = 0; j < m_rows.GetSize() - i - 1; j++)
                {
                    CString& text1 = m_rows[j].cells[col].text;
                    CString& text2 = m_rows[j + 1].cells[col].text;

                    int cmp = text1.Compare(text2);
                    BOOL shouldSwap = (order == ColumnSortOrder::Ascending) ? (cmp > 0) : (cmp < 0);

                    if (shouldSwap)
                    {
                        CMGridRow temp = m_rows[j];
                        m_rows[j] = m_rows[j + 1];
                        m_rows[j + 1] = temp;
                    }
                }
            }
        }

        if (m_columnSortHandler)
        {
            m_columnSortHandler(m_columnSortContext, col, order);
        }

        Invalidate();
    }

    void CMDataGrid::SetSelectionChangedHandler(GridSelectionChangedHandler handler, void* context)
    {
        m_selectionChangedHandler = handler;
        m_selectionChangedContext = context;
    }

    void CMDataGrid::SetCellClickHandler(GridCellClickHandler handler, void* context)
    {
        m_cellClickHandler = handler;
        m_cellClickContext = context;
    }

    void CMDataGrid::SetCellDoubleClickHandler(GridCellDoubleClickHandler handler, void* context)
    {
        m_cellDoubleClickHandler = handler;
        m_cellDoubleClickContext = context;
    }

    void CMDataGrid::SetColumnSortHandler(GridColumnSortHandler handler, void* context)
    {
        m_columnSortHandler = handler;
        m_columnSortContext = context;
    }

    void CMDataGrid::EnsureVisible(int row)
    {
        if (row < 0 || row >= m_rows.GetSize())
            return;

        CRect rect;
        GetClientRect(&rect);

        int headerOffset = m_showHeader ? m_headerHeight : 0;
        int visibleHeight = rect.Height() - headerOffset;

        int rowTop = row * m_rowHeight;
        int rowBottom = rowTop + m_rowHeight;

        if (rowTop < m_scrollY)
        {
            m_scrollY = rowTop;
        }
        else if (rowBottom > m_scrollY + visibleHeight)
        {
            m_scrollY = rowBottom - visibleHeight;
        }

        SetScrollPos(SB_VERT, m_scrollY, TRUE);
        Invalidate();
    }

    void CMDataGrid::OnDraw(CDC* pDC)
    {
        CRect rect;
        GetClientRect(&rect);

        const ThemeColors& colors = GetColors();

        // 배경
        pDC->FillSolidRect(rect, colors.surface.normal);

        // 테두리
        CPen pen(PS_SOLID, m_isFocused ? 2 : 1,
            m_isFocused ? colors.primary.normal : colors.border.normal);
        CPen* oldPen = pDC->SelectObject(&pen);
        CBrush* oldBrush = (CBrush*)pDC->SelectStockObject(NULL_BRUSH);
        pDC->Rectangle(rect);
        pDC->SelectObject(oldPen);
        pDC->SelectObject(oldBrush);

        // 클리핑
        CRect clipRect = rect;
        clipRect.DeflateRect(1, 1);
        pDC->IntersectClipRect(clipRect);

        // 헤더
        if (m_showHeader)
        {
            CRect headerRect = rect;
            headerRect.bottom = headerRect.top + m_headerHeight;
            DrawHeader(pDC, headerRect);
        }

        // 행들
        CRect rowsRect = rect;
        if (m_showHeader)
            rowsRect.top += m_headerHeight;
        DrawRows(pDC, rowsRect);
    }

    void CMDataGrid::DrawHeader(CDC* pDC, const CRect& rect)
    {
        const ThemeColors& colors = GetColors();

        // 헤더 배경
        pDC->FillSolidRect(rect, colors.surface.hover);

        // 하단 라인
        CPen pen(PS_SOLID, 1, colors.border.normal);
        CPen* oldPen = pDC->SelectObject(&pen);
        pDC->MoveTo(rect.left, rect.bottom - 1);
        pDC->LineTo(rect.right, rect.bottom - 1);
        pDC->SelectObject(oldPen);

        // 각 컬럼 헤더
        CFont* pFont = CMThemeManager::GetInstance().GetFont(TextStyle::BodyBold);
        CFont* oldFont = pDC->SelectObject(pFont);
        pDC->SetBkMode(TRANSPARENT);
        pDC->SetTextColor(colors.text);

        int x = -m_scrollX;
        for (INT_PTR i = 0; i < m_columns.GetSize(); i++)
        {
            const CMGridColumn& col = m_columns[i];

            CRect colRect(x, rect.top, x + col.width, rect.bottom);

            // 컬럼 텍스트
            CRect textRect = colRect;
            textRect.DeflateRect(8, 0);

            if (m_sortColumn == i && m_sortOrder != ColumnSortOrder::None)
            {
                textRect.right -= 16;
            }

            pDC->DrawText(col.header, textRect,
                col.textAlign | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);

            // 정렬 표시
            if (m_sortColumn == i && m_sortOrder != ColumnSortOrder::None)
            {
                CRect sortRect = colRect;
                sortRect.left = sortRect.right - 20;
                sortRect.DeflateRect(4, 0);
                DrawSortIndicator(pDC, sortRect, m_sortOrder);
            }

            // 수직 구분선
            if (m_showGridLines && i < m_columns.GetSize() - 1)
            {
                CPen linePen(PS_SOLID, 1, colors.border.normal);
                oldPen = pDC->SelectObject(&linePen);
                pDC->MoveTo(colRect.right - 1, rect.top);
                pDC->LineTo(colRect.right - 1, rect.bottom);
                pDC->SelectObject(oldPen);
            }

            x += col.width;
        }

        pDC->SelectObject(oldFont);
    }

    void CMDataGrid::DrawRows(CDC* pDC, const CRect& rect)
    {
        const ThemeColors& colors = GetColors();

        CFont* pFont = CMThemeManager::GetInstance().GetFont(TextStyle::Body);
        CFont* oldFont = pDC->SelectObject(pFont);
        pDC->SetBkMode(TRANSPARENT);

        int y = rect.top - m_scrollY;
        for (INT_PTR row = 0; row < m_rows.GetSize(); row++)
        {
            if (y + m_rowHeight < rect.top)
            {
                y += m_rowHeight;
                continue;
            }

            if (y > rect.bottom)
                break;

            CRect rowRect(rect.left, y, rect.right, y + m_rowHeight);

            // 행 배경
            COLORREF rowBg = colors.surface.normal;
            if (m_rows[row].selected)
            {
                rowBg = colors.primary.normal;
            }
            else if (m_alternateRowColors && (row % 2 == 1))
            {
                rowBg = colors.surface.hover;
            }

            pDC->FillSolidRect(rowRect, rowBg);

            // 각 셀
            int x = -m_scrollX;
            for (INT_PTR col = 0; col < m_columns.GetSize(); col++)
            {
                CRect cellRect(x, y, x + m_columns[col].width, y + m_rowHeight);
                DrawCell(pDC, (int)row, (int)col, cellRect);
                x += m_columns[col].width;
            }

            // 행 하단 라인
            if (m_showGridLines)
            {
                CPen linePen(PS_SOLID, 1, colors.border.normal);
                CPen* oldPen = pDC->SelectObject(&linePen);
                pDC->MoveTo(rect.left, y + m_rowHeight - 1);
                pDC->LineTo(rect.right, y + m_rowHeight - 1);
                pDC->SelectObject(oldPen);
            }

            y += m_rowHeight;
        }

        pDC->SelectObject(oldFont);
    }

    void CMDataGrid::DrawCell(CDC* pDC, int row, int col, const CRect& rect)
    {
        if (row < 0 || row >= m_rows.GetSize())
            return;
        if (col < 0 || col >= m_rows[row].cells.GetSize())
            return;

        const ThemeColors& colors = GetColors();
        const CMGridCell& cell = m_rows[row].cells[col];
        const CMGridColumn& column = m_columns[col];

        // 텍스트 색상
        COLORREF textColor;
        if (cell.useCustomColors)
        {
            textColor = cell.textColor;
        }
        else if (m_rows[row].selected)
        {
            textColor = colors.textOnPrimary;
        }
        else
        {
            textColor = colors.text;
        }

        pDC->SetTextColor(textColor);

        CRect contentRect = rect;
        contentRect.DeflateRect(8, 0);

        switch (column.type)
        {
        case CellType::Checkbox:
        {
            // 체크박스
            int checkSize = 16;
            CRect checkRect(
                contentRect.left,
                rect.CenterPoint().y - checkSize / 2,
                contentRect.left + checkSize,
                rect.CenterPoint().y + checkSize / 2
            );

            CPen pen(PS_SOLID, 1, colors.border.normal);
            CPen* oldPen = pDC->SelectObject(&pen);
            CBrush* oldBrush = (CBrush*)pDC->SelectStockObject(NULL_BRUSH);
            pDC->RoundRect(checkRect, CPoint(3, 3));
            pDC->SelectObject(oldPen);
            pDC->SelectObject(oldBrush);

            if (cell.checked)
            {
                CPen checkPen(PS_SOLID, 2, colors.primary.normal);
                oldPen = pDC->SelectObject(&checkPen);
                int cx = checkRect.CenterPoint().x;
                int cy = checkRect.CenterPoint().y;
                pDC->MoveTo(cx - 3, cy);
                pDC->LineTo(cx - 1, cy + 3);
                pDC->LineTo(cx + 4, cy - 2);
                pDC->SelectObject(oldPen);
            }
            break;
        }

        case CellType::Number:
            contentRect.right -= 4;
            pDC->DrawText(cell.text, contentRect,
                DT_RIGHT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
            break;

        case CellType::Text:
        default:
            pDC->DrawText(cell.text, contentRect,
                column.textAlign | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
            break;
        }

        // 수직 구분선
        if (m_showGridLines && col < m_columns.GetSize() - 1)
        {
            CPen linePen(PS_SOLID, 1, colors.border.normal);
            CPen* oldPen = pDC->SelectObject(&linePen);
            pDC->MoveTo(rect.right - 1, rect.top);
            pDC->LineTo(rect.right - 1, rect.bottom);
            pDC->SelectObject(oldPen);
        }
    }

    void CMDataGrid::DrawSortIndicator(CDC* pDC, const CRect& rect, ColumnSortOrder order)
    {
        const ThemeColors& colors = GetColors();

        int cx = rect.CenterPoint().x;
        int cy = rect.CenterPoint().y;
        int size = 4;

        POINT points[3];
        if (order == ColumnSortOrder::Ascending)
        {
            points[0] = { cx, cy - size };
            points[1] = { cx - size, cy + size / 2 };
            points[2] = { cx + size, cy + size / 2 };
        }
        else
        {
            points[0] = { cx, cy + size };
            points[1] = { cx - size, cy - size / 2 };
            points[2] = { cx + size, cy - size / 2 };
        }

        CBrush brush(colors.textSecondary);
        CPen pen(PS_SOLID, 1, colors.textSecondary);
        CPen* oldPen = pDC->SelectObject(&pen);
        CBrush* oldBrush = pDC->SelectObject(&brush);
        pDC->Polygon(points, 3);
        pDC->SelectObject(oldPen);
        pDC->SelectObject(oldBrush);
    }

    int CMDataGrid::GetColumnX(int col)
    {
        int x = 0;
        for (int i = 0; i < col && i < m_columns.GetSize(); i++)
        {
            x += m_columns[i].width;
        }
        return x - m_scrollX;
    }

    int CMDataGrid::HitTestColumn(CPoint point)
    {
        int x = -m_scrollX;
        for (INT_PTR i = 0; i < m_columns.GetSize(); i++)
        {
            if (point.x >= x && point.x < x + m_columns[i].width)
                return (int)i;
            x += m_columns[i].width;
        }
        return -1;
    }

    int CMDataGrid::HitTestRow(CPoint point)
    {
        int headerOffset = m_showHeader ? m_headerHeight : 0;

        if (point.y < headerOffset)
            return -1;

        int row = (point.y - headerOffset + m_scrollY) / m_rowHeight;

        if (row >= 0 && row < m_rows.GetSize())
            return row;

        return -1;
    }

    CRect CMDataGrid::GetCellRect(int row, int col)
    {
        int headerOffset = m_showHeader ? m_headerHeight : 0;
        int x = GetColumnX(col);
        int y = headerOffset + row * m_rowHeight - m_scrollY;

        return CRect(x, y, x + m_columns[col].width, y + m_rowHeight);
    }

    void CMDataGrid::CalculateTotalSize()
    {
        m_totalWidth = 0;
        for (INT_PTR i = 0; i < m_columns.GetSize(); i++)
        {
            m_totalWidth += m_columns[i].width;
        }

        m_totalHeight = (int)m_rows.GetSize() * m_rowHeight;
        if (m_showHeader)
            m_totalHeight += m_headerHeight;
    }

    void CMDataGrid::UpdateScrollBars()
    {
        CRect rect;
        GetClientRect(&rect);

        int headerOffset = m_showHeader ? m_headerHeight : 0;

        // 수직 스크롤바
        SCROLLINFO vsi = { 0 };
        vsi.cbSize = sizeof(SCROLLINFO);
        vsi.fMask = SIF_RANGE | SIF_PAGE | SIF_POS;
        vsi.nMin = 0;
        vsi.nMax = (int)m_rows.GetSize() * m_rowHeight;
        vsi.nPage = rect.Height() - headerOffset;
        vsi.nPos = m_scrollY;
        SetScrollInfo(SB_VERT, &vsi, TRUE);

        // 수평 스크롤바
        SCROLLINFO hsi = { 0 };
        hsi.cbSize = sizeof(SCROLLINFO);
        hsi.fMask = SIF_RANGE | SIF_PAGE | SIF_POS;
        hsi.nMin = 0;
        hsi.nMax = m_totalWidth;
        hsi.nPage = rect.Width();
        hsi.nPos = m_scrollX;
        SetScrollInfo(SB_HORZ, &hsi, TRUE);

        // 스크롤 위치 조정
        int maxVScroll = max(0, (int)m_rows.GetSize() * m_rowHeight - (rect.Height() - headerOffset));
        if (m_scrollY > maxVScroll)
            m_scrollY = maxVScroll;

        int maxHScroll = max(0, m_totalWidth - rect.Width());
        if (m_scrollX > maxHScroll)
            m_scrollX = maxHScroll;
    }

    void CMDataGrid::OnLButtonDown(UINT nFlags, CPoint point)
    {
        SetFocus();

        int headerOffset = m_showHeader ? m_headerHeight : 0;

        // 헤더 클릭 (정렬)
        if (m_showHeader && point.y < headerOffset)
        {
            int col = HitTestColumn(point);
            if (col >= 0 && m_columns[col].sortable)
            {
                ColumnSortOrder newOrder;
                if (m_sortColumn == col)
                {
                    switch (m_sortOrder)
                    {
                    case ColumnSortOrder::None:
                        newOrder = ColumnSortOrder::Ascending;
                        break;
                    case ColumnSortOrder::Ascending:
                        newOrder = ColumnSortOrder::Descending;
                        break;
                    default:
                        newOrder = ColumnSortOrder::None;
                        break;
                    }
                }
                else
                {
                    newOrder = ColumnSortOrder::Ascending;
                }

                SortByColumn(col, newOrder);
            }
        }
        else
        {
            // 셀 클릭
            int row = HitTestRow(point);
            int col = HitTestColumn(point);

            if (row >= 0)
            {
                // 체크박스 셀 처리
                if (col >= 0 && m_columns[col].type == CellType::Checkbox)
                {
                    m_rows[row].cells[col].checked = !m_rows[row].cells[col].checked;
                }

                SetSelectedCell(row, col);

                if (m_cellClickHandler)
                {
                    m_cellClickHandler(m_cellClickContext, row, col);
                }
            }
        }

        CMControlBase::OnLButtonDown(nFlags, point);
    }

    void CMDataGrid::OnLButtonDblClk(UINT nFlags, CPoint point)
    {
        int row = HitTestRow(point);
        int col = HitTestColumn(point);

        if (row >= 0 && m_cellDoubleClickHandler)
        {
            m_cellDoubleClickHandler(m_cellDoubleClickContext, row, col);
        }

        CMControlBase::OnLButtonDblClk(nFlags, point);
    }

    void CMDataGrid::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
    {
        switch (nChar)
        {
        case VK_UP:
            if (m_selectedRow > 0)
            {
                SetSelectedCell(m_selectedRow - 1, m_selectedCol);
                EnsureVisible(m_selectedRow);
            }
            break;

        case VK_DOWN:
            if (m_selectedRow < m_rows.GetSize() - 1)
            {
                SetSelectedCell(m_selectedRow + 1, m_selectedCol);
                EnsureVisible(m_selectedRow);
            }
            break;

        case VK_LEFT:
            if (m_selectedCol > 0)
            {
                SetSelectedCell(m_selectedRow, m_selectedCol - 1);
            }
            break;

        case VK_RIGHT:
            if (m_selectedCol < m_columns.GetSize() - 1)
            {
                SetSelectedCell(m_selectedRow, m_selectedCol + 1);
            }
            break;

        case VK_HOME:
            if (m_rows.GetSize() > 0)
            {
                SetSelectedCell(0, m_selectedCol);
                EnsureVisible(0);
            }
            break;

        case VK_END:
            if (m_rows.GetSize() > 0)
            {
                SetSelectedCell((int)m_rows.GetSize() - 1, m_selectedCol);
                EnsureVisible((int)m_rows.GetSize() - 1);
            }
            break;

        case VK_SPACE:
            if (m_selectedRow >= 0 && m_selectedCol >= 0 &&
                m_columns[m_selectedCol].type == CellType::Checkbox)
            {
                m_rows[m_selectedRow].cells[m_selectedCol].checked =
                    !m_rows[m_selectedRow].cells[m_selectedCol].checked;
                Invalidate();
            }
            break;
        }

        CMControlBase::OnKeyDown(nChar, nRepCnt, nFlags);
    }

    BOOL CMDataGrid::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
    {
        int delta = -zDelta / WHEEL_DELTA * m_rowHeight * 3;
        int newScroll = m_scrollY + delta;

        CRect rect;
        GetClientRect(&rect);
        int headerOffset = m_showHeader ? m_headerHeight : 0;
        int maxScroll = max(0, (int)m_rows.GetSize() * m_rowHeight - (rect.Height() - headerOffset));

        newScroll = max(0, min(maxScroll, newScroll));

        if (newScroll != m_scrollY)
        {
            m_scrollY = newScroll;
            SetScrollPos(SB_VERT, m_scrollY, TRUE);
            Invalidate();
        }

        return TRUE;
    }

    void CMDataGrid::OnSize(UINT nType, int cx, int cy)
    {
        CMControlBase::OnSize(nType, cx, cy);
        UpdateScrollBars();
    }

    void CMDataGrid::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
    {
        CRect rect;
        GetClientRect(&rect);

        int maxScroll = max(0, m_totalWidth - rect.Width());

        switch (nSBCode)
        {
        case SB_LINELEFT:
            m_scrollX = max(0, m_scrollX - 20);
            break;
        case SB_LINERIGHT:
            m_scrollX = min(maxScroll, m_scrollX + 20);
            break;
        case SB_PAGELEFT:
            m_scrollX = max(0, m_scrollX - rect.Width());
            break;
        case SB_PAGERIGHT:
            m_scrollX = min(maxScroll, m_scrollX + rect.Width());
            break;
        case SB_THUMBTRACK:
        case SB_THUMBPOSITION:
            m_scrollX = nPos;
            break;
        }

        SetScrollPos(SB_HORZ, m_scrollX, TRUE);
        Invalidate();

        CMControlBase::OnHScroll(nSBCode, nPos, pScrollBar);
    }

    void CMDataGrid::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
    {
        CRect rect;
        GetClientRect(&rect);
        int headerOffset = m_showHeader ? m_headerHeight : 0;
        int maxScroll = max(0, (int)m_rows.GetSize() * m_rowHeight - (rect.Height() - headerOffset));

        switch (nSBCode)
        {
        case SB_LINEUP:
            m_scrollY = max(0, m_scrollY - m_rowHeight);
            break;
        case SB_LINEDOWN:
            m_scrollY = min(maxScroll, m_scrollY + m_rowHeight);
            break;
        case SB_PAGEUP:
            m_scrollY = max(0, m_scrollY - (rect.Height() - headerOffset));
            break;
        case SB_PAGEDOWN:
            m_scrollY = min(maxScroll, m_scrollY + (rect.Height() - headerOffset));
            break;
        case SB_THUMBTRACK:
        case SB_THUMBPOSITION:
            m_scrollY = nPos;
            break;
        }

        SetScrollPos(SB_VERT, m_scrollY, TRUE);
        Invalidate();

        CMControlBase::OnVScroll(nSBCode, nPos, pScrollBar);
    }
}
