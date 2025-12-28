#include "pch.h"
#include "DemoMainFrame.h"

IMPLEMENT_DYNAMIC(CDemoMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CDemoMainFrame, CFrameWnd)
    ON_WM_CREATE()
    ON_WM_SIZE()
    ON_WM_PAINT()
    ON_WM_ERASEBKGND()
    ON_WM_DESTROY()
END_MESSAGE_MAP()

CDemoMainFrame::CDemoMainFrame()
    : m_currentPage(DemoPage::Buttons)
    , m_initialized(FALSE)
{
}

CDemoMainFrame::~CDemoMainFrame()
{
}

BOOL CDemoMainFrame::Create()
{
    CString className = AfxRegisterWndClass(
        CS_HREDRAW | CS_VREDRAW,
        LoadCursor(NULL, IDC_ARROW),
        (HBRUSH)GetStockObject(WHITE_BRUSH),
        LoadIcon(NULL, IDI_APPLICATION)
    );

    return CFrameWnd::Create(
        className,
        _T("MFC Modern UI Demo"),
        WS_OVERLAPPEDWINDOW,
        CRect(100, 100, 1200, 800)
    );
}

int CDemoMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
        return -1;

    // 사이드바 패널
    m_sidebar.Create(WS_CHILD | WS_VISIBLE, CRect(0, 0, SIDEBAR_WIDTH, 600), this, 1001);

    // 타이틀 라벨
    m_titleLabel.Create(_T("MFC Modern UI"), WS_CHILD | WS_VISIBLE, CRect(16, 20, SIDEBAR_WIDTH - 16, 50), &m_sidebar, 1002);
    m_titleLabel.SetTextStyle(TextStyle::H3);

    // 사이드바 버튼들
    int btnY = 70;
    const int btnHeight = 40;
    const int btnSpacing = 4;

    m_btnButtons.Create(_T("Buttons"), WS_CHILD | WS_VISIBLE, CRect(8, btnY, SIDEBAR_WIDTH - 8, btnY + btnHeight), &m_sidebar, 1010);
    m_btnButtons.SetButtonStyle(ButtonStyle::Ghost);
    m_btnButtons.SetClickHandler(OnButtonsPageClick, this);
    btnY += btnHeight + btnSpacing;

    m_btnInputs.Create(_T("Inputs"), WS_CHILD | WS_VISIBLE, CRect(8, btnY, SIDEBAR_WIDTH - 8, btnY + btnHeight), &m_sidebar, 1011);
    m_btnInputs.SetButtonStyle(ButtonStyle::Ghost);
    m_btnInputs.SetClickHandler(OnInputsPageClick, this);
    btnY += btnHeight + btnSpacing;

    m_btnSelection.Create(_T("Selection"), WS_CHILD | WS_VISIBLE, CRect(8, btnY, SIDEBAR_WIDTH - 8, btnY + btnHeight), &m_sidebar, 1012);
    m_btnSelection.SetButtonStyle(ButtonStyle::Ghost);
    m_btnSelection.SetClickHandler(OnSelectionPageClick, this);
    btnY += btnHeight + btnSpacing;

    m_btnDataDisplay.Create(_T("Data Display"), WS_CHILD | WS_VISIBLE, CRect(8, btnY, SIDEBAR_WIDTH - 8, btnY + btnHeight), &m_sidebar, 1013);
    m_btnDataDisplay.SetButtonStyle(ButtonStyle::Ghost);
    m_btnDataDisplay.SetClickHandler(OnDataDisplayPageClick, this);
    btnY += btnHeight + btnSpacing;

    m_btnFeedback.Create(_T("Feedback"), WS_CHILD | WS_VISIBLE, CRect(8, btnY, SIDEBAR_WIDTH - 8, btnY + btnHeight), &m_sidebar, 1014);
    m_btnFeedback.SetButtonStyle(ButtonStyle::Ghost);
    m_btnFeedback.SetClickHandler(OnFeedbackPageClick, this);
    btnY += btnHeight + btnSpacing;

    m_btnDateTime.Create(_T("Date & Time"), WS_CHILD | WS_VISIBLE, CRect(8, btnY, SIDEBAR_WIDTH - 8, btnY + btnHeight), &m_sidebar, 1015);
    m_btnDateTime.SetButtonStyle(ButtonStyle::Ghost);
    m_btnDateTime.SetClickHandler(OnDateTimePageClick, this);
    btnY += btnHeight + btnSpacing;

    m_btnTheme.Create(_T("Theme"), WS_CHILD | WS_VISIBLE, CRect(8, btnY, SIDEBAR_WIDTH - 8, btnY + btnHeight), &m_sidebar, 1016);
    m_btnTheme.SetButtonStyle(ButtonStyle::Ghost);
    m_btnTheme.SetClickHandler(OnThemePageClick, this);

    // 테마 토글 (사이드바 하단)
    m_themeLabel.Create(_T("Dark Mode"), WS_CHILD | WS_VISIBLE, CRect(16, 500, 100, 520), &m_sidebar, 1020);
    m_themeToggle.Create(WS_CHILD | WS_VISIBLE, CRect(120, 496, 180, 524), &m_sidebar, 1021);
    m_themeToggle.SetToggleChangedHandler(OnThemeToggle, this);

    // 컨텐츠 패널
    m_contentPanel.Create(WS_CHILD | WS_VISIBLE, CRect(SIDEBAR_WIDTH, 0, 1000, 600), this, 1030);

    // 데모 컨트롤들 생성
    CreateButtonsDemo();
    CreateInputsDemo();
    CreateSelectionDemo();
    CreateDataDisplayDemo();
    CreateFeedbackDemo();
    CreateDateTimeDemo();
    CreateThemeDemo();

    // 초기 페이지 표시
    m_initialized = TRUE;
    ShowPage(DemoPage::Buttons);

    return 0;
}

