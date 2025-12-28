#include "pch.h"
#include "CMModal.h"

#define WM_MODAL_BUTTON_CLICKED (WM_USER + 100)

namespace MFCModernUI
{
    // CMModalOverlay 구현
    BEGIN_MESSAGE_MAP(CMModalOverlay, CWnd)
        ON_WM_PAINT()
        ON_WM_ERASEBKGND()
    END_MESSAGE_MAP()

    CMModalOverlay::CMModalOverlay()
    {
    }

    CMModalOverlay::~CMModalOverlay()
    {
    }

    BOOL CMModalOverlay::Create(CWnd* pParentWnd)
    {
        CString className = AfxRegisterWndClass(
            CS_HREDRAW | CS_VREDRAW,
            LoadCursor(nullptr, IDC_ARROW),
            nullptr,
            nullptr
        );

        CRect parentRect;
        if (pParentWnd)
        {
            pParentWnd->GetWindowRect(&parentRect);
        }
        else
        {
            SystemParametersInfo(SPI_GETWORKAREA, 0, &parentRect, 0);
        }

        return CWnd::CreateEx(
            WS_EX_TOOLWINDOW | WS_EX_TOPMOST,
            className,
            nullptr,
            WS_POPUP,
            parentRect.left, parentRect.top,
            parentRect.Width(), parentRect.Height(),
            pParentWnd ? pParentWnd->GetSafeHwnd() : nullptr,
            nullptr
        );
    }

    void CMModalOverlay::OnPaint()
    {
        CPaintDC dc(this);

        CRect rect;
        GetClientRect(&rect);

        // 반투명 검정 배경
        dc.FillSolidRect(rect, RGB(0, 0, 0));
    }

    BOOL CMModalOverlay::OnEraseBkgnd(CDC* pDC)
    {
        return TRUE;
    }

    // CMModal 구현
    IMPLEMENT_DYNAMIC(CMModal, CWnd)

    BEGIN_MESSAGE_MAP(CMModal, CWnd)
        ON_WM_PAINT()
        ON_WM_ERASEBKGND()
        ON_WM_SIZE()
        ON_WM_KEYDOWN()
        ON_MESSAGE(WM_MODAL_BUTTON_CLICKED, OnButtonClicked)
    END_MESSAGE_MAP()

    CMModal::CMModal()
        : m_icon(ModalIcon::None)
        , m_buttonSet(ModalButtons::OK)
        , m_width(DEFAULT_WIDTH)
        , m_height(DEFAULT_HEIGHT)
        , m_result(ModalResult::None)
        , m_isModal(FALSE)
        , m_pOverlay(nullptr)
        , m_resultHandler(nullptr)
        , m_resultContext(nullptr)
    {
    }

    CMModal::~CMModal()
    {
        // 버튼 삭제
        for (INT_PTR i = 0; i < m_buttons.GetSize(); i++)
        {
            if (m_buttons[i].pButton)
            {
                m_buttons[i].pButton->DestroyWindow();
                delete m_buttons[i].pButton;
            }
        }
        m_buttons.RemoveAll();

        HideOverlay();
    }

    BOOL CMModal::Create(CWnd* pParentWnd)
    {
        CString className = AfxRegisterWndClass(
            CS_HREDRAW | CS_VREDRAW | CS_DROPSHADOW,
            LoadCursor(nullptr, IDC_ARROW),
            nullptr,
            nullptr
        );

        BOOL result = CWnd::CreateEx(
            WS_EX_TOOLWINDOW | WS_EX_TOPMOST,
            className,
            nullptr,
            WS_POPUP,
            0, 0, m_width, m_height,
            pParentWnd ? pParentWnd->GetSafeHwnd() : nullptr,
            nullptr
        );

        if (result)
        {
            CreateButtons();
        }

        return result;
    }

