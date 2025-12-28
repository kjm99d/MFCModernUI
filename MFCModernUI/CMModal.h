#pragma once

#include "CMControlBase.h"
#include "CMButton.h"

// MFC Modern UI - 모달 대화상자 컴포넌트
// SOLID: 단일 책임 원칙 - 모달 대화상자 표시만 담당

namespace MFCModernUI
{
    // 모달 결과
    enum class ModalResult
    {
        None,
        OK,
        Cancel,
        Yes,
        No,
        Close,
        Custom1,
        Custom2,
        Custom3
    };

    // 모달 버튼 세트
    enum class ModalButtons
    {
        OK,
        OKCancel,
        YesNo,
        YesNoCancel,
        Custom
    };

    // 모달 아이콘
    enum class ModalIcon
    {
        None,
        Info,
        Question,
        Warning,
        Error
    };

    // 모달 결과 핸들러
    typedef void (*ModalResultHandler)(void* context, ModalResult result);

    // 전방 선언
    class CMModalOverlay;

    class CMModal : public CWnd
    {
        DECLARE_DYNAMIC(CMModal)

    public:
        CMModal();
        virtual ~CMModal();

        // 생성 및 표시
        BOOL Create(CWnd* pParentWnd);
        ModalResult DoModal(CWnd* pParentWnd = nullptr);
        void EndModal(ModalResult result);

        // 내용 설정
        void SetTitle(LPCTSTR title);
        void SetMessage(LPCTSTR message);
        void SetIcon(ModalIcon icon);
        void SetButtons(ModalButtons buttons);

        // 커스텀 버튼
        void SetCustomButton(int index, LPCTSTR text, ModalResult result);

        // 크기
        void SetSize(int width, int height);

        // 내용 영역 (커스텀 컨트롤 배치용)
        CRect GetContentRect();

        // 이벤트 핸들러
        void SetResultHandler(ModalResultHandler handler, void* context);

        // 정적 헬퍼 메서드 (MessageBox 스타일)
        static ModalResult ShowMessage(LPCTSTR message, LPCTSTR title = nullptr,
            ModalButtons buttons = ModalButtons::OK, ModalIcon icon = ModalIcon::Info);
        static ModalResult ShowQuestion(LPCTSTR message, LPCTSTR title = nullptr);
        static ModalResult ShowWarning(LPCTSTR message, LPCTSTR title = nullptr);
        static ModalResult ShowError(LPCTSTR message, LPCTSTR title = nullptr);

    protected:
        virtual void OnDraw(CDC* pDC);

        afx_msg void OnPaint();
        afx_msg BOOL OnEraseBkgnd(CDC* pDC);
        afx_msg void OnSize(UINT nType, int cx, int cy);
        afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
        afx_msg LRESULT OnButtonClicked(WPARAM wParam, LPARAM lParam);

        DECLARE_MESSAGE_MAP()

    private:
        // 내용
        CString m_title;
        CString m_message;
        ModalIcon m_icon;
        ModalButtons m_buttonSet;

        // 크기
        int m_width;
        int m_height;

        // 결과
        ModalResult m_result;
        BOOL m_isModal;

        // 버튼
        struct ButtonInfo
        {
            CMButton* pButton;
            CString text;
            ModalResult result;
            BOOL isDefault;
            BOOL isCancel;
        };
        CArray<ButtonInfo> m_buttons;

        // 오버레이 (배경 어둡게)
        CMModalOverlay* m_pOverlay;

        // 이벤트
        ModalResultHandler m_resultHandler;
        void* m_resultContext;

        // 상수
        static const int PADDING = 24;
        static const int BUTTON_HEIGHT = 36;
        static const int BUTTON_WIDTH = 80;
        static const int BUTTON_SPACING = 8;
        static const int ICON_SIZE = 48;
        static const int TITLE_HEIGHT = 28;
        static const int DEFAULT_WIDTH = 400;
        static const int DEFAULT_HEIGHT = 200;

        // 헬퍼
        void CreateButtons();
        void LayoutButtons();
        void DrawIcon(CDC* pDC, const CRect& rect);
        void DrawTitle(CDC* pDC, const CRect& rect);
        void DrawMessage(CDC* pDC, const CRect& rect);
        COLORREF GetIconColor() const;
        const ThemeColors& GetColors() const;
        void CenterWindow(CWnd* pParent);
        void ShowOverlay(CWnd* pParent);
        void HideOverlay();
    };

    // 오버레이 창 (배경 어둡게)
    class CMModalOverlay : public CWnd
    {
    public:
        CMModalOverlay();
        virtual ~CMModalOverlay();

        BOOL CreateOverlay(CWnd* pParentWnd);

    protected:
        afx_msg void OnPaint();
        afx_msg BOOL OnEraseBkgnd(CDC* pDC);

        DECLARE_MESSAGE_MAP()
    };
}