void CDemoMainFrame::CreateButtonsDemo()
{
    int y = PADDING;
    const int spacing = 12;
    const int btnWidth = 120;
    const int btnHeight = 36;

    // 타이틀
    m_btnDemoTitle.Create(_T("Button Styles"), WS_CHILD, CRect(PADDING, y, 400, y + 30), &m_contentPanel, 2000);
    m_btnDemoTitle.SetTextStyle(TextStyle::H4);
    y += 40;

    // 버튼 스타일
    int x = PADDING;
    m_primaryBtn.Create(_T("Primary"), WS_CHILD, CRect(x, y, x + btnWidth, y + btnHeight), &m_contentPanel, 2001);
    m_primaryBtn.SetButtonStyle(ButtonStyle::Primary);
    x += btnWidth + spacing;

    m_secondaryBtn.Create(_T("Secondary"), WS_CHILD, CRect(x, y, x + btnWidth, y + btnHeight), &m_contentPanel, 2002);
    m_secondaryBtn.SetButtonStyle(ButtonStyle::Secondary);
    x += btnWidth + spacing;

    m_successBtn.Create(_T("Success"), WS_CHILD, CRect(x, y, x + btnWidth, y + btnHeight), &m_contentPanel, 2003);
    m_successBtn.SetButtonStyle(ButtonStyle::Success);
    x += btnWidth + spacing;

    m_dangerBtn.Create(_T("Danger"), WS_CHILD, CRect(x, y, x + btnWidth, y + btnHeight), &m_contentPanel, 2004);
    m_dangerBtn.SetButtonStyle(ButtonStyle::Danger);
    x += btnWidth + spacing;

    m_warningBtn.Create(_T("Warning"), WS_CHILD, CRect(x, y, x + btnWidth, y + btnHeight), &m_contentPanel, 2005);
    m_warningBtn.SetButtonStyle(ButtonStyle::Warning);
    y += btnHeight + spacing * 2;

    // Disabled 버튼
    m_disabledBtn.Create(_T("Disabled"), WS_CHILD, CRect(PADDING, y, PADDING + btnWidth, y + btnHeight), &m_contentPanel, 2006);
    m_disabledBtn.SetButtonStyle(ButtonStyle::Primary);
    m_disabledBtn.SetEnabled(FALSE);
    y += btnHeight + spacing * 3;

    // 크기 라벨
    m_btnSizeLabel.Create(_T("Button Sizes"), WS_CHILD, CRect(PADDING, y, 400, y + 30), &m_contentPanel, 2010);
    m_btnSizeLabel.SetTextStyle(TextStyle::H4);
    y += 40;

    // 크기별 버튼
    x = PADDING;
    m_smallBtn.Create(_T("Small"), WS_CHILD, CRect(x, y, x + 80, y + 28), &m_contentPanel, 2011);
    m_smallBtn.SetButtonStyle(ButtonStyle::Primary);
    m_smallBtn.SetControlSize(ControlSize::Small);
    x += 80 + spacing;

    m_mediumBtn.Create(_T("Medium"), WS_CHILD, CRect(x, y, x + btnWidth, y + btnHeight), &m_contentPanel, 2012);
    m_mediumBtn.SetButtonStyle(ButtonStyle::Primary);
    m_mediumBtn.SetControlSize(ControlSize::Medium);
    x += btnWidth + spacing;

    m_largeBtn.Create(_T("Large"), WS_CHILD, CRect(x, y, x + 140, y + 44), &m_contentPanel, 2013);
    m_largeBtn.SetButtonStyle(ButtonStyle::Primary);
    m_largeBtn.SetControlSize(ControlSize::Large);
}

