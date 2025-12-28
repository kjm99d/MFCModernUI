#include "pch.h"
#include "CMTooltip.h"
#include "CMThemeManager.h"

namespace MFCModernUI
{
    IMPLEMENT_DYNAMIC(CMTooltip, CWnd)

    BEGIN_MESSAGE_MAP(CMTooltip, CWnd)
        ON_WM_PAINT()
        ON_WM_TIMER()
        ON_WM_ERASEBKGND()
    END_MESSAGE_MAP()

    CMTooltip::CMTooltip()
        : m_position(TooltipPosition::Auto)
        , m_showDelay(500)
        , m_hideDelay(100)
        , m_maxWidth(300)
        , m_visible(FALSE)
        , m_currentTool(nullptr)
        , m_showTimerId(0)
        , m_hideTimerId(0)
    {
    }

    CMTooltip::~CMTooltip()
    {
        StopTimers();
    }

    BOOL CMTooltip::Create(CWnd* pParentWnd)
    {
        CString className = AfxRegisterWndClass(
            CS_HREDRAW | CS_VREDRAW | CS_DROPSHADOW,
            LoadCursor(nullptr, IDC_ARROW),
            nullptr,
            nullptr
        );

        // WS_POPUP으로 생성 (부모 위에 떠야 함)
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

    void CMTooltip::AddTool(CWnd* pWnd, LPCTSTR lpszText)
    {
        if (!pWnd || !lpszText)
            return;

        // 이미 등록된 경우 업데이트
        ToolInfo* existing = FindTool(pWnd);
        if (existing)
        {
            existing->text = lpszText;
            return;
        }

        ToolInfo info;
        info.pWnd = pWnd;
        info.text = lpszText;
        m_tools.Add(info);
    }

    void CMTooltip::AddTool(CWnd* pWnd, UINT nIDText)
    {
        CString text;
        text.LoadString(nIDText);
        AddTool(pWnd, text);
    }

    void CMTooltip::DelTool(CWnd* pWnd)
    {
        for (INT_PTR i = 0; i < m_tools.GetSize(); i++)
        {
            if (m_tools[i].pWnd == pWnd)
            {
                m_tools.RemoveAt(i);

                if (m_currentTool == pWnd)
                {
                    Hide();
                }
                break;
            }
        }
    }

    void CMTooltip::UpdateTipText(CWnd* pWnd, LPCTSTR lpszText)
    {
        ToolInfo* tool = FindTool(pWnd);
        if (tool)
        {
            tool->text = lpszText;

            // 현재 표시 중인 툴팁이면 업데이트
            if (m_currentTool == pWnd && m_visible)
            {
                m_currentText = lpszText;
                Invalidate();
            }
        }
    }

    void CMTooltip::SetPosition(TooltipPosition position)
    {
        m_position = position;
    }

    void CMTooltip::SetDelay(DWORD showDelay, DWORD hideDelay)
    {
        m_showDelay = showDelay;
        m_hideDelay = hideDelay > 0 ? hideDelay : showDelay / 5;
    }

    void CMTooltip::SetMaxWidth(int maxWidth)
    {
        m_maxWidth = maxWidth;
    }

    void CMTooltip::Show(CWnd* pWnd, CPoint point)
    {
        ToolInfo* tool = FindTool(pWnd);
        if (!tool || tool->text.IsEmpty())
            return;

        m_currentTool = pWnd;
        m_currentText = tool->text;
        m_mousePos = point;

        // 크기 계산
        CRect tooltipRect = CalculateTooltipRect(m_currentText, point);

        // 위치 조정
        CPoint pos = CalculatePosition(tooltipRect, point);

        // 창 위치/크기 설정
        SetWindowPos(&wndTopMost, pos.x, pos.y,
            tooltipRect.Width(), tooltipRect.Height(),
            SWP_SHOWWINDOW | SWP_NOACTIVATE);

        m_visible = TRUE;
        Invalidate();
    }

    void CMTooltip::Hide()
    {
        if (m_visible)
        {
            ShowWindow(SW_HIDE);
            m_visible = FALSE;
            m_currentTool = nullptr;
        }
        StopTimers();
    }

    void CMTooltip::RelayEvent(MSG* pMsg)
    {
        if (!pMsg)
            return;

        switch (pMsg->message)
        {
        case WM_MOUSEMOVE:
        {
            // 마우스 아래의 창 찾기
            CPoint pt(pMsg->pt);
            CWnd* pWnd = CWnd::WindowFromPoint(pt);

            // 툴팁 대상인지 확인
            ToolInfo* tool = FindTool(pWnd);

            if (tool)
            {
                if (m_currentTool != pWnd)
                {
                    // 새 대상으로 이동
                    Hide();
                    m_currentTool = pWnd;
                    m_mousePos = pt;
                    StartShowTimer();
                }
            }
            else
            {
                // 대상 아님 - 숨김
                if (m_visible)
                {
                    StartHideTimer();
                }
                else
                {
                    StopTimers();
                    m_currentTool = nullptr;
                }
            }
            break;
        }

        case WM_LBUTTONDOWN:
        case WM_RBUTTONDOWN:
        case WM_MBUTTONDOWN:
        case WM_KEYDOWN:
            // 클릭/키 입력 시 즉시 숨김
            Hide();
            break;
        }
    }

    void CMTooltip::OnPaint()
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

    void CMTooltip::OnDraw(CDC* pDC)
    {
        CRect rect;
        GetClientRect(&rect);

        const ThemeColors& colors = GetColors();

        // 배경색 (반전 - 다크모드에서 밝게, 라이트모드에서 어둡게)
        COLORREF bgColor = IsLightTheme() ? RGB(50, 50, 50) : RGB(250, 250, 250);
        COLORREF textColor = IsLightTheme() ? RGB(255, 255, 255) : RGB(20, 20, 20);
        COLORREF borderColor = IsLightTheme() ? RGB(30, 30, 30) : RGB(200, 200, 200);

        // 배경
        pDC->FillSolidRect(rect, bgColor);

        // 테두리
        CPen pen(PS_SOLID, 1, borderColor);
        CPen* oldPen = pDC->SelectObject(&pen);
        CBrush* oldBrush = (CBrush*)pDC->SelectStockObject(NULL_BRUSH);
        pDC->RoundRect(rect, CPoint(4, 4));
        pDC->SelectObject(oldPen);
        pDC->SelectObject(oldBrush);

        // 텍스트
        rect.DeflateRect(8, 6);

        CFont* pFont = CMThemeManager::GetInstance().GetFont(TextStyle::Caption);
        CFont* oldFont = pDC->SelectObject(pFont);
        pDC->SetBkMode(TRANSPARENT);
        pDC->SetTextColor(textColor);
        pDC->DrawText(m_currentText, rect, DT_LEFT | DT_WORDBREAK);
        pDC->SelectObject(oldFont);
    }

    BOOL CMTooltip::OnEraseBkgnd(CDC* pDC)
    {
        return TRUE;  // OnPaint에서 처리
    }

    void CMTooltip::OnTimer(UINT_PTR nIDEvent)
    {
        if (nIDEvent == SHOW_TIMER_ID)
        {
            KillTimer(m_showTimerId);
            m_showTimerId = 0;

            if (m_currentTool && FindTool(m_currentTool))
            {
                Show(m_currentTool, m_mousePos);
            }
        }
        else if (nIDEvent == HIDE_TIMER_ID)
        {
            KillTimer(m_hideTimerId);
            m_hideTimerId = 0;
            Hide();
        }

        CWnd::OnTimer(nIDEvent);
    }

    void CMTooltip::PreSubclassWindow()
    {
        CWnd::PreSubclassWindow();
    }

    BOOL CMTooltip::IsLightTheme() const
    {
        return CMThemeManager::GetInstance().GetTheme() == ThemeType::Light;
    }

    const ThemeColors& CMTooltip::GetColors() const
    {
        return CMThemeManager::GetInstance().GetColors();
    }

    CMTooltip::ToolInfo* CMTooltip::FindTool(CWnd* pWnd)
    {
        for (INT_PTR i = 0; i < m_tools.GetSize(); i++)
        {
            if (m_tools[i].pWnd == pWnd)
            {
                return &m_tools[i];
            }
        }
        return nullptr;
    }

    CRect CMTooltip::CalculateTooltipRect(const CString& text, CPoint anchor)
    {
        CDC* pDC = GetDC();
        if (!pDC)
            return CRect(0, 0, 100, 24);

        CFont* pFont = CMThemeManager::GetInstance().GetFont(TextStyle::Caption);
        CFont* oldFont = pDC->SelectObject(pFont);

        CRect textRect(0, 0, m_maxWidth - 16, 0);
        pDC->DrawText(text, textRect, DT_CALCRECT | DT_WORDBREAK);

        pDC->SelectObject(oldFont);
        ReleaseDC(pDC);

        // 패딩 추가
        int width = textRect.Width() + 16;
        int height = textRect.Height() + 12;

        return CRect(0, 0, width, height);
    }

    CPoint CMTooltip::CalculatePosition(CRect tooltipRect, CPoint anchor)
    {
        // 화면 영역 가져오기
        CRect screenRect;
        SystemParametersInfo(SPI_GETWORKAREA, 0, &screenRect, 0);

        int offsetX = 0;
        int offsetY = 16;  // 마우스 커서 아래

        CPoint pos;
        TooltipPosition actualPos = m_position;

        if (actualPos == TooltipPosition::Auto)
        {
            // 기본은 아래, 공간 없으면 위
            if (anchor.y + offsetY + tooltipRect.Height() > screenRect.bottom)
            {
                actualPos = TooltipPosition::Top;
            }
            else
            {
                actualPos = TooltipPosition::Bottom;
            }
        }

        switch (actualPos)
        {
        case TooltipPosition::Top:
            pos.x = anchor.x - tooltipRect.Width() / 2;
            pos.y = anchor.y - tooltipRect.Height() - 8;
            break;

        case TooltipPosition::Bottom:
            pos.x = anchor.x - tooltipRect.Width() / 2;
            pos.y = anchor.y + offsetY;
            break;

        case TooltipPosition::Left:
            pos.x = anchor.x - tooltipRect.Width() - 8;
            pos.y = anchor.y - tooltipRect.Height() / 2;
            break;

        case TooltipPosition::Right:
            pos.x = anchor.x + 16;
            pos.y = anchor.y - tooltipRect.Height() / 2;
            break;

        default:
            pos.x = anchor.x;
            pos.y = anchor.y + offsetY;
            break;
        }

        // 화면 경계 조정
        if (pos.x < screenRect.left)
            pos.x = screenRect.left;
        if (pos.x + tooltipRect.Width() > screenRect.right)
            pos.x = screenRect.right - tooltipRect.Width();
        if (pos.y < screenRect.top)
            pos.y = screenRect.top;
        if (pos.y + tooltipRect.Height() > screenRect.bottom)
            pos.y = screenRect.bottom - tooltipRect.Height();

        return pos;
    }

    void CMTooltip::StartShowTimer()
    {
        StopTimers();
        m_showTimerId = SetTimer(SHOW_TIMER_ID, m_showDelay, nullptr);
    }

    void CMTooltip::StartHideTimer()
    {
        if (m_hideTimerId == 0)
        {
            m_hideTimerId = SetTimer(HIDE_TIMER_ID, m_hideDelay, nullptr);
        }
    }

    void CMTooltip::StopTimers()
    {
        if (m_showTimerId)
        {
            KillTimer(m_showTimerId);
            m_showTimerId = 0;
        }
        if (m_hideTimerId)
        {
            KillTimer(m_hideTimerId);
            m_hideTimerId = 0;
        }
    }
}
