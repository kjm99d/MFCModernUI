#pragma once

#include "CMControlBase.h"
#include <vector>

// MFC Modern UI - 라디오 버튼 컴포넌트
// SOLID: 단일 책임 원칙 - 라디오 버튼 동작만 담당

namespace MFCModernUI
{
    class CMRadioButton;

    // 선택 변경 이벤트 핸들러
    typedef void (*RadioSelectionChangedHandler)(void* context, CMRadioButton* selected);

    // 라디오 버튼 그룹 관리자
    class CMRadioGroup
    {
    public:
        static CMRadioGroup& GetInstance();

        void RegisterRadio(int groupId, CMRadioButton* radio);
        void UnregisterRadio(CMRadioButton* radio);
        void SelectRadio(int groupId, CMRadioButton* selected);
        CMRadioButton* GetSelectedRadio(int groupId);

    private:
        CMRadioGroup() = default;
        std::vector<std::pair<int, CMRadioButton*>> m_radios;
    };

    class CMRadioButton : public CMControlBase
    {
        DECLARE_DYNAMIC(CMRadioButton)

    public:
        CMRadioButton();
        virtual ~CMRadioButton();

        // 생성
        BOOL Create(const CString& text, DWORD dwStyle, const CRect& rect,
            CWnd* pParentWnd, UINT nID, int groupId = 0);

        // 선택 상태
        void SetSelected(BOOL selected);
        BOOL IsSelected() const { return m_selected; }

        // 텍스트
        void SetText(const CString& text);
        const CString& GetText() const { return m_text; }

        // 그룹 ID
        int GetGroupId() const { return m_groupId; }

        // 이벤트 핸들러
        void SetSelectionChangedHandler(RadioSelectionChangedHandler handler, void* context);

    protected:
        virtual void OnDraw(CDC* pDC) override;
        virtual void SetState(ControlState newState) override;

        // 메시지 핸들러
        afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
        afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);

        DECLARE_MESSAGE_MAP()

    private:
        // 속성
        BOOL m_selected;
        CString m_text;
        int m_groupId;

        // 애니메이션
        float m_innerCircleScale;  // 0.0 ~ 1.0

        // 이벤트 핸들러
        RadioSelectionChangedHandler m_selectionChangedHandler;
        void* m_selectionChangedContext;

        // 헬퍼
        void DrawRadioButton(CDC* pDC, const CRect& circleRect);
        void StartSelectionAnimation();

        // Direct2D
        void OnDrawGdiPlus(CDC* pDC);
        void DrawRadioButtonD2D(const CRect& circleRect);

        friend class CMRadioGroup;
    };
}
