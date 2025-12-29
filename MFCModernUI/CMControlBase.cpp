#include "pch.h"
#include "CMControlBase.h"

namespace MFCModernUI
{
    // Direct2D 정적 멤버 초기화
    ID2D1Factory* CMControlBase::s_pD2DFactory = nullptr;
    IDWriteFactory* CMControlBase::s_pDWriteFactory = nullptr;
    int CMControlBase::s_d2dRefCount = 0;

    IMPLEMENT_DYNAMIC(CMControlBase, CWnd)

    BEGIN_MESSAGE_MAP(CMControlBase, CWnd)
        ON_WM_PAINT()
        ON_WM_MOUSEMOVE()
        ON_WM_MOUSELEAVE()
        ON_WM_LBUTTONDOWN()
        ON_WM_LBUTTONUP()
        ON_WM_SETFOCUS()
        ON_WM_KILLFOCUS()
        ON_WM_ENABLE()
        ON_WM_ERASEBKGND()
    END_MESSAGE_MAP()

    CMControlBase::CMControlBase()
        : m_enabled(TRUE)
        , m_visible(TRUE)
        , m_tabStop(TRUE)
        , m_state(ControlState::Normal)
        , m_controlSize(ControlSize::Medium)
        , m_isEnabled(TRUE)
        , m_isHovered(FALSE)
        , m_isFocused(FALSE)
        , m_isPressed(FALSE)
        , m_mouseTracking(FALSE)
        , m_mouseOver(FALSE)
        , m_currentAnimationId(0)
        , m_animationProgress(0.0f)
        , m_currentBgColor(RGB(255, 255, 255))
        , m_currentBorderColor(RGB(200, 200, 200))
        , m_isD2DDrawing(FALSE)
        , m_pRenderTarget(nullptr)
        , m_pSolidBrush(nullptr)
    {
        // Direct2D 팩토리 초기화
        InitD2DFactory();

        // 텍스트 포맷 초기화
        for (int i = 0; i < 4; i++)
            m_pTextFormats[i] = nullptr;

        // 테마 변경 콜백 등록
        CMThemeManager::GetInstance().RegisterThemeChangedCallback(OnThemeChanged, this);
    }

    CMControlBase::~CMControlBase()
    {
        // 테마 변경 콜백 해제
        CMThemeManager::GetInstance().UnregisterThemeChangedCallback(this);

        // 애니메이션 중지
        if (m_currentAnimationId > 0)
        {
            CMAnimationManager::GetInstance().StopAnimation(m_currentAnimationId);
        }

        // Direct2D 리소스 해제
        DiscardDeviceResources();
        ReleaseD2DFactory();
    }

    void CMControlBase::SetEnabled(BOOL enabled)
    {
        if (m_enabled != enabled)
        {
            m_enabled = enabled;
            m_isEnabled = enabled;
            SetState(enabled ? ControlState::Normal : ControlState::Disabled);
            EnableWindow(enabled);
            Invalidate();
        }
    }

    void CMControlBase::SetVisible(BOOL visible)
    {
        if (m_visible != visible)
        {
            m_visible = visible;
            ShowWindow(visible ? SW_SHOW : SW_HIDE);
        }
    }

    void CMControlBase::SetThemeClass(const CString& themeClass)
    {
        m_themeClass = themeClass;
        Invalidate();
    }

    void CMControlBase::SetTabStop(BOOL tabStop)
    {
        m_tabStop = tabStop;
        ModifyStyle(tabStop ? 0 : WS_TABSTOP, tabStop ? WS_TABSTOP : 0);
    }

    void CMControlBase::SetTooltip(const CString& tooltip)
    {
        m_tooltip = tooltip;
        UpdateTooltip();
    }

    void CMControlBase::SetControlSize(ControlSize size)
    {
        if (m_controlSize != size)
        {
            m_controlSize = size;
            Invalidate();
        }
    }

    void CMControlBase::SetState(ControlState newState)
    {
        if (m_state != newState)
        {
            ControlState oldState = m_state;
            m_state = newState;

            // 상태 플래그 동기화
            m_isEnabled = (newState != ControlState::Disabled);
            m_isHovered = (newState == ControlState::Hover);
            m_isFocused = (newState == ControlState::Focused);
            m_isPressed = (newState == ControlState::Pressed);

            Invalidate();
        }
    }

