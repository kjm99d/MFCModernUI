# MFC Modern UI Library - 컴포넌트 상세 명세서

> 각 컴포넌트의 상세 스펙, 상태, 동작 정의

---

## 1. 컴포넌트 공통 사항

### 1.1 공통 상태

| 상태 | 설명 | 트리거 |
|------|------|--------|
| `Normal` | 기본 상태 | 초기 상태, 이벤트 종료 |
| `Hover` | 마우스 위치 | MouseEnter |
| `Pressed` | 누름 | LButtonDown |
| `Focused` | 키보드 포커스 | SetFocus |
| `Disabled` | 비활성화 | SetEnabled(FALSE) |

### 1.2 공통 속성

| 속성 | 타입 | 기본값 | 설명 |
|------|------|--------|------|
| Enabled | BOOL | TRUE | 활성화 상태 |
| Visible | BOOL | TRUE | 표시 상태 |
| ThemeClass | CString | 컴포넌트명 | 테마 클래스 |
| TabStop | BOOL | TRUE | 탭 키 정지 |
| Tooltip | CString | 빈 문자열 | 툴팁 텍스트 |

### 1.3 공통 이벤트

| 이벤트 | 파라미터 | 설명 |
|--------|----------|------|
| OnStateChanged | State old, State new | 상태 변경 |
| OnEnabledChanged | BOOL enabled | 활성화 변경 |
| OnFocusChanged | BOOL focused | 포커스 변경 |

---

## 2. CMButton

### 2.1 개요

| 항목 | 내용 |
|------|------|
| **설명** | 다양한 스타일을 지원하는 모던 푸시 버튼 |
| **상속** | CMControlBase → CWnd |
| **테마 클래스** | `button` |

### 2.2 스타일 (Variant)

| 스타일 | 설명 | 용도 |
|--------|------|------|
| `Primary` | 주요 배경색, 흰색 텍스트 | CTA, 주요 액션 |
| `Secondary` | 회색 배경 | 보조 액션 |
| `Success` | 녹색 배경 | 확인, 완료 |
| `Danger` | 빨간색 배경 | 삭제, 위험 액션 |
| `Warning` | 노란색 배경 | 주의 필요 |
| `Info` | 파란색 배경 | 정보성 |
| `Light` | 밝은 배경, 어두운 텍스트 | 덜 강조 |
| `Dark` | 어두운 배경, 밝은 텍스트 | 강조 |
| `Outline` | 투명 배경, 테두리만 | 보조 액션 |
| `Ghost` | 투명 배경, 테두리 없음 | 최소 강조 |

### 2.3 속성

| 속성 | 타입 | 기본값 | 설명 |
|------|------|--------|------|
| Text | CString | 빈 문자열 | 버튼 텍스트 |
| Style | ButtonStyle | Primary | 버튼 스타일 |
| Icon | UINT | 0 | 아이콘 리소스 ID |
| IconPosition | IconPos | Left | 아이콘 위치 (Left/Right) |
| IconOnly | BOOL | FALSE | 아이콘만 표시 |
| Loading | BOOL | FALSE | 로딩 상태 |
| CornerRadius | float | 테마값 | 모서리 둥글기 (px) |

### 2.4 크기

| 크기 | 높이 | 패딩 (좌우) | 폰트 크기 |
|------|------|-------------|-----------|
| Small | 24px | 8px | 12px |
| Medium | 32px | 16px | 14px |
| Large | 40px | 20px | 16px |

### 2.5 상태별 시각적 변화

| 상태 | 배경 | 테두리 | 텍스트 | 그림자 |
|------|------|--------|--------|--------|
| Normal | primary | 없음 | textOnPrimary | none |
| Hover | primaryHover | 없음 | textOnPrimary | sm |
| Pressed | primaryPressed | 없음 | textOnPrimary | none |
| Focused | primary | borderFocus (2px) | textOnPrimary | none |
| Disabled | primaryDisabled | 없음 | textDisabled | none |

### 2.6 애니메이션

