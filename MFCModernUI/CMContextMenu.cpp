#include "pch.h"
#include "CMContextMenu.h"

namespace MFCModernUI
{
    IMPLEMENT_DYNAMIC(CMContextMenu, CWnd)

    BEGIN_MESSAGE_MAP(CMContextMenu, CWnd)
        ON_WM_PAINT()
        ON_WM_ERASEBKGND()
        ON_WM_MOUSEMOVE()
        ON_WM_LBUTTONUP()
        ON_WM_KEYDOWN()
        ON_WM_KILLFOCUS()
        ON_WM_TIMER()
    END_MESSAGE_MAP()

    CMContextMenu::CMContextMenu()
        : m_hoverIndex(-1)
        , m_selectedIndex(-1)
        , m_activeSubmenu(nullptr)
        , m_parentWnd(nullptr)
        , m_isSubmenu(FALSE)
        , m_clickHandler(nullptr)
        , m_clickContext(nullptr)
        , m_submenuTimerId(0)
    {
    }

    CMContextMenu::~CMContextMenu()
    {
        Clear();
    }

    void CMContextMenu::AddItem(UINT id, LPCTSTR text, LPCTSTR shortcut)
    {
        CMMenuItem item;
        item.id = id;
        item.text = text;
        if (shortcut)
            item.shortcut = shortcut;
        item.type = MenuItemType::Normal;
        m_items.Add(item);
    }

    void CMContextMenu::AddItem(UINT id, LPCTSTR text, HICON hIcon, LPCTSTR shortcut)
    {
        CMMenuItem item;
        item.id = id;
        item.text = text;
        if (shortcut)
            item.shortcut = shortcut;
        item.type = MenuItemType::Normal;
        item.hIcon = hIcon;
        m_items.Add(item);
    }

    void CMContextMenu::AddSeparator()
    {
        CMMenuItem item;
        item.type = MenuItemType::Separator;
        m_items.Add(item);
    }

    void CMContextMenu::AddCheckItem(UINT id, LPCTSTR text, BOOL checked)
    {
        CMMenuItem item;
        item.id = id;
        item.text = text;
        item.type = MenuItemType::Checkbox;
        item.checked = checked;
        m_items.Add(item);
    }

    void CMContextMenu::AddRadioItem(UINT id, LPCTSTR text, BOOL checked)
    {
        CMMenuItem item;
        item.id = id;
        item.text = text;
        item.type = MenuItemType::Radio;
        item.checked = checked;
        m_items.Add(item);
    }

    CMContextMenu* CMContextMenu::AddSubmenu(LPCTSTR text)
    {
        CMContextMenu* submenu = new CMContextMenu();
        submenu->m_isSubmenu = TRUE;
        m_submenus.Add(submenu);

        CMMenuItem item;
        item.text = text;
        item.type = MenuItemType::Submenu;
        item.submenu = &submenu->m_items;
        m_items.Add(item);

        return submenu;
    }

    void CMContextMenu::EnableItem(UINT id, BOOL enable)
    {
        CMMenuItem* item = FindItem(id);
        if (item)
        {
            item->enabled = enable;
        }
    }

    void CMContextMenu::CheckItem(UINT id, BOOL check)
    {
        CMMenuItem* item = FindItem(id);
        if (item)
        {
            item->checked = check;
        }
    }

    void CMContextMenu::SetItemText(UINT id, LPCTSTR text)
    {
        CMMenuItem* item = FindItem(id);
        if (item)
        {
            item->text = text;
        }
    }

    void CMContextMenu::Clear()
    {
        m_items.RemoveAll();

        // 서브메뉴 삭제
        for (INT_PTR i = 0; i < m_submenus.GetSize(); i++)
        {
            delete m_submenus[i];
        }
        m_submenus.RemoveAll();
    }

