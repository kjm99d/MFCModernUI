#include "pch.h"
#include "CMDatePicker.h"

namespace MFCModernUI
{
    // CMDatePicker 구현
    IMPLEMENT_DYNAMIC(CMDatePicker, CMControlBase)

    BEGIN_MESSAGE_MAP(CMDatePicker, CMControlBase)
        ON_WM_LBUTTONDOWN()
        ON_WM_KEYDOWN()
        ON_WM_KILLFOCUS()
    END_MESSAGE_MAP()

    CMDatePicker::CMDatePicker()
        : m_format(_T("yyyy-MM-dd"))
        , m_showTodayButton(TRUE)
        , m_firstDayOfWeek(0)
        , m_popup(nullptr)
        , m_dateChangedHandler(nullptr)
        , m_dateChangedContext(nullptr)
    {
        m_themeClass = _T("datepicker");
        m_tabStop = TRUE;

        // 현재 날짜로 초기화
        m_date = COleDateTime::GetCurrentTime();
        m_minDate = COleDateTime(1900, 1, 1, 0, 0, 0);
        m_maxDate = COleDateTime(2100, 12, 31, 0, 0, 0);
    }

    CMDatePicker::~CMDatePicker()
    {
        if (m_popup)
        {
            m_popup->DestroyWindow();
            delete m_popup;
        }
    }

    BOOL CMDatePicker::Create(DWORD dwStyle, const CRect& rect, CWnd* pParentWnd, UINT nID)
    {
        CString className = AfxRegisterWndClass(
            CS_HREDRAW | CS_VREDRAW,
            LoadCursor(nullptr, IDC_ARROW),
            nullptr,
            nullptr
        );

        return CWnd::Create(className, nullptr, dwStyle | WS_CHILD | WS_TABSTOP, rect, pParentWnd, nID);
    }

    void CMDatePicker::SetDate(const COleDateTime& date)
    {
        if (date < m_minDate || date > m_maxDate)
            return;

        if (m_date != date)
        {
            COleDateTime oldDate = m_date;
            m_date = date;
            NotifyDateChanged(oldDate);
            Invalidate();
        }
    }

    void CMDatePicker::SetMinDate(const COleDateTime& date)
    {
        m_minDate = date;
        if (m_date < m_minDate)
        {
            SetDate(m_minDate);
        }
    }

    void CMDatePicker::SetMaxDate(const COleDateTime& date)
    {
        m_maxDate = date;
        if (m_date > m_maxDate)
        {
            SetDate(m_maxDate);
        }
    }

    void CMDatePicker::SetFormat(LPCTSTR format)
    {
        m_format = format;
        Invalidate();
    }

    void CMDatePicker::SetShowTodayButton(BOOL show)
    {
        m_showTodayButton = show;
    }

    void CMDatePicker::SetFirstDayOfWeek(int day)
    {
        m_firstDayOfWeek = day % 7;
    }

    void CMDatePicker::SetDateChangedHandler(DateChangedHandler handler, void* context)
    {
        m_dateChangedHandler = handler;
        m_dateChangedContext = context;
    }

    void CMDatePicker::ShowDropDown()
    {
        if (!m_popup)
        {
            m_popup = new CMCalendarPopup(this);
            m_popup->Create();
        }

        CRect rect;
        GetWindowRect(&rect);

        m_popup->Show(CPoint(rect.left, rect.bottom));
    }

    void CMDatePicker::HideDropDown()
    {
        if (m_popup && m_popup->IsWindowVisible())
        {
            m_popup->Hide();
        }
    }

    BOOL CMDatePicker::IsDroppedDown() const
    {
        return m_popup && m_popup->IsWindowVisible();
    }

    void CMDatePicker::OnDraw(CDC* pDC)
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

        CString dateText = FormatDate(m_date);
        COLORREF textColor = m_isEnabled ? colors.text : colors.textDisabled;