| 트리거 | 애니메이션 | 지속시간 | 이징 |
|--------|------------|----------|------|
| Hover 진입 | 배경색 전환 | 150ms | easeOutQuad |
| Hover 이탈 | 배경색 전환 | 150ms | easeOutQuad |
| Press | Scale(0.98) + 색상 | 100ms | easeOutQuad |
| Release | Scale(1.0) | 100ms | easeOutQuad |
| Loading | 스피너 회전 | 무한 | linear |

### 2.7 이벤트

| 이벤트 | 파라미터 | 설명 |
|--------|----------|------|
| OnClick | 없음 | 클릭 완료 (LButtonUp) |
| OnDoubleClick | 없음 | 더블클릭 |

### 2.8 접근성

| 항목 | 지원 |
|------|------|
| 키보드 활성화 | Space, Enter |
| 스크린 리더 | 버튼 역할, 텍스트 읽기 |
| 고대비 모드 | 테마 지원 |

---

## 3. CMEdit

### 3.1 개요

| 항목 | 내용 |
|------|------|
| **설명** | 단일/다중 라인 텍스트 입력 컨트롤 |
| **상속** | CMControlBase → CWnd |
| **테마 클래스** | `edit` |

### 3.2 속성

| 속성 | 타입 | 기본값 | 설명 |
|------|------|--------|------|
| Text | CString | 빈 문자열 | 입력 텍스트 |
| Placeholder | CString | 빈 문자열 | 플레이스홀더 텍스트 |
| MaxLength | int | 0 (무제한) | 최대 글자 수 |
| Multiline | BOOL | FALSE | 다중 라인 |
| Password | BOOL | FALSE | 비밀번호 모드 |
| ReadOnly | BOOL | FALSE | 읽기 전용 |
| PrefixIcon | UINT | 0 | 앞쪽 아이콘 |
| SuffixIcon | UINT | 0 | 뒤쪽 아이콘 |
| ClearButton | BOOL | FALSE | 지우기 버튼 표시 |

### 3.3 유효성 상태

| 상태 | 테두리 색상 | 아이콘 | 용도 |
|------|-------------|--------|------|
| None | border | 없음 | 기본 |
| Valid | success | ✓ | 유효한 입력 |
| Invalid | danger | ✗ | 유효하지 않은 입력 |
| Warning | warning | ⚠ | 경고 |

### 3.4 상태별 시각적 변화

| 상태 | 배경 | 테두리 | 플레이스홀더 |
|------|------|--------|--------------|
| Normal | surface | border (1px) | textSecondary |
| Hover | surface | borderHover (1px) | textSecondary |
| Focused | surface | borderFocus (2px) | textSecondary |
| Disabled | surfaceDisabled | borderDisabled | textDisabled |

### 3.5 애니메이션

| 트리거 | 애니메이션 | 지속시간 |
|--------|------------|----------|
| Focus 진입 | 테두리 색상 + 두께 | 200ms |
| Focus 이탈 | 테두리 색상 + 두께 | 200ms |
| 유효성 변경 | 테두리 색상 | 200ms |

### 3.6 이벤트

| 이벤트 | 파라미터 | 설명 |
|--------|----------|------|
| OnTextChanged | 없음 | 텍스트 변경 |
| OnEnterPressed | 없음 | Enter 키 입력 |
| OnClearClicked | 없음 | 지우기 버튼 클릭 |

---

## 4. CMLabel

### 4.1 개요

| 항목 | 내용 |
|------|------|
| **설명** | 스타일링 가능한 텍스트 레이블 |
| **상속** | CMControlBase → CWnd |
| **테마 클래스** | `label` |

### 4.2 속성

| 속성 | 타입 | 기본값 | 설명 |
|------|------|--------|------|
| Text | CString | 빈 문자열 | 표시 텍스트 |
| TextStyle | TextStyle | Body | 텍스트 스타일 |
| TextAlign | Align | Left | 수평 정렬 |
| TextColor | ColorToken | text | 텍스트 색상 |
| Ellipsis | BOOL | FALSE | 말줄임 (...) |
| Selectable | BOOL | FALSE | 텍스트 선택 가능 |
| WordWrap | BOOL | FALSE | 줄바꿈 |

