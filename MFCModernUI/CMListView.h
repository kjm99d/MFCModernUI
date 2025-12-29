#pragma once

#include "CMControlBase.h"
#include <vector>

// MFC Modern UI - 모던 리스트 뷰
// SOLID: 단일 책임 원칙 - 리스트 표시/선택만 담당

namespace MFCModernUI
{
    // 열 정보
    struct ListViewColumn
    {
        CString header;
        int width;
        int minWidth;
        int maxWidth;
        BOOL resizable;
        BOOL sortable;
        TextAlign align;

        ListViewColumn(const CString& h = _T(""), int w = 100)
            : header(h), width(w), minWidth(50), maxWidth(500)
            , resizable(TRUE), sortable(TRUE), align(TextAlign::Left) {}
    };

    // 행 데이터
    struct ListViewItem
    {
        std::vector<CString> texts;
        DWORD_PTR data;
        BOOL checked;

        ListViewItem() : data(0), checked(FALSE) {}
    };

    // 이벤트 핸들러
    typedef void (*ListSelectionChangedHandler)(void* context);
    typedef void (*ListItemDoubleClickHandler)(void* context, int index);
    typedef void (*ListColumnClickHandler)(void* context, int column);

    class CMListView : public CMControlBase
    {
        DECLARE_DYNAMIC(CMListView)

    public:
        CMListView();
        virtual ~CMListView();

        // 생성
        BOOL Create(DWORD dwStyle, const CRect& rect, CWnd* pParentWnd, UINT nID);

        // 열 관리
        int AddColumn(const CString& header, int width = 100);
        void RemoveColumn(int index);
        void SetColumnWidth(int index, int width);
        int GetColumnCount() const { return static_cast<int>(m_columns.size()); }
        const ListViewColumn& GetColumn(int index) const { return m_columns[index]; }

        // 항목 관리
        int AddItem(const CString& text);
        int InsertItem(int index, const CString& text);
        void RemoveItem(int index);
        void RemoveAllItems();
        int GetItemCount() const { return static_cast<int>(m_items.size()); }

        // 서브아이템
        void SetItemText(int item, int subItem, const CString& text);
        CString GetItemText(int item, int subItem) const;
        void SetItemData(int item, DWORD_PTR data);
        DWORD_PTR GetItemData(int item) const;

        // 선택
        void SetSelectionMode(SelectionMode mode);
        SelectionMode GetSelectionMode() const { return m_selectionMode; }
        void SelectItem(int index, BOOL select = TRUE);
        void SelectAll();
        void ClearSelection();
        BOOL IsSelected(int index) const;
        int GetSelectedIndex() const;
        std::vector<int> GetSelectedIndices() const;

        // 체크박스
        void SetShowCheckboxes(BOOL show);
        BOOL GetShowCheckboxes() const { return m_showCheckboxes; }
        void SetItemChecked(int index, BOOL checked);
        BOOL IsItemChecked(int index) const;

        // 헤더
        void SetShowHeaders(BOOL show);
        BOOL GetShowHeaders() const { return m_showHeaders; }

        // 줄무늬
        void SetAlternateRows(BOOL alternate);
        BOOL GetAlternateRows() const { return m_alternateRows; }

        // 이벤트 핸들러
        void SetSelectionChangedHandler(ListSelectionChangedHandler handler, void* context);
        void SetItemDoubleClickHandler(ListItemDoubleClickHandler handler, void* context);
        void SetColumnClickHandler(ListColumnClickHandler handler, void* context);

    protected:
        virtual void OnDraw(CDC* pDC) override;

        afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
        afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
        afx_msg void OnMouseMove(UINT nFlags, CPoint point);
        afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
        afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
        afx_msg void OnSize(UINT nType, int cx, int cy);

        DECLARE_MESSAGE_MAP()

    private:
        // 열 및 항목
        std::vector<ListViewColumn> m_columns;
        std::vector<ListViewItem> m_items;
        std::vector<BOOL> m_selection;

        // 옵션
        SelectionMode m_selectionMode;
        BOOL m_showCheckboxes;
        BOOL m_showHeaders;
        BOOL m_alternateRows;

        // 스크롤
        int m_scrollY;
        int m_maxScrollY;

        // 호버
        int m_hoverIndex;

        // 상수
        static const int HEADER_HEIGHT = 36;
        static const int ROW_HEIGHT = 36;
        static const int CHECKBOX_SIZE = 18;

        // 이벤트 핸들러
        ListSelectionChangedHandler m_selectionChangedHandler;
        void* m_selectionChangedContext;
        ListItemDoubleClickHandler m_itemDoubleClickHandler;
        void* m_itemDoubleClickContext;
        ListColumnClickHandler m_columnClickHandler;
        void* m_columnClickContext;

        // 헬퍼
        void DrawHeader(CDC* pDC, const CRect& rect);
        void DrawItems(CDC* pDC, const CRect& rect);
        void DrawItem(CDC* pDC, int index, const CRect& itemRect);
        void UpdateScrollRange();
        int HitTest(CPoint point);
        int HitTestColumn(CPoint point);
        CRect GetItemRect(int index);

        // Direct2D
        void OnDrawGdiPlus(CDC* pDC);
        void DrawHeaderD2D(const CRect& rect);
        void DrawItemsD2D(const CRect& rect);
        void DrawItemD2D(int index, const CRect& itemRect);
    };
}
