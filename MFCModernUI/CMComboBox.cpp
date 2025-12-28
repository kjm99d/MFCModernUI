#include "pch.h"
#include "CMComboBox.h"

namespace MFCModernUI
{
    // CMComboBox 구현
    IMPLEMENT_DYNAMIC(CMComboBox, CMControlBase)

    BEGIN_MESSAGE_MAP(CMComboBox, CMControlBase)
        ON_WM_LBUTTONDOWN()
        ON_WM_KEYDOWN()
        ON_WM_KILLFOCUS()
    END_MESSAGE_MAP()

    CMComboBox::CMComboBox()
        : m_selectedIndex(-1)
        , m_placeholder(_T("선택..."))
        , m_dropdownVisible(FALSE)
        , m_maxDropdownHeight(300)
        , m_hoverIndex(-1)
        , m_pDropdownWnd(nullptr)
        , m_selectionChangedHandler(nullptr)
        , m_selectionChangedContext(nullptr)
    {
        m_themeClass = _T("combobox");
    }

    CMComboBox::~CMComboBox()
    {
        if (m_pDropdownWnd)
        {
            m_pDropdownWnd->DestroyWindow();
            delete m_pDropdownWnd;
        }
    }

    BOOL CMComboBox::Create(DWORD dwStyle, const CRect& rect, CWnd* pParentWnd, UINT nID)
    {
        CString className = AfxRegisterWndClass(
            CS_HREDRAW | CS_VREDRAW,
            LoadCursor(nullptr, IDC_HAND),
            nullptr,
            nullptr
        );

        return CWnd::Create(className, nullptr, dwStyle | WS_CHILD, rect, pParentWnd, nID);
    }

    int CMComboBox::AddItem(const CString& text, DWORD_PTR data)
    {
        ComboBoxItem item(text, data);
        m_items.push_back(item);
        return static_cast<int>(m_items.size()) - 1;
    }

    int CMComboBox::InsertItem(int index, const CString& text, DWORD_PTR data)
    {
        if (index < 0 || index > static_cast<int>(m_items.size()))
            index = static_cast<int>(m_items.size());

        ComboBoxItem item(text, data);
        m_items.insert(m_items.begin() + index, item);

        if (m_selectedIndex >= index)
            m_selectedIndex++;

        return index;
    }

    void CMComboBox::RemoveItem(int index)
    {
        if (index >= 0 && index < static_cast<int>(m_items.size()))
        {
            m_items.erase(m_items.begin() + index);

            if (m_selectedIndex == index)
                m_selectedIndex = -1;
            else if (m_selectedIndex > index)
                m_selectedIndex--;

            Invalidate();
        }
    }

    void CMComboBox::RemoveAllItems()
    {
        m_items.clear();
        m_selectedIndex = -1;
        Invalidate();
    }

    CString CMComboBox::GetItemText(int index) const
    {
        if (index >= 0 && index < static_cast<int>(m_items.size()))
            return m_items[index].text;
        return _T("");
    }

    DWORD_PTR CMComboBox::GetItemData(int index) const
    {
        if (index >= 0 && index < static_cast<int>(m_items.size()))
            return m_items[index].data;
        return 0;
    }

    void CMComboBox::SetItemText(int index, const CString& text)
    {
        if (index >= 0 && index < static_cast<int>(m_items.size()))
        {
            m_items[index].text = text;
            Invalidate();
        }
    }

    void CMComboBox::SetItemData(int index, DWORD_PTR data)
    {
        if (index >= 0 && index < static_cast<int>(m_items.size()))
            m_items[index].data = data;
    }

    void CMComboBox::SetItemDisabled(int index, BOOL disabled)
    {
        if (index >= 0 && index < static_cast<int>(m_items.size()))
            m_items[index].disabled = disabled;
    }

    void CMComboBox::AddSeparator()
    {
        ComboBoxItem item;
        item.separator = TRUE;
        m_items.push_back(item);
    }

    void CMComboBox::SetSelectedIndex(int index)
    {
        if (index >= -1 && index < static_cast<int>(m_items.size()))
        {
            if (index >= 0 && m_items[index].disabled)
                return;

            int oldIndex = m_selectedIndex;
            m_selectedIndex = index;

            if (oldIndex != index)
            {
                if (m_selectionChangedHandler)
                {
                    m_selectionChangedHandler(m_selectionChangedContext, oldIndex, index);
                }

                Invalidate();
            }
        }
    }

