#pragma once

// MFC Modern UI - 색상 토큰 정의
// SOLID: 단일 책임 원칙 - 색상 정의만 담당

namespace MFCModernUI
{
    // 색상 토큰 구조체
    struct ColorToken
    {
        COLORREF normal;
        COLORREF hover;
        COLORREF pressed;
        COLORREF disabled;

        ColorToken(COLORREF n = RGB(0, 0, 0),
                   COLORREF h = RGB(0, 0, 0),
                   COLORREF p = RGB(0, 0, 0),
                   COLORREF d = RGB(0, 0, 0))
            : normal(n), hover(h), pressed(p), disabled(d) {}
    };

    // 테마 색상 집합
    struct ThemeColors
    {
        // Primary 색상
        ColorToken primary;

        // Secondary 색상
        ColorToken secondary;

        // Success 색상
        ColorToken success;

        // Danger 색상
        ColorToken danger;

        // Warning 색상
        ColorToken warning;

        // Info 색상
        ColorToken info;

        // 표면/배경 색상
        ColorToken surface;
        ColorToken background;

        // 테두리 색상
        ColorToken border;
        COLORREF borderFocus;

        // 텍스트 색상
        COLORREF text;
        COLORREF textSecondary;
        COLORREF textOnPrimary;
        COLORREF textDisabled;

        // 기본 라이트 테마
        static ThemeColors CreateLightTheme()
        {
            ThemeColors colors;

            // Primary - 파란색 계열
            colors.primary = ColorToken(
                RGB(59, 130, 246),   // normal
                RGB(37, 99, 235),    // hover
                RGB(29, 78, 216),    // pressed
                RGB(147, 197, 253)   // disabled
            );

            // Secondary - 회색 계열
            colors.secondary = ColorToken(
                RGB(107, 114, 128),
                RGB(75, 85, 99),
                RGB(55, 65, 81),
                RGB(209, 213, 219)
            );

            // Success - 녹색 계열
            colors.success = ColorToken(
                RGB(34, 197, 94),
                RGB(22, 163, 74),
                RGB(21, 128, 61),
                RGB(134, 239, 172)
            );

            // Danger - 빨간색 계열
            colors.danger = ColorToken(
                RGB(239, 68, 68),
                RGB(220, 38, 38),
                RGB(185, 28, 28),
                RGB(252, 165, 165)
            );

            // Warning - 노란색 계열
            colors.warning = ColorToken(
                RGB(245, 158, 11),
                RGB(217, 119, 6),
                RGB(180, 83, 9),
                RGB(252, 211, 77)
            );

            // Info - 하늘색 계열
            colors.info = ColorToken(
                RGB(6, 182, 212),
                RGB(8, 145, 178),
                RGB(14, 116, 144),
                RGB(103, 232, 249)
            );

            // Surface/Background
            colors.surface = ColorToken(
                RGB(255, 255, 255),
                RGB(249, 250, 251),
                RGB(243, 244, 246),
                RGB(229, 231, 235)
            );

            colors.background = ColorToken(
                RGB(249, 250, 251),
                RGB(243, 244, 246),
                RGB(229, 231, 235),
                RGB(229, 231, 235)
            );

            // Border
            colors.border = ColorToken(
                RGB(209, 213, 219),
                RGB(156, 163, 175),
                RGB(107, 114, 128),
                RGB(229, 231, 235)
            );
            colors.borderFocus = RGB(59, 130, 246);

            // Text
            colors.text = RGB(17, 24, 39);
            colors.textSecondary = RGB(107, 114, 128);
            colors.textOnPrimary = RGB(255, 255, 255);
            colors.textDisabled = RGB(156, 163, 175);

            return colors;
        }

        // 다크 테마
        static ThemeColors CreateDarkTheme()
        {
            ThemeColors colors;

            // Primary
            colors.primary = ColorToken(
                RGB(96, 165, 250),
                RGB(147, 197, 253),
                RGB(59, 130, 246),
                RGB(30, 64, 175)
            );

            // Secondary
            colors.secondary = ColorToken(
                RGB(156, 163, 175),
                RGB(209, 213, 219),
                RGB(107, 114, 128),
                RGB(55, 65, 81)
            );

            // Success
            colors.success = ColorToken(
                RGB(74, 222, 128),
                RGB(134, 239, 172),
                RGB(34, 197, 94),
                RGB(20, 83, 45)
            );

            // Danger
            colors.danger = ColorToken(
                RGB(248, 113, 113),
                RGB(252, 165, 165),
                RGB(239, 68, 68),
                RGB(127, 29, 29)
            );

            // Warning
            colors.warning = ColorToken(
                RGB(251, 191, 36),
                RGB(252, 211, 77),
                RGB(245, 158, 11),
                RGB(120, 53, 15)
            );

            // Info
            colors.info = ColorToken(
                RGB(34, 211, 238),
                RGB(103, 232, 249),
                RGB(6, 182, 212),
                RGB(22, 78, 99)
            );

            // Surface/Background
            colors.surface = ColorToken(
                RGB(31, 41, 55),
                RGB(55, 65, 81),
                RGB(17, 24, 39),
                RGB(17, 24, 39)
            );

            colors.background = ColorToken(
                RGB(17, 24, 39),
                RGB(31, 41, 55),
                RGB(55, 65, 81),
                RGB(17, 24, 39)
            );

            // Border
            colors.border = ColorToken(
                RGB(75, 85, 99),
                RGB(107, 114, 128),
                RGB(55, 65, 81),
                RGB(55, 65, 81)
            );
            colors.borderFocus = RGB(96, 165, 250);

            // Text
            colors.text = RGB(249, 250, 251);
            colors.textSecondary = RGB(156, 163, 175);
            colors.textOnPrimary = RGB(17, 24, 39);
            colors.textDisabled = RGB(107, 114, 128);

            return colors;
        }
    };
}
