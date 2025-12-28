#pragma once

// MFC Modern UI - 공통 타입 정의
// SOLID: 단일 책임 원칙 - 타입 정의만 담당

namespace MFCModernUI
{
    // 컨트롤 상태
    enum class ControlState
    {
        Normal = 0,
        Hover,
        Pressed,
        Focused,
        Disabled
    };

    // 버튼 스타일
    enum class ButtonStyle
    {
        Primary = 0,
        Secondary,
        Success,
        Danger,
        Warning,
        Info,
        Light,
        Dark,
        Outline,
        Ghost
    };

    // 컨트롤 크기
    enum class ControlSize
    {
        Small = 0,
        Medium,
        Large
    };

    // 아이콘 위치
    enum class IconPosition
    {
        Left = 0,
        Right
    };

    // 텍스트 정렬
    enum class TextAlign
    {
        Left = 0,
        Center,
        Right
    };

    // 텍스트 스타일
    enum class TextStyle
    {
        H1 = 0,
        H2,
        H3,
        H4,
        H5,
        H6,
        Body,
        BodyBold,
        BodySmall,
        Caption
    };

    // 유효성 상태
    enum class ValidationState
    {
        None = 0,
        Valid,
        Invalid,
        Warning
    };

    // 패널 변형
    enum class PanelVariant
    {
        Flat = 0,
        Outlined,
        Elevated,
        Filled
    };

    // 그림자 레벨
    enum class ShadowLevel
    {
        None = 0,
        Small,
        Medium,
        Large
    };

    // 위치/방향
    enum class Position
    {
        Left = 0,
        Right,
        Top,
        Bottom
    };

    // 선택 모드
    enum class SelectionMode
    {
        None = 0,
        Single,
        Multiple,
        Extended
    };

    // 크기 상수
    struct SizeConstants
    {
        static constexpr int SmallHeight = 24;
        static constexpr int MediumHeight = 32;
        static constexpr int LargeHeight = 40;

        static constexpr int SmallPadding = 8;
        static constexpr int MediumPadding = 16;
        static constexpr int LargePadding = 20;

        static constexpr int SmallFontSize = 12;
        static constexpr int MediumFontSize = 14;
        static constexpr int LargeFontSize = 16;

        static constexpr int CheckBoxSize = 20;
        static constexpr int RadioButtonSize = 20;
        static constexpr int RadioInnerSize = 10;
    };

    // 애니메이션 상수
    struct AnimationConstants
    {
        static constexpr int FastDuration = 100;
        static constexpr int NormalDuration = 150;
        static constexpr int SlowDuration = 200;
        static constexpr int ValueChangeDuration = 300;
    };
}