### 4.3 텍스트 스타일

| 스타일 | 폰트 크기 | 굵기 | 용도 |
|--------|-----------|------|------|
| H1 | 4xl (40px) | bold | 페이지 제목 |
| H2 | 3xl (32px) | bold | 섹션 제목 |
| H3 | 2xl (24px) | semibold | 서브섹션 |
| H4 | xl (20px) | semibold | 소제목 |
| H5 | lg (16px) | semibold | 작은 제목 |
| H6 | base (14px) | semibold | 최소 제목 |
| Body | base (14px) | normal | 기본 본문 |
| BodySmall | sm (12px) | normal | 작은 본문 |
| Caption | xs (10px) | normal | 캡션, 각주 |

---

## 5. CMCheckBox

### 5.1 개요

| 항목 | 내용 |
|------|------|
| **설명** | 체크박스 컨트롤 (선택/해제) |
| **상속** | CMControlBase → CWnd |
| **테마 클래스** | `checkbox` |

### 5.2 속성

| 속성 | 타입 | 기본값 | 설명 |
|------|------|--------|------|
| Checked | BOOL | FALSE | 체크 상태 |
| Indeterminate | BOOL | FALSE | 불확정 상태 |
| Text | CString | 빈 문자열 | 레이블 텍스트 |
| TextPosition | Position | Right | 텍스트 위치 |

### 5.3 상태

| 상태 | 배경 | 테두리 | 체크 마크 |
|------|------|--------|-----------|
| Unchecked | 투명 | border | 없음 |
| Unchecked + Hover | 투명 | borderHover | 없음 |
| Checked | primary | 없음 | 흰색 ✓ |
| Checked + Hover | primaryHover | 없음 | 흰색 ✓ |
| Indeterminate | primary | 없음 | 흰색 ━ |
| Disabled | surfaceDisabled | borderDisabled | textDisabled |

### 5.4 크기

| 항목 | 값 |
|------|-----|
| 박스 크기 | 20 × 20 px |
| 체크 마크 두께 | 2px |
| 텍스트 간격 | 8px |

### 5.5 애니메이션

| 트리거 | 애니메이션 | 지속시간 |
|--------|------------|----------|
| Check | 체크 마크 그리기 (path) | 150ms |
| Uncheck | 체크 마크 사라짐 | 100ms |
| 상태 전환 | 배경색/테두리 | 150ms |

### 5.6 이벤트

| 이벤트 | 파라미터 | 설명 |
|--------|----------|------|
| OnCheckedChanged | BOOL checked | 체크 상태 변경 |

---

## 6. CMRadioButton

### 6.1 개요

| 항목 | 내용 |
|------|------|
| **설명** | 라디오 버튼 (그룹 내 단일 선택) |
| **상속** | CMControlBase → CWnd |
| **테마 클래스** | `radio` |

### 6.2 속성

| 속성 | 타입 | 기본값 | 설명 |
|------|------|--------|------|
| Selected | BOOL | FALSE | 선택 상태 |
| Text | CString | 빈 문자열 | 레이블 텍스트 |
| GroupID | int | 0 | 그룹 식별자 |

### 6.3 상태별 시각적 변화

| 상태 | 외부 원 | 내부 원 |
|------|---------|---------|
| Unselected | border 테두리 | 없음 |
| Unselected + Hover | borderHover 테두리 | 없음 |
| Selected | primary 테두리 | primary 채움 |
| Selected + Hover | primaryHover 테두리 | primaryHover 채움 |
| Disabled | borderDisabled | textDisabled |

### 6.4 크기

| 항목 | 값 |
|------|-----|
| 외부 원 지름 | 20px |
| 내부 원 지름 | 10px |
| 테두리 두께 | 2px |
| 텍스트 간격 | 8px |

### 6.5 애니메이션

| 트리거 | 애니메이션 | 지속시간 |
|--------|------------|----------|
| Select | 내부 원 Scale(0→1) | 150ms |
| Deselect | 내부 원 Scale(1→0) | 100ms |

