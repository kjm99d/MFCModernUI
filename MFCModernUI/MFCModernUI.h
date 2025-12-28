#pragma once

// MFC Modern UI Library
// 모던 UI 컴포넌트 라이브러리 통합 헤더
//
// 사용법:
//   #include "MFCModernUI.h"
//   using namespace MFCModernUI;

// 기본 타입 및 색상
#include "CMTypes.h"
#include "CMColors.h"

// 테마 시스템
#include "IThemeProvider.h"
#include "CMThemeManager.h"

// 애니메이션 시스템
#include "IAnimatable.h"
#include "CMAnimationManager.h"

// 기본 컨트롤
#include "CMControlBase.h"

// Phase 2 컴포넌트 (P0)
#include "CMButton.h"
#include "CMLabel.h"
#include "CMEdit.h"
#include "CMCheckBox.h"
#include "CMRadioButton.h"
#include "CMPanel.h"
#include "CMProgressBar.h"

// Phase 3 컴포넌트 (P1)
#include "CMToggleSwitch.h"
#include "CMComboBox.h"
#include "CMListView.h"
#include "CMTabCtrl.h"
#include "CMSlider.h"
#include "CMSpinner.h"
#include "CMTooltip.h"
#include "CMContextMenu.h"

// Phase 4 컴포넌트 (P2)
#include "CMTreeView.h"
#include "CMDataGrid.h"
#include "CMDatePicker.h"
#include "CMTimePicker.h"
#include "CMColorPicker.h"
#include "CMNotification.h"
#include "CMModal.h"

// 버전 정보
#define MFCMODERNUI_VERSION_MAJOR 1
#define MFCMODERNUI_VERSION_MINOR 0
#define MFCMODERNUI_VERSION_PATCH 0
#define MFCMODERNUI_VERSION_STRING "1.0.0"