void CDemoMainFrame::CreateInputsDemo()
{
    int y = PADDING;
    const int spacing = 12;
    const int controlWidth = 300;
    const int labelWidth = 120;

    // 타이틀
    m_inputDemoTitle.Create(_T("Input Controls"), WS_CHILD, CRect(PADDING, y, 400, y + 30), &m_contentPanel, 3000);
    m_inputDemoTitle.SetTextStyle(TextStyle::H4);
    y += 40;

    // Edit 컨트롤들
    m_editLabel.Create(_T("Text Input:"), WS_CHILD, CRect(PADDING, y + 6, PADDING + labelWidth, y + 26), &m_contentPanel, 3001);

    m_edit.Create(WS_CHILD, CRect(PADDING + labelWidth, y, PADDING + labelWidth + controlWidth, y + 32), &m_contentPanel, 3002);
    y += 44;

    m_editPlaceholder.Create(WS_CHILD, CRect(PADDING + labelWidth, y, PADDING + labelWidth + controlWidth, y + 32), &m_contentPanel, 3003);
    m_editPlaceholder.SetPlaceholder(_T("Enter your email..."));
    y += 44;

    m_editPassword.Create(WS_CHILD, CRect(PADDING + labelWidth, y, PADDING + labelWidth + controlWidth, y + 32), &m_contentPanel, 3004);
    m_editPassword.SetPassword(TRUE);
    m_editPassword.SetPlaceholder(_T("Password"));
    y += 44;

    m_editReadonly.Create(WS_CHILD, CRect(PADDING + labelWidth, y, PADDING + labelWidth + controlWidth, y + 32), &m_contentPanel, 3005);
    m_editReadonly.SetText(_T("Read-only text"));
    m_editReadonly.SetReadOnly(TRUE);
    y += 56;

    // ComboBox
    m_comboLabel.Create(_T("ComboBox:"), WS_CHILD, CRect(PADDING, y + 6, PADDING + labelWidth, y + 26), &m_contentPanel, 3010);

    m_combo.Create(WS_CHILD, CRect(PADDING + labelWidth, y, PADDING + labelWidth + controlWidth, y + 32), &m_contentPanel, 3011);
    m_combo.AddItem(_T("Option 1"));
    m_combo.AddItem(_T("Option 2"));
    m_combo.AddItem(_T("Option 3"));
    m_combo.AddItem(_T("Option 4"));
    m_combo.SetSelectedIndex(0);
    y += 56;

    // Slider
    m_sliderLabel.Create(_T("Slider:"), WS_CHILD, CRect(PADDING, y + 6, PADDING + labelWidth, y + 26), &m_contentPanel, 3020);

    m_slider.Create(WS_CHILD, CRect(PADDING + labelWidth, y, PADDING + labelWidth + controlWidth, y + 32), &m_contentPanel, 3021);
    m_slider.SetRange(0, 100);
    m_slider.SetValue(50);
    // SetValueChangedHandler는 (oldValue, newValue) 시그니처 사용
    y += 56;

    // Spinner
    m_spinner.Create(WS_CHILD, CRect(PADDING + labelWidth, y, PADDING + labelWidth + 150, y + 32), &m_contentPanel, 3030);
    m_spinner.SetRange(0, 100);
    m_spinner.SetValue(25);
}

