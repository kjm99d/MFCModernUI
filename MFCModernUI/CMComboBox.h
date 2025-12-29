#pragma once

#include "CMControlBase.h"
#include <vector>

// MFC Modern UI - 드롭다운 콤보박스
// SOLID: 단일 책임 원칙 - 드롭다운 선택만 담당

namespace MFCModernUI
{
    // 콤보박스 항목
    struct ComboBoxItem
    {
        CString text;
        DWORD_PTR data;
        UINT iconId;
        BOOL disabled;
        BOOL separator;

        ComboBoxItem(const CString& t = _T(""), DWORD_PTR d = 0)
            : text(t), data(d), iconId(0), disabled(FALSE), separator(FALSE) {}
    };

    // 선택 변경 이벤트 핸들러
    typedef void (*ComboSelectionChangedHandler)(void* context, int oldIndex, int newIndex);

    class CMComboDropdown;  // 전방 선언

    class CMComboBox : public CMControlBase
    {
        DECLARE_DYNAMIC(CMComboBox)
        friend class CMComboDropdown;  // friend 선언

    public:
        CMComboBox();
        virtual ~CMComboBox();

        // 생성
        BOOL Create(DWORD dwStyle, const CRect& rect, CWnd* pParentWnd, UINT nID);

        // 항목 관리
        int AddItem(const CString& text, DWORD_PTR data = 0);
        int InsertItem(int index, const CString& text, DWORD_PTR data = 0);
        void RemoveItem(int index);
        void RemoveAllItems();
        int GetItemCount() const { return static_cast<int>(m_items.size()); }

        // 항목 접근
        CString GetItemText(int index) const;
        DWORD_PTR GetItemData(int index) const;
        void SetItemText(int index, const CString& text);
        void SetItemData(int index, DWORD_PTR data);
        void SetItemDisabled(int index, BOOL disabled);
        void AddSeparator();

        // 선택
        void SetSelectedIndex(int index);
        int GetSelectedIndex() const { return m_selectedIndex; }
        CString GetSelectedText() const;
        DWORD_PTR GetSelectedData() const;

        // 플레이스홀더
        void SetPlaceholder(const CString& placeholder);
        const CString& GetPlaceholder() const { return m_placeholder; }

        // 드롭다운
        void ShowDropdown();
        void HideDropdown();
        BOOL IsDropdownVisible() const { return m_dropdownVisible; }

        // 최대 높이
        void SetMaxDropdownHeight(int height);
        int GetMaxDropdownHeight() const { return m_maxDropdownHeight; }

        // 이벤트 핸들러
        void SetSelectionChangedHandler(ComboSelectionChangedHandler handler, void* context);

    protected:
        virtual void OnDraw(CDC* pDC) override;

        afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
        afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
        afx_msg void OnKillFocus(CWnd* pNewWnd);

        DECLARE_MESSAGE_MAP()

    private:
        // 항목
        std::vector<ComboBoxItem> m_items;
        int m_selectedIndex;
        CString m_placeholder;

        // 드롭다운
        BOOL m_dropdownVisible;
        int m_maxDropdownHeight;
        int m_hoverIndex;
        CRect m_dropdownRect;

        // 드롭다운 윈도우
        CWnd* m_pDropdownWnd;

        // 이벤트 핸들러
        ComboSelectionChangedHandler m_selectionChangedHandler;
        void* m_selectionChangedContext;

        // 헬퍼
        void DrawMainControl(CDC* pDC);
        void DrawDropdownArrow(CDC* pDC, const CRect& rect);
        void CreateDropdownWindow();
        void PositionDropdown();
        int GetItemHeight() const { return 32; }
        void SelectItem(int index);

        // Direct2D
        void OnDrawGdiPlus(CDC* pDC);
        void DrawMainControlD2D();
        void DrawDropdownArrowD2D(const CRect& rect);
    };

    // 드롭다운 윈도우 클래스
    class CMComboDropdown : public CWnd
    {
    public:
        CMComboDropdown(CMComboBox* pOwner);
        virtual ~CMComboDropdown();

        BOOL Create(CWnd* pParentWnd);
        void UpdateItems();

    protected:
        afx_msg void OnPaint();
        afx_msg void OnMouseMove(UINT nFlags, CPoint point);
        afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
        afx_msg void OnMouseLeave();
        afx_msg BOOL OnEraseBkgnd(CDC* pDC);

        DECLARE_MESSAGE_MAP()

    private:
        CMComboBox* m_pOwner;
        int m_hoverIndex;
        BOOL m_mouseTracking;

        int HitTest(CPoint point);
    };
}