    ModalResult CMModal::DoModal(CWnd* pParentWnd)
    {
        m_isModal = TRUE;
        m_result = ModalResult::None;

        if (!Create(pParentWnd))
            return ModalResult::Cancel;

        ShowOverlay(pParentWnd);
        CenterWindow(pParentWnd);
        ShowWindow(SW_SHOW);
        SetFocus();

        // 메시지 루프
        MSG msg;
        while (m_result == ModalResult::None && GetMessage(&msg, nullptr, 0, 0))
        {
            if (msg.message == WM_QUIT)
            {
                PostQuitMessage((int)msg.wParam);
                break;
            }

            if (!IsDialogMessage(&msg))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }

        HideOverlay();
        DestroyWindow();

        return m_result;
    }

    void CMModal::EndModal(ModalResult result)
    {
        m_result = result;

        if (m_resultHandler)
        {
            m_resultHandler(m_resultContext, result);
        }

        if (!m_isModal)
        {
            HideOverlay();
            DestroyWindow();
        }
    }

    void CMModal::SetTitle(LPCTSTR title)
    {
        m_title = title;
        if (GetSafeHwnd())
            Invalidate();
    }

    void CMModal::SetMessage(LPCTSTR message)
    {
        m_message = message;
        if (GetSafeHwnd())
            Invalidate();
    }

    void CMModal::SetIcon(ModalIcon icon)
    {
        m_icon = icon;
        if (GetSafeHwnd())
            Invalidate();
    }

    void CMModal::SetButtons(ModalButtons buttons)
    {
        m_buttonSet = buttons;
    }

    void CMModal::SetCustomButton(int index, LPCTSTR text, ModalResult result)
    {
        if (index >= 0 && index < m_buttons.GetSize())
        {
            m_buttons[index].text = text;
            m_buttons[index].result = result;

            if (m_buttons[index].pButton)
            {
                m_buttons[index].pButton->SetText(text);
            }
        }
    }

    void CMModal::SetSize(int width, int height)
    {
        m_width = width;
        m_height = height;

        if (GetSafeHwnd())
        {
            SetWindowPos(nullptr, 0, 0, width, height, SWP_NOMOVE | SWP_NOZORDER);
        }
    }

    CRect CMModal::GetContentRect()
    {
        CRect rect;
        GetClientRect(&rect);

        rect.DeflateRect(PADDING, PADDING);

        // 제목 영역 제외
        if (!m_title.IsEmpty())
        {
            rect.top += TITLE_HEIGHT + 8;
        }

        // 버튼 영역 제외
        rect.bottom -= BUTTON_HEIGHT + PADDING;

        // 아이콘 영역 제외
        if (m_icon != ModalIcon::None)
        {
            rect.left += ICON_SIZE + 16;
        }

        return rect;
    }

    void CMModal::SetResultHandler(ModalResultHandler handler, void* context)
    {
        m_resultHandler = handler;
        m_resultContext = context;
    }

    ModalResult CMModal::ShowMessage(LPCTSTR message, LPCTSTR title,
        ModalButtons buttons, ModalIcon icon)
    {
        CMModal modal;
        modal.SetTitle(title ? title : _T("알림"));
        modal.SetMessage(message);
        modal.SetButtons(buttons);
        modal.SetIcon(icon);
        return modal.DoModal();
    }

    ModalResult CMModal::ShowQuestion(LPCTSTR message, LPCTSTR title)
    {
        return ShowMessage(message, title ? title : _T("확인"),
            ModalButtons::YesNo, ModalIcon::Question);
    }

    ModalResult CMModal::ShowWarning(LPCTSTR message, LPCTSTR title)
    {
        return ShowMessage(message, title ? title : _T("경고"),
            ModalButtons::OK, ModalIcon::Warning);
    }

    ModalResult CMModal::ShowError(LPCTSTR message, LPCTSTR title)
    {
        return ShowMessage(message, title ? title : _T("오류"),
            ModalButtons::OK, ModalIcon::Error);
    }

