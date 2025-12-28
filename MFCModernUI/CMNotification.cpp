#include "pch.h"
#include "CMNotification.h"

namespace MFCModernUI
{
    // CMNotificationWindow 구현
    BEGIN_MESSAGE_MAP(CMNotificationWindow, CWnd)
        ON_WM_PAINT()
        ON_WM_ERASEBKGND()
        ON_WM_TIMER()
        ON_WM_LBUTTONDOWN()
        ON_WM_MOUSEMOVE()
        ON_WM_MOUSELEAVE()
    END_MESSAGE_MAP()

    CMNotificationWindow::CMNotificationWindow()
        : m_id(0)
        , m_type(NotificationType::Info)
        , m_duration(0)
        , m_showClose(TRUE)
        , m_isHovered(FALSE)
        , m_closeHovered(FALSE)
        , m_timerId(0)
        , m_clickHandler(nullptr)
        , m_clickContext(nullptr)
        , m_closeHandler(nullptr)
        , m_closeContext(nullptr)
    {
    }

    CMNotificationWindow::~CMNotificationWindow()
    {
        if (m_timerId)
        {
            KillTimer(m_timerId);
        }
    }

    BOOL CMNotificationWindow::Create(CWnd* pParentWnd)
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
            0, 0, WIDTH, MIN_HEIGHT,
            pParentWnd ? pParentWnd->GetSafeHwnd() : nullptr,
            nullptr
        );
    }

    void CMNotificationWindow::Show(int id, NotificationType type, LPCTSTR title,
        LPCTSTR message, DWORD duration, BOOL showClose)
    {
        m_id = id;
        m_type = type;
        m_title = title;
        m_message = message;
        m_duration = duration;
        m_showClose = showClose;

        // 높이 계산
        int height = GetHeight();
        SetWindowPos(nullptr, 0, 0, WIDTH, height, SWP_NOMOVE | SWP_NOZORDER);

        ShowWindow(SW_SHOWNOACTIVATE);

        // 자동 숨김 타이머
        if (m_duration > 0)
        {
            m_timerId = SetTimer(HIDE_TIMER_ID, m_duration, nullptr);
        }
    }

    void CMNotificationWindow::Hide()
    {
        if (m_timerId)
        {
            KillTimer(m_timerId);
            m_timerId = 0;
        }

        ShowWindow(SW_HIDE);

        if (m_closeHandler)
        {
            m_closeHandler(m_closeContext, m_id);
        }
    }

    int CMNotificationWindow::GetHeight() const
    {
        // 높이 계산 (제목 + 메시지 + 패딩)
        CDC* pDC = const_cast<CMNotificationWindow*>(this)->GetDC();
        if (!pDC)
            return MIN_HEIGHT;

        int contentWidth = WIDTH - PADDING * 3 - ICON_SIZE;
        if (m_showClose)
            contentWidth -= CLOSE_SIZE;

        CFont* pFont = CMThemeManager::GetInstance().GetFont(TextStyle::Body);
        CFont* oldFont = pDC->SelectObject(pFont);

        int height = PADDING * 2;

        // 제목
        if (!m_title.IsEmpty())
        {
            height += 20;
        }

        // 메시지
        if (!m_message.IsEmpty())
        {
            CRect textRect(0, 0, contentWidth, 0);
            pDC->DrawText(m_message, textRect, DT_CALCRECT | DT_WORDBREAK);
            height += textRect.Height();
        }

        pDC->SelectObject(oldFont);
        const_cast<CMNotificationWindow*>(this)->ReleaseDC(pDC);

        return max(MIN_HEIGHT, height);
    }

    void CMNotificationWindow::SetClickHandler(NotificationClickHandler handler, void* context)
    {
        m_clickHandler = handler;
        m_clickContext = context;
    }

    void CMNotificationWindow::SetCloseHandler(NotificationCloseHandler handler, void* context)
    {
        m_closeHandler = handler;
        m_closeContext = context;
    }

    void CMNotificationWindow::OnPaint()
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

    BOOL CMNotificationWindow::OnEraseBkgnd(CDC* pDC)
    {
        return TRUE;
    }

    void CMNotificationWindow::OnDraw(CDC* pDC)
    {
        CRect rect;
        GetClientRect(&rect);

        const ThemeColors& colors = GetColors();

        // 배경
        COLORREF bgColor = colors.surface.normal;
        pDC->FillSolidRect(rect, bgColor);

        // 왼쪽 색상 띠
        CRect stripRect = rect;
        stripRect.right = stripRect.left + 4;
        pDC->FillSolidRect(stripRect, GetTypeColor());

        // 테두리
        CPen pen(PS_SOLID, 1, colors.border.normal);
        CPen* oldPen = pDC->SelectObject(&pen);
        CBrush* oldBrush = (CBrush*)pDC->SelectStockObject(NULL_BRUSH);
        pDC->RoundRect(rect, CPoint(4, 4));
        pDC->SelectObject(oldPen);
        pDC->SelectObject(oldBrush);

        // 아이콘
        CRect iconRect(PADDING + 4, PADDING, PADDING + 4 + ICON_SIZE, PADDING + ICON_SIZE);
        DrawIcon(pDC, iconRect);

        // 닫기 버튼
        if (m_showClose)
        {
            DrawCloseButton(pDC, GetCloseButtonRect());
        }

        // 내용 영역
        CRect contentRect = rect;
        contentRect.left = iconRect.right + PADDING;
        contentRect.top = PADDING;
        contentRect.right -= PADDING;
        if (m_showClose)
            contentRect.right -= CLOSE_SIZE + 4;
        contentRect.bottom -= PADDING;

        // 제목
        if (!m_title.IsEmpty())
        {
            CFont* pFont = CMThemeManager::GetInstance().GetFont(TextStyle::BodyBold);
            CFont* oldFont = pDC->SelectObject(pFont);
            pDC->SetBkMode(TRANSPARENT);
            pDC->SetTextColor(colors.text);

            CRect titleRect = contentRect;
            titleRect.bottom = titleRect.top + 18;
            pDC->DrawText(m_title, titleRect, DT_LEFT | DT_SINGLELINE | DT_END_ELLIPSIS);

            contentRect.top = titleRect.bottom + 2;
            pDC->SelectObject(oldFont);
        }

        // 메시지
        if (!m_message.IsEmpty())
        {
            CFont* pFont = CMThemeManager::GetInstance().GetFont(TextStyle::Body);
            CFont* oldFont = pDC->SelectObject(pFont);
            pDC->SetBkMode(TRANSPARENT);
            pDC->SetTextColor(colors.textSecondary);
            pDC->DrawText(m_message, contentRect, DT_LEFT | DT_WORDBREAK);
            pDC->SelectObject(oldFont);
        }
    }

    void CMNotificationWindow::DrawIcon(CDC* pDC, const CRect& rect)
    {
        COLORREF color = GetTypeColor();
        int cx = rect.CenterPoint().x;
        int cy = rect.CenterPoint().y;

        CPen pen(PS_SOLID, 2, color);
        CPen* oldPen = pDC->SelectObject(&pen);
        CBrush* oldBrush = (CBrush*)pDC->SelectStockObject(NULL_BRUSH);

        switch (m_type)
        {
        case NotificationType::Info:
            // i 아이콘 (원 + i)
            pDC->Ellipse(cx - 10, cy - 10, cx + 10, cy + 10);
            pDC->MoveTo(cx, cy - 4);
            pDC->LineTo(cx, cy + 5);
            pDC->Ellipse(cx - 2, cy - 7, cx + 2, cy - 5);
            break;

        case NotificationType::Success:
            // 체크마크
            pDC->Ellipse(cx - 10, cy - 10, cx + 10, cy + 10);
            pDC->MoveTo(cx - 5, cy);
            pDC->LineTo(cx - 1, cy + 4);
            pDC->LineTo(cx + 6, cy - 4);
            break;

        case NotificationType::Warning:
            // 삼각형 + !
            {
                POINT points[3] = {
                    { cx, cy - 10 },
                    { cx - 10, cy + 8 },
                    { cx + 10, cy + 8 }
                };
                pDC->Polygon(points, 3);
                pDC->MoveTo(cx, cy - 3);
                pDC->LineTo(cx, cy + 2);
                pDC->Ellipse(cx - 1, cy + 4, cx + 1, cy + 6);
            }
            break;

        case NotificationType::Error:
            // X 아이콘
            pDC->Ellipse(cx - 10, cy - 10, cx + 10, cy + 10);
            pDC->MoveTo(cx - 5, cy - 5);
            pDC->LineTo(cx + 5, cy + 5);
            pDC->MoveTo(cx + 5, cy - 5);
            pDC->LineTo(cx - 5, cy + 5);
            break;
        }

        pDC->SelectObject(oldPen);
        pDC->SelectObject(oldBrush);
    }

    void CMNotificationWindow::DrawCloseButton(CDC* pDC, const CRect& rect)
    {
        const ThemeColors& colors = GetColors();

        // 호버 배경
        if (m_closeHovered)
        {
            CBrush brush(colors.surface.hover);
            pDC->FillRect(rect, &brush);
        }

        // X 아이콘
        int cx = rect.CenterPoint().x;
        int cy = rect.CenterPoint().y;
        int size = 4;

        COLORREF color = m_closeHovered ? colors.text : colors.textSecondary;
        CPen pen(PS_SOLID, 2, color);
        CPen* oldPen = pDC->SelectObject(&pen);

        pDC->MoveTo(cx - size, cy - size);
        pDC->LineTo(cx + size, cy + size);
        pDC->MoveTo(cx + size, cy - size);
        pDC->LineTo(cx - size, cy + size);

        pDC->SelectObject(oldPen);
    }

    CRect CMNotificationWindow::GetCloseButtonRect()
    {
        CRect rect;
        GetClientRect(&rect);

        return CRect(
            rect.right - PADDING - CLOSE_SIZE,
            PADDING,
            rect.right - PADDING,
            PADDING + CLOSE_SIZE
        );
    }

    COLORREF CMNotificationWindow::GetTypeColor() const
    {
        switch (m_type)
        {
        case NotificationType::Success:
            return RGB(76, 175, 80);    // 녹색
        case NotificationType::Warning:
            return RGB(255, 152, 0);    // 주황
        case NotificationType::Error:
            return RGB(244, 67, 54);    // 빨강
        case NotificationType::Info:
        default:
            return RGB(33, 150, 243);   // 파랑
        }
    }

    const ThemeColors& CMNotificationWindow::GetColors() const
    {
        return CMThemeManager::GetInstance().GetColors();
    }

    void CMNotificationWindow::OnTimer(UINT_PTR nIDEvent)
    {
        if (nIDEvent == HIDE_TIMER_ID)
        {
            Hide();
        }
        CWnd::OnTimer(nIDEvent);
    }

    void CMNotificationWindow::OnLButtonDown(UINT nFlags, CPoint point)
    {
        // 닫기 버튼 클릭
        if (m_showClose && GetCloseButtonRect().PtInRect(point))
        {
            Hide();
            return;
        }

        // 알림 클릭
        if (m_clickHandler)
        {
            m_clickHandler(m_clickContext, m_id);
        }

        CWnd::OnLButtonDown(nFlags, point);
    }

    void CMNotificationWindow::OnMouseMove(UINT nFlags, CPoint point)
    {
        if (!m_isHovered)
        {
            m_isHovered = TRUE;

            TRACKMOUSEEVENT tme;
            tme.cbSize = sizeof(tme);
            tme.dwFlags = TME_LEAVE;
            tme.hwndTrack = m_hWnd;
            TrackMouseEvent(&tme);

            // 호버 시 타이머 일시 중지
            if (m_timerId)
            {
                KillTimer(m_timerId);
                m_timerId = 0;
            }
        }

        BOOL newCloseHovered = m_showClose && GetCloseButtonRect().PtInRect(point);
        if (newCloseHovered != m_closeHovered)
        {
            m_closeHovered = newCloseHovered;
            Invalidate();
        }

        CWnd::OnMouseMove(nFlags, point);
    }

    void CMNotificationWindow::OnMouseLeave()
    {
        m_isHovered = FALSE;
        m_closeHovered = FALSE;
        Invalidate();

        // 타이머 재시작
        if (m_duration > 0 && m_timerId == 0)
        {
            m_timerId = SetTimer(HIDE_TIMER_ID, m_duration, nullptr);
        }

        CWnd::OnMouseLeave();
    }

    // CMNotificationManager 구현
    CMNotificationManager& CMNotificationManager::GetInstance()
    {
        static CMNotificationManager instance;
        return instance;
    }

    CMNotificationManager::CMNotificationManager()
        : m_nextId(1)
        , m_position(NotificationPosition::TopRight)
        , m_maxVisible(5)
        , m_spacing(8)
        , m_clickHandler(nullptr)
        , m_clickContext(nullptr)
        , m_closeHandler(nullptr)
        , m_closeContext(nullptr)
    {
    }

    CMNotificationManager::~CMNotificationManager()
    {
        HideAllNotifications();

        for (INT_PTR i = 0; i < m_notifications.GetSize(); i++)
        {
            delete m_notifications[i];
        }
        m_notifications.RemoveAll();
    }

    int CMNotificationManager::ShowNotification(NotificationType type, LPCTSTR title,
        LPCTSTR message, DWORD duration, BOOL showClose)
    {
        // 최대 개수 초과 시 가장 오래된 것 제거
        while (m_notifications.GetSize() >= m_maxVisible)
        {
            CMNotificationWindow* oldest = m_notifications[0];
            oldest->Hide();
            m_notifications.RemoveAt(0);
            delete oldest;
        }

        // 새 알림 창 생성
        CMNotificationWindow* notification = CreateNotificationWindow();
        if (!notification)
            return 0;

        int id = m_nextId++;
        notification->SetCloseHandler(StaticCloseHandler, this);
        notification->SetClickHandler(m_clickHandler, m_clickContext);
        notification->Show(id, type, title, message, duration, showClose);

        m_notifications.Add(notification);
        RepositionNotifications();

        return id;
    }

    int CMNotificationManager::ShowInfo(LPCTSTR message, DWORD duration)
    {
        return ShowNotification(NotificationType::Info, _T("정보"), message, duration);
    }

    int CMNotificationManager::ShowSuccess(LPCTSTR message, DWORD duration)
    {
        return ShowNotification(NotificationType::Success, _T("성공"), message, duration);
    }

    int CMNotificationManager::ShowWarning(LPCTSTR message, DWORD duration)
    {
        return ShowNotification(NotificationType::Warning, _T("경고"), message, duration);
    }

    int CMNotificationManager::ShowError(LPCTSTR message, DWORD duration)
    {
        return ShowNotification(NotificationType::Error, _T("오류"), message, duration);
    }

    void CMNotificationManager::HideNotification(int id)
    {
        for (INT_PTR i = 0; i < m_notifications.GetSize(); i++)
        {
            if (m_notifications[i]->GetNotificationId() == id)
            {
                m_notifications[i]->Hide();
                break;
            }
        }
    }

    void CMNotificationManager::HideAllNotifications()
    {
        for (INT_PTR i = 0; i < m_notifications.GetSize(); i++)
        {
            m_notifications[i]->Hide();
        }
    }

    void CMNotificationManager::SetPosition(NotificationPosition position)
    {
        m_position = position;
        RepositionNotifications();
    }

    void CMNotificationManager::SetMaxVisible(int max)
    {
        m_maxVisible = max(1, max);
    }

    void CMNotificationManager::SetSpacing(int spacing)
    {
        m_spacing = spacing;
        RepositionNotifications();
    }

    void CMNotificationManager::SetClickHandler(NotificationClickHandler handler, void* context)
    {
        m_clickHandler = handler;
        m_clickContext = context;
    }

    void CMNotificationManager::SetCloseHandler(NotificationCloseHandler handler, void* context)
    {
        m_closeHandler = handler;
        m_closeContext = context;
    }

    CMNotificationWindow* CMNotificationManager::CreateNotificationWindow()
    {
        CMNotificationWindow* notification = new CMNotificationWindow();

        if (!notification->Create(nullptr))
        {
            delete notification;
            return nullptr;
        }

        return notification;
    }

    void CMNotificationManager::RepositionNotifications()
    {
        CRect screenRect;
        SystemParametersInfo(SPI_GETWORKAREA, 0, &screenRect, 0);

        int margin = 16;
        int y;
        BOOL fromTop = (m_position == NotificationPosition::TopLeft ||
            m_position == NotificationPosition::TopCenter ||
            m_position == NotificationPosition::TopRight);

        if (fromTop)
        {
            y = screenRect.top + margin;
        }
        else
        {
            y = screenRect.bottom - margin;
        }

        for (INT_PTR i = 0; i < m_notifications.GetSize(); i++)
        {
            CMNotificationWindow* notification = m_notifications[i];

            if (!notification->IsWindowVisible())
                continue;

            CRect rect;
            notification->GetWindowRect(&rect);

            int x;
            switch (m_position)
            {
            case NotificationPosition::TopLeft:
            case NotificationPosition::BottomLeft:
                x = screenRect.left + margin;
                break;

            case NotificationPosition::TopCenter:
            case NotificationPosition::BottomCenter:
                x = (screenRect.left + screenRect.right - rect.Width()) / 2;
                break;

            case NotificationPosition::TopRight:
            case NotificationPosition::BottomRight:
            default:
                x = screenRect.right - margin - rect.Width();
                break;
            }

            if (!fromTop)
            {
                y -= rect.Height();
            }

            notification->SetWindowPos(nullptr, x, y, 0, 0,
                SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);

            if (fromTop)
            {
                y += rect.Height() + m_spacing;
            }
            else
            {
                y -= m_spacing;
            }
        }
    }

    void CMNotificationManager::OnNotificationClosed(int id)
    {
        for (INT_PTR i = 0; i < m_notifications.GetSize(); i++)
        {
            if (m_notifications[i]->GetNotificationId() == id)
            {
                CMNotificationWindow* notification = m_notifications[i];
                m_notifications.RemoveAt(i);

                // 외부 핸들러 호출
                if (m_closeHandler)
                {
                    m_closeHandler(m_closeContext, id);
                }

                // 창 삭제 (지연)
                notification->PostMessage(WM_CLOSE);

                RepositionNotifications();
                break;
            }
        }
    }

    void CMNotificationManager::StaticCloseHandler(void* context, int id)
    {
        CMNotificationManager* manager = static_cast<CMNotificationManager*>(context);
        manager->OnNotificationClosed(id);
    }
}
