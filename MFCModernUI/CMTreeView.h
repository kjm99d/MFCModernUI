#pragma once

#include "CMControlBase.h"

// MFC Modern UI - 트리뷰 컴포넌트
// SOLID: 단일 책임 원칙 - 계층적 데이터 표시만 담당

namespace MFCModernUI
{
    // 트리 노드
    class CMTreeNode
    {
    public:
        CMTreeNode();
        ~CMTreeNode();

        // 데이터
        CString text;
        LPARAM data;
        HICON hIcon;

        // 상태
        BOOL expanded;
        BOOL selected;
        BOOL checked;

        // 계층
        CMTreeNode* parent;
        CArray<CMTreeNode*> children;

        // 헬퍼
        int GetLevel() const;
        BOOL HasChildren() const { return children.GetSize() > 0; }
        void RemoveAllChildren();
    };

    // 이벤트 핸들러
    typedef void (*TreeSelectionChangedHandler)(void* context, CMTreeNode* oldNode, CMTreeNode* newNode);
    typedef void (*TreeNodeExpandedHandler)(void* context, CMTreeNode* node, BOOL expanded);
    typedef void (*TreeNodeDoubleClickHandler)(void* context, CMTreeNode* node);

    class CMTreeView : public CMControlBase
    {
        DECLARE_DYNAMIC(CMTreeView)

    public:
        CMTreeView();
        virtual ~CMTreeView();

        // 생성
        BOOL Create(DWORD dwStyle, const CRect& rect, CWnd* pParentWnd, UINT nID);

        // 노드 관리
        CMTreeNode* AddNode(LPCTSTR text, CMTreeNode* parent = nullptr);
        CMTreeNode* AddNode(LPCTSTR text, HICON hIcon, CMTreeNode* parent = nullptr);
        CMTreeNode* InsertNode(LPCTSTR text, CMTreeNode* parent, int index);
        void RemoveNode(CMTreeNode* node);
        void RemoveAllNodes();

        // 노드 검색
        CMTreeNode* GetRootNode(int index = 0);
        int GetRootNodeCount() const;
        CMTreeNode* FindNode(LPARAM data);
        CMTreeNode* HitTest(CPoint point);

        // 선택
        void SetSelectedNode(CMTreeNode* node);
        CMTreeNode* GetSelectedNode() const { return m_selectedNode; }

        // 확장/축소
        void ExpandNode(CMTreeNode* node, BOOL expand = TRUE);
        void ExpandAll();
        void CollapseAll();

        // 옵션
        void SetShowCheckboxes(BOOL show);
        BOOL GetShowCheckboxes() const { return m_showCheckboxes; }

        void SetShowLines(BOOL show);
        BOOL GetShowLines() const { return m_showLines; }

        void SetShowRootLines(BOOL show);
        BOOL GetShowRootLines() const { return m_showRootLines; }

        void SetIndent(int indent);
        int GetIndent() const { return m_indent; }

        // 이벤트 핸들러
        void SetSelectionChangedHandler(TreeSelectionChangedHandler handler, void* context);
        void SetNodeExpandedHandler(TreeNodeExpandedHandler handler, void* context);
        void SetNodeDoubleClickHandler(TreeNodeDoubleClickHandler handler, void* context);

        // 스크롤
        void EnsureVisible(CMTreeNode* node);

    protected:
        virtual void OnDraw(CDC* pDC) override;

        afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
        afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
        afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
        afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
        afx_msg void OnSize(UINT nType, int cx, int cy);

        DECLARE_MESSAGE_MAP()

    private:
        // 루트 노드들
        CArray<CMTreeNode*> m_rootNodes;

        // 상태
        CMTreeNode* m_selectedNode;
        CMTreeNode* m_hoverNode;

        // 옵션
        BOOL m_showCheckboxes;
        BOOL m_showLines;
        BOOL m_showRootLines;
        int m_indent;

        // 스크롤
        int m_scrollY;
        int m_totalHeight;

        // 상수
        static const int ITEM_HEIGHT = 24;
        static const int ICON_SIZE = 16;
        static const int EXPANDER_SIZE = 16;
        static const int CHECKBOX_SIZE = 16;

        // 이벤트 핸들러
        TreeSelectionChangedHandler m_selectionChangedHandler;
        void* m_selectionChangedContext;
        TreeNodeExpandedHandler m_nodeExpandedHandler;
        void* m_nodeExpandedContext;
        TreeNodeDoubleClickHandler m_nodeDoubleClickHandler;
        void* m_nodeDoubleClickContext;

        // 헬퍼
        void DrawNode(CDC* pDC, CMTreeNode* node, int& y, int level);
        void DrawExpander(CDC* pDC, const CRect& rect, BOOL expanded, BOOL hovered);
        void DrawCheckbox(CDC* pDC, const CRect& rect, BOOL checked);
        void DrawNodeLines(CDC* pDC, CMTreeNode* node, const CRect& rect, int level);

        int CalculateTotalHeight();
        int CalculateNodeHeight(CMTreeNode* node);
        CMTreeNode* HitTestNode(CMTreeNode* node, CPoint point, int& y, int level);
        int GetNodeX(int level);
        void UpdateScrollBars();
        void CollectVisibleNodes(CArray<CMTreeNode*>& nodes, CMTreeNode* parent = nullptr);
        CMTreeNode* FindNodeByData(CMTreeNode* node, LPARAM data);
    };
}
