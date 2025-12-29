#pragma once

#include "CMControlBase.h"
#include <vector>

// MFC Modern UI - 탭 컨트롤
// SOLID: 단일 책임 원칙 - 탭 전환만 담당

namespace MFCModernUI
{
    // 탭 항목
    struct TabItem
    {
        CString text;
        UINT iconId;
        BOOL closable;
        BOOL disabled;
        CString badge;

        TabItem(const CString& t = _T(""))
            : text(t), iconId(0), closable(FALSE), disabled(FALSE) {}
    };

    // 이벤트 핸들러
    typedef void (*TabChangedHandler)(void* context, int oldIndex, int newIndex);
    typedef BOOL (*TabClosingHandler)(void* context, int index);
    typedef void (*TabClosedHandler)(void* context, int index);

    class CMTabCtrl : public CMControlBase
    {
        DECLARE_DYNAMIC(CMTabCtrl)

    public:
        CMTabCtrl();
        virtual ~CMTabCtrl();

        // 생성
        BOOL Create(DWORD dwStyle, const CRect& rect, CWnd* pParentWnd, UINT nID);

        // 탭 관리
        int AddTab(const CString& text, BOOL closable = FALSE);
        int InsertTab(int index, const CString& text, BOOL closable = FALSE);
        void RemoveTab(int index);
        void RemoveAllTabs();
        int GetTabCount() const { return static_cast<int>(m_tabs.size()); }

        // 탭 속성
        void SetTabText(int index, const CString& text);
        CString GetTabText(int index) const;
        void SetTabIcon(int index, UINT iconId);
        void SetTabClosable(int index, BOOL closable);
        void SetTabDisabled(int index, BOOL disabled);
        void SetTabBadge(int index, const CString& badge);

        // 선택
        void SetSelectedIndex(int index);
        int GetSelectedIndex() const { return m_selectedIndex; }

        // 탭 위치
        void SetTabPosition(Position position);
        Position GetTabPosition() const { return m_tabPosition; }

        // 닫기 버튼
        void SetShowCloseButton(BOOL show);
        BOOL GetShowCloseButton() const { return m_showCloseButton; }

        // 스크롤 버튼
        void SetScrollButtons(BOOL show);
        BOOL GetScrollButtons() const { return m_scrollButtons; }

        // 이벤트 핸들러
        void SetTabChangedHandler(TabChangedHandler handler, void* context);
        void SetTabClosingHandler(TabClosingHandler handler, void* context);
        void SetTabClosedHandler(TabClosedHandler handler, void* context);

        // 탭 영역 높이/너비 가져오기
        int GetTabAreaSize() const;

    protected:
        virtual void OnDraw(CDC* pDC) override;

        afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
        afx_msg void OnMouseMove(UINT nFlags, CPoint point);
        afx_msg void OnMouseLeave();

        DECLARE_MESSAGE_MAP()

    private:
        // 탭
        std::vector<TabItem> m_tabs;
        int m_selectedIndex;

        // 옵션
        Position m_tabPosition;
        BOOL m_showCloseButton;
        BOOL m_scrollButtons;

        // 스크롤
        int m_scrollOffset;
        int m_maxScrollOffset;

        // 호버
        int m_hoverIndex;
        int m_hoverCloseIndex;

        // 상수
        static const int TAB_HEIGHT = 40;
        static const int TAB_MIN_WIDTH = 80;
        static const int TAB_MAX_WIDTH = 200;
        static const int TAB_PADDING = 16;
        static const int CLOSE_BUTTON_SIZE = 16;

        // 이벤트 핸들러
        TabChangedHandler m_tabChangedHandler;
        void* m_tabChangedContext;
        TabClosingHandler m_tabClosingHandler;
        void* m_tabClosingContext;
        TabClosedHandler m_tabClosedHandler;
        void* m_tabClosedContext;

        // 헬퍼
        void DrawTab(CDC* pDC, int index, const CRect& tabRect);
        void DrawCloseButton(CDC* pDC, const CRect& rect, BOOL hover);
        void DrawBadge(CDC* pDC, const CRect& tabRect, const CString& badge);
        int HitTest(CPoint point);
        int HitTestClose(CPoint point);
        CRect GetTabRect(int index);
        int CalculateTabWidth(int index);
        void UpdateScrollRange();
        void CloseTab(int index);

        // Direct2D
        void DrawTabD2D(int index, const CRect& tabRect);
        void DrawCloseButtonD2D(const CRect& rect, BOOL hover);
        void DrawBadgeD2D(const CRect& tabRect, const CString& badge);
    };
}