void CDemoMainFrame::CreateSelectionDemo()
{
    int y = PADDING;
    const int spacing = 12;

    // 타이틀
    m_selectionDemoTitle.Create(_T("Selection Controls"), WS_CHILD, CRect(PADDING, y, 400, y + 30), &m_contentPanel, 4000);
    m_selectionDemoTitle.SetTextStyle(TextStyle::H4);
    y += 50;

    // CheckBox
    m_check1.Create(_T("Option A - Checked"), WS_CHILD, CRect(PADDING, y, 300, y + 24), &m_contentPanel, 4001);
    m_check1.SetChecked(TRUE);
    y += 32;

    m_check2.Create(_T("Option B - Unchecked"), WS_CHILD, CRect(PADDING, y, 300, y + 24), &m_contentPanel, 4002);
    y += 32;

    m_check3.Create(_T("Option C - Disabled"), WS_CHILD, CRect(PADDING, y, 300, y + 24), &m_contentPanel, 4003);
    m_check3.SetEnabled(FALSE);
    y += 50;

    // RadioButton (groupId는 Create의 마지막 파라미터로 전달)
    m_radio1.Create(_T("Choice 1"), WS_CHILD, CRect(PADDING, y, 200, y + 24), &m_contentPanel, 4010, 1);
    m_radio1.SetSelected(TRUE);
    y += 32;

    m_radio2.Create(_T("Choice 2"), WS_CHILD, CRect(PADDING, y, 200, y + 24), &m_contentPanel, 4011, 1);
    y += 32;

    m_radio3.Create(_T("Choice 3"), WS_CHILD, CRect(PADDING, y, 200, y + 24), &m_contentPanel, 4012, 1);
    y += 50;

    // ToggleSwitch
    m_toggle1.Create(WS_CHILD, CRect(PADDING, y, 200, y + 28), &m_contentPanel, 4020);
    m_toggle1.SetChecked(TRUE);
    y += 40;

    m_toggle2.Create(WS_CHILD, CRect(PADDING, y, 200, y + 28), &m_contentPanel, 4021);
    m_toggle2.SetChecked(FALSE);
}

void CDemoMainFrame::CreateDataDisplayDemo()
{
    int y = PADDING;

    // 타이틀
    m_dataDemoTitle.Create(_T("Data Display"), WS_CHILD, CRect(PADDING, y, 400, y + 30), &m_contentPanel, 5000);
    m_dataDemoTitle.SetTextStyle(TextStyle::H4);
    y += 50;

    // ProgressBar
    m_progressBar.Create(WS_CHILD, CRect(PADDING, y, 400, y + 20), &m_contentPanel, 5001);
    m_progressBar.SetValue(65);
    y += 40;

    m_progressBarAnim.Create(WS_CHILD, CRect(PADDING, y, 400, y + 20), &m_contentPanel, 5002);
    m_progressBarAnim.SetIndeterminate(TRUE);
    y += 50;

    // ListView
    m_listView.Create(WS_CHILD, CRect(PADDING, y, 500, y + 200), &m_contentPanel, 5010);
    m_listView.AddItem(_T("Item 1 - First item in the list"));
    m_listView.AddItem(_T("Item 2 - Second item"));
    m_listView.AddItem(_T("Item 3 - Third item"));
    m_listView.AddItem(_T("Item 4 - Fourth item"));
    m_listView.AddItem(_T("Item 5 - Fifth item"));
}