        DrawText(pDC, dateText, textRect, textColor, TextStyle::Body,
            DT_LEFT | DT_VCENTER | DT_SINGLELINE);
    }

    void CMDatePicker::DrawButton(CDC* pDC, const CRect& rect)
    {
        const ThemeColors& colors = GetColors();

        // 배경
        COLORREF bgColor = m_isHovered ? colors.surface.hover : colors.surface.normal;
        pDC->FillSolidRect(rect, bgColor);

        // 캘린더 아이콘 그리기 (간단한 그리드)
        int iconSize = 14;
        int cx = rect.CenterPoint().x;
        int cy = rect.CenterPoint().y;
        int left = cx - iconSize / 2;
        int top = cy - iconSize / 2;

        COLORREF iconColor = m_isEnabled ? colors.textSecondary : colors.textDisabled;
        CPen pen(PS_SOLID, 1, iconColor);
        CPen* oldPen = pDC->SelectObject(&pen);

        // 캘린더 외곽
        CRect iconRect(left, top, left + iconSize, top + iconSize);
        pDC->RoundRect(iconRect, CPoint(2, 2));

        // 상단 바
        pDC->MoveTo(left, top + 4);
        pDC->LineTo(left + iconSize, top + 4);

        // 그리드 (2x2)
        int gridTop = top + 6;
        int cellW = (iconSize - 2) / 2;
        int cellH = (iconSize - 6) / 2;

        for (int i = 0; i < 2; i++)
        {
            for (int j = 0; j < 2; j++)
            {
                CRect cellRect(
                    left + 1 + j * cellW,
                    gridTop + i * cellH,
                    left + 1 + (j + 1) * cellW,
                    gridTop + (i + 1) * cellH
                );
                pDC->Rectangle(cellRect);
            }
        }

        pDC->SelectObject(oldPen);
    }

    CString CMDatePicker::FormatDate(const COleDateTime& date)
    {
        CString result = m_format;

        CString year, month, day;
        year.Format(_T("%04d"), date.GetYear());
        month.Format(_T("%02d"), date.GetMonth());
        day.Format(_T("%02d"), date.GetDay());

        result.Replace(_T("yyyy"), year);
        result.Replace(_T("MM"), month);
        result.Replace(_T("dd"), day);

        return result;
    }

    void CMDatePicker::NotifyDateChanged(const COleDateTime& oldDate)
    {
        if (m_dateChangedHandler)
        {
            m_dateChangedHandler(m_dateChangedContext, oldDate, m_date);
        }
    }

    void CMDatePicker::OnLButtonDown(UINT nFlags, CPoint point)
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

    void CMDatePicker::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
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
            SetDate(COleDateTime(m_date.GetYear(), m_date.GetMonth(), m_date.GetDay() - 1, 0, 0, 0));
            break;

        case VK_LEFT:
            SetDate(COleDateTime(m_date.GetYear(), m_date.GetMonth(), m_date.GetDay() - 1, 0, 0, 0));
            break;

        case VK_RIGHT:
            SetDate(COleDateTime(m_date.GetYear(), m_date.GetMonth(), m_date.GetDay() + 1, 0, 0, 0));
            break;
        }

        CMControlBase::OnKeyDown(nChar, nRepCnt, nFlags);
    }

    void CMDatePicker::OnKillFocus(CWnd* pNewWnd)
    {
        // 팝업으로 포커스가 이동한 경우는 닫지 않음
        if (m_popup && pNewWnd == m_popup)
            return;

        CMControlBase::OnKillFocus(pNewWnd);
    }

    // CMCalendarPopup 구현
    BEGIN_MESSAGE_MAP(CMCalendarPopup, CWnd)
        ON_WM_PAINT()
        ON_WM_ERASEBKGND()
        ON_WM_LBUTTONDOWN()
        ON_WM_MOUSEMOVE()
        ON_WM_KILLFOCUS()
        ON_WM_KEYDOWN()
    END_MESSAGE_MAP()

    CMCalendarPopup::CMCalendarPopup(CMDatePicker* pOwner)
        : m_pOwner(pOwner)
        , m_hoverDay(-1)
        , m_hoverMonth(0)
    {
        m_viewDate = pOwner->GetDate();
    }

    CMCalendarPopup::~CMCalendarPopup()
    {
    }

    BOOL CMCalendarPopup::Create()
    {
        CString className = AfxRegisterWndClass(
            CS_HREDRAW | CS_VREDRAW | CS_DROPSHADOW,
            LoadCursor(nullptr, IDC_ARROW),
            nullptr,
            nullptr
        );

        int width = 7 * CELL_WIDTH + 16;
        int height = HEADER_HEIGHT + WEEKDAY_HEIGHT + 6 * CELL_HEIGHT + 8;

        if (m_pOwner->GetShowTodayButton())
            height += FOOTER_HEIGHT;

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

    void CMCalendarPopup::Show(CPoint pt)
    {
        m_viewDate = m_pOwner->GetDate();

        CRect rect;
        GetWindowRect(&rect);

        // 화면 경계 확인
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

    void CMCalendarPopup::Hide()
    {
        ShowWindow(SW_HIDE);
        m_pOwner->Invalidate();
    }

    void CMCalendarPopup::OnPaint()
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

    BOOL CMCalendarPopup::OnEraseBkgnd(CDC* pDC)
    {
        return TRUE;
    }

    void CMCalendarPopup::OnDraw(CDC* pDC)
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

        // 헤더
        CRect headerRect = rect;
        headerRect.DeflateRect(8, 4, 8, 0);
        headerRect.bottom = headerRect.top + HEADER_HEIGHT;
        DrawHeader(pDC, headerRect);

        // 요일
        CRect weekdayRect = headerRect;
        weekdayRect.top = headerRect.bottom;
        weekdayRect.bottom = weekdayRect.top + WEEKDAY_HEIGHT;
        DrawWeekDays(pDC, weekdayRect);

        // 날짜
        CRect daysRect = weekdayRect;
        daysRect.top = weekdayRect.bottom;
        daysRect.bottom = daysRect.top + 6 * CELL_HEIGHT;
        DrawDays(pDC, daysRect);

        // 푸터 (오늘 버튼)
        if (m_pOwner->GetShowTodayButton())
        {
            CRect footerRect = daysRect;
            footerRect.top = daysRect.bottom;
            footerRect.bottom = footerRect.top + FOOTER_HEIGHT;
            DrawFooter(pDC, footerRect);
        }
    }

    void CMCalendarPopup::DrawHeader(CDC* pDC, const CRect& rect)
    {
        const ThemeColors& colors = GetColors();

        // 이전 월 버튼
        CRect prevRect = rect;
        prevRect.right = prevRect.left + CELL_WIDTH;
        COLORREF prevColor = (m_hoverMonth == -1) ? colors.primary.normal : colors.textSecondary;

        // 왼쪽 화살표
        int cx = prevRect.CenterPoint().x;
        int cy = prevRect.CenterPoint().y;
        POINT prevPoints[3] = {
            { cx - 4, cy },
            { cx + 2, cy - 6 },
            { cx + 2, cy + 6 }
        };
        CBrush prevBrush(prevColor);
        CPen prevPen(PS_SOLID, 1, prevColor);
        CPen* oldPen = pDC->SelectObject(&prevPen);
        CBrush* oldBrush = pDC->SelectObject(&prevBrush);
        pDC->Polygon(prevPoints, 3);

        // 다음 월 버튼
        CRect nextRect = rect;
        nextRect.left = nextRect.right - CELL_WIDTH;
        COLORREF nextColor = (m_hoverMonth == 1) ? colors.primary.normal : colors.textSecondary;

        // 오른쪽 화살표
        cx = nextRect.CenterPoint().x;
        POINT nextPoints[3] = {
            { cx + 4, cy },
            { cx - 2, cy - 6 },
            { cx - 2, cy + 6 }
        };
        CBrush nextBrush(nextColor);
        CPen nextPen(PS_SOLID, 1, nextColor);
        pDC->SelectObject(&nextPen);
        pDC->SelectObject(&nextBrush);
        pDC->Polygon(nextPoints, 3);

        pDC->SelectObject(oldPen);
        pDC->SelectObject(oldBrush);

        // 월/년 표시
        CString monthYear;
        monthYear.Format(_T("%d년 %d월"), m_viewDate.GetYear(), m_viewDate.GetMonth());

        CRect textRect = rect;
        textRect.left = prevRect.right;
        textRect.right = nextRect.left;

        CFont* pFont = CMThemeManager::GetInstance().GetFont(TextStyle::BodyBold);
        CFont* oldFont = pDC->SelectObject(pFont);
        pDC->SetBkMode(TRANSPARENT);
        pDC->SetTextColor(colors.text);
        pDC->DrawText(monthYear, textRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        pDC->SelectObject(oldFont);
    }

    void CMCalendarPopup::DrawWeekDays(CDC* pDC, const CRect& rect)
    {
        const ThemeColors& colors = GetColors();

        LPCTSTR weekDays[] = { _T("일"), _T("월"), _T("화"), _T("수"), _T("목"), _T("금"), _T("토") };

        CFont* pFont = CMThemeManager::GetInstance().GetFont(TextStyle::Caption);
        CFont* oldFont = pDC->SelectObject(pFont);
        pDC->SetBkMode(TRANSPARENT);

        int firstDay = m_pOwner->GetFirstDayOfWeek();

        for (int i = 0; i < 7; i++)
        {
            CRect dayRect(
                rect.left + i * CELL_WIDTH,
                rect.top,
                rect.left + (i + 1) * CELL_WIDTH,
                rect.bottom
            );

            int dayIndex = (firstDay + i) % 7;

            // 일요일은 빨강, 토요일은 파랑
            COLORREF textColor = colors.textSecondary;
            if (dayIndex == 0)
                textColor = RGB(220, 53, 69);
            else if (dayIndex == 6)
                textColor = RGB(0, 123, 255);

            pDC->SetTextColor(textColor);
            pDC->DrawText(weekDays[dayIndex], dayRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        }

        pDC->SelectObject(oldFont);
    }

    void CMCalendarPopup::DrawDays(CDC* pDC, const CRect& rect)
    {
        const ThemeColors& colors = GetColors();

        int year = m_viewDate.GetYear();
        int month = m_viewDate.GetMonth();
        int daysInMonth = GetDaysInMonth(year, month);
        int firstDayOfMonth = GetFirstDayOfMonth(year, month);
        int firstDayOfWeek = m_pOwner->GetFirstDayOfWeek();

        int startOffset = (firstDayOfMonth - firstDayOfWeek + 7) % 7;

        COleDateTime today = COleDateTime::GetCurrentTime();
        COleDateTime selectedDate = m_pOwner->GetDate();

        CFont* pFont = CMThemeManager::GetInstance().GetFont(TextStyle::Body);
        CFont* oldFont = pDC->SelectObject(pFont);
        pDC->SetBkMode(TRANSPARENT);

        for (int day = 1; day <= daysInMonth; day++)
        {
            int cellIndex = startOffset + day - 1;
            int row = cellIndex / 7;
            int col = cellIndex % 7;

            CRect dayRect(
                rect.left + col * CELL_WIDTH,
                rect.top + row * CELL_HEIGHT,
                rect.left + (col + 1) * CELL_WIDTH,
                rect.top + (row + 1) * CELL_HEIGHT
            );

            COleDateTime date(year, month, day, 0, 0, 0);
            BOOL isValid = IsDateValid(date);
            BOOL isToday = (date.GetYear() == today.GetYear() &&
                date.GetMonth() == today.GetMonth() &&
                date.GetDay() == today.GetDay());
            BOOL isSelected = (date.GetYear() == selectedDate.GetYear() &&
                date.GetMonth() == selectedDate.GetMonth() &&
                date.GetDay() == selectedDate.GetDay());
            BOOL isHovered = (day == m_hoverDay);

            // 배경
            if (isSelected)
            {
                CRect bgRect = dayRect;
                bgRect.DeflateRect(2, 2);
                CBrush brush(colors.primary.normal);
                CPen pen(PS_SOLID, 1, colors.primary.normal);
                CPen* oldPen = pDC->SelectObject(&pen);
                CBrush* oldBrush = pDC->SelectObject(&brush);
                pDC->RoundRect(bgRect, CPoint(4, 4));
                pDC->SelectObject(oldPen);
                pDC->SelectObject(oldBrush);
            }
            else if (isHovered && isValid)
            {
                CRect bgRect = dayRect;
                bgRect.DeflateRect(2, 2);
                CBrush brush(colors.surface.hover);
                CPen pen(PS_SOLID, 1, colors.border.normal);
                CPen* oldPen = pDC->SelectObject(&pen);
                CBrush* oldBrush = pDC->SelectObject(&brush);
                pDC->RoundRect(bgRect, CPoint(4, 4));
                pDC->SelectObject(oldPen);
                pDC->SelectObject(oldBrush);
            }

            // 오늘 표시 (원형 테두리)
            if (isToday && !isSelected)
            {
                CRect bgRect = dayRect;
                bgRect.DeflateRect(2, 2);
                CPen pen(PS_SOLID, 2, colors.primary.normal);
                CPen* oldPen = pDC->SelectObject(&pen);
                CBrush* oldBrush = (CBrush*)pDC->SelectStockObject(NULL_BRUSH);
                pDC->RoundRect(bgRect, CPoint(4, 4));
                pDC->SelectObject(oldPen);
                pDC->SelectObject(oldBrush);
            }

            // 텍스트
            COLORREF textColor;
            if (isSelected)
            {
                textColor = colors.textOnPrimary;
            }
            else if (!isValid)
            {
                textColor = colors.textDisabled;
            }
            else
            {
                int dayOfWeek = (firstDayOfWeek + col) % 7;
                if (dayOfWeek == 0)
                    textColor = RGB(220, 53, 69);
                else if (dayOfWeek == 6)
                    textColor = RGB(0, 123, 255);
                else
                    textColor = colors.text;
            }

            CString dayText;
            dayText.Format(_T("%d"), day);

            pDC->SetTextColor(textColor);
            pDC->DrawText(dayText, dayRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        }

        pDC->SelectObject(oldFont);
    }

    void CMCalendarPopup::DrawFooter(CDC* pDC, const CRect& rect)
    {
        const ThemeColors& colors = GetColors();

        CRect buttonRect = rect;
        buttonRect.DeflateRect(8, 4);

        // "오늘" 버튼
        CBrush brush(colors.surface.hover);
        CPen pen(PS_SOLID, 1, colors.border.normal);
        CPen* oldPen = pDC->SelectObject(&pen);
        CBrush* oldBrush = pDC->SelectObject(&brush);
        pDC->RoundRect(buttonRect, CPoint(4, 4));
        pDC->SelectObject(oldPen);
        pDC->SelectObject(oldBrush);

        CFont* pFont = CMThemeManager::GetInstance().GetFont(TextStyle::Body);
        CFont* oldFont = pDC->SelectObject(pFont);
        pDC->SetBkMode(TRANSPARENT);
        pDC->SetTextColor(colors.primary.normal);
        pDC->DrawText(_T("오늘"), buttonRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        pDC->SelectObject(oldFont);
    }

    void CMCalendarPopup::OnLButtonDown(UINT nFlags, CPoint point)
    {
        CRect rect;
        GetClientRect(&rect);
        rect.DeflateRect(8, 4, 8, 0);

        // 헤더 영역
        CRect headerRect = rect;
        headerRect.bottom = headerRect.top + HEADER_HEIGHT;

        CRect prevRect = headerRect;
        prevRect.right = prevRect.left + CELL_WIDTH;

        CRect nextRect = headerRect;
        nextRect.left = nextRect.right - CELL_WIDTH;

        if (prevRect.PtInRect(point))
        {
            // 이전 월
            m_viewDate = COleDateTime(
                m_viewDate.GetYear() - (m_viewDate.GetMonth() == 1 ? 1 : 0),
                m_viewDate.GetMonth() == 1 ? 12 : m_viewDate.GetMonth() - 1,
                1, 0, 0, 0);
            Invalidate();
            return;
        }

        if (nextRect.PtInRect(point))
        {
            // 다음 월
            m_viewDate = COleDateTime(
                m_viewDate.GetYear() + (m_viewDate.GetMonth() == 12 ? 1 : 0),
                m_viewDate.GetMonth() == 12 ? 1 : m_viewDate.GetMonth() + 1,
                1, 0, 0, 0);
            Invalidate();
            return;
        }

        // 푸터 영역 (오늘 버튼)
        if (m_pOwner->GetShowTodayButton())
        {
            CRect footerRect = rect;
            footerRect.top = rect.top + HEADER_HEIGHT + WEEKDAY_HEIGHT + 6 * CELL_HEIGHT;
            footerRect.bottom = footerRect.top + FOOTER_HEIGHT;

            if (footerRect.PtInRect(point))
            {
                COleDateTime today = COleDateTime::GetCurrentTime();
                if (IsDateValid(today))
                {
                    m_pOwner->SetDate(today);
                    Hide();
                }
                return;
            }
        }

        // 날짜 선택
        int day = HitTestDay(point);
        if (day > 0)
        {
            COleDateTime date(m_viewDate.GetYear(), m_viewDate.GetMonth(), day, 0, 0, 0);
            if (IsDateValid(date))
            {
                m_pOwner->SetDate(date);
                Hide();
            }
        }

        CWnd::OnLButtonDown(nFlags, point);
    }

    void CMCalendarPopup::OnMouseMove(UINT nFlags, CPoint point)
    {
        CRect rect;
        GetClientRect(&rect);
        rect.DeflateRect(8, 4, 8, 0);

        // 헤더 영역 확인
        CRect headerRect = rect;
        headerRect.bottom = headerRect.top + HEADER_HEIGHT;

        CRect prevRect = headerRect;
        prevRect.right = prevRect.left + CELL_WIDTH;

        CRect nextRect = headerRect;
        nextRect.left = nextRect.right - CELL_WIDTH;

        int newHoverMonth = 0;
        if (prevRect.PtInRect(point))
            newHoverMonth = -1;
        else if (nextRect.PtInRect(point))
            newHoverMonth = 1;

        // 날짜 확인
        int newHoverDay = HitTestDay(point);

        if (newHoverMonth != m_hoverMonth || newHoverDay != m_hoverDay)
        {
            m_hoverMonth = newHoverMonth;
            m_hoverDay = newHoverDay;
            Invalidate();
        }

        CWnd::OnMouseMove(nFlags, point);
    }

    void CMCalendarPopup::OnKillFocus(CWnd* pNewWnd)
    {
        if (pNewWnd != m_pOwner)
        {
            Hide();
        }
        CWnd::OnKillFocus(pNewWnd);
    }

    void CMCalendarPopup::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
    {
        switch (nChar)
        {
        case VK_ESCAPE:
            Hide();
            break;

        case VK_RETURN:
            Hide();
            break;
        }

        CWnd::OnKeyDown(nChar, nRepCnt, nFlags);
    }

    int CMCalendarPopup::HitTestDay(CPoint point)
    {
        CRect rect;
        GetClientRect(&rect);
        rect.DeflateRect(8, 4, 8, 0);

        CRect daysRect = rect;
        daysRect.top = rect.top + HEADER_HEIGHT + WEEKDAY_HEIGHT;
        daysRect.bottom = daysRect.top + 6 * CELL_HEIGHT;

        if (!daysRect.PtInRect(point))
            return -1;

        int col = (point.x - daysRect.left) / CELL_WIDTH;
        int row = (point.y - daysRect.top) / CELL_HEIGHT;

        if (col < 0 || col >= 7 || row < 0 || row >= 6)
            return -1;

        int cellIndex = row * 7 + col;
        int firstDayOfMonth = GetFirstDayOfMonth(m_viewDate.GetYear(), m_viewDate.GetMonth());
        int firstDayOfWeek = m_pOwner->GetFirstDayOfWeek();
        int startOffset = (firstDayOfMonth - firstDayOfWeek + 7) % 7;

        int day = cellIndex - startOffset + 1;
        int daysInMonth = GetDaysInMonth(m_viewDate.GetYear(), m_viewDate.GetMonth());

        if (day < 1 || day > daysInMonth)
            return -1;

        return day;
    }

    CRect CMCalendarPopup::GetDayRect(int day)
    {
        CRect rect;
        GetClientRect(&rect);
        rect.DeflateRect(8, 4, 8, 0);

        CRect daysRect = rect;
        daysRect.top = rect.top + HEADER_HEIGHT + WEEKDAY_HEIGHT;

        int firstDayOfMonth = GetFirstDayOfMonth(m_viewDate.GetYear(), m_viewDate.GetMonth());
        int firstDayOfWeek = m_pOwner->GetFirstDayOfWeek();
        int startOffset = (firstDayOfMonth - firstDayOfWeek + 7) % 7;
        int cellIndex = startOffset + day - 1;
        int row = cellIndex / 7;
        int col = cellIndex % 7;

        return CRect(
            daysRect.left + col * CELL_WIDTH,
            daysRect.top + row * CELL_HEIGHT,
            daysRect.left + (col + 1) * CELL_WIDTH,
            daysRect.top + (row + 1) * CELL_HEIGHT
        );
    }

    int CMCalendarPopup::GetDaysInMonth(int year, int month)
    {
        int days[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

        if (month == 2)
        {
            // 윤년 확인
            if ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0))
                return 29;
        }

        return days[month - 1];
    }

    int CMCalendarPopup::GetFirstDayOfMonth(int year, int month)
    {
        COleDateTime date(year, month, 1, 0, 0, 0);
        return date.GetDayOfWeek() - 1;  // 0=일요일, 1=월요일, ...
    }

    BOOL CMCalendarPopup::IsDateValid(const COleDateTime& date)
    {
        return date >= m_pOwner->m_minDate && date <= m_pOwner->m_maxDate;
    }

    const ThemeColors& CMCalendarPopup::GetColors() const
    {
        return CMThemeManager::GetInstance().GetColors();
    }
}
