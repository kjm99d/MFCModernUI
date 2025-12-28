#include "pch.h"
#include "CMTimePicker.h"

namespace MFCModernUI
{
    // CMTimePicker 구현
    IMPLEMENT_DYNAMIC(CMTimePicker, CMControlBase)

    BEGIN_MESSAGE_MAP(CMTimePicker, CMControlBase)
        ON_WM_LBUTTONDOWN()
        ON_WM_KEYDOWN()
        ON_WM_KILLFOCUS()
    END_MESSAGE_MAP()

    CMTimePicker::CMTimePicker()
        : m_hour(0)
        , m_minute(0)
        , m_second(0)
        , m_use24Hour(TRUE)
        , m_minuteStep(1)
        , m_showSeconds(FALSE)
        , m_popup(nullptr)
        , m_timeChangedHandler(nullptr)
        , m_timeChangedContext(nullptr)
    {
        m_themeClass = _T("timepicker");
        m_tabStop = TRUE;

        // 현재 시간으로 초기화
        COleDateTime now = COleDateTime::GetCurrentTime();
        m_hour = now.GetHour();
        m_minute = now.GetMinute();
        m_second = now.GetSecond();
    }

    CMTimePicker::~CMTimePicker()
    {
        if (m_popup)
        {
            m_popup->DestroyWindow();
            delete m_popup;
        }
    }

    BOOL CMTimePicker::Create(DWORD dwStyle, const CRect& rect, CWnd* pParentWnd, UINT nID)
    {
        CString className = AfxRegisterWndClass(
            CS_HREDRAW | CS_VREDRAW,
            LoadCursor(nullptr, IDC_ARROW),
            nullptr,
            nullptr
        );

        return CWnd::Create(className, nullptr, dwStyle | WS_CHILD | WS_TABSTOP, rect, pParentWnd, nID);
    }

    void CMTimePicker::SetTime(int hour, int minute)
    {
        hour = max(0, min(23, hour));
        minute = max(0, min(59, minute));

        if (m_hour != hour || m_minute != minute)
        {
            int oldHour = m_hour;
            int oldMinute = m_minute;
            m_hour = hour;
            m_minute = minute;
            NotifyTimeChanged(oldHour, oldMinute);
            Invalidate();
        }
    }

    void CMTimePicker::Set24HourFormat(BOOL use24Hour)
    {
        if (m_use24Hour != use24Hour)
        {
            m_use24Hour = use24Hour;
            Invalidate();
        }
    }

    void CMTimePicker::SetMinuteStep(int step)
    {
        m_minuteStep = max(1, min(30, step));
    }

    void CMTimePicker::SetShowSeconds(BOOL show)
    {
        if (m_showSeconds != show)
        {
            m_showSeconds = show;
            Invalidate();
        }
    }

    void CMTimePicker::SetSecond(int second)
    {
        m_second = max(0, min(59, second));
        if (m_showSeconds)
            Invalidate();
    }

    void CMTimePicker::SetTimeChangedHandler(TimeChangedHandler handler, void* context)
    {
        m_timeChangedHandler = handler;
        m_timeChangedContext = context;
    }

    void CMTimePicker::ShowDropDown()
    {
        if (!m_popup)
        {
            m_popup = new CMTimePopup(this);
            m_popup->Create();
        }

        CRect rect;
        GetWindowRect(&rect);

        m_popup->Show(CPoint(rect.left, rect.bottom));
    }

    void CMTimePicker::HideDropDown()
    {
        if (m_popup && m_popup->IsWindowVisible())
        {
            m_popup->Hide();
        }
    }

    BOOL CMTimePicker::IsDroppedDown() const
    {
        return m_popup && m_popup->IsWindowVisible();
    }