void CDemoMainFrame::CreateFeedbackDemo()
{
    int y = PADDING;
    const int btnWidth = 150;
    const int btnHeight = 36;
    const int spacing = 12;

    // 타이틀
    m_feedbackDemoTitle.Create(_T("Feedback Components"), WS_CHILD, CRect(PADDING, y, 400, y + 30), &m_contentPanel, 6000);
    m_feedbackDemoTitle.SetTextStyle(TextStyle::H4);
    y += 50;

    // Notification 버튼들
    int x = PADDING;
    m_infoNotifyBtn.Create(_T("Info"), WS_CHILD, CRect(x, y, x + btnWidth, y + btnHeight), &m_contentPanel, 6001);
    m_infoNotifyBtn.SetButtonStyle(ButtonStyle::Primary);
    m_infoNotifyBtn.SetClickHandler(OnInfoNotifyClick, this);
    x += btnWidth + spacing;

    m_successNotifyBtn.Create(_T("Success"), WS_CHILD, CRect(x, y, x + btnWidth, y + btnHeight), &m_contentPanel, 6002);
    m_successNotifyBtn.SetButtonStyle(ButtonStyle::Success);
    m_successNotifyBtn.SetClickHandler(OnSuccessNotifyClick, this);
    x += btnWidth + spacing;

    m_warningNotifyBtn.Create(_T("Warning"), WS_CHILD, CRect(x, y, x + btnWidth, y + btnHeight), &m_contentPanel, 6003);
    m_warningNotifyBtn.SetButtonStyle(ButtonStyle::Warning);
    m_warningNotifyBtn.SetClickHandler(OnWarningNotifyClick, this);
    x += btnWidth + spacing;

    m_errorNotifyBtn.Create(_T("Error"), WS_CHILD, CRect(x, y, x + btnWidth, y + btnHeight), &m_contentPanel, 6004);
    m_errorNotifyBtn.SetButtonStyle(ButtonStyle::Danger);
    m_errorNotifyBtn.SetClickHandler(OnErrorNotifyClick, this);
    y += btnHeight + spacing * 3;

    // Modal 버튼
    m_modalBtn.Create(_T("Show Modal Dialog"), WS_CHILD, CRect(PADDING, y, PADDING + 200, y + btnHeight), &m_contentPanel, 6010);
    m_modalBtn.SetButtonStyle(ButtonStyle::Secondary);
    m_modalBtn.SetClickHandler(OnModalBtnClick, this);
}

void CDemoMainFrame::CreateDateTimeDemo()
{
    int y = PADDING;
    const int labelWidth = 120;
    const int controlWidth = 200;

    // 타이틀
    m_dateTimeDemoTitle.Create(_T("Date, Time && Color Pickers"), WS_CHILD, CRect(PADDING, y, 400, y + 30), &m_contentPanel, 7000);
    m_dateTimeDemoTitle.SetTextStyle(TextStyle::H4);
    y += 50;

    // DatePicker
    m_datePickerLabel.Create(_T("Date:"), WS_CHILD, CRect(PADDING, y + 6, PADDING + labelWidth, y + 26), &m_contentPanel, 7001);
    m_datePicker.Create(WS_CHILD, CRect(PADDING + labelWidth, y, PADDING + labelWidth + controlWidth, y + 32), &m_contentPanel, 7002);
    y += 50;

    // TimePicker
    m_timePickerLabel.Create(_T("Time:"), WS_CHILD, CRect(PADDING, y + 6, PADDING + labelWidth, y + 26), &m_contentPanel, 7010);
    m_timePicker.Create(WS_CHILD, CRect(PADDING + labelWidth, y, PADDING + labelWidth + controlWidth, y + 32), &m_contentPanel, 7011);
    y += 50;

    // ColorPicker
    m_colorPickerLabel.Create(_T("Color:"), WS_CHILD, CRect(PADDING, y + 6, PADDING + labelWidth, y + 26), &m_contentPanel, 7020);
    m_colorPicker.Create(WS_CHILD, CRect(PADDING + labelWidth, y, PADDING + labelWidth + 150, y + 32), &m_contentPanel, 7021);
}

void CDemoMainFrame::CreateThemeDemo()
{
    int y = PADDING;

    // 타이틀
    m_themeDemoTitle.Create(_T("Theme System"), WS_CHILD, CRect(PADDING, y, 400, y + 30), &m_contentPanel, 8000);
    m_themeDemoTitle.SetTextStyle(TextStyle::H4);
    y += 50;

    // 설명
    m_themeDesc.Create(_T("Toggle dark mode using the switch in the sidebar. All components will automatically update their appearance."),
        WS_CHILD, CRect(PADDING, y, 600, y + 60), &m_contentPanel, 8001);
    m_themeDesc.SetWordWrap(TRUE);
    y += 80;

    // 색상 미리보기 패널
    m_colorPreview.Create(WS_CHILD, CRect(PADDING, y, 300, y + 150), &m_contentPanel, 8010);
}