    int CMControlBase::GetHeight() const
    {
        switch (m_controlSize)
        {
        case ControlSize::Small:
            return SizeConstants::SmallHeight;
        case ControlSize::Large:
            return SizeConstants::LargeHeight;
        default:
            return SizeConstants::MediumHeight;
        }
    }

    int CMControlBase::GetPadding() const
    {
        switch (m_controlSize)
        {
        case ControlSize::Small:
            return SizeConstants::SmallPadding;
        case ControlSize::Large:
            return SizeConstants::LargePadding;
        default:
            return SizeConstants::MediumPadding;
        }
    }

    int CMControlBase::GetFontSize() const
    {
        switch (m_controlSize)
        {
        case ControlSize::Small:
            return SizeConstants::SmallFontSize;
        case ControlSize::Large:
            return SizeConstants::LargeFontSize;
        default:
            return SizeConstants::MediumFontSize;
        }
    }

    void CMControlBase::OnPaint()
    {
        CPaintDC dc(this);
        // Direct2D 컨트롤은 OnDraw에서 BeginD2DDraw/EndD2DDraw 호출
        OnDraw(&dc);
    }

    void CMControlBase::OnMouseMove(UINT nFlags, CPoint point)
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

        if (!m_mouseOver && m_enabled)
        {
            m_mouseOver = TRUE;
            if (m_state != ControlState::Pressed)
            {
                SetState(ControlState::Hover);
            }
        }