    void CMTimePicker::OnDraw(CDC* pDC)
    {
        CRect rect;
        GetClientRect(&rect);

        const ThemeColors& colors = GetColors();

        // 배경
        COLORREF bgColor = m_isEnabled ? colors.surface.normal : colors.surface.disabled;
        pDC->FillSolidRect(rect, bgColor);

        // 테두리
        COLORREF borderColor;
        if (!m_isEnabled)
            borderColor = colors.border.disabled;
        else if (m_isFocused || IsDroppedDown())
            borderColor = colors.primary.normal;
        else if (m_isHovered)
            borderColor = colors.border.hover;
        else
            borderColor = colors.border.normal;

        int borderWidth = (m_isFocused || IsDroppedDown()) ? 2 : 1;
        CPen pen(PS_SOLID, borderWidth, borderColor);
        CPen* oldPen = pDC->SelectObject(&pen);
        CBrush* oldBrush = (CBrush*)pDC->SelectStockObject(NULL_BRUSH);
        pDC->RoundRect(rect, CPoint(4, 4));
        pDC->SelectObject(oldPen);
        pDC->SelectObject(oldBrush);

        // 버튼 영역
        CRect buttonRect = rect;
        buttonRect.left = buttonRect.right - BUTTON_WIDTH;
        DrawButton(pDC, buttonRect);

        // 텍스트 영역
        CRect textRect = rect;
        textRect.right = buttonRect.left;
        textRect.DeflateRect(8, 0);

        CString timeText = FormatTime();
        COLORREF textColor = m_isEnabled ? colors.text : colors.textDisabled;

        DrawText(pDC, timeText, textRect, textColor, TextStyle::Body,
            DT_LEFT | DT_VCENTER | DT_SINGLELINE);
    }

    void CMTimePicker::DrawButton(CDC* pDC, const CRect& rect)
    {
        const ThemeColors& colors = GetColors();

        // 배경
        COLORREF bgColor = m_isHovered ? colors.surface.hover : colors.surface.normal;
        pDC->FillSolidRect(rect, bgColor);

        // 시계 아이콘 그리기
        int iconSize = 14;
        int cx = rect.CenterPoint().x;
        int cy = rect.CenterPoint().y;

        COLORREF iconColor = m_isEnabled ? colors.textSecondary : colors.textDisabled;
        CPen pen(PS_SOLID, 1, iconColor);
        CPen* oldPen = pDC->SelectObject(&pen);
        CBrush* oldBrush = (CBrush*)pDC->SelectStockObject(NULL_BRUSH);

        // 원형 시계
        CRect clockRect(cx - iconSize / 2, cy - iconSize / 2,
            cx + iconSize / 2, cy + iconSize / 2);
        pDC->Ellipse(clockRect);

        // 시계 바늘
        pDC->MoveTo(cx, cy);
        pDC->LineTo(cx, cy - iconSize / 3);  // 시침 (위)
        pDC->MoveTo(cx, cy);
        pDC->LineTo(cx + iconSize / 4, cy);  // 분침 (오른쪽)

        pDC->SelectObject(oldPen);
        pDC->SelectObject(oldBrush);
    }

    CString CMTimePicker::FormatTime()
    {
        CString result;

        if (m_use24Hour)
        {
            if (m_showSeconds)
                result.Format(_T("%02d:%02d:%02d"), m_hour, m_minute, m_second);
            else
                result.Format(_T("%02d:%02d"), m_hour, m_minute);
        }
        else
        {
            int displayHour = m_hour % 12;
            if (displayHour == 0) displayHour = 12;
            LPCTSTR ampm = (m_hour < 12) ? _T("AM") : _T("PM");

            if (m_showSeconds)
                result.Format(_T("%d:%02d:%02d %s"), displayHour, m_minute, m_second, ampm);
            else
                result.Format(_T("%d:%02d %s"), displayHour, m_minute, ampm);
        }

        return result;
    }

    void CMTimePicker::NotifyTimeChanged(int oldHour, int oldMinute)
    {
        if (m_timeChangedHandler)
        {
            m_timeChangedHandler(m_timeChangedContext, oldHour, oldMinute, m_hour, m_minute);
        }
    }

    void CMTimePicker::OnLButtonDown(UINT nFlags, CPoint point)
    {
        SetFocus();

        if (IsDroppedDown())
        {
            HideDropDown();
        }
        else
        {
            ShowDropDown();
        }

        CMControlBase::OnLButtonDown(nFlags, point);
    }

