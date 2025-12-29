#include "pch.h"
#include "CMTreeView.h"

namespace MFCModernUI
{
    // CMTreeNode 구현
    CMTreeNode::CMTreeNode()
        : data(0)
        , hIcon(nullptr)
        , expanded(FALSE)
        , selected(FALSE)
        , checked(FALSE)
        , parent(nullptr)
    {
    }

    CMTreeNode::~CMTreeNode()
    {
        RemoveAllChildren();
    }

    int CMTreeNode::GetLevel() const
    {
        int level = 0;
        CMTreeNode* p = parent;
        while (p)
        {
            level++;
            p = p->parent;
        }
        return level;
    }

    void CMTreeNode::RemoveAllChildren()
    {
        for (INT_PTR i = 0; i < children.GetSize(); i++)
        {
            delete children[i];
        }
        children.RemoveAll();
    }

    // CMTreeView 구현
    IMPLEMENT_DYNAMIC(CMTreeView, CMControlBase)

    BEGIN_MESSAGE_MAP(CMTreeView, CMControlBase)
        ON_WM_LBUTTONDOWN()
        ON_WM_LBUTTONDBLCLK()
        ON_WM_KEYDOWN()
        ON_WM_MOUSEWHEEL()
        ON_WM_SIZE()
    END_MESSAGE_MAP()

    CMTreeView::CMTreeView()
        : m_selectedNode(nullptr)
        , m_hoverNode(nullptr)
        , m_showCheckboxes(FALSE)
        , m_showLines(TRUE)
        , m_showRootLines(TRUE)
        , m_indent(20)
        , m_scrollY(0)
        , m_totalHeight(0)
        , m_selectionChangedHandler(nullptr)
        , m_selectionChangedContext(nullptr)
        , m_nodeExpandedHandler(nullptr)
        , m_nodeExpandedContext(nullptr)
        , m_nodeDoubleClickHandler(nullptr)
        , m_nodeDoubleClickContext(nullptr)
    {
        m_themeClass = _T("treeview");
        m_tabStop = TRUE;
    }

    CMTreeView::~CMTreeView()
    {
        RemoveAllNodes();
    }

    BOOL CMTreeView::Create(DWORD dwStyle, const CRect& rect, CWnd* pParentWnd, UINT nID)
    {
        CString className = AfxRegisterWndClass(
            CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS,
            LoadCursor(nullptr, IDC_ARROW),
            nullptr,
            nullptr
        );

        return CWnd::Create(className, nullptr, dwStyle | WS_CHILD | WS_TABSTOP | WS_VSCROLL, rect, pParentWnd, nID);
    }

    CMTreeNode* CMTreeView::AddNode(LPCTSTR text, CMTreeNode* parent)
    {
        return AddNode(text, nullptr, parent);
    }

    CMTreeNode* CMTreeView::AddNode(LPCTSTR text, HICON hIcon, CMTreeNode* parent)
    {
        CMTreeNode* node = new CMTreeNode();
        node->text = text;
        node->hIcon = hIcon;
        node->parent = parent;

        if (parent)
        {
            parent->children.Add(node);
        }
        else
        {
            m_rootNodes.Add(node);
        }

        UpdateScrollBars();
        Invalidate();
        return node;
    }

    CMTreeNode* CMTreeView::InsertNode(LPCTSTR text, CMTreeNode* parent, int index)
    {
        CMTreeNode* node = new CMTreeNode();
        node->text = text;
        node->parent = parent;

        if (parent)
        {
            if (index < 0 || index >= parent->children.GetSize())
                parent->children.Add(node);
            else
                parent->children.InsertAt(index, node);
        }
        else
        {
            if (index < 0 || index >= m_rootNodes.GetSize())
                m_rootNodes.Add(node);
            else
                m_rootNodes.InsertAt(index, node);
        }

        UpdateScrollBars();
        Invalidate();
        return node;
    }

