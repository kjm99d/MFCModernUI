#pragma once

#include "IThemeProvider.h"
#include "CMTypes.h"
#include <map>

// MFC Modern UI - 테마 매니저
// SOLID: 단일 책임 원칙 - 테마 관리만 담당
// SOLID: 개방-폐쇄 원칙 - 새 테마 추가 시 확장 가능

namespace MFCModernUI
{
    class CMThemeManager : public IThemeProvider
    {
    public:
        // 싱글톤 인스턴스
        static CMThemeManager& GetInstance();

        // 복사/이동 금지
        CMThemeManager(const CMThemeManager&) = delete;
        CMThemeManager& operator=(const CMThemeManager&) = delete;

        // IThemeProvider 구현
        const ThemeColors& GetColors() const override;
        bool IsDarkMode() const override;
        int GetCornerRadius() const override;
        CFont* GetFont(TextStyle style) override;

        // 테마 전환
        void SetDarkMode(bool darkMode);
        void ToggleTheme();

        // 코너 라디우스 설정
        void SetCornerRadius(int radius);

        // 폰트 패밀리 설정
        void SetFontFamily(const CString& fontFamily);

        // 테마 변경 알림을 위한 콜백 등록
        using ThemeChangedCallback = void(*)(void* context);
        void RegisterThemeChangedCallback(ThemeChangedCallback callback, void* context);
        void UnregisterThemeChangedCallback(void* context);

    private:
        CMThemeManager();
        ~CMThemeManager();

        void InitializeFonts();
        void NotifyThemeChanged();

        bool m_isDarkMode;
        int m_cornerRadius;
        CString m_fontFamily;

        ThemeColors m_lightTheme;
        ThemeColors m_darkTheme;

        std::map<TextStyle, CFont*> m_fonts;
        std::map<void*, ThemeChangedCallback> m_callbacks;
    };
}