    void CMModal::CreateButtons()
    {
        // 기존 버튼 삭제
        for (INT_PTR i = 0; i < m_buttons.GetSize(); i++)
        {
            if (m_buttons[i].pButton)
            {
                m_buttons[i].pButton->DestroyWindow();
                delete m_buttons[i].pButton;
            }
        }
        m_buttons.RemoveAll();

        // 버튼 정의
        switch (m_buttonSet)
        {
        case ModalButtons::OK:
        {
            ButtonInfo btn = { nullptr, _T("확인"), ModalResult::OK, TRUE, FALSE };
            m_buttons.Add(btn);
            break;
        }
        case ModalButtons::OKCancel:
        {
            ButtonInfo btn1 = { nullptr, _T("확인"), ModalResult::OK, TRUE, FALSE };
            ButtonInfo btn2 = { nullptr, _T("취소"), ModalResult::Cancel, FALSE, TRUE };
            m_buttons.Add(btn1);
            m_buttons.Add(btn2);
            break;
        }
        case ModalButtons::YesNo:
        {
            ButtonInfo btn1 = { nullptr, _T("예"), ModalResult::Yes, TRUE, FALSE };
            ButtonInfo btn2 = { nullptr, _T("아니오"), ModalResult::No, FALSE, TRUE };
            m_buttons.Add(btn1);
            m_buttons.Add(btn2);
            break;
        }
        case ModalButtons::YesNoCancel:
        {
            ButtonInfo btn1 = { nullptr, _T("예"), ModalResult::Yes, TRUE, FALSE };
            ButtonInfo btn2 = { nullptr, _T("아니오"), ModalResult::No, FALSE, FALSE };
            ButtonInfo btn3 = { nullptr, _T("취소"), ModalResult::Cancel, FALSE, TRUE };
            m_buttons.Add(btn1);
            m_buttons.Add(btn2);
            m_buttons.Add(btn3);
            break;
        }
        case ModalButtons::Custom:
        default:
            break;
        }

        // 버튼 생성
        for (INT_PTR i = 0; i < m_buttons.GetSize(); i++)
        {
            CMButton* pButton = new CMButton();

            CRect btnRect(0, 0, BUTTON_WIDTH, BUTTON_HEIGHT);
            pButton->Create(WS_CHILD | WS_VISIBLE | WS_TABSTOP, btnRect, this, 1000 + (UINT)i);
            pButton->SetText(m_buttons[i].text);

            if (m_buttons[i].isDefault)
            {
                pButton->SetStyle(ButtonStyle::Primary);
            }
            else
            {
                pButton->SetStyle(ButtonStyle::Outline);
            }

            m_buttons[i].pButton = pButton;
        }

        LayoutButtons();
    }

    void CMModal::LayoutButtons()
    {
        CRect rect;
        GetClientRect(&rect);

        int totalWidth = (int)m_buttons.GetSize() * BUTTON_WIDTH +
            ((int)m_buttons.GetSize() - 1) * BUTTON_SPACING;
        int startX = (rect.Width() - totalWidth) / 2;
        int y = rect.bottom - PADDING - BUTTON_HEIGHT;

        for (INT_PTR i = 0; i < m_buttons.GetSize(); i++)
        {
            if (m_buttons[i].pButton)
            {
                int x = startX + (int)i * (BUTTON_WIDTH + BUTTON_SPACING);
                m_buttons[i].pButton->SetWindowPos(nullptr, x, y,
                    BUTTON_WIDTH, BUTTON_HEIGHT, SWP_NOZORDER);
            }
        }
    }

    void CMModal::OnPaint()
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

    BOOL CMModal::OnEraseBkgnd(CDC* pDC)
    {
        return TRUE;
    }

    void CMModal::OnDraw(CDC* pDC)
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
        pDC->RoundRect(rect, CPoint(8, 8));
        pDC->SelectObject(oldPen);
        pDC->SelectObject(oldBrush);