        CWnd::OnMouseMove(nFlags, point);
    }

    void CMControlBase::OnMouseLeave()
    {
        m_mouseTracking = FALSE;
        m_mouseOver = FALSE;

        if (m_enabled && m_state != ControlState::Focused)
        {
            SetState(ControlState::Normal);
        }
        else if (m_state == ControlState::Focused)
        {
            Invalidate();
        }

        CWnd::OnMouseLeave();
    }

    void CMControlBase::OnLButtonDown(UINT nFlags, CPoint point)
    {
        if (m_enabled)
        {
            SetFocus();
            SetCapture();
            SetState(ControlState::Pressed);
        }

        CWnd::OnLButtonDown(nFlags, point);
    }

    void CMControlBase::OnLButtonUp(UINT nFlags, CPoint point)
    {
        if (GetCapture() == this)
        {
            ReleaseCapture();

            if (m_enabled)
            {
                // 마우스가 컨트롤 내부에 있으면 Hover, 외부면 Normal
                CRect rect;
                GetClientRect(&rect);
                if (rect.PtInRect(point))
                {
                    SetState(ControlState::Hover);
                }
                else
                {
                    SetState(ControlState::Normal);
                }
            }
        }

        CWnd::OnLButtonUp(nFlags, point);
    }

    void CMControlBase::OnSetFocus(CWnd* pOldWnd)
    {
        CWnd::OnSetFocus(pOldWnd);

        if (m_enabled && m_state != ControlState::Pressed)
        {
            SetState(ControlState::Focused);
        }
    }

    void CMControlBase::OnKillFocus(CWnd* pNewWnd)
    {
        CWnd::OnKillFocus(pNewWnd);

        if (m_enabled)
        {
            SetState(m_mouseOver ? ControlState::Hover : ControlState::Normal);
        }
    }

    void CMControlBase::OnEnable(BOOL bEnable)
    {
        CWnd::OnEnable(bEnable);
        m_enabled = bEnable;
        SetState(bEnable ? ControlState::Normal : ControlState::Disabled);
    }

    BOOL CMControlBase::OnEraseBkgnd(CDC* pDC)
    {
        // 깜빡임 방지를 위해 배경 지우기 무시
        return TRUE;
    }

    void CMControlBase::OnThemeChanged(void* context)
    {
        CMControlBase* pThis = static_cast<CMControlBase*>(context);
        if (pThis)
        {
            pThis->HandleThemeChanged();
        }
    }

    void CMControlBase::HandleThemeChanged()
    {
        Invalidate();
    }

    void CMControlBase::UpdateTooltip()
    {
        if (m_hWnd && !m_tooltip.IsEmpty())
        {
            if (!m_tooltipCtrl.m_hWnd)
            {
                m_tooltipCtrl.Create(this, TTS_ALWAYSTIP);
                m_tooltipCtrl.Activate(TRUE);
            }

            CRect rect;
            GetClientRect(&rect);
            m_tooltipCtrl.AddTool(this, m_tooltip, &rect, 1);
        }
    }

    // ========================================
    // Direct2D 구현
    // ========================================

    BOOL CMControlBase::InitD2DFactory()
    {
        if (s_pD2DFactory == nullptr)
        {
            HRESULT hr = D2D1CreateFactory(
                D2D1_FACTORY_TYPE_SINGLE_THREADED,
                &s_pD2DFactory
            );

            if (FAILED(hr))
                return FALSE;

            hr = DWriteCreateFactory(
                DWRITE_FACTORY_TYPE_SHARED,
                __uuidof(IDWriteFactory),
                reinterpret_cast<IUnknown**>(&s_pDWriteFactory)
            );

            if (FAILED(hr))
            {
                s_pD2DFactory->Release();
                s_pD2DFactory = nullptr;
                return FALSE;
            }
        }

        s_d2dRefCount++;
        return TRUE;
    }

    void CMControlBase::ReleaseD2DFactory()
    {
        s_d2dRefCount--;

        if (s_d2dRefCount <= 0)
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

            s_d2dRefCount = 0;
        }
    }

    BOOL CMControlBase::CreateDeviceResources()
    {
        if (m_pRenderTarget)
            return TRUE;

        if (!s_pD2DFactory || !m_hWnd)
            return FALSE;

        CRect rect;
        GetClientRect(&rect);

        D2D1_SIZE_U size = D2D1::SizeU(rect.Width(), rect.Height());

        D2D1_RENDER_TARGET_PROPERTIES rtProps = D2D1::RenderTargetProperties();
        rtProps.type = D2D1_RENDER_TARGET_TYPE_DEFAULT;
        rtProps.pixelFormat = D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED);

        D2D1_HWND_RENDER_TARGET_PROPERTIES hwndRtProps = D2D1::HwndRenderTargetProperties(m_hWnd, size);
        hwndRtProps.presentOptions = D2D1_PRESENT_OPTIONS_IMMEDIATELY;

        HRESULT hr = s_pD2DFactory->CreateHwndRenderTarget(
            rtProps,
            hwndRtProps,
            &m_pRenderTarget
        );

        if (FAILED(hr))
            return FALSE;

        // 안티앨리어싱 설정
        m_pRenderTarget->SetAntialiasMode(D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
        m_pRenderTarget->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_CLEARTYPE);

        // 브러시 생성
        hr = m_pRenderTarget->CreateSolidColorBrush(
            D2D1::ColorF(D2D1::ColorF::White),
            &m_pSolidBrush
        );

        if (FAILED(hr))
        {
            m_pRenderTarget->Release();
            m_pRenderTarget = nullptr;
            return FALSE;
        }

        // 텍스트 포맷 생성
        if (s_pDWriteFactory)
        {
            // Body
            s_pDWriteFactory->CreateTextFormat(
                L"Segoe UI", nullptr,
                DWRITE_FONT_WEIGHT_NORMAL,
                DWRITE_FONT_STYLE_NORMAL,
                DWRITE_FONT_STRETCH_NORMAL,
                14.0f, L"ko-KR",
                &m_pTextFormats[0]
            );

            // Caption
            s_pDWriteFactory->CreateTextFormat(
                L"Segoe UI", nullptr,
                DWRITE_FONT_WEIGHT_NORMAL,
                DWRITE_FONT_STYLE_NORMAL,
                DWRITE_FONT_STRETCH_NORMAL,
                12.0f, L"ko-KR",
                &m_pTextFormats[1]
            );

            // Subtitle
            s_pDWriteFactory->CreateTextFormat(
                L"Segoe UI", nullptr,
                DWRITE_FONT_WEIGHT_SEMI_BOLD,
                DWRITE_FONT_STYLE_NORMAL,
                DWRITE_FONT_STRETCH_NORMAL,
                16.0f, L"ko-KR",
                &m_pTextFormats[2]
            );

            // Title
            s_pDWriteFactory->CreateTextFormat(
                L"Segoe UI", nullptr,
                DWRITE_FONT_WEIGHT_BOLD,
                DWRITE_FONT_STYLE_NORMAL,
                DWRITE_FONT_STRETCH_NORMAL,
                20.0f, L"ko-KR",
                &m_pTextFormats[3]
            );
        }

        return TRUE;
    }

    void CMControlBase::DiscardDeviceResources()
    {
        for (int i = 0; i < 4; i++)
        {
            if (m_pTextFormats[i])
            {
                m_pTextFormats[i]->Release();
                m_pTextFormats[i] = nullptr;
            }
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
    }

    D2D1_COLOR_F CMControlBase::ToD2DColor(COLORREF color, float alpha)
    {
        return D2D1::ColorF(
            GetRValue(color) / 255.0f,
            GetGValue(color) / 255.0f,
            GetBValue(color) / 255.0f,
            alpha
        );
    }

    ID2D1RenderTarget* CMControlBase::GetRenderTarget()
    {
        if (!m_pRenderTarget)
            CreateDeviceResources();
        return m_pRenderTarget;
    }

    ID2D1SolidColorBrush* CMControlBase::GetSolidBrush(COLORREF color, float alpha)
    {
        if (m_pSolidBrush)
        {
            m_pSolidBrush->SetColor(ToD2DColor(color, alpha));
        }
        return m_pSolidBrush;
    }

    IDWriteTextFormat* CMControlBase::GetTextFormat(TextStyle style)
    {
        int index = 0;
        switch (style)
        {
        case TextStyle::Caption:
        case TextStyle::BodySmall:
            index = 1;  // Small
            break;
        case TextStyle::H3:
        case TextStyle::H4:
        case TextStyle::H5:
        case TextStyle::H6:
            index = 2;  // Medium/Subtitle
            break;
        case TextStyle::H1:
        case TextStyle::H2:
            index = 3;  // Large/Title
            break;
        case TextStyle::Body:
        case TextStyle::BodyBold:
        default:
            index = 0;  // Normal
            break;
        }
        return m_pTextFormats[index];
    }

    BOOL CMControlBase::BeginD2DDraw()
    {
        if (!CreateDeviceResources())
            return FALSE;

        // 렌더 타겟 크기 확인 및 조정
        CRect rect;
        GetClientRect(&rect);
        D2D1_SIZE_U size = D2D1::SizeU(rect.Width(), rect.Height());
        D2D1_SIZE_U currentSize = m_pRenderTarget->GetPixelSize();

        if (size.width != currentSize.width || size.height != currentSize.height)
        {
            m_pRenderTarget->Resize(size);
        }

        m_pRenderTarget->BeginDraw();
        m_pRenderTarget->Clear(ToD2DColor(GetColors().surface.normal));
        m_isD2DDrawing = TRUE;

        return TRUE;
    }

    void CMControlBase::EndD2DDraw()
    {
        if (m_pRenderTarget && m_isD2DDrawing)
        {
            HRESULT hr = m_pRenderTarget->EndDraw();

            if (hr == D2DERR_RECREATE_TARGET)
            {
                DiscardDeviceResources();
            }
        }
        m_isD2DDrawing = FALSE;
    }

    void CMControlBase::DrawRoundedRectD2D(const CRect& rect, int radius,
        COLORREF fillColor, COLORREF borderColor, int borderWidth)
    {
        if (!m_pRenderTarget || !m_isD2DDrawing)
            return;

        D2D1_ROUNDED_RECT roundedRect = D2D1::RoundedRect(
            D2D1::RectF(
                static_cast<float>(rect.left),
                static_cast<float>(rect.top),
                static_cast<float>(rect.right),
                static_cast<float>(rect.bottom)
            ),
            static_cast<float>(radius),
            static_cast<float>(radius)
        );

        // 테두리가 있으면 영역 조정
        if (borderColor != CLR_INVALID && borderWidth > 0)
        {
            float halfBorder = borderWidth / 2.0f;
            roundedRect.rect.left += halfBorder;
            roundedRect.rect.top += halfBorder;
            roundedRect.rect.right -= halfBorder;
            roundedRect.rect.bottom -= halfBorder;
        }

        // 채우기
        m_pSolidBrush->SetColor(ToD2DColor(fillColor));
        m_pRenderTarget->FillRoundedRectangle(roundedRect, m_pSolidBrush);

        // 테두리
        if (borderColor != CLR_INVALID && borderWidth > 0)
        {
            m_pSolidBrush->SetColor(ToD2DColor(borderColor));
            m_pRenderTarget->DrawRoundedRectangle(roundedRect, m_pSolidBrush, static_cast<float>(borderWidth));
        }
    }

    void CMControlBase::DrawCircleD2D(const CRect& rect, COLORREF fillColor,
        COLORREF borderColor, int borderWidth)
    {
        DrawEllipseD2D(rect, fillColor, borderColor, borderWidth);
    }

    void CMControlBase::DrawEllipseD2D(const CRect& rect, COLORREF fillColor,
        COLORREF borderColor, int borderWidth)
    {
        if (!m_pRenderTarget || !m_isD2DDrawing)
            return;

        float cx = (rect.left + rect.right) / 2.0f;
        float cy = (rect.top + rect.bottom) / 2.0f;
        float rx = rect.Width() / 2.0f;
        float ry = rect.Height() / 2.0f;

        // 테두리가 있으면 영역 조정
        if (borderColor != CLR_INVALID && borderWidth > 0)
        {
            float halfBorder = borderWidth / 2.0f;
            rx -= halfBorder;
            ry -= halfBorder;
        }

        D2D1_ELLIPSE ellipse = D2D1::Ellipse(D2D1::Point2F(cx, cy), rx, ry);

        // 채우기
        m_pSolidBrush->SetColor(ToD2DColor(fillColor));
        m_pRenderTarget->FillEllipse(ellipse, m_pSolidBrush);

        // 테두리
        if (borderColor != CLR_INVALID && borderWidth > 0)
        {
            m_pSolidBrush->SetColor(ToD2DColor(borderColor));
            m_pRenderTarget->DrawEllipse(ellipse, m_pSolidBrush, static_cast<float>(borderWidth));
        }
    }

    void CMControlBase::DrawLineD2D(int x1, int y1, int x2, int y2, COLORREF color, int width)
    {
        if (!m_pRenderTarget || !m_isD2DDrawing)
            return;

        m_pSolidBrush->SetColor(ToD2DColor(color));

        D2D1_STROKE_STYLE_PROPERTIES strokeProps = D2D1::StrokeStyleProperties();
        strokeProps.startCap = D2D1_CAP_STYLE_ROUND;
        strokeProps.endCap = D2D1_CAP_STYLE_ROUND;
        strokeProps.lineJoin = D2D1_LINE_JOIN_ROUND;

        ID2D1StrokeStyle* pStrokeStyle = nullptr;
        s_pD2DFactory->CreateStrokeStyle(strokeProps, nullptr, 0, &pStrokeStyle);

        m_pRenderTarget->DrawLine(
            D2D1::Point2F(static_cast<float>(x1), static_cast<float>(y1)),
            D2D1::Point2F(static_cast<float>(x2), static_cast<float>(y2)),
            m_pSolidBrush,
            static_cast<float>(width),
            pStrokeStyle
        );

        if (pStrokeStyle)
            pStrokeStyle->Release();
    }

    void CMControlBase::DrawTextD2D(const CString& text, const CRect& rect,
        COLORREF color, TextStyle style, UINT format)
    {
        if (!m_pRenderTarget || !m_isD2DDrawing)
            return;

        IDWriteTextFormat* pTextFormat = GetTextFormat(style);
        if (!pTextFormat)
            return;

        // 정렬 설정
        if (format & DT_CENTER)
            pTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
        else if (format & DT_RIGHT)
            pTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_TRAILING);
        else
            pTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);

        if (format & DT_VCENTER)
            pTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
        else if (format & DT_BOTTOM)
            pTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_FAR);
        else
            pTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR);

        D2D1_RECT_F layoutRect = D2D1::RectF(
            static_cast<float>(rect.left),
            static_cast<float>(rect.top),
            static_cast<float>(rect.right),
            static_cast<float>(rect.bottom)
        );

        m_pSolidBrush->SetColor(ToD2DColor(color));
        m_pRenderTarget->DrawText(
            text,
            text.GetLength(),
            pTextFormat,
            layoutRect,
            m_pSolidBrush
        );
    }
}
