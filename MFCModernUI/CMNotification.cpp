#include "pch.h"
#include "CMNotification.h"

namespace MFCModernUI
{
    // Direct2D 정적 멤버 초기화
    ID2D1Factory* CMNotificationWindow::s_pD2DFactory = nullptr;
    IDWriteFactory* CMNotificationWindow::s_pDWriteFactory = nullptr;
    int CMNotificationWindow::s_d2dRefCount = 0;

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
        , m_pRenderTarget(nullptr)
        , m_pSolidBrush(nullptr)
        , m_pTextFormat(nullptr)
        , m_isD2DDrawing(FALSE)
    {
        InitD2D();
    }

    CMNotificationWindow::~CMNotificationWindow()
    {
        if (m_timerId)
        {
            KillTimer(m_timerId);
        }
        ReleaseD2D();
    }

    void CMNotificationWindow::InitD2D()
    {
        if (s_d2dRefCount == 0)
        {
            D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &s_pD2DFactory);
            DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED,
                __uuidof(IDWriteFactory), reinterpret_cast<IUnknown**>(&s_pDWriteFactory));
        }
        s_d2dRefCount++;
    }

    void CMNotificationWindow::ReleaseD2D()
    {
        if (m_pTextFormat)
        {
            m_pTextFormat->Release();
            m_pTextFormat = nullptr;
        }
        if (m_pSolidBrush)
        {
            m_pSolidBrush->Release();
            m_pSolidBrush = nullptr;
        }
        if (m_pRenderTarget)
        {
            m_pRenderTarget->Release();
            m_pRenderTarget = nullptr;
        }

        s_d2dRefCount--;
        if (s_d2dRefCount == 0)
        {
            if (s_pDWriteFactory)
            {
                s_pDWriteFactory->Release();
                s_pDWriteFactory = nullptr;
            }
            if (s_pD2DFactory)
            {
                s_pD2DFactory->Release();
                s_pD2DFactory = nullptr;
            }
        }
    }

    BOOL CMNotificationWindow::BeginD2DDraw()
    {
        if (!s_pD2DFactory || !GetSafeHwnd())
            return FALSE;

        if (!m_pRenderTarget)
        {
            CRect rect;
            GetClientRect(&rect);

            D2D1_RENDER_TARGET_PROPERTIES rtProps = D2D1::RenderTargetProperties();
            D2D1_HWND_RENDER_TARGET_PROPERTIES hwndProps = D2D1::HwndRenderTargetProperties(
                m_hWnd, D2D1::SizeU(rect.Width(), rect.Height()));

            HRESULT hr = s_pD2DFactory->CreateHwndRenderTarget(rtProps, hwndProps, &m_pRenderTarget);
            if (FAILED(hr))
                return FALSE;
        }

        if (!m_pSolidBrush)
        {
            m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black), &m_pSolidBrush);
        }

        if (!m_pTextFormat && s_pDWriteFactory)
        {
            s_pDWriteFactory->CreateTextFormat(
                L"Segoe UI", nullptr, DWRITE_FONT_WEIGHT_NORMAL,
                DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL,
                12.0f, L"ko-kr", &m_pTextFormat);
        }

        m_pRenderTarget->BeginDraw();
        m_isD2DDrawing = TRUE;
        return TRUE;
    }

    void CMNotificationWindow::EndD2DDraw()
    {
        if (m_pRenderTarget && m_isD2DDrawing)
        {
            HRESULT hr = m_pRenderTarget->EndDraw();
            if (hr == D2DERR_RECREATE_TARGET)
            {
                if (m_pSolidBrush)
                {
                    m_pSolidBrush->Release();
                    m_pSolidBrush = nullptr;
                }
                if (m_pTextFormat)
                {
                    m_pTextFormat->Release();
                    m_pTextFormat = nullptr;
                }
                m_pRenderTarget->Release();
                m_pRenderTarget = nullptr;
            }
        }
        m_isD2DDrawing = FALSE;
    }

    D2D1_COLOR_F CMNotificationWindow::ToD2DColor(COLORREF color, float alpha)
    {
        return D2D1::ColorF(
            GetRValue(color) / 255.0f,
            GetGValue(color) / 255.0f,
            GetBValue(color) / 255.0f,
            alpha);
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

        // Direct2D로 렌더링
        if (BeginD2DDraw())
        {
            OnDrawD2D();
            EndD2DDraw();
        }
        else
        {
            // D2D 실패 시 GDI 폴백
            OnDraw(&dc);
        }
    }

    BOOL CMNotificationWindow::OnEraseBkgnd(CDC* pDC)
    {
        return TRUE;
    }

    void CMNotificationWindow::OnDrawD2D()
    {
        if (!m_pRenderTarget || !m_pSolidBrush)
            return;

        CRect rect;
        GetClientRect(&rect);

        const ThemeColors& colors = GetColors();
        COLORREF bgColor = colors.surface.normal;
        int radius = 8;

        // 배경 그리기 - 둥근 사각형
        D2D1_ROUNDED_RECT roundedRect = D2D1::RoundedRect(
            D2D1::RectF((FLOAT)rect.left, (FLOAT)rect.top, (FLOAT)rect.right, (FLOAT)rect.bottom),
            (FLOAT)radius, (FLOAT)radius);

        m_pSolidBrush->SetColor(ToD2DColor(bgColor));
        m_pRenderTarget->FillRoundedRectangle(roundedRect, m_pSolidBrush);

        // 왼쪽 색상 띠
        COLORREF typeColor = GetTypeColor();
        D2D1_ROUNDED_RECT stripRect = D2D1::RoundedRect(
            D2D1::RectF((FLOAT)rect.left, (FLOAT)rect.top, (FLOAT)(rect.left + 4), (FLOAT)rect.bottom),
            (FLOAT)radius, (FLOAT)radius);

        // 클리핑을 위한 레이어 사용
        ID2D1RoundedRectangleGeometry* pClipGeom = nullptr;
        s_pD2DFactory->CreateRoundedRectangleGeometry(roundedRect, &pClipGeom);
        if (pClipGeom)
        {
            m_pRenderTarget->PushLayer(D2D1::LayerParameters(D2D1::InfiniteRect(), pClipGeom), nullptr);

            m_pSolidBrush->SetColor(ToD2DColor(typeColor));
            m_pRenderTarget->FillRectangle(
                D2D1::RectF((FLOAT)rect.left, (FLOAT)rect.top, (FLOAT)(rect.left + 4), (FLOAT)rect.bottom),
                m_pSolidBrush);

            m_pRenderTarget->PopLayer();
            pClipGeom->Release();
        }

        // 테두리
        m_pSolidBrush->SetColor(ToD2DColor(colors.border.normal));
        m_pRenderTarget->DrawRoundedRectangle(roundedRect, m_pSolidBrush, 1.0f);

        // 아이콘
        CRect iconRect(PADDING + 4, PADDING, PADDING + 4 + ICON_SIZE, PADDING + ICON_SIZE);
        DrawIconD2D(iconRect);

        // 닫기 버튼
        if (m_showClose)
        {
            DrawCloseButtonD2D(GetCloseButtonRect());
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
        if (!m_title.IsEmpty() && m_pTextFormat)
        {
            IDWriteTextFormat* pTitleFormat = nullptr;
            s_pDWriteFactory->CreateTextFormat(
                L"Segoe UI", nullptr, DWRITE_FONT_WEIGHT_SEMI_BOLD,
                DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL,
                13.0f, L"ko-kr", &pTitleFormat);

            if (pTitleFormat)
            {
                m_pSolidBrush->SetColor(ToD2DColor(colors.text));
                D2D1_RECT_F titleRect = D2D1::RectF(
                    (FLOAT)contentRect.left, (FLOAT)contentRect.top,
                    (FLOAT)contentRect.right, (FLOAT)(contentRect.top + 18));
                m_pRenderTarget->DrawText(m_title, m_title.GetLength(), pTitleFormat, titleRect, m_pSolidBrush);
                pTitleFormat->Release();
            }
            contentRect.top += 20;
        }

        // 메시지
        if (!m_message.IsEmpty() && m_pTextFormat)
        {
            m_pSolidBrush->SetColor(ToD2DColor(colors.textSecondary));
            D2D1_RECT_F msgRect = D2D1::RectF(
                (FLOAT)contentRect.left, (FLOAT)contentRect.top,
                (FLOAT)contentRect.right, (FLOAT)contentRect.bottom);
            m_pRenderTarget->DrawText(m_message, m_message.GetLength(), m_pTextFormat, msgRect, m_pSolidBrush);
        }
    }

    void CMNotificationWindow::DrawIconD2D(const CRect& rect)
    {
        if (!m_pRenderTarget || !m_pSolidBrush)
            return;

        COLORREF color = GetTypeColor();
        FLOAT cx = (FLOAT)rect.CenterPoint().x;
        FLOAT cy = (FLOAT)rect.CenterPoint().y;

        m_pSolidBrush->SetColor(ToD2DColor(color));

        // 스트로크 스타일 생성 (둥근 끝)
        ID2D1StrokeStyle* pStrokeStyle = nullptr;
        D2D1_STROKE_STYLE_PROPERTIES strokeProps = D2D1::StrokeStyleProperties(
            D2D1_CAP_STYLE_ROUND, D2D1_CAP_STYLE_ROUND, D2D1_CAP_STYLE_ROUND,
            D2D1_LINE_JOIN_ROUND, 10.0f, D2D1_DASH_STYLE_SOLID, 0.0f);
        s_pD2DFactory->CreateStrokeStyle(strokeProps, nullptr, 0, &pStrokeStyle);

        switch (m_type)
        {
        case NotificationType::Info:
            // i 아이콘 (원 + i)
            m_pRenderTarget->DrawEllipse(D2D1::Ellipse(D2D1::Point2F(cx, cy), 10.0f, 10.0f), m_pSolidBrush, 2.0f, pStrokeStyle);
            m_pRenderTarget->DrawLine(D2D1::Point2F(cx, cy - 4), D2D1::Point2F(cx, cy + 5), m_pSolidBrush, 2.0f, pStrokeStyle);
            m_pRenderTarget->FillEllipse(D2D1::Ellipse(D2D1::Point2F(cx, cy - 8), 2.0f, 2.0f), m_pSolidBrush);
            break;

        case NotificationType::Success:
            // 체크마크
            m_pRenderTarget->DrawEllipse(D2D1::Ellipse(D2D1::Point2F(cx, cy), 10.0f, 10.0f), m_pSolidBrush, 2.0f, pStrokeStyle);
            {
                ID2D1PathGeometry* pPath = nullptr;
                s_pD2DFactory->CreatePathGeometry(&pPath);
                if (pPath)
                {
                    ID2D1GeometrySink* pSink = nullptr;
                    pPath->Open(&pSink);
                    if (pSink)
                    {
                        pSink->BeginFigure(D2D1::Point2F(cx - 5, cy), D2D1_FIGURE_BEGIN_HOLLOW);
                        pSink->AddLine(D2D1::Point2F(cx - 1, cy + 4));
                        pSink->AddLine(D2D1::Point2F(cx + 6, cy - 4));
                        pSink->EndFigure(D2D1_FIGURE_END_OPEN);
                        pSink->Close();
                        pSink->Release();
                    }
                    m_pRenderTarget->DrawGeometry(pPath, m_pSolidBrush, 2.0f, pStrokeStyle);
                    pPath->Release();
                }
            }
            break;

        case NotificationType::Warning:
            // 삼각형 + !
            {
                ID2D1PathGeometry* pPath = nullptr;
                s_pD2DFactory->CreatePathGeometry(&pPath);
                if (pPath)
                {
                    ID2D1GeometrySink* pSink = nullptr;
                    pPath->Open(&pSink);
                    if (pSink)
                    {
                        pSink->BeginFigure(D2D1::Point2F(cx, cy - 10), D2D1_FIGURE_BEGIN_HOLLOW);
                        pSink->AddLine(D2D1::Point2F(cx - 10, cy + 8));
                        pSink->AddLine(D2D1::Point2F(cx + 10, cy + 8));
                        pSink->EndFigure(D2D1_FIGURE_END_CLOSED);
                        pSink->Close();
                        pSink->Release();
                    }
                    m_pRenderTarget->DrawGeometry(pPath, m_pSolidBrush, 2.0f, pStrokeStyle);
                    pPath->Release();
                }
                m_pRenderTarget->DrawLine(D2D1::Point2F(cx, cy - 3), D2D1::Point2F(cx, cy + 2), m_pSolidBrush, 2.0f, pStrokeStyle);
                m_pRenderTarget->FillEllipse(D2D1::Ellipse(D2D1::Point2F(cx, cy + 5), 2.0f, 2.0f), m_pSolidBrush);
            }
            break;

        case NotificationType::Error:
            // X 아이콘
            m_pRenderTarget->DrawEllipse(D2D1::Ellipse(D2D1::Point2F(cx, cy), 10.0f, 10.0f), m_pSolidBrush, 2.0f, pStrokeStyle);
            m_pRenderTarget->DrawLine(D2D1::Point2F(cx - 5, cy - 5), D2D1::Point2F(cx + 5, cy + 5), m_pSolidBrush, 2.0f, pStrokeStyle);
            m_pRenderTarget->DrawLine(D2D1::Point2F(cx + 5, cy - 5), D2D1::Point2F(cx - 5, cy + 5), m_pSolidBrush, 2.0f, pStrokeStyle);
            break;
        }

        if (pStrokeStyle)
            pStrokeStyle->Release();
    }

    void CMNotificationWindow::DrawCloseButtonD2D(const CRect& rect)
    {
        if (!m_pRenderTarget || !m_pSolidBrush)
            return;

        const ThemeColors& colors = GetColors();

        // 호버 배경 (둥근 사각형)
        if (m_closeHovered)
        {
            D2D1_ROUNDED_RECT roundedRect = D2D1::RoundedRect(
                D2D1::RectF((FLOAT)rect.left, (FLOAT)rect.top, (FLOAT)rect.right, (FLOAT)rect.bottom),
                4.0f, 4.0f);
            m_pSolidBrush->SetColor(ToD2DColor(colors.surface.hover));
            m_pRenderTarget->FillRoundedRectangle(roundedRect, m_pSolidBrush);
        }

        // X 아이콘
        FLOAT cx = (FLOAT)rect.CenterPoint().x;
        FLOAT cy = (FLOAT)rect.CenterPoint().y;
        FLOAT size = 4.0f;

        COLORREF color = m_closeHovered ? colors.text : colors.textSecondary;
        m_pSolidBrush->SetColor(ToD2DColor(color));

        ID2D1StrokeStyle* pStrokeStyle = nullptr;
        D2D1_STROKE_STYLE_PROPERTIES strokeProps = D2D1::StrokeStyleProperties(
            D2D1_CAP_STYLE_ROUND, D2D1_CAP_STYLE_ROUND, D2D1_CAP_STYLE_ROUND,
            D2D1_LINE_JOIN_ROUND, 10.0f, D2D1_DASH_STYLE_SOLID, 0.0f);
        s_pD2DFactory->CreateStrokeStyle(strokeProps, nullptr, 0, &pStrokeStyle);

        m_pRenderTarget->DrawLine(D2D1::Point2F(cx - size, cy - size), D2D1::Point2F(cx + size, cy + size), m_pSolidBrush, 2.0f, pStrokeStyle);
        m_pRenderTarget->DrawLine(D2D1::Point2F(cx + size, cy - size), D2D1::Point2F(cx - size, cy + size), m_pSolidBrush, 2.0f, pStrokeStyle);

        if (pStrokeStyle)
            pStrokeStyle->Release();
    }

    void CMNotificationWindow::OnDraw(CDC* pDC)
    {
        // GDI 폴백 - 기본 렌더링
        CRect rect;
        GetClientRect(&rect);

        const ThemeColors& colors = GetColors();
        pDC->FillSolidRect(rect, colors.surface.normal);

        // 왼쪽 색상 띠
        COLORREF typeColor = GetTypeColor();
        pDC->FillSolidRect(rect.left, rect.top, 4, rect.Height(), typeColor);

        // 테두리
        CBrush borderBrush(colors.border.normal);
        pDC->FrameRect(rect, &borderBrush);
    }

    void CMNotificationWindow::DrawIcon(CDC* pDC, const CRect& rect)
    {
        // GDI 폴백 - 기본 아이콘
    }

    void CMNotificationWindow::DrawCloseButton(CDC* pDC, const CRect& rect)
    {
        // GDI 폴백 - 기본 닫기 버튼
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
