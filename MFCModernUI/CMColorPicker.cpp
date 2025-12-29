#include "pch.h"
#include "CMColorPicker.h"
#include <math.h>

namespace MFCModernUI
{
    // 기본 팔레트 색상
    const COLORREF CMColorPopup::s_paletteColors[] = {
        // 빨강 계열
        RGB(255, 235, 238), RGB(255, 205, 210), RGB(239, 154, 154), RGB(229, 115, 115), RGB(239, 83, 80),
        RGB(244, 67, 54), RGB(229, 57, 53), RGB(211, 47, 47), RGB(198, 40, 40), RGB(183, 28, 28),
        // 주황 계열
        RGB(255, 243, 224), RGB(255, 224, 178), RGB(255, 204, 128), RGB(255, 183, 77), RGB(255, 167, 38),
        RGB(255, 152, 0), RGB(251, 140, 0), RGB(245, 124, 0), RGB(239, 108, 0), RGB(230, 81, 0),
        // 노랑 계열
        RGB(255, 253, 231), RGB(255, 249, 196), RGB(255, 245, 157), RGB(255, 241, 118), RGB(255, 238, 88),
        RGB(255, 235, 59), RGB(253, 216, 53), RGB(251, 192, 45), RGB(249, 168, 37), RGB(245, 127, 23),
        // 초록 계열
        RGB(232, 245, 233), RGB(200, 230, 201), RGB(165, 214, 167), RGB(129, 199, 132), RGB(102, 187, 106),
        RGB(76, 175, 80), RGB(67, 160, 71), RGB(56, 142, 60), RGB(46, 125, 50), RGB(27, 94, 32),
        // 파랑 계열
        RGB(227, 242, 253), RGB(187, 222, 251), RGB(144, 202, 249), RGB(100, 181, 246), RGB(66, 165, 245),
        RGB(33, 150, 243), RGB(30, 136, 229), RGB(25, 118, 210), RGB(21, 101, 192), RGB(13, 71, 161)
    };

    // CMColorPicker 구현
    IMPLEMENT_DYNAMIC(CMColorPicker, CMControlBase)

    BEGIN_MESSAGE_MAP(CMColorPicker, CMControlBase)
        ON_WM_LBUTTONDOWN()
        ON_WM_KEYDOWN()
        ON_WM_KILLFOCUS()
    END_MESSAGE_MAP()

    CMColorPicker::CMColorPicker()
        : m_color(RGB(0, 0, 0))
        , m_alpha(255)
        , m_alphaEnabled(FALSE)
        , m_popup(nullptr)
        , m_colorChangedHandler(nullptr)
        , m_colorChangedContext(nullptr)
    {
        m_themeClass = _T("colorpicker");
        m_tabStop = TRUE;
    }

    CMColorPicker::~CMColorPicker()
    {
        if (m_popup)
        {
            m_popup->DestroyWindow();
            delete m_popup;
        }
    }

    BOOL CMColorPicker::Create(DWORD dwStyle, const CRect& rect, CWnd* pParentWnd, UINT nID)
    {
        CString className = AfxRegisterWndClass(
            CS_HREDRAW | CS_VREDRAW,
            LoadCursor(nullptr, IDC_ARROW),
            nullptr,
            nullptr
        );

        return CWnd::Create(className, nullptr, dwStyle | WS_CHILD | WS_TABSTOP, rect, pParentWnd, nID);
    }

    void CMColorPicker::SetColor(COLORREF color)
    {
        if (m_color != color)
        {
            COLORREF oldColor = m_color;
            m_color = color;
            NotifyColorChanged(oldColor);
            Invalidate();
        }
    }

    void CMColorPicker::SetAlphaEnabled(BOOL enable)
    {
        m_alphaEnabled = enable;
        Invalidate();
    }

    void CMColorPicker::SetAlpha(BYTE alpha)
    {
        m_alpha = alpha;
        Invalidate();
    }

    void CMColorPicker::SetCustomColors(const COLORREF* colors, int count)
    {
        m_customColors.RemoveAll();
        for (int i = 0; i < count; i++)
        {
            m_customColors.Add(colors[i]);
        }
    }

    void CMColorPicker::AddCustomColor(COLORREF color)
    {
        // 중복 체크
        for (INT_PTR i = 0; i < m_customColors.GetSize(); i++)
        {
            if (m_customColors[i] == color)
                return;
        }

        // 최대 10개까지
        if (m_customColors.GetSize() >= 10)
        {
            m_customColors.RemoveAt(0);
        }

        m_customColors.Add(color);
    }