    BOOL CMContextMenu::Create(CWnd* pParentWnd)
    {
        CString className = AfxRegisterWndClass(
            CS_HREDRAW | CS_VREDRAW | CS_DROPSHADOW,
            LoadCursor(nullptr, IDC_ARROW),
            nullptr,
            nullptr
        );

        return CWnd::CreateEx(
            WS_EX_TOOLWINDOW | WS_EX_TOPMOST,
            className,
            nullptr,
            WS_POPUP,
            0, 0, 0, 0,
            pParentWnd ? pParentWnd->GetSafeHwnd() : nullptr,
            nullptr
        );
    }

    UINT CMContextMenu::TrackPopupMenu(CPoint point, CWnd* pParentWnd)
    {
        m_parentWnd = pParentWnd;
        m_clickHandler = nullptr;

        if (!Create(pParentWnd))
            return 0;

        CSize size = CalculateSize();

        // 화면 경계 확인
        CRect screenRect;
        SystemParametersInfo(SPI_GETWORKAREA, 0, &screenRect, 0);

        if (point.x + size.cx > screenRect.right)
            point.x = screenRect.right - size.cx;
        if (point.y + size.cy > screenRect.bottom)
            point.y = screenRect.bottom - size.cy;

        SetWindowPos(&wndTopMost, point.x, point.y, size.cx, size.cy,
            SWP_SHOWWINDOW);
        SetFocus();

        // 메시지 루프
        MSG msg;
        UINT result = 0;

        while (GetMessage(&msg, nullptr, 0, 0))
        {
            if (msg.message == WM_QUIT)
            {
                PostQuitMessage((int)msg.wParam);
                break;
            }

            // 메뉴 외부 클릭 확인
            if (msg.message == WM_LBUTTONDOWN || msg.message == WM_RBUTTONDOWN)
            {
                CPoint pt(msg.pt);
                CRect windowRect;
                GetWindowRect(&windowRect);

                if (!windowRect.PtInRect(pt))
                {
                    // 서브메뉴도 확인
                    BOOL inSubmenu = FALSE;
                    if (m_activeSubmenu && m_activeSubmenu->GetSafeHwnd())
                    {
                        CRect submenuRect;
                        m_activeSubmenu->GetWindowRect(&submenuRect);
                        if (submenuRect.PtInRect(pt))
                            inSubmenu = TRUE;
                    }

                    if (!inSubmenu)
                    {
                        Close();
                        break;
                    }
                }
            }

            TranslateMessage(&msg);
            DispatchMessage(&msg);

            // 메뉴가 닫혔는지 확인
            if (!IsWindowVisible())
            {
                result = m_selectedIndex >= 0 && m_selectedIndex < m_items.GetSize()
                    ? m_items[m_selectedIndex].id : 0;
                break;
            }
        }

        if (GetSafeHwnd())
            DestroyWindow();

        return result;
    }

    void CMContextMenu::ShowAsync(CPoint point, CWnd* pParentWnd, MenuItemClickHandler handler, void* context)
    {
        m_parentWnd = pParentWnd;
        m_clickHandler = handler;
        m_clickContext = context;

        if (!Create(pParentWnd))
            return;

        CSize size = CalculateSize();

        // 화면 경계 확인
        CRect screenRect;
        SystemParametersInfo(SPI_GETWORKAREA, 0, &screenRect, 0);

        if (point.x + size.cx > screenRect.right)
            point.x = screenRect.right - size.cx;
        if (point.y + size.cy > screenRect.bottom)
            point.y = screenRect.bottom - size.cy;

        SetWindowPos(&wndTopMost, point.x, point.y, size.cx, size.cy,
            SWP_SHOWWINDOW);
        SetFocus();
    }

    void CMContextMenu::OnPaint()
    {
        CPaintDC dc(this);

        CRect rect;
        GetClientRect(&rect);

        // 더블 버퍼링
        CDC memDC;
        memDC.CreateCompatibleDC(&dc);
        CBitmap bitmap;
        bitmap.CreateCompatibleBitmap(&dc, rect.Width(), rect.Height());
        CBitmap* oldBitmap = memDC.SelectObject(&bitmap);

        OnDraw(&memDC);

        dc.BitBlt(0, 0, rect.Width(), rect.Height(), &memDC, 0, 0, SRCCOPY);
        memDC.SelectObject(oldBitmap);
    }