    void CMTreeView::RemoveNode(CMTreeNode* node)
    {
        if (!node)
            return;

        // 선택 해제
        if (m_selectedNode == node)
            m_selectedNode = nullptr;

        // 부모에서 제거
        if (node->parent)
        {
            for (INT_PTR i = 0; i < node->parent->children.GetSize(); i++)
            {
                if (node->parent->children[i] == node)
                {
                    node->parent->children.RemoveAt(i);
                    break;
                }
            }
        }
        else
        {
            for (INT_PTR i = 0; i < m_rootNodes.GetSize(); i++)
            {
                if (m_rootNodes[i] == node)
                {
                    m_rootNodes.RemoveAt(i);
                    break;
                }
            }
        }

        delete node;
        UpdateScrollBars();
        Invalidate();
    }

    void CMTreeView::RemoveAllNodes()
    {
        m_selectedNode = nullptr;
        m_hoverNode = nullptr;

        for (INT_PTR i = 0; i < m_rootNodes.GetSize(); i++)
        {
            delete m_rootNodes[i];
        }
        m_rootNodes.RemoveAll();

        UpdateScrollBars();
        Invalidate();
    }

    CMTreeNode* CMTreeView::GetRootNode(int index)
    {
        if (index >= 0 && index < m_rootNodes.GetSize())
            return m_rootNodes[index];
        return nullptr;
    }

    int CMTreeView::GetRootNodeCount() const
    {
        return (int)m_rootNodes.GetSize();
    }

    CMTreeNode* CMTreeView::FindNode(LPARAM data)
    {
        for (INT_PTR i = 0; i < m_rootNodes.GetSize(); i++)
        {
            CMTreeNode* found = FindNodeByData(m_rootNodes[i], data);
            if (found)
                return found;
        }
        return nullptr;
    }

    CMTreeNode* CMTreeView::HitTest(CPoint point)
    {
        int y = -m_scrollY;
        for (INT_PTR i = 0; i < m_rootNodes.GetSize(); i++)
        {
            CMTreeNode* node = HitTestNode(m_rootNodes[i], point, y, 0);
            if (node)
                return node;
        }
        return nullptr;
    }

    void CMTreeView::SetSelectedNode(CMTreeNode* node)
    {
        if (m_selectedNode != node)
        {
            CMTreeNode* oldNode = m_selectedNode;

            if (m_selectedNode)
                m_selectedNode->selected = FALSE;

            m_selectedNode = node;

            if (m_selectedNode)
                m_selectedNode->selected = TRUE;

            if (m_selectionChangedHandler)
            {
                m_selectionChangedHandler(m_selectionChangedContext, oldNode, node);
            }

            Invalidate();
        }
    }

    void CMTreeView::ExpandNode(CMTreeNode* node, BOOL expand)
    {
        if (!node)
            return;

        if (node->expanded != expand)
        {
            node->expanded = expand;

            if (m_nodeExpandedHandler)
            {
                m_nodeExpandedHandler(m_nodeExpandedContext, node, expand);
            }

            UpdateScrollBars();
            Invalidate();
        }
    }

    void CMTreeView::ExpandAll()
    {
        CArray<CMTreeNode*> nodes;
        CollectVisibleNodes(nodes);

        for (INT_PTR i = 0; i < nodes.GetSize(); i++)
        {
            if (nodes[i]->HasChildren())
                nodes[i]->expanded = TRUE;
        }

        UpdateScrollBars();
        Invalidate();
    }

    void CMTreeView::CollapseAll()
    {
        for (INT_PTR i = 0; i < m_rootNodes.GetSize(); i++)
        {
            m_rootNodes[i]->expanded = FALSE;
        }

        UpdateScrollBars();
        Invalidate();
    }

    void CMTreeView::SetShowCheckboxes(BOOL show)
    {
        if (m_showCheckboxes != show)
        {
            m_showCheckboxes = show;
            Invalidate();
        }
    }

    void CMTreeView::SetShowLines(BOOL show)
    {
        if (m_showLines != show)
        {
            m_showLines = show;
            Invalidate();
        }
    }

    void CMTreeView::SetShowRootLines(BOOL show)
    {
        if (m_showRootLines != show)
        {
            m_showRootLines = show;
            Invalidate();
        }
    }

    void CMTreeView::SetIndent(int indent)
    {
        if (m_indent != indent)
        {
            m_indent = indent;
            Invalidate();
        }
    }