    void CMTimePicker::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
    {
        switch (nChar)
        {
        case VK_DOWN:
        case VK_F4:
            if (!IsDroppedDown())
                ShowDropDown();
            break;

        case VK_ESCAPE:
            if (IsDroppedDown())
                HideDropDown();
            break;

        case VK_UP:
            SetTime(m_hour, m_minute + m_minuteStep);
            break;
        }

        CMControlBase::OnKeyDown(nChar, nRepCnt, nFlags);
    }

    void CMTimePicker::OnKillFocus(CWnd* pNewWnd)
    {
        if (m_popup && pNewWnd == m_popup)
            return;

        CMControlBase::OnKillFocus(pNewWnd);
    }

    // CMTimePopup 구현
    BEGIN_MESSAGE_MAP(CMTimePopup, CWnd)
        ON_WM_PAINT()
        ON_WM_ERASEBKGND()
        ON_WM_LBUTTONDOWN()
        ON_WM_MOUSEMOVE()
        ON_WM_KILLFOCUS()
        ON_WM_MOUSEWHEEL()
    END_MESSAGE_MAP()

    CMTimePopup::CMTimePopup(CMTimePicker* pOwner)
        : m_pOwner(pOwner)
        , m_hoverColumn(-1)
        , m_hoverRow(-1)
    {
        m_tempHour = pOwner->GetHour();
        m_tempMinute = pOwner->GetMinute();
        m_tempSecond = pOwner->GetSecond();
    }

    CMTimePopup::~CMTimePopup()
    {
    }

    BOOL CMTimePopup::Create()
    {
        CString className = AfxRegisterWndClass(
            CS_HREDRAW | CS_VREDRAW | CS_DROPSHADOW,
            LoadCursor(nullptr, IDC_ARROW),
            nullptr,
            nullptr
        );

        int columns = 2;  // 시, 분
        if (m_pOwner->GetShowSeconds())
            columns = 3;
        if (!m_pOwner->Is24HourFormat())
            columns++;  // AM/PM

        int width = columns * COLUMN_WIDTH + 16;
        int height = BUTTON_HEIGHT + ROW_HEIGHT + BUTTON_HEIGHT + 16;

        return CWnd::CreateEx(
            WS_EX_TOOLWINDOW | WS_EX_TOPMOST,
            className,
            nullptr,
            WS_POPUP,
            0, 0, width, height,
            m_pOwner->GetSafeHwnd(),
            nullptr
        );
    }

    void CMTimePopup::Show(CPoint pt)
    {
        m_tempHour = m_pOwner->GetHour();
        m_tempMinute = m_pOwner->GetMinute();
        m_tempSecond = m_pOwner->GetSecond();

        CRect rect;
        GetWindowRect(&rect);

        CRect screenRect;
        SystemParametersInfo(SPI_GETWORKAREA, 0, &screenRect, 0);

        if (pt.x + rect.Width() > screenRect.right)
            pt.x = screenRect.right - rect.Width();
        if (pt.y + rect.Height() > screenRect.bottom)
            pt.y -= rect.Height() + m_pOwner->GetClientRect(&rect) + rect.Height();

        SetWindowPos(&wndTopMost, pt.x, pt.y, 0, 0,
            SWP_NOSIZE | SWP_SHOWWINDOW);
        SetFocus();
    }

    void CMTimePopup::Hide()
    {
        ShowWindow(SW_HIDE);
        m_pOwner->Invalidate();
    }

    void CMTimePopup::OnPaint()
    {
        CPaintDC dc(this);

        CRect rect;
        GetClientRect(&rect);

        CDC memDC;
        memDC.CreateCompatibleDC(&dc);
        CBitmap bitmap;
        bitmap.CreateCompatibleBitmap(&dc, rect.Width(), rect.Height());
        CBitmap* oldBitmap = memDC.SelectObject(&bitmap);

        OnDraw(&memDC);

        dc.BitBlt(0, 0, rect.Width(), rect.Height(), &memDC, 0, 0, SRCCOPY);
        memDC.SelectObject(oldBitmap);
    }

    BOOL CMTimePopup::OnEraseBkgnd(CDC* pDC)
    {
        return TRUE;
    }

    void CMTimePopup::OnDraw(CDC* pDC)
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

        // 각 컬럼 그리기
        int x = 8;