    void CMColorPicker::SetColorChangedHandler(ColorChangedHandler handler, void* context)
    {
        m_colorChangedHandler = handler;
        m_colorChangedContext = context;
    }

    void CMColorPicker::ShowDropDown()
    {
        if (!m_popup)
        {
            m_popup = new CMColorPopup(this);
            m_popup->Create();
        }

        CRect rect;
        GetWindowRect(&rect);

        m_popup->Show(CPoint(rect.left, rect.bottom));
    }

    void CMColorPicker::HideDropDown()
    {
        if (m_popup && m_popup->IsWindowVisible())
        {
            m_popup->Hide();
        }
    }

    BOOL CMColorPicker::IsDroppedDown() const
    {
        return m_popup && m_popup->IsWindowVisible();
    }

    void CMColorPicker::OnDraw(CDC* pDC)
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

        // 색상 미리보기
        CRect previewRect = rect;
        previewRect.right = buttonRect.left;
        previewRect.DeflateRect(4, 4);
        DrawColorPreview(pDC, previewRect);

        // HEX 코드
        CRect textRect = previewRect;
        textRect.left += 24;

        CString hexText = ColorToHex(m_color);
        COLORREF textColor = m_isEnabled ? colors.text : colors.textDisabled;

        CFont* pFont = CMThemeManager::GetInstance().GetFont(TextStyle::Body);
        CFont* oldFont = pDC->SelectObject(pFont);
        pDC->SetBkMode(TRANSPARENT);
        pDC->SetTextColor(textColor);
        pDC->DrawText(hexText, textRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
        pDC->SelectObject(oldFont);
    }

    void CMColorPicker::DrawColorPreview(CDC* pDC, const CRect& rect)
    {
        // 체커보드 패턴 (투명도 표시용)
        CRect colorRect = rect;
        colorRect.right = colorRect.left + 20;

        // 색상 사각형
        CBrush brush(m_color);
        CPen pen(PS_SOLID, 1, GetColors().border.normal);
        CPen* oldPen = pDC->SelectObject(&pen);
        CBrush* oldBrush = pDC->SelectObject(&brush);
        pDC->RoundRect(colorRect, CPoint(3, 3));
        pDC->SelectObject(oldPen);
        pDC->SelectObject(oldBrush);
    }

    void CMColorPicker::DrawButton(CDC* pDC, const CRect& rect)
    {
        const ThemeColors& colors = GetColors();

        // 배경
        COLORREF bgColor = m_isHovered ? colors.surface.hover : colors.surface.normal;
        pDC->FillSolidRect(rect, bgColor);

        // 아래 화살표
        int cx = rect.CenterPoint().x;
        int cy = rect.CenterPoint().y;
        int size = 4;

        COLORREF arrowColor = m_isEnabled ? colors.textSecondary : colors.textDisabled;

        POINT points[3] = {
            { cx, cy + size },
            { cx - size, cy - size / 2 },
            { cx + size, cy - size / 2 }
        };

        CBrush brush(arrowColor);
        CPen pen(PS_SOLID, 1, arrowColor);
        CPen* oldPen = pDC->SelectObject(&pen);
        CBrush* oldBrush = pDC->SelectObject(&brush);
        pDC->Polygon(points, 3);
        pDC->SelectObject(oldPen);
        pDC->SelectObject(oldBrush);
    }

    CString CMColorPicker::ColorToHex(COLORREF color)
    {
        CString hex;
        hex.Format(_T("#%02X%02X%02X"), GetRValue(color), GetGValue(color), GetBValue(color));
        return hex;
    }

    void CMColorPicker::NotifyColorChanged(COLORREF oldColor)
    {
        if (m_colorChangedHandler)
        {
            m_colorChangedHandler(m_colorChangedContext, oldColor, m_color);
        }
    }

    void CMColorPicker::OnLButtonDown(UINT nFlags, CPoint point)
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

