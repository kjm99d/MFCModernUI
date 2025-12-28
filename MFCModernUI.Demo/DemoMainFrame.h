#pragma once

#include "pch.h"

// Demo Page IDs
enum class DemoPage
{
    Buttons,
    Inputs,
    Selection,
    DataDisplay,
    Feedback,
    DateTime,
    Theme
};

// Demo Main Frame - 탭으로 각 컴포넌트 데모를 표시하는 메인 윈도우
class CDemoMainFrame : public CFrameWnd
{
    DECLARE_DYNAMIC(CDemoMainFrame)

public:
    CDemoMainFrame();
    virtual ~CDemoMainFrame();

    BOOL Create();

protected:
    // 사이드바 (탭 역할)
    CMPanel m_sidebar;
    CMLabel m_titleLabel;
    CMButton m_btnButtons;
    CMButton m_btnInputs;
    CMButton m_btnSelection;
    CMButton m_btnDataDisplay;
    CMButton m_btnFeedback;
    CMButton m_btnDateTime;
    CMButton m_btnTheme;

    // 테마 토글
    CMToggleSwitch m_themeToggle;
    CMLabel m_themeLabel;

    // 컨텐츠 영역
    CMPanel m_contentPanel;

    // 현재 페이지
    DemoPage m_currentPage;

    // 버튼 데모 컨트롤
    CMLabel m_btnDemoTitle;
    CMButton m_primaryBtn;
    CMButton m_secondaryBtn;
    CMButton m_successBtn;
    CMButton m_dangerBtn;
    CMButton m_warningBtn;
    CMButton m_disabledBtn;
    CMLabel m_btnSizeLabel;
    CMButton m_smallBtn;
    CMButton m_mediumBtn;
    CMButton m_largeBtn;

    // 입력 데모 컨트롤
    CMLabel m_inputDemoTitle;
    CMLabel m_editLabel;
    CMEdit m_edit;
    CMEdit m_editPlaceholder;
    CMEdit m_editPassword;
    CMEdit m_editReadonly;
    CMLabel m_comboLabel;
    CMComboBox m_combo;
    CMLabel m_sliderLabel;
    CMSlider m_slider;
    CMSpinner m_spinner;

    // 선택 데모 컨트롤
    CMLabel m_selectionDemoTitle;
    CMCheckBox m_check1;
    CMCheckBox m_check2;
    CMCheckBox m_check3;
    CMRadioButton m_radio1;
    CMRadioButton m_radio2;
    CMRadioButton m_radio3;
    CMToggleSwitch m_toggle1;
    CMToggleSwitch m_toggle2;

    // 데이터 표시 데모 컨트롤
    CMLabel m_dataDemoTitle;
    CMProgressBar m_progressBar;
    CMProgressBar m_progressBarAnim;
    CMListView m_listView;

    // 피드백 데모 컨트롤
    CMLabel m_feedbackDemoTitle;
    CMButton m_infoNotifyBtn;
    CMButton m_successNotifyBtn;
    CMButton m_warningNotifyBtn;
    CMButton m_errorNotifyBtn;
    CMButton m_modalBtn;

    // 날짜/시간 데모 컨트롤
    CMLabel m_dateTimeDemoTitle;
    CMLabel m_datePickerLabel;
    CMDatePicker m_datePicker;
    CMLabel m_timePickerLabel;
    CMTimePicker m_timePicker;
    CMLabel m_colorPickerLabel;
    CMColorPicker m_colorPicker;

    // 테마 데모 컨트롤
    CMLabel m_themeDemoTitle;
    CMLabel m_themeDesc;
    CMPanel m_colorPreview;

    // 페이지 전환
    void ShowPage(DemoPage page);
    void CreateButtonsDemo();
    void CreateInputsDemo();
    void CreateSelectionDemo();
    void CreateDataDisplayDemo();
    void CreateFeedbackDemo();
    void CreateDateTimeDemo();
    void CreateThemeDemo();

    void HideAllDemoControls();
    void UpdateSidebarButtons(DemoPage page);
    void LayoutControls();

    // 이벤트 핸들러
    static void OnButtonsPageClick(void* context);
    static void OnInputsPageClick(void* context);
    static void OnSelectionPageClick(void* context);
    static void OnDataDisplayPageClick(void* context);
    static void OnFeedbackPageClick(void* context);
    static void OnDateTimePageClick(void* context);
    static void OnThemePageClick(void* context);
    static void OnThemeToggle(void* context, BOOL isOn);
    static void OnInfoNotifyClick(void* context);
    static void OnSuccessNotifyClick(void* context);
    static void OnWarningNotifyClick(void* context);
    static void OnErrorNotifyClick(void* context);
    static void OnModalBtnClick(void* context);
    static void OnSliderValueChanged(void* context, int oldValue, int newValue);

    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnPaint();
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
    afx_msg void OnDestroy();

    DECLARE_MESSAGE_MAP()

private:
    static const int SIDEBAR_WIDTH = 200;
    static const int PADDING = 16;
    BOOL m_initialized;
};