        // 시
        CRect hourRect(x, 8, x + COLUMN_WIDTH, rect.bottom - 8);
        DrawColumn(pDC, 0, hourRect);
        x += COLUMN_WIDTH;

        // 분
        CRect minRect(x, 8, x + COLUMN_WIDTH, rect.bottom - 8);
        DrawColumn(pDC, 1, minRect);
        x += COLUMN_WIDTH;

        // 초
        if (m_pOwner->GetShowSeconds())
        {
            CRect secRect(x, 8, x + COLUMN_WIDTH, rect.bottom - 8);
            DrawColumn(pDC, 2, secRect);
            x += COLUMN_WIDTH;
        }

        // AM/PM
        if (!m_pOwner->Is24HourFormat())
        {
            CRect ampmRect(x, 8, x + COLUMN_WIDTH, rect.bottom - 8);
            DrawColumn(pDC, 3, ampmRect);
        }
    }

    void CMTimePopup::DrawColumn(CDC* pDC, int col, const CRect& rect)
    {
        const ThemeColors& colors = GetColors();

        // 위 버튼
        CRect upRect = rect;
        upRect.bottom = upRect.top + BUTTON_HEIGHT;
        BOOL upHovered = (m_hoverColumn == col && m_hoverRow == -1);
        DrawArrowButton(pDC, upRect, TRUE, upHovered);

        // 값
        CRect valueRect = rect;
        valueRect.top = upRect.bottom;
        valueRect.bottom = valueRect.top + ROW_HEIGHT;

        CString valueText;
        switch (col)
        {
        case 0:  // 시
            if (m_pOwner->Is24HourFormat())
                valueText.Format(_T("%02d"), m_tempHour);
            else
            {
                int displayHour = m_tempHour % 12;
                if (displayHour == 0) displayHour = 12;
                valueText.Format(_T("%d"), displayHour);
            }
            break;
        case 1:  // 분
            valueText.Format(_T("%02d"), m_tempMinute);
            break;
        case 2:  // 초
            valueText.Format(_T("%02d"), m_tempSecond);
            break;
        case 3:  // AM/PM
            valueText = (m_tempHour < 12) ? _T("AM") : _T("PM");
            break;
        }

        CFont* pFont = CMThemeManager::GetInstance().GetFont(TextStyle::H4);
        CFont* oldFont = pDC->SelectObject(pFont);
        pDC->SetBkMode(TRANSPARENT);
        pDC->SetTextColor(colors.text);
        pDC->DrawText(valueText, valueRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        pDC->SelectObject(oldFont);

        // 아래 버튼
        CRect downRect = rect;
        downRect.top = valueRect.bottom;
        downRect.bottom = downRect.top + BUTTON_HEIGHT;
        BOOL downHovered = (m_hoverColumn == col && m_hoverRow == 1);
        DrawArrowButton(pDC, downRect, FALSE, downHovered);
    }

    void CMTimePopup::DrawArrowButton(CDC* pDC, const CRect& rect, BOOL up, BOOL hovered)
    {
        const ThemeColors& colors = GetColors();

        // 배경
        if (hovered)
        {
            CRect bgRect = rect;
            bgRect.DeflateRect(4, 2);
            CBrush brush(colors.surface.hover);
            CPen pen(PS_SOLID, 1, colors.border.normal);
            CPen* oldPen = pDC->SelectObject(&pen);
            CBrush* oldBrush = pDC->SelectObject(&brush);
            pDC->RoundRect(bgRect, CPoint(4, 4));
            pDC->SelectObject(oldPen);
            pDC->SelectObject(oldBrush);
        }

        // 화살표
        int cx = rect.CenterPoint().x;
        int cy = rect.CenterPoint().y;
        int size = 5;

        COLORREF arrowColor = hovered ? colors.primary.normal : colors.textSecondary;

        POINT points[3];
        if (up)
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

        CBrush brush(arrowColor);
        CPen pen(PS_SOLID, 1, arrowColor);
        CPen* oldPen = pDC->SelectObject(&pen);
        CBrush* oldBrush = pDC->SelectObject(&brush);
        pDC->Polygon(points, 3);
        pDC->SelectObject(oldPen);
        pDC->SelectObject(oldBrush);
    }

    void CMTimePopup::OnLButtonDown(UINT nFlags, CPoint point)
    {
        int col = HitTestColumn(point);
        int row = HitTestRow(point);

        if (col >= 0 && row != 0)
        {
            if (row == -1)
                IncrementValue(col);
            else
                DecrementValue(col);

            Apply();
            Invalidate();
        }

        CWnd::OnLButtonDown(nFlags, point);
    }

    void CMTimePopup::OnMouseMove(UINT nFlags, CPoint point)
    {
        int newCol = HitTestColumn(point);
        int newRow = HitTestRow(point);

        if (newCol != m_hoverColumn || newRow != m_hoverRow)
        {
            m_hoverColumn = newCol;
            m_hoverRow = newRow;
            Invalidate();
        }

        CWnd::OnMouseMove(nFlags, point);
    }

    void CMTimePopup::OnKillFocus(CWnd* pNewWnd)
    {
        if (pNewWnd != m_pOwner)
        {
            Hide();
        }
        CWnd::OnKillFocus(pNewWnd);
    }

    BOOL CMTimePopup::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
    {
        ScreenToClient(&pt);
        int col = HitTestColumn(pt);

        if (col >= 0)
        {
            if (zDelta > 0)
                IncrementValue(col);
            else
                DecrementValue(col);

            Apply();
            Invalidate();
        }

        return TRUE;
    }

    int CMTimePopup::HitTestColumn(CPoint point)
    {
        int x = 8;
        int col = 0;

        // 시
        if (point.x >= x && point.x < x + COLUMN_WIDTH)
            return 0;
        x += COLUMN_WIDTH;

        // 분
        if (point.x >= x && point.x < x + COLUMN_WIDTH)
            return 1;
        x += COLUMN_WIDTH;

        // 초
        if (m_pOwner->GetShowSeconds())
        {
            if (point.x >= x && point.x < x + COLUMN_WIDTH)
                return 2;
            x += COLUMN_WIDTH;
        }

        // AM/PM
        if (!m_pOwner->Is24HourFormat())
        {
            if (point.x >= x && point.x < x + COLUMN_WIDTH)
                return 3;
        }

        return -1;
    }

    int CMTimePopup::HitTestRow(CPoint point)
    {
        int y = 8;

        if (point.y >= y && point.y < y + BUTTON_HEIGHT)
            return -1;  // 위 버튼
        y += BUTTON_HEIGHT;

        if (point.y >= y && point.y < y + ROW_HEIGHT)
            return 0;   // 값
        y += ROW_HEIGHT;

        if (point.y >= y && point.y < y + BUTTON_HEIGHT)
            return 1;   // 아래 버튼

        return -2;
    }

    void CMTimePopup::IncrementValue(int col)
    {
        switch (col)
        {
        case 0:  // 시
            m_tempHour = (m_tempHour + 1) % 24;
            break;
        case 1:  // 분
            m_tempMinute = (m_tempMinute + m_pOwner->GetMinuteStep()) % 60;
            break;
        case 2:  // 초
            m_tempSecond = (m_tempSecond + 1) % 60;
            break;
        case 3:  // AM/PM
            m_tempHour = (m_tempHour + 12) % 24;
            break;
        }
    }

    void CMTimePopup::DecrementValue(int col)
    {
        switch (col)
        {
        case 0:  // 시
            m_tempHour = (m_tempHour + 23) % 24;
            break;
        case 1:  // 분
            m_tempMinute = (m_tempMinute + 60 - m_pOwner->GetMinuteStep()) % 60;
            break;
        case 2:  // 초
            m_tempSecond = (m_tempSecond + 59) % 60;
            break;
        case 3:  // AM/PM
            m_tempHour = (m_tempHour + 12) % 24;
            break;
        }
    }

    void CMTimePopup::Apply()
    {
        m_pOwner->SetTime(m_tempHour, m_tempMinute);
        if (m_pOwner->GetShowSeconds())
            m_pOwner->SetSecond(m_tempSecond);
    }

    const ThemeColors& CMTimePopup::GetColors() const
    {
        return CMThemeManager::GetInstance().GetColors();
    }
}