void CDemoMainFrame::ShowPage(DemoPage page)
{
    m_currentPage = page;
    HideAllDemoControls();
    UpdateSidebarButtons(page);

    switch (page)
    {
    case DemoPage::Buttons:
        m_btnDemoTitle.ShowWindow(SW_SHOW);
        m_primaryBtn.ShowWindow(SW_SHOW);
        m_secondaryBtn.ShowWindow(SW_SHOW);
        m_successBtn.ShowWindow(SW_SHOW);
        m_dangerBtn.ShowWindow(SW_SHOW);
        m_warningBtn.ShowWindow(SW_SHOW);
        m_disabledBtn.ShowWindow(SW_SHOW);
        m_btnSizeLabel.ShowWindow(SW_SHOW);
        m_smallBtn.ShowWindow(SW_SHOW);
        m_mediumBtn.ShowWindow(SW_SHOW);
        m_largeBtn.ShowWindow(SW_SHOW);
        break;

    case DemoPage::Inputs:
        m_inputDemoTitle.ShowWindow(SW_SHOW);
        m_editLabel.ShowWindow(SW_SHOW);
        m_edit.ShowWindow(SW_SHOW);
        m_editPlaceholder.ShowWindow(SW_SHOW);
        m_editPassword.ShowWindow(SW_SHOW);
        m_editReadonly.ShowWindow(SW_SHOW);
        m_comboLabel.ShowWindow(SW_SHOW);
        m_combo.ShowWindow(SW_SHOW);
        m_sliderLabel.ShowWindow(SW_SHOW);
        m_slider.ShowWindow(SW_SHOW);
        m_spinner.ShowWindow(SW_SHOW);
        break;

    case DemoPage::Selection:
        m_selectionDemoTitle.ShowWindow(SW_SHOW);
        m_check1.ShowWindow(SW_SHOW);
        m_check2.ShowWindow(SW_SHOW);
        m_check3.ShowWindow(SW_SHOW);
        m_radio1.ShowWindow(SW_SHOW);
        m_radio2.ShowWindow(SW_SHOW);
        m_radio3.ShowWindow(SW_SHOW);
        m_toggle1.ShowWindow(SW_SHOW);
        m_toggle2.ShowWindow(SW_SHOW);
        break;

    case DemoPage::DataDisplay:
        m_dataDemoTitle.ShowWindow(SW_SHOW);
        m_progressBar.ShowWindow(SW_SHOW);
        m_progressBarAnim.ShowWindow(SW_SHOW);
        m_listView.ShowWindow(SW_SHOW);
        break;

    case DemoPage::Feedback:
        m_feedbackDemoTitle.ShowWindow(SW_SHOW);
        m_infoNotifyBtn.ShowWindow(SW_SHOW);
        m_successNotifyBtn.ShowWindow(SW_SHOW);
        m_warningNotifyBtn.ShowWindow(SW_SHOW);
        m_errorNotifyBtn.ShowWindow(SW_SHOW);
        m_modalBtn.ShowWindow(SW_SHOW);
        break;

    case DemoPage::DateTime:
        m_dateTimeDemoTitle.ShowWindow(SW_SHOW);
        m_datePickerLabel.ShowWindow(SW_SHOW);
        m_datePicker.ShowWindow(SW_SHOW);
        m_timePickerLabel.ShowWindow(SW_SHOW);
        m_timePicker.ShowWindow(SW_SHOW);
        m_colorPickerLabel.ShowWindow(SW_SHOW);
        m_colorPicker.ShowWindow(SW_SHOW);
        break;

    case DemoPage::Theme:
        m_themeDemoTitle.ShowWindow(SW_SHOW);
        m_themeDesc.ShowWindow(SW_SHOW);
        m_colorPreview.ShowWindow(SW_SHOW);
        break;
    }

    m_contentPanel.Invalidate();
}