        // 내용 영역
        CRect contentRect = rect;
        contentRect.DeflateRect(PADDING, PADDING);

        // 제목
        if (!m_title.IsEmpty())
        {
            CRect titleRect = contentRect;
            titleRect.bottom = titleRect.top + TITLE_HEIGHT;
            DrawTitle(pDC, titleRect);
            contentRect.top = titleRect.bottom + 8;
        }

        // 버튼 영역 제외
        contentRect.bottom -= BUTTON_HEIGHT + PADDING;

        // 아이콘
        if (m_icon != ModalIcon::None)
        {
            CRect iconRect = contentRect;
            iconRect.right = iconRect.left + ICON_SIZE;
            iconRect.bottom = iconRect.top + ICON_SIZE;
            DrawIcon(pDC, iconRect);
            contentRect.left = iconRect.right + 16;
        }

        // 메시지
        if (!m_message.IsEmpty())
        {
            DrawMessage(pDC, contentRect);
        }
    }

    void CMModal::DrawIcon(CDC* pDC, const CRect& rect)
    {
        COLORREF color = GetIconColor();
        int cx = rect.CenterPoint().x;
        int cy = rect.CenterPoint().y;
        int size = ICON_SIZE / 2 - 4;

        CPen pen(PS_SOLID, 3, color);
        CPen* oldPen = pDC->SelectObject(&pen);
        CBrush* oldBrush = (CBrush*)pDC->SelectStockObject(NULL_BRUSH);

        switch (m_icon)
        {
        case ModalIcon::Info:
            // i 아이콘
            pDC->Ellipse(cx - size, cy - size, cx + size, cy + size);
            pDC->MoveTo(cx, cy - 6);
            pDC->LineTo(cx, cy + 8);
            pDC->Ellipse(cx - 2, cy - 12, cx + 2, cy - 8);
            break;

        case ModalIcon::Question:
            // ? 아이콘
            pDC->Ellipse(cx - size, cy - size, cx + size, cy + size);
            // 물음표 그리기 (간단하게)
            {
                CFont* pFont = CMThemeManager::GetInstance().GetFont(TextStyle::H3);
                CFont* oldFont = pDC->SelectObject(pFont);
                pDC->SetBkMode(TRANSPARENT);
                pDC->SetTextColor(color);
                pDC->DrawText(_T("?"), rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
                pDC->SelectObject(oldFont);
            }
            break;

        case ModalIcon::Warning:
            // 삼각형 + !
            {
                POINT points[3] = {
                    { cx, cy - size },
                    { cx - size, cy + size - 4 },
                    { cx + size, cy + size - 4 }
                };
                pDC->Polygon(points, 3);
                pDC->MoveTo(cx, cy - 4);
                pDC->LineTo(cx, cy + 6);
                pDC->Ellipse(cx - 2, cy + 8, cx + 2, cy + 12);
            }
            break;

        case ModalIcon::Error:
            // X 아이콘
            pDC->Ellipse(cx - size, cy - size, cx + size, cy + size);
            pDC->MoveTo(cx - 8, cy - 8);
            pDC->LineTo(cx + 8, cy + 8);
            pDC->MoveTo(cx + 8, cy - 8);
            pDC->LineTo(cx - 8, cy + 8);
            break;
        }

        pDC->SelectObject(oldPen);
        pDC->SelectObject(oldBrush);
    }

    void CMModal::DrawTitle(CDC* pDC, const CRect& rect)
    {
        const ThemeColors& colors = GetColors();

        CFont* pFont = CMThemeManager::GetInstance().GetFont(TextStyle::H4);
        CFont* oldFont = pDC->SelectObject(pFont);
        pDC->SetBkMode(TRANSPARENT);
        pDC->SetTextColor(colors.text);
        pDC->DrawText(m_title, const_cast<CRect*>(&rect),
            DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
        pDC->SelectObject(oldFont);
    }

    void CMModal::DrawMessage(CDC* pDC, const CRect& rect)
    {
        const ThemeColors& colors = GetColors();

        CFont* pFont = CMThemeManager::GetInstance().GetFont(TextStyle::Body);
        CFont* oldFont = pDC->SelectObject(pFont);
        pDC->SetBkMode(TRANSPARENT);
        pDC->SetTextColor(colors.textSecondary);
        pDC->DrawText(m_message, const_cast<CRect*>(&rect), DT_LEFT | DT_WORDBREAK);
        pDC->SelectObject(oldFont);
    }

    COLORREF CMModal::GetIconColor() const
    {
        switch (m_icon)
        {
        case ModalIcon::Info:
            return RGB(33, 150, 243);   // 파랑
        case ModalIcon::Question:
            return RGB(103, 58, 183);   // 보라
        case ModalIcon::Warning:
            return RGB(255, 152, 0);    // 주황
        case ModalIcon::Error:
            return RGB(244, 67, 54);    // 빨강
        default:
            return RGB(128, 128, 128);
        }
    }

    const ThemeColors& CMModal::GetColors() const
    {
        return CMThemeManager::GetInstance().GetColors();
    }

    void CMModal::CenterWindow(CWnd* pParent)
    {
        CRect parentRect;
        if (pParent)
        {
            pParent->GetWindowRect(&parentRect);
        }
        else
        {
            SystemParametersInfo(SPI_GETWORKAREA, 0, &parentRect, 0);
        }

        CRect rect;
        GetWindowRect(&rect);

        int x = parentRect.left + (parentRect.Width() - rect.Width()) / 2;
        int y = parentRect.top + (parentRect.Height() - rect.Height()) / 2;

        SetWindowPos(nullptr, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
    }

    void CMModal::ShowOverlay(CWnd* pParent)
    {
        m_pOverlay = new CMModalOverlay();
        if (m_pOverlay->Create(pParent))
        {
            // 알파 블렌딩 적용
            SetLayeredWindowAttributes(*m_pOverlay, 0, 128, LWA_ALPHA);
            m_pOverlay->ModifyStyleEx(0, WS_EX_LAYERED);
            m_pOverlay->SetLayeredWindowAttributes(0, 128, LWA_ALPHA);
            m_pOverlay->ShowWindow(SW_SHOW);
        }
    }

    void CMModal::HideOverlay()
    {
        if (m_pOverlay)
        {
            m_pOverlay->DestroyWindow();
            delete m_pOverlay;
            m_pOverlay = nullptr;
        }
    }

    void CMModal::OnSize(UINT nType, int cx, int cy)
    {
        CWnd::OnSize(nType, cx, cy);
        LayoutButtons();
    }

    void CMModal::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
    {
        if (nChar == VK_ESCAPE)
        {
            // ESC로 취소
            for (INT_PTR i = 0; i < m_buttons.GetSize(); i++)
            {
                if (m_buttons[i].isCancel)
                {
                    EndModal(m_buttons[i].result);
                    return;
                }
            }
            EndModal(ModalResult::Cancel);
        }
        else if (nChar == VK_RETURN)
        {
            // Enter로 기본 버튼
            for (INT_PTR i = 0; i < m_buttons.GetSize(); i++)
            {
                if (m_buttons[i].isDefault)
                {
                    EndModal(m_buttons[i].result);
                    return;
                }
            }
        }

        CWnd::OnKeyDown(nChar, nRepCnt, nFlags);
    }

    LRESULT CMModal::OnButtonClicked(WPARAM wParam, LPARAM lParam)
    {
        int index = (int)wParam - 1000;
        if (index >= 0 && index < m_buttons.GetSize())
        {
            EndModal(m_buttons[index].result);
        }
        return 0;
    }
}