    void CMTreeView::SetSelectionChangedHandler(TreeSelectionChangedHandler handler, void* context)
    {
        m_selectionChangedHandler = handler;
        m_selectionChangedContext = context;
    }

    void CMTreeView::SetNodeExpandedHandler(TreeNodeExpandedHandler handler, void* context)
    {
        m_nodeExpandedHandler = handler;
        m_nodeExpandedContext = context;
    }

    void CMTreeView::SetNodeDoubleClickHandler(TreeNodeDoubleClickHandler handler, void* context)
    {
        m_nodeDoubleClickHandler = handler;
        m_nodeDoubleClickContext = context;
    }

    void CMTreeView::EnsureVisible(CMTreeNode* node)
    {
        if (!node)
            return;

        // 부모들을 모두 확장
        CMTreeNode* parent = node->parent;
        while (parent)
        {
            parent->expanded = TRUE;
            parent = parent->parent;
        }

        UpdateScrollBars();

        // 노드 위치 계산
        int y = 0;
        for (INT_PTR i = 0; i < m_rootNodes.GetSize(); i++)
        {
            // 간단히 구현 - 실제로는 정확한 위치 계산 필요
            y += CalculateNodeHeight(m_rootNodes[i]);
        }

        Invalidate();
    }

    void CMTreeView::OnDraw(CDC* pDC)
    {
        CRect rect;
        GetClientRect(&rect);

        const ThemeColors& colors = GetColors();

        // 배경
        pDC->FillSolidRect(rect, colors.surface.normal);

        // 테두리
        if (m_isFocused)
        {
            CPen pen(PS_SOLID, 2, colors.primary.normal);
            CPen* oldPen = pDC->SelectObject(&pen);
            CBrush* oldBrush = (CBrush*)pDC->SelectStockObject(NULL_BRUSH);
            pDC->Rectangle(rect);
            pDC->SelectObject(oldPen);
            pDC->SelectObject(oldBrush);
        }
        else
        {
            CPen pen(PS_SOLID, 1, colors.border.normal);
            CPen* oldPen = pDC->SelectObject(&pen);
            CBrush* oldBrush = (CBrush*)pDC->SelectStockObject(NULL_BRUSH);
            pDC->Rectangle(rect);
            pDC->SelectObject(oldPen);
            pDC->SelectObject(oldBrush);
        }

        // 클리핑 영역 설정
        CRect clipRect = rect;
        clipRect.DeflateRect(1, 1);
        pDC->IntersectClipRect(clipRect);

        // 노드 그리기
        int y = -m_scrollY + 2;
        for (INT_PTR i = 0; i < m_rootNodes.GetSize(); i++)
        {
            DrawNode(pDC, m_rootNodes[i], y, 0);
        }
    }