### 6.6 그룹 동작

| 동작 | 설명 |
|------|------|
| 선택 시 | 같은 그룹 내 다른 항목 자동 해제 |
| 그룹 관리 | GroupID로 자동 그룹핑 |

---

## 7. CMPanel

### 7.1 개요

| 항목 | 내용 |
|------|------|
| **설명** | 컨테이너/카드 패널 |
| **상속** | CMControlBase → CWnd |
| **테마 클래스** | `panel` |

### 7.2 속성

| 속성 | 타입 | 기본값 | 설명 |
|------|------|--------|------|
| BackgroundColor | ColorToken | surface | 배경색 |
| BorderColor | ColorToken | border | 테두리 색상 |
| BorderWidth | float | 0 | 테두리 두께 |
| CornerRadius | float | 테마값 | 모서리 둥글기 |
| Shadow | ShadowLevel | none | 그림자 레벨 |
| Padding | Spacing | 4 (16px) | 내부 패딩 |

### 7.3 변형

| 변형 | 배경 | 테두리 | 그림자 |
|------|------|--------|--------|
| Flat | surface | 없음 | none |
| Outlined | 투명 | border | none |
| Elevated | surface | 없음 | md |
| Filled | surface | 없음 | none |

---

## 8. CMProgressBar

### 8.1 개요

| 항목 | 내용 |
|------|------|
| **설명** | 진행률 표시 바 |
| **상속** | CMControlBase → CWnd |
| **테마 클래스** | `progress` |

### 8.2 속성

| 속성 | 타입 | 기본값 | 설명 |
|------|------|--------|------|
| Value | int | 0 | 현재 값 |
| Min | int | 0 | 최소 값 |
| Max | int | 100 | 최대 값 |
| Indeterminate | BOOL | FALSE | 불확정 모드 |
| ShowText | BOOL | FALSE | 퍼센트 텍스트 표시 |
| BarColor | ColorToken | primary | 바 색상 |
| TrackColor | ColorToken | surface | 트랙 색상 |

### 8.3 크기

| 크기 | 높이 |
|------|------|
| Small | 4px |
| Medium | 8px |
| Large | 16px |

### 8.4 애니메이션

| 모드 | 애니메이션 |
|------|------------|
| 확정 (Determinate) | 값 변경 시 너비 전환 (300ms) |
| 불확정 (Indeterminate) | 좌→우 반복 이동 (1.5s 사이클) |

---

## 9. CMToggleSwitch

### 9.1 개요

| 항목 | 내용 |
|------|------|
| **설명** | iOS 스타일 토글 스위치 |
| **상속** | CMControlBase → CWnd |
| **테마 클래스** | `toggle` |

### 9.2 속성

| 속성 | 타입 | 기본값 | 설명 |
|------|------|--------|------|
| Checked | BOOL | FALSE | On/Off 상태 |
| Text | CString | 빈 문자열 | 레이블 텍스트 |
| OnText | CString | 빈 문자열 | On 상태 텍스트 |
| OffText | CString | 빈 문자열 | Off 상태 텍스트 |

### 9.3 크기

| 항목 | 값 |
|------|-----|
| 트랙 너비 | 44px |
| 트랙 높이 | 24px |
| 노브 지름 | 20px |
| 노브 여백 | 2px |

### 9.4 상태별 시각적 변화

| 상태 | 트랙 배경 | 노브 위치 | 노브 색상 |
|------|-----------|-----------|-----------|
| Off | surface | 좌측 | background |
| Off + Hover | surfaceHover | 좌측 | background |
| On | primary | 우측 | background |
| On + Hover | primaryHover | 우측 | background |
| Disabled | surfaceDisabled | 현재 위치 | backgroundDisabled |

### 9.5 애니메이션

| 트리거 | 애니메이션 | 지속시간 | 이징 |
|--------|------------|----------|------|
| Toggle | 노브 슬라이드 + 배경색 | 200ms | easeOutCubic |

---