    CString CMComboBox::GetSelectedText() const
    {
        return GetItemText(m_selectedIndex);
    }

    DWORD_PTR CMComboBox::GetSelectedData() const
    {
        return GetItemData(m_selectedIndex);
    }

    void CMComboBox::SetPlaceholder(const CString& placeholder)
    {
        m_placeholder = placeholder;
        Invalidate();
    }

    void CMComboBox::ShowDropdown()
    {
        if (m_dropdownVisible || m_items.empty())
            return;

        CreateDropdownWindow();
        PositionDropdown();

        m_pDropdownWnd->ShowWindow(SW_SHOWNA);
        m_dropdownVisible = TRUE;
        Invalidate();
    }

    void CMComboBox::HideDropdown()
    {
        if (!m_dropdownVisible)
            return;

        if (m_pDropdownWnd)
        {
            m_pDropdownWnd->ShowWindow(SW_HIDE);
        }

        m_dropdownVisible = FALSE;
        Invalidate();
    }

    void CMComboBox::SetMaxDropdownHeight(int height)
    {
        m_maxDropdownHeight = height;
    }

    void CMComboBox::SetSelectionChangedHandler(ComboSelectionChangedHandler handler, void* context)
    {
        m_selectionChangedHandler = handler;
        m_selectionChangedContext = context;
    }

    void CMComboBox::CreateDropdownWindow()
    {
        if (!m_pDropdownWnd)
        {
            m_pDropdownWnd = new CMComboDropdown(this);
            ((CMComboDropdown*)m_pDropdownWnd)->Create(this);
        }
        ((CMComboDropdown*)m_pDropdownWnd)->UpdateItems();
    }

    void CMComboBox::PositionDropdown()
    {
        if (!m_pDropdownWnd)
            return;

        CRect rect;
        GetWindowRect(&rect);

        int itemHeight = GetItemHeight();
        int totalHeight = 0;

        for (const auto& item : m_items)
        {
            totalHeight += item.separator ? 9 : itemHeight;
        }

        int dropdownHeight = min(totalHeight + 4, m_maxDropdownHeight);

        m_pDropdownWnd->SetWindowPos(nullptr,
            rect.left,
            rect.bottom,
            rect.Width(),
            dropdownHeight,
            SWP_NOZORDER | SWP_NOACTIVATE);
    }

    void CMComboBox::SelectItem(int index)
    {
        SetSelectedIndex(index);
        HideDropdown();
    }

    void CMComboBox::OnDraw(CDC* pDC)
    {
        DrawMainControl(pDC);
    }

    void CMComboBox::DrawMainControl(CDC* pDC)
    {
        CRect rect;
        GetClientRect(&rect);

        const ThemeColors& colors = GetColors();
        int radius = GetThemeManager().GetCornerRadius();

        // 배경
        COLORREF bgColor = m_enabled ? colors.surface.normal : colors.surface.disabled;

        // 테두리
        COLORREF borderColor;
        int borderWidth = 1;

        if (!m_enabled)
        {
            borderColor = colors.border.disabled;
        }
        else if (m_dropdownVisible || m_state == ControlState::Focused)
        {
            borderColor = colors.borderFocus;
            borderWidth = 2;
        }
        else if (m_state == ControlState::Hover)
        {
            borderColor = colors.border.hover;
        }
        else
        {
            borderColor = colors.border.normal;
        }

        DrawRoundedRect(pDC, rect, radius, bgColor, borderColor, borderWidth);

        // 텍스트 영역
        CRect textRect = rect;
        textRect.DeflateRect(12, 0, 32, 0);

        CString displayText;
        COLORREF textColor;

        if (m_selectedIndex >= 0)
        {
            displayText = m_items[m_selectedIndex].text;
            textColor = m_enabled ? colors.text : colors.textDisabled;
        }
        else
        {
            displayText = m_placeholder;
            textColor = colors.textSecondary;
        }

        DrawText(pDC, displayText, textRect, textColor, TextStyle::Body,
            DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);

        // 화살표
        DrawDropdownArrow(pDC, rect);
    }