    void CMTreeView::DrawNode(CDC* pDC, CMTreeNode* node, int& y, int level)
    {
        CRect rect;
        GetClientRect(&rect);

        const ThemeColors& colors = GetColors();

        // 노드 영역
        CRect nodeRect(0, y, rect.right, y + ITEM_HEIGHT);

        // 화면에 보이는 경우만 그리기
        if (nodeRect.bottom > 0 && nodeRect.top < rect.bottom)
        {
            int x = GetNodeX(level);

            // 선택 배경
            if (node->selected)
            {
                CRect selRect = nodeRect;
                selRect.left = x;
                selRect.DeflateRect(2, 1);
                CBrush brush(colors.primary.normal);
                CPen pen(PS_SOLID, 1, colors.primary.normal);
                CPen* pOldPen = pDC->SelectObject(&pen);
                CBrush* pOldBrush = pDC->SelectObject(&brush);
                pDC->RoundRect(selRect, CPoint(4, 4));
                pDC->SelectObject(pOldPen);
                pDC->SelectObject(pOldBrush);
            }

            // 연결선
            if (m_showLines && (level > 0 || m_showRootLines))
            {
                DrawNodeLines(pDC, node, nodeRect, level);
            }

            // 확장 버튼
            if (node->HasChildren())
            {
                CRect expanderRect(
                    x - EXPANDER_SIZE - 4,
                    nodeRect.top + (ITEM_HEIGHT - EXPANDER_SIZE) / 2,
                    x - 4,
                    nodeRect.top + (ITEM_HEIGHT + EXPANDER_SIZE) / 2
                );
                DrawExpander(pDC, expanderRect, node->expanded, node == m_hoverNode);
            }

            // 체크박스
            if (m_showCheckboxes)
            {
                CRect checkRect(
                    x,
                    nodeRect.top + (ITEM_HEIGHT - CHECKBOX_SIZE) / 2,
                    x + CHECKBOX_SIZE,
                    nodeRect.top + (ITEM_HEIGHT + CHECKBOX_SIZE) / 2
                );
                DrawCheckbox(pDC, checkRect, node->checked);
                x += CHECKBOX_SIZE + 4;
            }

            // 아이콘
            if (node->hIcon)
            {
                int iconY = nodeRect.top + (ITEM_HEIGHT - ICON_SIZE) / 2;
                DrawIconEx(pDC->GetSafeHdc(), x, iconY, node->hIcon,
                    ICON_SIZE, ICON_SIZE, 0, nullptr, DI_NORMAL);
                x += ICON_SIZE + 4;
            }

            // 텍스트
            COLORREF textColor = node->selected ? colors.textOnPrimary : colors.text;

            CFont* pFont = CMThemeManager::GetInstance().GetFont(TextStyle::Body);
            CFont* oldFont = pDC->SelectObject(pFont);
            pDC->SetBkMode(TRANSPARENT);
            pDC->SetTextColor(textColor);

            CRect textRect = nodeRect;
            textRect.left = x;
            textRect.right -= 4;
            pDC->DrawText(node->text, textRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
            pDC->SelectObject(oldFont);
        }

        y += ITEM_HEIGHT;

        // 자식 노드들
        if (node->expanded)
        {
            for (INT_PTR i = 0; i < node->children.GetSize(); i++)
            {
                DrawNode(pDC, node->children[i], y, level + 1);
            }
        }
    }

    void CMTreeView::DrawExpander(CDC* pDC, const CRect& rect, BOOL expanded, BOOL hovered)
    {
        const ThemeColors& colors = GetColors();

        int cx = rect.CenterPoint().x;
        int cy = rect.CenterPoint().y;
        int size = 4;

        COLORREF arrowColor = hovered ? colors.primary.normal : colors.textSecondary;

        POINT points[3];
        if (expanded)
        {
            points[0] = { cx - size, cy - size / 2 };
            points[1] = { cx + size, cy - size / 2 };
            points[2] = { cx, cy + size / 2 };
        }
        else
        {
            points[0] = { cx - size / 2, cy - size };
            points[1] = { cx - size / 2, cy + size };
            points[2] = { cx + size / 2, cy };
        }

        CBrush brush(arrowColor);
        CPen pen(PS_SOLID, 1, arrowColor);
        CPen* oldPen = pDC->SelectObject(&pen);
        CBrush* oldBrush = pDC->SelectObject(&brush);
        pDC->Polygon(points, 3);
        pDC->SelectObject(oldPen);
        pDC->SelectObject(oldBrush);
    }

    void CMTreeView::DrawCheckbox(CDC* pDC, const CRect& rect, BOOL checked)
    {
        const ThemeColors& colors = GetColors();

        // 테두리
        CPen pen(PS_SOLID, 1, colors.border.normal);
        CPen* oldPen = pDC->SelectObject(&pen);
        CBrush* oldBrush = (CBrush*)pDC->SelectStockObject(NULL_BRUSH);
        pDC->RoundRect(rect, CPoint(3, 3));
        pDC->SelectObject(oldPen);
        pDC->SelectObject(oldBrush);

        if (checked)
        {
            // 체크 표시
            int cx = rect.CenterPoint().x;
            int cy = rect.CenterPoint().y;

            CPen checkPen(PS_SOLID, 2, colors.primary.normal);
            oldPen = pDC->SelectObject(&checkPen);
            pDC->MoveTo(cx - 3, cy);
            pDC->LineTo(cx - 1, cy + 3);
            pDC->LineTo(cx + 4, cy - 2);
            pDC->SelectObject(oldPen);
        }
    }

    void CMTreeView::DrawNodeLines(CDC* pDC, CMTreeNode* node, const CRect& rect, int level)
    {
        const ThemeColors& colors = GetColors();

        CPen pen(PS_DOT, 1, colors.border.normal);
        CPen* oldPen = pDC->SelectObject(&pen);

        int x = GetNodeX(level) - m_indent / 2;
        int cy = rect.CenterPoint().y;

        // 수평선
        pDC->MoveTo(x - m_indent / 2 + 4, cy);
        pDC->LineTo(x, cy);

        // 수직선
        if (level > 0 || m_showRootLines)
        {
            int top = rect.top;
            int bottom = rect.bottom;

            // 마지막 자식이면 중간까지만
            if (node->parent)
            {
                INT_PTR childIndex = -1;
                for (INT_PTR i = 0; i < node->parent->children.GetSize(); i++)
                {
                    if (node->parent->children[i] == node)
                    {
                        childIndex = i;
                        break;
                    }
                }
                if (childIndex == node->parent->children.GetSize() - 1)
                {
                    bottom = cy;
                }
            }

            pDC->MoveTo(x - m_indent / 2 + 4, top);
            pDC->LineTo(x - m_indent / 2 + 4, bottom);
        }

        pDC->SelectObject(oldPen);
    }

    int CMTreeView::CalculateTotalHeight()
    {
        int height = 0;
        for (INT_PTR i = 0; i < m_rootNodes.GetSize(); i++)
        {
            height += CalculateNodeHeight(m_rootNodes[i]);
        }
        return height;
    }

    int CMTreeView::CalculateNodeHeight(CMTreeNode* node)
    {
        int height = ITEM_HEIGHT;

        if (node->expanded)
        {
            for (INT_PTR i = 0; i < node->children.GetSize(); i++)
            {
                height += CalculateNodeHeight(node->children[i]);
            }
        }

        return height;
    }

    CMTreeNode* CMTreeView::HitTestNode(CMTreeNode* node, CPoint point, int& y, int level)
    {
        CRect rect;
        GetClientRect(&rect);

        CRect nodeRect(0, y, rect.right, y + ITEM_HEIGHT);

        if (nodeRect.PtInRect(point))
            return node;

        y += ITEM_HEIGHT;

        if (node->expanded)
        {
            for (INT_PTR i = 0; i < node->children.GetSize(); i++)
            {
                CMTreeNode* found = HitTestNode(node->children[i], point, y, level + 1);
                if (found)
                    return found;
            }
        }

        return nullptr;
    }

    int CMTreeView::GetNodeX(int level)
    {
        return 8 + (level + 1) * m_indent;
    }

    void CMTreeView::UpdateScrollBars()
    {
        CRect rect;
        GetClientRect(&rect);

        m_totalHeight = CalculateTotalHeight();

        SCROLLINFO si = { 0 };
        si.cbSize = sizeof(SCROLLINFO);
        si.fMask = SIF_RANGE | SIF_PAGE | SIF_POS;
        si.nMin = 0;
        si.nMax = m_totalHeight;
        si.nPage = rect.Height();
        si.nPos = m_scrollY;

        SetScrollInfo(SB_VERT, &si, TRUE);

        // 스크롤 위치 조정
        int maxScroll = max(0, m_totalHeight - rect.Height());
        if (m_scrollY > maxScroll)
            m_scrollY = maxScroll;
    }

    void CMTreeView::CollectVisibleNodes(CArray<CMTreeNode*>& nodes, CMTreeNode* parent)
    {
        CArray<CMTreeNode*>* children = parent ? &parent->children : &m_rootNodes;

        for (INT_PTR i = 0; i < children->GetSize(); i++)
        {
            CMTreeNode* node = children->GetAt(i);
            nodes.Add(node);

            if (node->expanded)
            {
                CollectVisibleNodes(nodes, node);
            }
        }
    }

    CMTreeNode* CMTreeView::FindNodeByData(CMTreeNode* node, LPARAM data)
    {
        if (node->data == data)
            return node;

        for (INT_PTR i = 0; i < node->children.GetSize(); i++)
        {
            CMTreeNode* found = FindNodeByData(node->children[i], data);
            if (found)
                return found;
        }

        return nullptr;
    }

    void CMTreeView::OnLButtonDown(UINT nFlags, CPoint point)
    {
        SetFocus();

        CMTreeNode* node = HitTest(point);
        if (node)
        {
            int level = node->GetLevel();
            int x = GetNodeX(level);

            // 확장 버튼 영역 확인
            CRect expanderRect(
                x - EXPANDER_SIZE - 4 - 8,
                0,
                x - 4 + 8,
                ITEM_HEIGHT
            );

            // Y 좌표는 대략적으로 계산
            int nodeY = 0;
            // ... (실제로는 정확한 Y 계산 필요)

            // 확장 버튼 클릭
            if (node->HasChildren() && point.x < x)
            {
                ExpandNode(node, !node->expanded);
            }
            else if (m_showCheckboxes && point.x >= x && point.x < x + CHECKBOX_SIZE)
            {
                // 체크박스 클릭
                node->checked = !node->checked;
                Invalidate();
            }
            else
            {
                // 선택
                SetSelectedNode(node);
            }
        }

        CMControlBase::OnLButtonDown(nFlags, point);
    }

    void CMTreeView::OnLButtonDblClk(UINT nFlags, CPoint point)
    {
        CMTreeNode* node = HitTest(point);
        if (node)
        {
            if (node->HasChildren())
            {
                ExpandNode(node, !node->expanded);
            }

            if (m_nodeDoubleClickHandler)
            {
                m_nodeDoubleClickHandler(m_nodeDoubleClickContext, node);
            }
        }

        CMControlBase::OnLButtonDblClk(nFlags, point);
    }

    void CMTreeView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
    {
        CArray<CMTreeNode*> visibleNodes;
        CollectVisibleNodes(visibleNodes);

        int currentIndex = -1;
        for (INT_PTR i = 0; i < visibleNodes.GetSize(); i++)
        {
            if (visibleNodes[i] == m_selectedNode)
            {
                currentIndex = (int)i;
                break;
            }
        }

        switch (nChar)
        {
        case VK_UP:
            if (currentIndex > 0)
            {
                SetSelectedNode(visibleNodes[currentIndex - 1]);
            }
            break;

        case VK_DOWN:
            if (currentIndex < visibleNodes.GetSize() - 1)
            {
                SetSelectedNode(visibleNodes[currentIndex + 1]);
            }
            break;

        case VK_LEFT:
            if (m_selectedNode)
            {
                if (m_selectedNode->expanded)
                {
                    ExpandNode(m_selectedNode, FALSE);
                }
                else if (m_selectedNode->parent)
                {
                    SetSelectedNode(m_selectedNode->parent);
                }
            }
            break;

        case VK_RIGHT:
            if (m_selectedNode)
            {
                if (!m_selectedNode->expanded && m_selectedNode->HasChildren())
                {
                    ExpandNode(m_selectedNode, TRUE);
                }
                else if (m_selectedNode->HasChildren())
                {
                    SetSelectedNode(m_selectedNode->children[0]);
                }
            }
            break;

        case VK_SPACE:
            if (m_showCheckboxes && m_selectedNode)
            {
                m_selectedNode->checked = !m_selectedNode->checked;
                Invalidate();
            }
            break;

        case VK_RETURN:
            if (m_selectedNode && m_nodeDoubleClickHandler)
            {
                m_nodeDoubleClickHandler(m_nodeDoubleClickContext, m_selectedNode);
            }
            break;
        }

        CMControlBase::OnKeyDown(nChar, nRepCnt, nFlags);
    }

    BOOL CMTreeView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
    {
        int delta = -zDelta / WHEEL_DELTA * ITEM_HEIGHT * 3;
        int newScroll = m_scrollY + delta;

        CRect rect;
        GetClientRect(&rect);
        int maxScroll = max(0, m_totalHeight - rect.Height());

        newScroll = max(0, min(maxScroll, newScroll));

        if (newScroll != m_scrollY)
        {
            m_scrollY = newScroll;
            SetScrollPos(SB_VERT, m_scrollY, TRUE);
            Invalidate();
        }

        return TRUE;
    }

    void CMTreeView::OnSize(UINT nType, int cx, int cy)
    {
        CMControlBase::OnSize(nType, cx, cy);
        UpdateScrollBars();
    }
}