    void CMColorPicker::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
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
        }

        CMControlBase::OnKeyDown(nChar, nRepCnt, nFlags);
    }

    void CMColorPicker::OnKillFocus(CWnd* pNewWnd)
    {
        // 팝업으로 포커스가 이동한 경우는 포커스 상태 유지
        if (m_popup && pNewWnd == m_popup)
        {
            return;
        }

        // 팝업이 열려있고 포커스가 다른 곳으로 이동한 경우
        if (m_popup && m_popup->IsWindowVisible())
        {
            HideDropDown();
        }

        CMControlBase::OnKillFocus(pNewWnd);
    }

    // CMColorPopup 구현
    BEGIN_MESSAGE_MAP(CMColorPopup, CWnd)
        ON_WM_PAINT()
        ON_WM_ERASEBKGND()
        ON_WM_LBUTTONDOWN()
        ON_WM_MOUSEMOVE()
        ON_WM_KILLFOCUS()
        ON_WM_CLOSE()
    END_MESSAGE_MAP()

    CMColorPopup::CMColorPopup(CMColorPicker* pOwner)
        : m_pOwner(pOwner)
        , m_hoverPaletteIndex(-1)
        , m_hoverCustomIndex(-1)
        , m_hoverMoreButton(FALSE)
        , m_hue(0)
        , m_saturation(0)
        , m_value(0)
    {
        m_tempColor = pOwner->GetColor();
    }

    CMColorPopup::~CMColorPopup()
    {
    }

    BOOL CMColorPopup::Create()
    {
        CString className = AfxRegisterWndClass(
            CS_HREDRAW | CS_VREDRAW | CS_DROPSHADOW,
            LoadCursor(nullptr, IDC_ARROW),
            nullptr,
            nullptr
        );

        int width = PALETTE_COLS * CELL_SIZE + PADDING * 2;
        int height = PADDING + PALETTE_ROWS * CELL_SIZE + SECTION_GAP;

        // 사용자 정의 색상 섹션
        if (m_pOwner->m_customColors.GetSize() > 0)
        {
            height += CELL_SIZE + SECTION_GAP;
        }

        // 미리보기 및 더보기 버튼
        height += 32 + PADDING;

        // WS_EX_NOACTIVATE: 팝업이 활성화되어도 포커스를 가져가지 않음
        return CWnd::CreateEx(
            WS_EX_TOOLWINDOW | WS_EX_TOPMOST | WS_EX_NOACTIVATE,
            className,
            nullptr,
            WS_POPUP,
            0, 0, width, height,
            m_pOwner->GetSafeHwnd(),
            nullptr
        );
    }

    void CMColorPopup::Show(CPoint pt)
    {
        m_tempColor = m_pOwner->GetColor();
        RGBToHSV(m_tempColor, m_hue, m_saturation, m_value);
        m_hoverPaletteIndex = -1;
        m_hoverCustomIndex = -1;
        m_hoverMoreButton = FALSE;

        CRect rect;
        GetWindowRect(&rect);

        CRect screenRect;
        SystemParametersInfo(SPI_GETWORKAREA, 0, &screenRect, 0);

        if (pt.x + rect.Width() > screenRect.right)
            pt.x = screenRect.right - rect.Width();
        if (pt.y + rect.Height() > screenRect.bottom)
        {
            CRect ownerRect;
            m_pOwner->GetClientRect(&ownerRect);
            pt.y -= rect.Height() + ownerRect.Height();
        }

        // SWP_NOACTIVATE 플래그를 사용하여 포커스 변경 방지
        SetWindowPos(&wndTopMost, pt.x, pt.y, 0, 0,
            SWP_NOSIZE | SWP_SHOWWINDOW | SWP_NOACTIVATE);

        // 마우스 캡처로 외부 클릭 감지
        SetCapture();
    }

    void CMColorPopup::Hide()
    {
        if (GetCapture() == this)
        {
            ReleaseCapture();
        }
        ShowWindow(SW_HIDE);
        m_pOwner->Invalidate();
    }

    void CMColorPopup::OnPaint()
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

    BOOL CMColorPopup::OnEraseBkgnd(CDC* pDC)
    {
        return TRUE;
    }

    void CMColorPopup::OnDraw(CDC* pDC)
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

        // 팔레트
        DrawPalette(pDC, GetPaletteRect());

        // 사용자 정의 색상
        if (m_pOwner->m_customColors.GetSize() > 0)
        {
            DrawCustomColors(pDC, GetCustomRect());
        }

        // 미리보기 및 더보기
        DrawPreview(pDC, GetPreviewRect());
        DrawMoreButton(pDC, GetMoreButtonRect());
    }

    void CMColorPopup::DrawPalette(CDC* pDC, const CRect& rect)
    {
        for (int row = 0; row < PALETTE_ROWS; row++)
        {
            for (int col = 0; col < PALETTE_COLS; col++)
            {
                int index = row * PALETTE_COLS + col;
                COLORREF color = s_paletteColors[index];

                CRect cellRect(
                    rect.left + col * CELL_SIZE,
                    rect.top + row * CELL_SIZE,
                    rect.left + (col + 1) * CELL_SIZE,
                    rect.top + (row + 1) * CELL_SIZE
                );
                cellRect.DeflateRect(1, 1);

                // 선택/호버 표시
                if (color == m_pOwner->GetColor())
                {
                    CRect selRect = cellRect;
                    selRect.InflateRect(1, 1);
                    CPen pen(PS_SOLID, 2, GetColors().primary.normal);
                    CPen* oldPen = pDC->SelectObject(&pen);
                    CBrush* oldBrush = (CBrush*)pDC->SelectStockObject(NULL_BRUSH);
                    pDC->Rectangle(selRect);
                    pDC->SelectObject(oldPen);
                    pDC->SelectObject(oldBrush);
                }
                else if (index == m_hoverPaletteIndex)
                {
                    CRect hoverRect = cellRect;
                    hoverRect.InflateRect(1, 1);
                    CPen pen(PS_SOLID, 1, GetColors().border.hover);
                    CPen* oldPen = pDC->SelectObject(&pen);
                    CBrush* oldBrush = (CBrush*)pDC->SelectStockObject(NULL_BRUSH);
                    pDC->Rectangle(hoverRect);
                    pDC->SelectObject(oldPen);
                    pDC->SelectObject(oldBrush);
                }

                // 색상 셀
                CBrush brush(color);
                pDC->FillRect(cellRect, &brush);
            }
        }
    }

    void CMColorPopup::DrawCustomColors(CDC* pDC, const CRect& rect)
    {
        const ThemeColors& colors = GetColors();

        // 레이블
        CFont* pFont = CMThemeManager::GetInstance().GetFont(TextStyle::Caption);
        CFont* oldFont = pDC->SelectObject(pFont);
        pDC->SetBkMode(TRANSPARENT);
        pDC->SetTextColor(colors.textSecondary);

        CRect labelRect = rect;
        labelRect.bottom = labelRect.top + 16;
        pDC->DrawText(_T("최근 사용한 색상"), labelRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
        pDC->SelectObject(oldFont);

        // 색상들
        int y = rect.top + 16;
        for (INT_PTR i = 0; i < m_pOwner->m_customColors.GetSize(); i++)
        {
            CRect cellRect(
                rect.left + (int)i * CELL_SIZE,
                y,
                rect.left + (int)(i + 1) * CELL_SIZE,
                y + CELL_SIZE
            );
            cellRect.DeflateRect(1, 1);

            COLORREF color = m_pOwner->m_customColors[i];

            // 호버 표시
            if ((int)i == m_hoverCustomIndex)
            {
                CRect hoverRect = cellRect;
                hoverRect.InflateRect(1, 1);
                CPen pen(PS_SOLID, 1, colors.border.hover);
                CPen* oldPen = pDC->SelectObject(&pen);
                CBrush* oldBrush = (CBrush*)pDC->SelectStockObject(NULL_BRUSH);
                pDC->Rectangle(hoverRect);
                pDC->SelectObject(oldPen);
                pDC->SelectObject(oldBrush);
            }

            CBrush brush(color);
            pDC->FillRect(cellRect, &brush);
        }
    }

    void CMColorPopup::DrawPreview(CDC* pDC, const CRect& rect)
    {
        const ThemeColors& colors = GetColors();

        // 현재 색상 미리보기
        CRect colorRect = rect;
        colorRect.right = colorRect.left + 32;

        CBrush brush(m_tempColor);
        CPen pen(PS_SOLID, 1, colors.border.normal);
        CPen* oldPen = pDC->SelectObject(&pen);
        CBrush* oldBrush = pDC->SelectObject(&brush);
        pDC->RoundRect(colorRect, CPoint(4, 4));
        pDC->SelectObject(oldPen);
        pDC->SelectObject(oldBrush);

        // HEX 코드
        CRect textRect = rect;
        textRect.left = colorRect.right + 8;
        textRect.right = textRect.left + 60;

        CString hex;
        hex.Format(_T("#%02X%02X%02X"), GetRValue(m_tempColor), GetGValue(m_tempColor), GetBValue(m_tempColor));

        CFont* pFont = CMThemeManager::GetInstance().GetFont(TextStyle::Caption);
        CFont* oldFont = pDC->SelectObject(pFont);
        pDC->SetBkMode(TRANSPARENT);
        pDC->SetTextColor(colors.text);
        pDC->DrawText(hex, textRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
        pDC->SelectObject(oldFont);
    }

    void CMColorPopup::DrawMoreButton(CDC* pDC, const CRect& rect)
    {
        const ThemeColors& colors = GetColors();

        // 버튼 배경
        COLORREF bgColor = m_hoverMoreButton ? colors.surface.hover : colors.surface.normal;
        CPen pen(PS_SOLID, 1, colors.border.normal);
        CBrush brush(bgColor);
        CPen* oldPen = pDC->SelectObject(&pen);
        CBrush* oldBrush = pDC->SelectObject(&brush);
        pDC->RoundRect(rect, CPoint(4, 4));
        pDC->SelectObject(oldPen);
        pDC->SelectObject(oldBrush);

        // 텍스트
        CFont* pFont = CMThemeManager::GetInstance().GetFont(TextStyle::Caption);
        CFont* oldFont = pDC->SelectObject(pFont);
        pDC->SetBkMode(TRANSPARENT);
        pDC->SetTextColor(colors.primary.normal);
        CRect textRect = rect;
        pDC->DrawText(_T("More colors..."), &textRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        pDC->SelectObject(oldFont);
    }

    CRect CMColorPopup::GetPaletteRect()
    {
        return CRect(PADDING, PADDING,
            PADDING + PALETTE_COLS * CELL_SIZE,
            PADDING + PALETTE_ROWS * CELL_SIZE);
    }

    CRect CMColorPopup::GetCustomRect()
    {
        CRect paletteRect = GetPaletteRect();
        return CRect(PADDING, paletteRect.bottom + SECTION_GAP,
            PADDING + PALETTE_COLS * CELL_SIZE,
            paletteRect.bottom + SECTION_GAP + CELL_SIZE + 16);
    }

    CRect CMColorPopup::GetPreviewRect()
    {
        CRect rect;
        GetClientRect(&rect);

        return CRect(PADDING, rect.bottom - PADDING - 24,
            PADDING + 100, rect.bottom - PADDING);
    }

    CRect CMColorPopup::GetMoreButtonRect()
    {
        CRect rect;
        GetClientRect(&rect);

        return CRect(rect.right - PADDING - 80, rect.bottom - PADDING - 24,
            rect.right - PADDING, rect.bottom - PADDING);
    }

    int CMColorPopup::HitTestPalette(CPoint point)
    {
        CRect paletteRect = GetPaletteRect();
        if (!paletteRect.PtInRect(point))
            return -1;

        int col = (point.x - paletteRect.left) / CELL_SIZE;
        int row = (point.y - paletteRect.top) / CELL_SIZE;

        if (col >= 0 && col < PALETTE_COLS && row >= 0 && row < PALETTE_ROWS)
        {
            return row * PALETTE_COLS + col;
        }

        return -1;
    }

    int CMColorPopup::HitTestCustom(CPoint point)
    {
        if (m_pOwner->m_customColors.GetSize() == 0)
            return -1;

        CRect customRect = GetCustomRect();
        customRect.top += 16;  // 레이블 제외

        if (!customRect.PtInRect(point))
            return -1;

        int col = (point.x - customRect.left) / CELL_SIZE;
        if (col >= 0 && col < m_pOwner->m_customColors.GetSize())
        {
            return col;
        }

        return -1;
    }

    BOOL CMColorPopup::HitTestMoreButton(CPoint point)
    {
        return GetMoreButtonRect().PtInRect(point);
    }

    void CMColorPopup::OnLButtonDown(UINT nFlags, CPoint point)
    {
        CRect rect;
        GetClientRect(&rect);

        // 팝업 영역 외부 클릭 시 닫기
        if (!rect.PtInRect(point))
        {
            Hide();
            return;
        }

        // 팔레트 클릭
        int paletteIndex = HitTestPalette(point);
        if (paletteIndex >= 0)
        {
            COLORREF color = s_paletteColors[paletteIndex];
            m_pOwner->SetColor(color);
            m_pOwner->AddCustomColor(color);
            Hide();
            return;
        }

        // 사용자 정의 색상 클릭
        int customIndex = HitTestCustom(point);
        if (customIndex >= 0)
        {
            COLORREF color = m_pOwner->m_customColors[customIndex];
            m_pOwner->SetColor(color);
            Hide();
            return;
        }

        // 더보기 버튼
        if (HitTestMoreButton(point))
        {
            Hide();

            // Windows 색상 선택 대화상자
            CColorDialog dlg(m_pOwner->GetColor(), CC_FULLOPEN, m_pOwner);
            if (dlg.DoModal() == IDOK)
            {
                COLORREF color = dlg.GetColor();
                m_pOwner->SetColor(color);
                m_pOwner->AddCustomColor(color);
            }
            return;
        }

        CWnd::OnLButtonDown(nFlags, point);
    }

    void CMColorPopup::OnMouseMove(UINT nFlags, CPoint point)
    {
        int newPaletteIndex = HitTestPalette(point);
        int newCustomIndex = HitTestCustom(point);
        BOOL newMoreButton = HitTestMoreButton(point);

        if (newPaletteIndex != m_hoverPaletteIndex ||
            newCustomIndex != m_hoverCustomIndex ||
            newMoreButton != m_hoverMoreButton)
        {
            m_hoverPaletteIndex = newPaletteIndex;
            m_hoverCustomIndex = newCustomIndex;
            m_hoverMoreButton = newMoreButton;

            // 임시 색상 업데이트
            if (m_hoverPaletteIndex >= 0)
            {
                m_tempColor = s_paletteColors[m_hoverPaletteIndex];
            }
            else if (m_hoverCustomIndex >= 0)
            {
                m_tempColor = m_pOwner->m_customColors[m_hoverCustomIndex];
            }
            else
            {
                m_tempColor = m_pOwner->GetColor();
            }

            Invalidate();
        }

        CWnd::OnMouseMove(nFlags, point);
    }

    void CMColorPopup::OnKillFocus(CWnd* pNewWnd)
    {
        // WS_EX_NOACTIVATE 사용으로 이 핸들러는 거의 호출되지 않음
        // 마우스 캡처로 외부 클릭을 감지함
        CWnd::OnKillFocus(pNewWnd);
    }

    void CMColorPopup::OnClose()
    {
        Hide();
    }

    void CMColorPopup::RGBToHSV(COLORREF color, float& h, float& s, float& v)
    {
        float r = GetRValue(color) / 255.0f;
        float g = GetGValue(color) / 255.0f;
        float b = GetBValue(color) / 255.0f;

        float maxVal = max(max(r, g), b);
        float minVal = min(min(r, g), b);
        float delta = maxVal - minVal;

        v = maxVal;

        if (maxVal == 0)
        {
            s = 0;
            h = 0;
        }
        else
        {
            s = delta / maxVal;

            if (delta == 0)
            {
                h = 0;
            }
            else if (maxVal == r)
            {
                h = 60.0f * fmod(((g - b) / delta), 6.0f);
            }
            else if (maxVal == g)
            {
                h = 60.0f * (((b - r) / delta) + 2.0f);
            }
            else
            {
                h = 60.0f * (((r - g) / delta) + 4.0f);
            }

            if (h < 0) h += 360.0f;
        }
    }

    COLORREF CMColorPopup::HSVToRGB(float h, float s, float v)
    {
        float c = v * s;
        float x = c * (1 - fabs(fmod(h / 60.0f, 2.0f) - 1));
        float m = v - c;

        float r, g, b;

        if (h < 60)
        {
            r = c; g = x; b = 0;
        }
        else if (h < 120)
        {
            r = x; g = c; b = 0;
        }
        else if (h < 180)
        {
            r = 0; g = c; b = x;
        }
        else if (h < 240)
        {
            r = 0; g = x; b = c;
        }
        else if (h < 300)
        {
            r = x; g = 0; b = c;
        }
        else
        {
            r = c; g = 0; b = x;
        }

        return RGB(
            (int)((r + m) * 255),
            (int)((g + m) * 255),
            (int)((b + m) * 255)
        );
    }

    const ThemeColors& CMColorPopup::GetColors() const
    {
        return CMThemeManager::GetInstance().GetColors();
    }
}