## 10. CMComboBox

### 10.1 개요

| 항목 | 내용 |
|------|------|
| **설명** | 드롭다운 선택 컨트롤 |
| **상속** | CMControlBase → CWnd |
| **테마 클래스** | `combobox` |

### 10.2 속성

| 속성 | 타입 | 기본값 | 설명 |
|------|------|--------|------|
| SelectedIndex | int | -1 | 선택된 항목 인덱스 |
| Placeholder | CString | "선택..." | 미선택 시 표시 |
| Searchable | BOOL | FALSE | 검색 기능 |
| MultiSelect | BOOL | FALSE | 다중 선택 |
| MaxDropdownHeight | int | 300 | 드롭다운 최대 높이 |

### 10.3 항목 구조

| 필드 | 타입 | 설명 |
|------|------|------|
| Text | CString | 표시 텍스트 |
| Value | any | 연결 값 |
| Icon | UINT | 아이콘 (선택) |
| Disabled | BOOL | 비활성화 |
| Separator | BOOL | 구분선 여부 |

### 10.4 상태

| 상태 | 메인 컨트롤 | 드롭다운 |
|------|-------------|----------|
| Closed | 테두리, 선택값/플레이스홀더 | 숨김 |
| Open | 포커스 테두리 | 표시, 그림자 |
| Searching | 입력 필드 활성 | 필터링된 목록 |

### 10.5 애니메이션

| 트리거 | 애니메이션 | 지속시간 |
|--------|------------|----------|
| Open | 드롭다운 Fade + Scale(0.95→1) | 150ms |
| Close | 드롭다운 Fade | 100ms |
| 항목 Hover | 배경색 전환 | 100ms |

### 10.6 이벤트

| 이벤트 | 파라미터 | 설명 |
|--------|----------|------|
| OnSelectionChanged | int oldIndex, int newIndex | 선택 변경 |
| OnDropdownOpened | 없음 | 드롭다운 열림 |
| OnDropdownClosed | 없음 | 드롭다운 닫힘 |

---

## 11. CMListView

### 11.1 개요

| 항목 | 내용 |
|------|------|
| **설명** | 모던 리스트 뷰 컨트롤 |
| **상속** | CMControlBase → CWnd |
| **테마 클래스** | `listview` |

### 11.2 속성

| 속성 | 타입 | 기본값 | 설명 |
|------|------|--------|------|
| SelectionMode | SelectMode | Single | 선택 모드 |
| ShowCheckboxes | BOOL | FALSE | 체크박스 표시 |
| ShowHeaders | BOOL | TRUE | 헤더 표시 |
| AlternateRows | BOOL | FALSE | 줄무늬 배경 |
| VirtualMode | BOOL | FALSE | 가상화 모드 |

### 11.3 선택 모드

| 모드 | 설명 |
|------|------|
| None | 선택 불가 |
| Single | 단일 선택 |
| Multiple | 다중 선택 (Ctrl+클릭) |
| Extended | 확장 선택 (Shift+클릭) |

### 11.4 열 정의

| 필드 | 타입 | 설명 |
|------|------|------|
| Header | CString | 헤더 텍스트 |
| Width | int | 열 너비 |
| MinWidth | int | 최소 너비 |
| MaxWidth | int | 최대 너비 |
| Resizable | BOOL | 리사이즈 가능 |
| Sortable | BOOL | 정렬 가능 |
| Align | Align | 정렬 방향 |

### 11.5 가상화

| 항목 | 설명 |
|------|------|
| 활성화 조건 | VirtualMode = TRUE |
| 필요 콜백 | OnGetItemCount, OnGetItemData |
| 최적화 | 화면에 보이는 항목만 렌더링 |
| 권장 사용 | 1,000개 이상 항목 |

### 11.6 이벤트

| 이벤트 | 파라미터 | 설명 |
|--------|----------|------|
| OnSelectionChanged | 없음 | 선택 변경 |
| OnItemDoubleClick | int index | 항목 더블클릭 |
| OnColumnClick | int column | 헤더 클릭 |
| OnItemChecked | int index, BOOL checked | 체크 변경 |