    void CMComboBox::DrawDropdownArrow(CDC* pDC, const CRect& rect)
    {
        const ThemeColors& colors = GetColors();
        COLORREF arrowColor = m_enabled ? colors.textSecondary : colors.textDisabled;

        int arrowX = rect.right - 20;
        int arrowY = rect.CenterPoint().y;

        CPen pen(PS_SOLID, 2, arrowColor);
        CPen* pOldPen = pDC->SelectObject(&pen);

        // 아래 화살표
        pDC->MoveTo(arrowX - 4, arrowY - 2);
        pDC->LineTo(arrowX, arrowY + 2);
        pDC->LineTo(arrowX + 4, arrowY - 2);

        pDC->SelectObject(pOldPen);
    }

    void CMComboBox::OnLButtonDown(UINT nFlags, CPoint point)
    {
        if (m_enabled)
        {
            SetFocus();

            if (m_dropdownVisible)
            {
                HideDropdown();
            }
            else
            {
                ShowDropdown();
            }
        }

        // 기본 처리 생략 (드롭다운 처리만)
    }

    void CMComboBox::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
    {
        if (!m_enabled)
        {
            CMControlBase::OnKeyDown(nChar, nRepCnt, nFlags);
            return;
        }

        switch (nChar)
        {
        case VK_SPACE:
        case VK_RETURN:
            if (m_dropdownVisible)
                HideDropdown();
            else
                ShowDropdown();
            break;

        case VK_ESCAPE:
            HideDropdown();
            break;

        case VK_UP:
            if (m_selectedIndex > 0)
            {
                int newIndex = m_selectedIndex - 1;
                while (newIndex >= 0 && (m_items[newIndex].separator || m_items[newIndex].disabled))
                    newIndex--;
                if (newIndex >= 0)
                    SetSelectedIndex(newIndex);
            }
            break;

        case VK_DOWN:
            if (m_selectedIndex < static_cast<int>(m_items.size()) - 1)
            {
                int newIndex = m_selectedIndex + 1;
                while (newIndex < static_cast<int>(m_items.size()) &&
                       (m_items[newIndex].separator || m_items[newIndex].disabled))
                    newIndex++;
                if (newIndex < static_cast<int>(m_items.size()))
                    SetSelectedIndex(newIndex);
            }
            break;
        }

        CMControlBase::OnKeyDown(nChar, nRepCnt, nFlags);
    }

    void CMComboBox::OnKillFocus(CWnd* pNewWnd)
    {
        // 드롭다운으로 포커스 이동 시 무시
        if (m_pDropdownWnd && pNewWnd == m_pDropdownWnd)
            return;

        HideDropdown();
        CMControlBase::OnKillFocus(pNewWnd);
    }

    // CMComboDropdown 구현
    BEGIN_MESSAGE_MAP(CMComboDropdown, CWnd)
        ON_WM_PAINT()
        ON_WM_MOUSEMOVE()
        ON_WM_LBUTTONDOWN()
        ON_WM_MOUSELEAVE()
        ON_WM_ERASEBKGND()
    END_MESSAGE_MAP()

    CMComboDropdown::CMComboDropdown(CMComboBox* pOwner)
        : m_pOwner(pOwner)
        , m_hoverIndex(-1)
        , m_mouseTracking(FALSE)
    {
    }

    CMComboDropdown::~CMComboDropdown()
    {
    }

    BOOL CMComboDropdown::Create(CWnd* pParentWnd)
    {
        CString className = AfxRegisterWndClass(
            CS_HREDRAW | CS_VREDRAW | CS_DROPSHADOW,
            LoadCursor(nullptr, IDC_ARROW),
            nullptr,
            nullptr
        );

        return CWnd::CreateEx(
            WS_EX_TOPMOST | WS_EX_TOOLWINDOW,
            className,
            nullptr,
            WS_POPUP,
            0, 0, 100, 100,
            pParentWnd->GetSafeHwnd(),
            nullptr
        );
    }

    void CMComboDropdown::UpdateItems()
    {
        Invalidate();
    }

