#pragma once

#include "CMControlBase.h"

// MFC Modern UI - 텍스트 입력 컴포넌트
// SOLID: 단일 책임 원칙 - 텍스트 입력 처리만 담당

namespace MFCModernUI
{
    // 텍스트 변경 이벤트 핸들러
    typedef void (*TextChangedHandler)(void* context, const CString& text);

    class CMEdit : public CMControlBase
    {
        DECLARE_DYNAMIC(CMEdit)

    public:
        CMEdit();
        virtual ~CMEdit();

        // 생성
        BOOL Create(DWORD dwStyle, const CRect& rect, CWnd* pParentWnd, UINT nID);

        // 텍스트
        void SetText(const CString& text);
        CString GetText() const;

        // 플레이스홀더
        void SetPlaceholder(const CString& placeholder);
        const CString& GetPlaceholder() const { return m_placeholder; }

        // 최대 길이
        void SetMaxLength(int maxLength);
        int GetMaxLength() const { return m_maxLength; }

        // 다중 라인
        void SetMultiline(BOOL multiline);
        BOOL IsMultiline() const { return m_multiline; }

        // 비밀번호 모드
        void SetPassword(BOOL password);
        BOOL IsPassword() const { return m_password; }

        // 읽기 전용
        void SetReadOnly(BOOL readOnly);
        BOOL IsReadOnly() const { return m_readOnly; }

        // 지우기 버튼
        void SetClearButton(BOOL show);
        BOOL HasClearButton() const { return m_clearButton; }

        // 유효성 상태
        void SetValidationState(ValidationState state);
        ValidationState GetValidationState() const { return m_validationState; }

        // 이벤트 핸들러
        void SetTextChangedHandler(TextChangedHandler handler, void* context);

    protected:
        virtual void OnDraw(CDC* pDC) override;
        virtual void SetState(ControlState newState) override;

        // 내부 에디트 컨트롤 생성
        void CreateInnerEdit();
        void UpdateInnerEditPosition();

        // 테두리 색상 결정
        COLORREF GetBorderColor() const;

        // 메시지 핸들러
        afx_msg void OnSize(UINT nType, int cx, int cy);
        afx_msg void OnSetFocus(CWnd* pOldWnd);
        afx_msg void OnKillFocus(CWnd* pNewWnd);
        afx_msg LRESULT OnEditChange(WPARAM wParam, LPARAM lParam);
        afx_msg void OnLButtonDown(UINT nFlags, CPoint point);

        DECLARE_MESSAGE_MAP()

    private:
        // 속성
        CString m_placeholder;
        int m_maxLength;
        BOOL m_multiline;
        BOOL m_password;
        BOOL m_readOnly;
        BOOL m_clearButton;
        ValidationState m_validationState;

        // 내부 에디트 컨트롤
        CEdit m_innerEdit;
        BOOL m_innerEditCreated;

        // 애니메이션
        COLORREF m_targetBorderColor;

        // 이벤트 핸들러
        TextChangedHandler m_textChangedHandler;
        void* m_textChangedContext;

        // 지우기 버튼 영역
        CRect m_clearButtonRect;
        BOOL m_clearButtonHover;

        // 헬퍼
        void StartBorderAnimation(COLORREF targetColor);
        BOOL IsClearButtonVisible() const;
        void DrawClearButton(CDC* pDC);
        void DrawValidationIcon(CDC* pDC, const CRect& rect);
    };
}