void CDemoMainFrame::HideAllDemoControls()
{
    // Buttons
    m_btnDemoTitle.ShowWindow(SW_HIDE);
    m_primaryBtn.ShowWindow(SW_HIDE);
    m_secondaryBtn.ShowWindow(SW_HIDE);
    m_successBtn.ShowWindow(SW_HIDE);
    m_dangerBtn.ShowWindow(SW_HIDE);
    m_warningBtn.ShowWindow(SW_HIDE);
    m_disabledBtn.ShowWindow(SW_HIDE);
    m_btnSizeLabel.ShowWindow(SW_HIDE);
    m_smallBtn.ShowWindow(SW_HIDE);
    m_mediumBtn.ShowWindow(SW_HIDE);
    m_largeBtn.ShowWindow(SW_HIDE);

    // Inputs
    m_inputDemoTitle.ShowWindow(SW_HIDE);
    m_editLabel.ShowWindow(SW_HIDE);
    m_edit.ShowWindow(SW_HIDE);
    m_editPlaceholder.ShowWindow(SW_HIDE);
    m_editPassword.ShowWindow(SW_HIDE);
    m_editReadonly.ShowWindow(SW_HIDE);
    m_comboLabel.ShowWindow(SW_HIDE);
    m_combo.ShowWindow(SW_HIDE);
    m_sliderLabel.ShowWindow(SW_HIDE);
    m_slider.ShowWindow(SW_HIDE);
    m_spinner.ShowWindow(SW_HIDE);

    // Selection
    m_selectionDemoTitle.ShowWindow(SW_HIDE);
    m_check1.ShowWindow(SW_HIDE);
    m_check2.ShowWindow(SW_HIDE);
    m_check3.ShowWindow(SW_HIDE);
    m_radio1.ShowWindow(SW_HIDE);
    m_radio2.ShowWindow(SW_HIDE);
    m_radio3.ShowWindow(SW_HIDE);
    m_toggle1.ShowWindow(SW_HIDE);
    m_toggle2.ShowWindow(SW_HIDE);

    // Data Display
    m_dataDemoTitle.ShowWindow(SW_HIDE);
    m_progressBar.ShowWindow(SW_HIDE);
    m_progressBarAnim.ShowWindow(SW_HIDE);
    m_listView.ShowWindow(SW_HIDE);

    // Feedback
    m_feedbackDemoTitle.ShowWindow(SW_HIDE);
    m_infoNotifyBtn.ShowWindow(SW_HIDE);
    m_successNotifyBtn.ShowWindow(SW_HIDE);
    m_warningNotifyBtn.ShowWindow(SW_HIDE);
    m_errorNotifyBtn.ShowWindow(SW_HIDE);
    m_modalBtn.ShowWindow(SW_HIDE);

    // DateTime
    m_dateTimeDemoTitle.ShowWindow(SW_HIDE);
    m_datePickerLabel.ShowWindow(SW_HIDE);
    m_datePicker.ShowWindow(SW_HIDE);
    m_timePickerLabel.ShowWindow(SW_HIDE);
    m_timePicker.ShowWindow(SW_HIDE);
    m_colorPickerLabel.ShowWindow(SW_HIDE);
    m_colorPicker.ShowWindow(SW_HIDE);

    // Theme
    m_themeDemoTitle.ShowWindow(SW_HIDE);
    m_themeDesc.ShowWindow(SW_HIDE);
    m_colorPreview.ShowWindow(SW_HIDE);
}

void CDemoMainFrame::UpdateSidebarButtons(DemoPage page)
{
    // 모든 버튼을 Ghost로
    m_btnButtons.SetButtonStyle(page == DemoPage::Buttons ? ButtonStyle::Primary : ButtonStyle::Ghost);
    m_btnInputs.SetButtonStyle(page == DemoPage::Inputs ? ButtonStyle::Primary : ButtonStyle::Ghost);
    m_btnSelection.SetButtonStyle(page == DemoPage::Selection ? ButtonStyle::Primary : ButtonStyle::Ghost);
    m_btnDataDisplay.SetButtonStyle(page == DemoPage::DataDisplay ? ButtonStyle::Primary : ButtonStyle::Ghost);
    m_btnFeedback.SetButtonStyle(page == DemoPage::Feedback ? ButtonStyle::Primary : ButtonStyle::Ghost);
    m_btnDateTime.SetButtonStyle(page == DemoPage::DateTime ? ButtonStyle::Primary : ButtonStyle::Ghost);
    m_btnTheme.SetButtonStyle(page == DemoPage::Theme ? ButtonStyle::Primary : ButtonStyle::Ghost);
}

