#pragma once

#include "CMControlBase.h"

// MFC Modern UI - 데이터 그리드 컴포넌트
// SOLID: 단일 책임 원칙 - 테이블 형태의 데이터 표시만 담당

namespace MFCModernUI
{
    // 컬럼 정렬
    enum class ColumnSortOrder
    {
        None,
        Ascending,
        Descending
    };

    // 셀 타입
    enum class CellType
    {
        Text,
        Number,
        Checkbox,
        Button,
        ComboBox
    };

    // 그리드 컬럼
    struct CMGridColumn
    {
        CString header;
        int width;
        CellType type;
        BOOL sortable;
        BOOL resizable;
        BOOL editable;
        int textAlign;  // DT_LEFT, DT_CENTER, DT_RIGHT

        CMGridColumn()
            : width(100)
            , type(CellType::Text)
            , sortable(TRUE)
            , resizable(TRUE)
            , editable(FALSE)
            , textAlign(DT_LEFT)
        {}
    };

    // 그리드 셀
    struct CMGridCell
    {
        CString text;
        LPARAM data;
        BOOL checked;
        COLORREF textColor;
        COLORREF bgColor;
        BOOL useCustomColors;

        CMGridCell()
            : data(0)
            , checked(FALSE)
            , textColor(0)
            , bgColor(0)
            , useCustomColors(FALSE)
        {}
    };

    // 그리드 행
    struct CMGridRow
    {
        CArray<CMGridCell> cells;
        LPARAM data;
        BOOL selected;

        CMGridRow() : data(0), selected(FALSE) {}
    };

    // 이벤트 핸들러
    typedef void (*GridSelectionChangedHandler)(void* context, int row, int col);
    typedef void (*GridCellClickHandler)(void* context, int row, int col);
    typedef void (*GridCellDoubleClickHandler)(void* context, int row, int col);
    typedef void (*GridColumnSortHandler)(void* context, int col, ColumnSortOrder order);

    class CMDataGrid : public CMControlBase
    {
        DECLARE_DYNAMIC(CMDataGrid)

    public:
        CMDataGrid();
        virtual ~CMDataGrid();

        // 생성
        BOOL Create(DWORD dwStyle, const CRect& rect, CWnd* pParentWnd, UINT nID);

        // 컬럼 관리
        int AddColumn(LPCTSTR header, int width = 100, CellType type = CellType::Text);
        void SetColumn(int col, const CMGridColumn& column);
        CMGridColumn* GetColumn(int col);
        int GetColumnCount() const { return (int)m_columns.GetSize(); }
        void RemoveColumn(int col);
        void RemoveAllColumns();

        // 행 관리
        int AddRow();
        int InsertRow(int row);
        void RemoveRow(int row);
        void RemoveAllRows();
        int GetRowCount() const { return (int)m_rows.GetSize(); }

        // 셀 데이터
        void SetCellText(int row, int col, LPCTSTR text);
        CString GetCellText(int row, int col);
        void SetCellData(int row, int col, LPARAM data);
        LPARAM GetCellData(int row, int col);
        void SetCellChecked(int row, int col, BOOL checked);
        BOOL GetCellChecked(int row, int col);
        void SetCellColors(int row, int col, COLORREF textColor, COLORREF bgColor);

        // 행 데이터
        void SetRowData(int row, LPARAM data);
        LPARAM GetRowData(int row);

        // 선택
        void SetSelectedCell(int row, int col);
        int GetSelectedRow() const { return m_selectedRow; }
        int GetSelectedCol() const { return m_selectedCol; }
        void SetMultiSelect(BOOL multi);
        BOOL IsMultiSelect() const { return m_multiSelect; }

        // 옵션
        void SetShowHeader(BOOL show);
        BOOL GetShowHeader() const { return m_showHeader; }

        void SetShowGridLines(BOOL show);
        BOOL GetShowGridLines() const { return m_showGridLines; }

        void SetRowHeight(int height);
        int GetRowHeight() const { return m_rowHeight; }

        void SetHeaderHeight(int height);
        int GetHeaderHeight() const { return m_headerHeight; }

        void SetAlternateRowColors(BOOL alternate);
        BOOL GetAlternateRowColors() const { return m_alternateRowColors; }

        // 정렬
        void SortByColumn(int col, ColumnSortOrder order);
        int GetSortColumn() const { return m_sortColumn; }
        ColumnSortOrder GetSortOrder() const { return m_sortOrder; }

        // 이벤트 핸들러
        void SetSelectionChangedHandler(GridSelectionChangedHandler handler, void* context);
        void SetCellClickHandler(GridCellClickHandler handler, void* context);
        void SetCellDoubleClickHandler(GridCellDoubleClickHandler handler, void* context);
        void SetColumnSortHandler(GridColumnSortHandler handler, void* context);

        // 스크롤
        void EnsureVisible(int row);

    protected:
        virtual void OnDraw(CDC* pDC) override;

        afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
        afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
        afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
        afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
        afx_msg void OnSize(UINT nType, int cx, int cy);
        afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
        afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);

        DECLARE_MESSAGE_MAP()

    private:
        // 데이터
        CArray<CMGridColumn> m_columns;
        CArray<CMGridRow> m_rows;

        // 선택
        int m_selectedRow;
        int m_selectedCol;
        BOOL m_multiSelect;

        // 정렬
        int m_sortColumn;
        ColumnSortOrder m_sortOrder;

        // 옵션
        BOOL m_showHeader;
        BOOL m_showGridLines;
        int m_rowHeight;
        int m_headerHeight;
        BOOL m_alternateRowColors;

        // 스크롤
        int m_scrollX;
        int m_scrollY;
        int m_totalWidth;
        int m_totalHeight;

        // 이벤트 핸들러
        GridSelectionChangedHandler m_selectionChangedHandler;
        void* m_selectionChangedContext;
        GridCellClickHandler m_cellClickHandler;
        void* m_cellClickContext;
        GridCellDoubleClickHandler m_cellDoubleClickHandler;
        void* m_cellDoubleClickContext;
        GridColumnSortHandler m_columnSortHandler;
        void* m_columnSortContext;

        // 헬퍼
        void DrawHeader(CDC* pDC, const CRect& rect);
        void DrawRows(CDC* pDC, const CRect& rect);
        void DrawCell(CDC* pDC, int row, int col, const CRect& rect);
        void DrawSortIndicator(CDC* pDC, const CRect& rect, ColumnSortOrder order);

        int GetColumnX(int col);
        int HitTestColumn(CPoint point);
        int HitTestRow(CPoint point);
        CRect GetCellRect(int row, int col);
        void UpdateScrollBars();
        void CalculateTotalSize();
    };
}
