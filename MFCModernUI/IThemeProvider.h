#pragma once

#include "CMTypes.h"
#include "CMColors.h"

// MFC Modern UI - 테마 제공자 인터페이스
// SOLID: 의존성 역전 원칙 - 추상화에 의존
// SOLID: 인터페이스 분리 원칙 - 테마 관련 기능만 정의

namespace MFCModernUI
{
    // 테마 제공자 인터페이스
    class IThemeProvider
    {
    public:
        virtual ~IThemeProvider() = default;

        // 현재 테마 색상 가져오기
        virtual const ThemeColors& GetColors() const = 0;

        // 다크 모드 여부
        virtual bool IsDarkMode() const = 0;

        // 코너 라디우스 가져오기
        virtual int GetCornerRadius() const = 0;

        // 폰트 가져오기
        virtual CFont* GetFont(TextStyle style) = 0;
    };
}