    void CMComboDropdown::OnPaint()
    {
        CPaintDC dc(this);

        CRect rect;
        GetClientRect(&rect);

        const ThemeColors& colors = CMThemeManager::GetInstance().GetColors();
        int radius = CMThemeManager::GetInstance().GetCornerRadius();

        // 배경
        dc.FillSolidRect(rect, colors.surface.normal);

        // 테두리
        CPen pen(PS_SOLID, 1, colors.border.normal);
        CPen* pOldPen = dc.SelectObject(&pen);
        CBrush* pOldBrush = (CBrush*)dc.SelectStockObject(NULL_BRUSH);
        dc.RoundRect(rect, CPoint(radius, radius));
        dc.SelectObject(pOldPen);
        dc.SelectObject(pOldBrush);

        // 항목 그리기
        int y = 2;
        int itemHeight = 32;
        int itemCount = m_pOwner->GetItemCount();

        for (int i = 0; i < itemCount; i++)
        {
            CString text = m_pOwner->GetItemText(i);

            // 구분선 체크 (간단히 빈 텍스트로 처리)
            if (text.IsEmpty())
            {
                // 구분선
                CRect sepRect(rect.left + 8, y + 4, rect.right - 8, y + 5);
                dc.FillSolidRect(sepRect, colors.border.normal);
                y += 9;
                continue;
            }

            CRect itemRect(rect.left + 2, y, rect.right - 2, y + itemHeight);

            // 호버/선택 배경
            if (i == m_hoverIndex)
            {
                CRect bgRect = itemRect;
                bgRect.DeflateRect(2, 0);

                CBrush hoverBrush(colors.surface.hover);
                CPen nullPen(PS_NULL, 0, RGB(0, 0, 0));

                dc.SelectObject(&hoverBrush);
                dc.SelectObject(&nullPen);
                dc.RoundRect(bgRect, CPoint(4, 4));
            }

            if (i == m_pOwner->GetSelectedIndex())
            {
                // 선택된 항목 표시
                CRect checkRect(itemRect.left + 8, itemRect.top, itemRect.left + 24, itemRect.bottom);

                CFont* pFont = CMThemeManager::GetInstance().GetFont(TextStyle::Body);
                CFont* pOldFont = dc.SelectObject(pFont);
                dc.SetTextColor(colors.primary.normal);
                dc.SetBkMode(TRANSPARENT);
                dc.DrawText(_T("\x2713"), checkRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
                dc.SelectObject(pOldFont);
            }

            // 텍스트
            CRect textRect = itemRect;
            textRect.left += 28;
            textRect.right -= 8;

            CFont* pFont = CMThemeManager::GetInstance().GetFont(TextStyle::Body);
            CFont* pOldFont = dc.SelectObject(pFont);
            dc.SetTextColor(colors.text);
            dc.SetBkMode(TRANSPARENT);
            dc.DrawText(text, textRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
            dc.SelectObject(pOldFont);

            y += itemHeight;
        }
    }

    int CMComboDropdown::HitTest(CPoint point)
    {
        CRect rect;
        GetClientRect(&rect);

        if (!rect.PtInRect(point))
            return -1;

        int y = 2;
        int itemHeight = 32;
        int itemCount = m_pOwner->GetItemCount();

        for (int i = 0; i < itemCount; i++)
        {
            CString text = m_pOwner->GetItemText(i);

            if (text.IsEmpty())
            {
                y += 9;
                continue;
            }

            CRect itemRect(rect.left, y, rect.right, y + itemHeight);

            if (itemRect.PtInRect(point))
                return i;

            y += itemHeight;
        }

        return -1;
    }

    void CMComboDropdown::OnMouseMove(UINT nFlags, CPoint point)
    {
        if (!m_mouseTracking)
        {
            TRACKMOUSEEVENT tme;
            tme.cbSize = sizeof(TRACKMOUSEEVENT);
            tme.dwFlags = TME_LEAVE;
            tme.hwndTrack = m_hWnd;
            TrackMouseEvent(&tme);
            m_mouseTracking = TRUE;
        }

        int newHover = HitTest(point);
        if (newHover != m_hoverIndex)
        {
            m_hoverIndex = newHover;
            Invalidate();
        }

        CWnd::OnMouseMove(nFlags, point);
    }

    void CMComboDropdown::OnLButtonDown(UINT nFlags, CPoint point)
    {
        int index = HitTest(point);
        if (index >= 0)
        {
            m_pOwner->SelectItem(index);
        }

        CWnd::OnLButtonDown(nFlags, point);
    }

    void CMComboDropdown::OnMouseLeave()
    {
        m_mouseTracking = FALSE;
        m_hoverIndex = -1;
        Invalidate();

        CWnd::OnMouseLeave();
    }

    BOOL CMComboDropdown::OnEraseBkgnd(CDC* pDC)
    {
        return TRUE;
    }
}