---

## 12. CMTabCtrl

### 12.1 개요

| 항목 | 내용 |
|------|------|
| **설명** | 탭 컨트롤 |
| **상속** | CMControlBase → CWnd |
| **테마 클래스** | `tabs` |

### 12.2 속성

| 속성 | 타입 | 기본값 | 설명 |
|------|------|--------|------|
| SelectedIndex | int | 0 | 선택된 탭 |
| TabPosition | Position | Top | 탭 위치 |
| ShowCloseButton | BOOL | FALSE | 닫기 버튼 |
| ScrollButtons | BOOL | TRUE | 스크롤 버튼 |

### 12.3 탭 위치

| 위치 | 설명 |
|------|------|
| Top | 상단 (기본) |
| Bottom | 하단 |
| Left | 좌측 (세로) |
| Right | 우측 (세로) |

### 12.4 탭 항목 구조

| 필드 | 타입 | 설명 |
|------|------|------|
| Text | CString | 탭 텍스트 |
| Icon | UINT | 아이콘 |
| Closable | BOOL | 닫기 가능 |
| Disabled | BOOL | 비활성화 |
| Badge | CString | 배지 텍스트 |

### 12.5 이벤트

| 이벤트 | 파라미터 | 설명 |
|--------|----------|------|
| OnTabChanged | int oldIndex, int newIndex | 탭 변경 |
| OnTabClosing | int index, BOOL* cancel | 탭 닫기 전 |
| OnTabClosed | int index | 탭 닫힘 |

---

## 13. CMSlider

### 13.1 개요

| 항목 | 내용 |
|------|------|
| **설명** | 슬라이더/트랙바 컨트롤 |
| **상속** | CMControlBase → CWnd |
| **테마 클래스** | `slider` |

### 13.2 속성

| 속성 | 타입 | 기본값 | 설명 |
|------|------|--------|------|
| Value | int | 0 | 현재 값 |
| Min | int | 0 | 최소 값 |
| Max | int | 100 | 최대 값 |
| Step | int | 1 | 증가 단위 |
| Orientation | Orient | Horizontal | 방향 |
| ShowTicks | BOOL | FALSE | 눈금 표시 |
| ShowValue | BOOL | FALSE | 값 표시 |

### 13.3 크기

| 항목 | 값 |
|------|-----|
| 트랙 두께 | 4px |
| 썸(Thumb) 지름 | 16px |
| 썸 호버 크기 | 20px |

### 13.4 애니메이션

| 트리거 | 애니메이션 |
|--------|------------|
| Thumb Hover | 크기 확대 (150ms) |
| Value Change | 부드러운 이동 (100ms) |

### 13.5 이벤트

| 이벤트 | 파라미터 | 설명 |
|--------|----------|------|
| OnValueChanged | int oldValue, int newValue | 값 변경 |
| OnValueChanging | int value | 드래그 중 실시간 |

---

## 14. 컴포넌트 우선순위 요약

### 14.1 Phase별 컴포넌트

| Phase | 컴포넌트 | 우선순위 |
|-------|----------|----------|
| **Phase 2** | CMButton | P0 |
| | CMEdit | P0 |
| | CMLabel | P0 |
| | CMCheckBox | P0 |
| | CMRadioButton | P0 |
| | CMPanel | P0 |
| | CMProgressBar | P0 |
| **Phase 3** | CMToggleSwitch | P1 |
| | CMComboBox | P1 |
| | CMListView | P1 |
| | CMTabCtrl | P1 |
| | CMSlider | P1 |
| | CMSpinner | P1 |
| | CMTooltip | P1 |
| | CMContextMenu | P1 |
| **Phase 4** | CMTreeView | P2 |
| | CMDataGrid | P2 |
| | CMDatePicker | P2 |
| | CMTimePicker | P2 |
| | CMColorPicker | P2 |
| | CMNotification | P2 |
| | CMModal | P2 |

---

*문서 버전: 1.0*  
*최종 수정일: 2025년 1월*
