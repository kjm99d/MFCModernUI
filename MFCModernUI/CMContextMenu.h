#pragma once

#include "CMTypes.h"
#include "CMThemeManager.h"

// MFC Modern UI - 컨텍스트 메뉴 컴포넌트
// SOLID: 단일 책임 원칙 - 팝업 메뉴 표시만 담당

namespace MFCModernUI
{
    // 메뉴 아이템 타입
    enum class MenuItemType
    {
        Normal,
        Separator,
        Submenu,
        Checkbox,
        Radio
    };

    // 메뉴 아이템
    struct CMMenuItem
    {
        UINT id;
        CString text;
        CString shortcut;
        MenuItemType type;
        BOOL enabled;
        BOOL checked;
        HICON hIcon;
        CArray<CMMenuItem>* submenu;

        CMMenuItem()
            : id(0)
            , type(MenuItemType::Normal)
            , enabled(TRUE)
            , checked(FALSE)
            , hIcon(nullptr)
            , submenu(nullptr)
        {}
    };

    // 메뉴 클릭 핸들러
    typedef void (*MenuItemClickHandler)(void* context, UINT itemId);

    class CMContextMenu : public CWnd
    {
        DECLARE_DYNAMIC(CMContextMenu)

    public:
        CMContextMenu();
        virtual ~CMContextMenu();

        // 메뉴 구성
        void AddItem(UINT id, LPCTSTR text, LPCTSTR shortcut = nullptr);
        void AddItem(UINT id, LPCTSTR text, HICON hIcon, LPCTSTR shortcut = nullptr);
        void AddSeparator();
        void AddCheckItem(UINT id, LPCTSTR text, BOOL checked = FALSE);
        void AddRadioItem(UINT id, LPCTSTR text, BOOL checked = FALSE);

        // 서브메뉴
        CMContextMenu* AddSubmenu(LPCTSTR text);

        // 아이템 상태 변경
        void EnableItem(UINT id, BOOL enable);
        void CheckItem(UINT id, BOOL check);
        void SetItemText(UINT id, LPCTSTR text);

        // 메뉴 삭제
        void Clear();

        // 메뉴 표시
        UINT TrackPopupMenu(CPoint point, CWnd* pParentWnd);
        void ShowAsync(CPoint point, CWnd* pParentWnd, MenuItemClickHandler handler, void* context);

    protected:
        BOOL Create(CWnd* pParentWnd);

        virtual void OnDraw(CDC* pDC);

        afx_msg void OnPaint();
        afx_msg BOOL OnEraseBkgnd(CDC* pDC);
        afx_msg void OnMouseMove(UINT nFlags, CPoint point);
        afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
        afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
        afx_msg void OnKillFocus(CWnd* pNewWnd);
        afx_msg void OnTimer(UINT_PTR nIDEvent);

        DECLARE_MESSAGE_MAP()

    private:
        // 메뉴 아이템
        CArray<CMMenuItem> m_items;

        // 서브메뉴 (소유)
        CArray<CMContextMenu*> m_submenus;

        // 상태
        int m_hoverIndex;
        int m_selectedIndex;
        CMContextMenu* m_activeSubmenu;
        CWnd* m_parentWnd;
        BOOL m_isSubmenu;

        // 비동기 모드
        MenuItemClickHandler m_clickHandler;
        void* m_clickContext;

        // 크기
        static const int ITEM_HEIGHT = 28;
        static const int SEPARATOR_HEIGHT = 9;
        static const int ICON_SIZE = 16;
        static const int PADDING_H = 12;
        static const int PADDING_V = 4;
        static const int SHORTCUT_GAP = 32;
        static const int SUBMENU_ARROW_WIDTH = 16;

        // 타이머
        static const UINT_PTR SUBMENU_TIMER_ID = 4001;
        UINT_PTR m_submenuTimerId;

        // 테마
        BOOL IsLightTheme() const;
        const ThemeColors& GetColors() const;

        // 헬퍼
        CSize CalculateSize();
        int ItemFromPoint(CPoint point);
        CRect GetItemRect(int index);
        void DrawItem(CDC* pDC, int index, const CRect& rect);
        void DrawSeparator(CDC* pDC, const CRect& rect);
        void ShowSubmenu(int index);
        void HideSubmenu();
        CMMenuItem* FindItem(UINT id);
        void Close(UINT resultId = 0);
    };
}