void CDemoMainFrame::LayoutControls()
{
    if (!m_initialized)
        return;

    CRect clientRect;
    GetClientRect(&clientRect);

    // 사이드바
    m_sidebar.MoveWindow(0, 0, SIDEBAR_WIDTH, clientRect.Height());

    // 테마 토글 위치 (사이드바 하단)
    int toggleY = clientRect.Height() - 50;
    m_themeLabel.MoveWindow(16, toggleY + 4, 100, 20);
    m_themeToggle.MoveWindow(120, toggleY, 60, 28);

    // 컨텐츠 패널
    m_contentPanel.MoveWindow(SIDEBAR_WIDTH, 0, clientRect.Width() - SIDEBAR_WIDTH, clientRect.Height());
}

// 페이지 네비게이션 핸들러
void CDemoMainFrame::OnButtonsPageClick(void* context)
{
    static_cast<CDemoMainFrame*>(context)->ShowPage(DemoPage::Buttons);
}

void CDemoMainFrame::OnInputsPageClick(void* context)
{
    static_cast<CDemoMainFrame*>(context)->ShowPage(DemoPage::Inputs);
}

void CDemoMainFrame::OnSelectionPageClick(void* context)
{
    static_cast<CDemoMainFrame*>(context)->ShowPage(DemoPage::Selection);
}

void CDemoMainFrame::OnDataDisplayPageClick(void* context)
{
    static_cast<CDemoMainFrame*>(context)->ShowPage(DemoPage::DataDisplay);
}

void CDemoMainFrame::OnFeedbackPageClick(void* context)
{
    static_cast<CDemoMainFrame*>(context)->ShowPage(DemoPage::Feedback);
}

void CDemoMainFrame::OnDateTimePageClick(void* context)
{
    static_cast<CDemoMainFrame*>(context)->ShowPage(DemoPage::DateTime);
}

void CDemoMainFrame::OnThemePageClick(void* context)
{
    static_cast<CDemoMainFrame*>(context)->ShowPage(DemoPage::Theme);
}

void CDemoMainFrame::OnThemeToggle(void* context, BOOL isOn)
{
    CMThemeManager::GetInstance().SetDarkMode(isOn);
}

// Notification 핸들러 (API: ShowInfo(message, duration))
void CDemoMainFrame::OnInfoNotifyClick(void* context)
{
    CMNotificationManager::GetInstance().ShowInfo(_T("This is an info notification."));
}

void CDemoMainFrame::OnSuccessNotifyClick(void* context)
{
    CMNotificationManager::GetInstance().ShowSuccess(_T("Operation completed successfully!"));
}

void CDemoMainFrame::OnWarningNotifyClick(void* context)
{
    CMNotificationManager::GetInstance().ShowWarning(_T("This is a warning message."));
}

void CDemoMainFrame::OnErrorNotifyClick(void* context)
{
    CMNotificationManager::GetInstance().ShowError(_T("An error has occurred."));
}

void CDemoMainFrame::OnModalBtnClick(void* context)
{
    ModalResult result = CMModal::ShowQuestion(
        _T("Do you want to continue with this operation?"),
        _T("Confirmation")
    );

    if (result == ModalResult::Yes)
    {
        CMNotificationManager::GetInstance().ShowSuccess(_T("You clicked Yes!"));
    }
    else
    {
        CMNotificationManager::GetInstance().ShowInfo(_T("You clicked No."));
    }
}

void CDemoMainFrame::OnSliderValueChanged(void* context, int oldValue, int newValue)
{
    // 슬라이더 값 변경 시 처리 (필요시)
}

void CDemoMainFrame::OnSize(UINT nType, int cx, int cy)
{
    CFrameWnd::OnSize(nType, cx, cy);
    LayoutControls();
}

void CDemoMainFrame::OnPaint()
{
    CPaintDC dc(this);
}

BOOL CDemoMainFrame::OnEraseBkgnd(CDC* pDC)
{
    CRect rect;
    GetClientRect(&rect);

    const ThemeColors& colors = CMThemeManager::GetInstance().GetColors();
    pDC->FillSolidRect(rect, colors.background.normal);

    return TRUE;
}

void CDemoMainFrame::OnDestroy()
{
    CFrameWnd::OnDestroy();
    PostQuitMessage(0);
}