    void CMContextMenu::OnDraw(CDC* pDC)
    {
        CRect rect;
        GetClientRect(&rect);

        const ThemeColors& colors = GetColors();

        // 배경
        pDC->FillSolidRect(rect, colors.surface.normal);

        // 테두리
        CPen pen(PS_SOLID, 1, colors.border.normal);
        CPen* oldPen = pDC->SelectObject(&pen);
        CBrush* oldBrush = (CBrush*)pDC->SelectStockObject(NULL_BRUSH);
        pDC->Rectangle(rect);
        pDC->SelectObject(oldPen);
        pDC->SelectObject(oldBrush);

        // 아이템들
        for (INT_PTR i = 0; i < m_items.GetSize(); i++)
        {
            CRect itemRect = GetItemRect((int)i);

            if (m_items[i].type == MenuItemType::Separator)
            {
                DrawSeparator(pDC, itemRect);
            }
            else
            {
                DrawItem(pDC, (int)i, itemRect);
            }
        }
    }

    BOOL CMContextMenu::OnEraseBkgnd(CDC* pDC)
    {
        return TRUE;
    }

    void CMContextMenu::DrawItem(CDC* pDC, int index, const CRect& rect)
    {
        const ThemeColors& colors = GetColors();
        const CMMenuItem& item = m_items[index];

        BOOL isHovered = (index == m_hoverIndex);

        // 배경
        if (isHovered && item.enabled)
        {
            pDC->FillSolidRect(rect, colors.primary.normal);
        }

        // 체크 표시
        if (item.type == MenuItemType::Checkbox || item.type == MenuItemType::Radio)
        {
            if (item.checked)
            {
                CRect checkRect = rect;
                checkRect.left += 4;
                checkRect.right = checkRect.left + 20;

                COLORREF checkColor = isHovered ? colors.textOnPrimary : colors.text;

                if (item.type == MenuItemType::Checkbox)
                {
                    // 체크 마크
                    CPen pen(PS_SOLID, 2, checkColor);
                    CPen* oldPen = pDC->SelectObject(&pen);
                    int cx = checkRect.CenterPoint().x;
                    int cy = checkRect.CenterPoint().y;
                    pDC->MoveTo(cx - 4, cy);
                    pDC->LineTo(cx - 1, cy + 3);
                    pDC->LineTo(cx + 5, cy - 3);
                    pDC->SelectObject(oldPen);
                }
                else
                {
                    // 라디오 점
                    CBrush brush(checkColor);
                    CPen pen(PS_SOLID, 1, checkColor);
                    CPen* oldPen = pDC->SelectObject(&pen);
                    CBrush* oldBrush = pDC->SelectObject(&brush);
                    int cx = checkRect.CenterPoint().x;
                    int cy = checkRect.CenterPoint().y;
                    pDC->Ellipse(cx - 3, cy - 3, cx + 3, cy + 3);
                    pDC->SelectObject(oldPen);
                    pDC->SelectObject(oldBrush);
                }
            }
        }

        // 아이콘
        int textLeft = rect.left + PADDING_H;
        if (item.hIcon)
        {
            int iconTop = rect.top + (rect.Height() - ICON_SIZE) / 2;
            DrawIconEx(pDC->GetSafeHdc(), textLeft, iconTop, item.hIcon,
                ICON_SIZE, ICON_SIZE, 0, nullptr, DI_NORMAL);
            textLeft += ICON_SIZE + 8;
        }
        else if (item.type == MenuItemType::Checkbox || item.type == MenuItemType::Radio)
        {
            textLeft += 24;
        }

        // 텍스트
        COLORREF textColor;
        if (!item.enabled)
        {
            textColor = colors.textDisabled;
        }
        else if (isHovered)
        {
            textColor = colors.textOnPrimary;
        }
        else
        {
            textColor = colors.text;
        }

        CFont* pFont = CMThemeManager::GetInstance().GetFont(TextStyle::Body);
        CFont* oldFont = pDC->SelectObject(pFont);
        pDC->SetBkMode(TRANSPARENT);
        pDC->SetTextColor(textColor);

        CRect textRect = rect;
        textRect.left = textLeft;
        textRect.right -= PADDING_H;

        if (item.type == MenuItemType::Submenu)
        {
            textRect.right -= SUBMENU_ARROW_WIDTH;
        }
        else if (!item.shortcut.IsEmpty())
        {
            textRect.right -= SHORTCUT_GAP;
        }

        pDC->DrawText(item.text, textRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);

        // 단축키
        if (!item.shortcut.IsEmpty())
        {
            CRect shortcutRect = rect;
            shortcutRect.right -= PADDING_H;
            pDC->SetTextColor(isHovered && item.enabled ? colors.textOnPrimary : colors.textSecondary);
            pDC->DrawText(item.shortcut, shortcutRect, DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
        }

        // 서브메뉴 화살표
        if (item.type == MenuItemType::Submenu)
        {
            COLORREF arrowColor = isHovered && item.enabled ? colors.textOnPrimary : colors.textSecondary;

            int cx = rect.right - PADDING_H;
            int cy = rect.CenterPoint().y;

            POINT points[3] = {
                { cx - 4, cy - 4 },
                { cx, cy },
                { cx - 4, cy + 4 }
            };

            CBrush brush(arrowColor);
            CPen pen(PS_SOLID, 1, arrowColor);
            CPen* oldPen = pDC->SelectObject(&pen);
            CBrush* oldBrush = pDC->SelectObject(&brush);
            pDC->Polygon(points, 3);
            pDC->SelectObject(oldPen);
            pDC->SelectObject(oldBrush);
        }

        pDC->SelectObject(oldFont);
    }

    void CMContextMenu::DrawSeparator(CDC* pDC, const CRect& rect)
    {
        const ThemeColors& colors = GetColors();

        int y = rect.CenterPoint().y;
        CPen pen(PS_SOLID, 1, colors.border.normal);
        CPen* oldPen = pDC->SelectObject(&pen);
        pDC->MoveTo(rect.left + PADDING_H, y);
        pDC->LineTo(rect.right - PADDING_H, y);
        pDC->SelectObject(oldPen);
    }

    void CMContextMenu::OnMouseMove(UINT nFlags, CPoint point)
    {
        int newHover = ItemFromPoint(point);
        if (newHover != m_hoverIndex)
        {
            m_hoverIndex = newHover;
            Invalidate();

            // 서브메뉴 처리
            if (m_hoverIndex >= 0 && m_items[m_hoverIndex].type == MenuItemType::Submenu)
            {
                // 약간의 딜레이 후 서브메뉴 표시
                if (m_submenuTimerId)
                    KillTimer(m_submenuTimerId);
                m_submenuTimerId = SetTimer(SUBMENU_TIMER_ID, 300, nullptr);
            }
            else
            {
                HideSubmenu();
            }
        }

        CWnd::OnMouseMove(nFlags, point);
    }

    void CMContextMenu::OnLButtonUp(UINT nFlags, CPoint point)
    {
        int index = ItemFromPoint(point);
        if (index >= 0 && index < m_items.GetSize())
        {
            const CMMenuItem& item = m_items[index];

            if (item.type == MenuItemType::Separator || !item.enabled)
                return;

            if (item.type == MenuItemType::Submenu)
            {
                ShowSubmenu(index);
                return;
            }

            m_selectedIndex = index;

            // 체크 상태 토글
            if (item.type == MenuItemType::Checkbox)
            {
                m_items[index].checked = !m_items[index].checked;
            }

            // 비동기 모드
            if (m_clickHandler)
            {
                m_clickHandler(m_clickContext, item.id);
                Close();
            }
            else
            {
                Close(item.id);
            }
        }

        CWnd::OnLButtonUp(nFlags, point);
    }

    void CMContextMenu::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
    {
        switch (nChar)
        {
        case VK_UP:
            if (m_hoverIndex > 0)
            {
                m_hoverIndex--;
                // 구분선 건너뛰기
                while (m_hoverIndex >= 0 && m_items[m_hoverIndex].type == MenuItemType::Separator)
                    m_hoverIndex--;
                Invalidate();
            }
            break;

        case VK_DOWN:
            if (m_hoverIndex < m_items.GetSize() - 1)
            {
                m_hoverIndex++;
                while (m_hoverIndex < m_items.GetSize() && m_items[m_hoverIndex].type == MenuItemType::Separator)
                    m_hoverIndex++;
                if (m_hoverIndex >= m_items.GetSize())
                    m_hoverIndex = (int)m_items.GetSize() - 1;
                Invalidate();
            }
            break;

        case VK_RIGHT:
            if (m_hoverIndex >= 0 && m_items[m_hoverIndex].type == MenuItemType::Submenu)
            {
                ShowSubmenu(m_hoverIndex);
            }
            break;

        case VK_LEFT:
            if (m_isSubmenu)
            {
                Close();
            }
            break;

        case VK_RETURN:
            if (m_hoverIndex >= 0)
            {
                CPoint pt;
                CRect itemRect = GetItemRect(m_hoverIndex);
                pt = itemRect.CenterPoint();
                OnLButtonUp(0, pt);
            }
            break;

        case VK_ESCAPE:
            Close();
            break;
        }

        CWnd::OnKeyDown(nChar, nRepCnt, nFlags);
    }

    void CMContextMenu::OnKillFocus(CWnd* pNewWnd)
    {
        // 서브메뉴로 포커스 이동한 경우는 닫지 않음
        if (m_activeSubmenu && pNewWnd == m_activeSubmenu)
            return;

        Close();
        CWnd::OnKillFocus(pNewWnd);
    }

    void CMContextMenu::OnTimer(UINT_PTR nIDEvent)
    {
        if (nIDEvent == SUBMENU_TIMER_ID)
        {
            KillTimer(m_submenuTimerId);
            m_submenuTimerId = 0;

            if (m_hoverIndex >= 0 && m_items[m_hoverIndex].type == MenuItemType::Submenu)
            {
                ShowSubmenu(m_hoverIndex);
            }
        }

        CWnd::OnTimer(nIDEvent);
    }

    BOOL CMContextMenu::IsLightTheme() const
    {
        return CMThemeManager::GetInstance().GetTheme() == ThemeType::Light;
    }

    const ThemeColors& CMContextMenu::GetColors() const
    {
        return CMThemeManager::GetInstance().GetColors();
    }

    CSize CMContextMenu::CalculateSize()
    {
        CDC* pDC = GetDC();
        if (!pDC)
            return CSize(100, 100);

        CFont* pFont = CMThemeManager::GetInstance().GetFont(TextStyle::Body);
        CFont* oldFont = pDC->SelectObject(pFont);

        int maxWidth = 100;
        int totalHeight = PADDING_V * 2;

        for (INT_PTR i = 0; i < m_items.GetSize(); i++)
        {
            const CMMenuItem& item = m_items[i];

            if (item.type == MenuItemType::Separator)
            {
                totalHeight += SEPARATOR_HEIGHT;
            }
            else
            {
                totalHeight += ITEM_HEIGHT;

                CSize textSize = pDC->GetTextExtent(item.text);
                int itemWidth = PADDING_H * 2 + textSize.cx;

                if (item.hIcon)
                    itemWidth += ICON_SIZE + 8;
                else if (item.type == MenuItemType::Checkbox || item.type == MenuItemType::Radio)
                    itemWidth += 24;

                if (!item.shortcut.IsEmpty())
                {
                    CSize shortcutSize = pDC->GetTextExtent(item.shortcut);
                    itemWidth += SHORTCUT_GAP + shortcutSize.cx;
                }

                if (item.type == MenuItemType::Submenu)
                    itemWidth += SUBMENU_ARROW_WIDTH;

                maxWidth = max(maxWidth, itemWidth);
            }
        }

        pDC->SelectObject(oldFont);
        ReleaseDC(pDC);

        return CSize(maxWidth, totalHeight);
    }

    int CMContextMenu::ItemFromPoint(CPoint point)
    {
        for (INT_PTR i = 0; i < m_items.GetSize(); i++)
        {
            CRect itemRect = GetItemRect((int)i);
            if (itemRect.PtInRect(point))
            {
                return (int)i;
            }
        }
        return -1;
    }

    CRect CMContextMenu::GetItemRect(int index)
    {
        CRect rect;
        GetClientRect(&rect);

        int y = PADDING_V;
        for (int i = 0; i < index; i++)
        {
            if (m_items[i].type == MenuItemType::Separator)
                y += SEPARATOR_HEIGHT;
            else
                y += ITEM_HEIGHT;
        }

        int height = (m_items[index].type == MenuItemType::Separator) ? SEPARATOR_HEIGHT : ITEM_HEIGHT;

        return CRect(1, y, rect.right - 1, y + height);
    }

    void CMContextMenu::ShowSubmenu(int index)
    {
        HideSubmenu();

        if (index < 0 || index >= m_items.GetSize())
            return;

        const CMMenuItem& item = m_items[index];
        if (item.type != MenuItemType::Submenu)
            return;

        // 서브메뉴 찾기
        int submenuIndex = 0;
        for (INT_PTR i = 0; i < index; i++)
        {
            if (m_items[i].type == MenuItemType::Submenu)
                submenuIndex++;
        }

        if (submenuIndex >= m_submenus.GetSize())
            return;

        CMContextMenu* submenu = m_submenus[submenuIndex];
        if (!submenu)
            return;

        // 위치 계산
        CRect itemRect = GetItemRect(index);
        ClientToScreen(&itemRect);

        CPoint pt(itemRect.right, itemRect.top);

        // 화면 경계 확인
        CSize submenuSize = submenu->CalculateSize();
        CRect screenRect;
        SystemParametersInfo(SPI_GETWORKAREA, 0, &screenRect, 0);

        if (pt.x + submenuSize.cx > screenRect.right)
        {
            pt.x = itemRect.left - submenuSize.cx;
        }

        submenu->Create(this);
        submenu->SetWindowPos(&wndTopMost, pt.x, pt.y, submenuSize.cx, submenuSize.cy,
            SWP_SHOWWINDOW);
        submenu->SetFocus();

        m_activeSubmenu = submenu;
    }

    void CMContextMenu::HideSubmenu()
    {
        if (m_activeSubmenu)
        {
            if (m_activeSubmenu->GetSafeHwnd())
            {
                m_activeSubmenu->HideSubmenu();  // 재귀적으로 하위 서브메뉴도 닫기
                m_activeSubmenu->DestroyWindow();
            }
            m_activeSubmenu = nullptr;
        }
    }

    CMMenuItem* CMContextMenu::FindItem(UINT id)
    {
        for (INT_PTR i = 0; i < m_items.GetSize(); i++)
        {
            if (m_items[i].id == id)
                return &m_items[i];
        }

        // 서브메뉴에서도 찾기
        for (INT_PTR i = 0; i < m_submenus.GetSize(); i++)
        {
            CMMenuItem* found = m_submenus[i]->FindItem(id);
            if (found)
                return found;
        }

        return nullptr;
    }

    void CMContextMenu::Close(UINT resultId)
    {
        HideSubmenu();

        if (m_submenuTimerId)
        {
            KillTimer(m_submenuTimerId);
            m_submenuTimerId = 0;
        }

        ShowWindow(SW_HIDE);
    }
}
