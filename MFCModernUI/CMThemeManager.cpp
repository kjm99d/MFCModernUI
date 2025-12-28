#include "pch.h"
#include "CMThemeManager.h"

namespace MFCModernUI
{
    CMThemeManager& CMThemeManager::GetInstance()
    {
        static CMThemeManager instance;
        return instance;
    }

    CMThemeManager::CMThemeManager()
        : m_isDarkMode(false)
        , m_cornerRadius(6)
        , m_fontFamily(_T("Segoe UI"))
    {
        m_lightTheme = ThemeColors::CreateLightTheme();
        m_darkTheme = ThemeColors::CreateDarkTheme();
        InitializeFonts();
    }

    CMThemeManager::~CMThemeManager()
    {
        for (auto& pair : m_fonts)
        {
            if (pair.second)
            {
                pair.second->DeleteObject();
                delete pair.second;
            }
        }
        m_fonts.clear();
    }

    void CMThemeManager::InitializeFonts()
    {
        // 폰트 크기 정의 (TextStyle에 따라)
        struct FontSpec
        {
            TextStyle style;
            int size;
            int weight;
        };

        FontSpec specs[] = {
            { TextStyle::H1, 40, FW_BOLD },
            { TextStyle::H2, 32, FW_BOLD },
            { TextStyle::H3, 24, FW_SEMIBOLD },
            { TextStyle::H4, 20, FW_SEMIBOLD },
            { TextStyle::H5, 16, FW_SEMIBOLD },
            { TextStyle::H6, 14, FW_SEMIBOLD },
            { TextStyle::Body, 14, FW_NORMAL },
            { TextStyle::BodySmall, 12, FW_NORMAL },
            { TextStyle::Caption, 10, FW_NORMAL }
        };

        for (const auto& spec : specs)
        {
            CFont* pFont = new CFont();
            pFont->CreateFont(
                -spec.size,              // 높이
                0,                       // 너비
                0,                       // 회전 각도
                0,                       // 기울기
                spec.weight,             // 굵기
                FALSE,                   // 이탤릭
                FALSE,                   // 밑줄
                FALSE,                   // 취소선
                DEFAULT_CHARSET,
                OUT_DEFAULT_PRECIS,
                CLIP_DEFAULT_PRECIS,
                CLEARTYPE_QUALITY,
                DEFAULT_PITCH | FF_SWISS,
                m_fontFamily
            );
            m_fonts[spec.style] = pFont;
        }
    }

    const ThemeColors& CMThemeManager::GetColors() const
    {
        return m_isDarkMode ? m_darkTheme : m_lightTheme;
    }

    bool CMThemeManager::IsDarkMode() const
    {
        return m_isDarkMode;
    }

    int CMThemeManager::GetCornerRadius() const
    {
        return m_cornerRadius;
    }

    CFont* CMThemeManager::GetFont(TextStyle style)
    {
        auto it = m_fonts.find(style);
        if (it != m_fonts.end())
        {
            return it->second;
        }
        return m_fonts[TextStyle::Body];
    }

    void CMThemeManager::SetDarkMode(bool darkMode)
    {
        if (m_isDarkMode != darkMode)
        {
            m_isDarkMode = darkMode;
            NotifyThemeChanged();
        }
    }

    void CMThemeManager::ToggleTheme()
    {
        SetDarkMode(!m_isDarkMode);
    }

    void CMThemeManager::SetCornerRadius(int radius)
    {
        if (m_cornerRadius != radius)
        {
            m_cornerRadius = radius;
            NotifyThemeChanged();
        }
    }

    void CMThemeManager::SetFontFamily(const CString& fontFamily)
    {
        if (m_fontFamily != fontFamily)
        {
            m_fontFamily = fontFamily;

            // 폰트 재생성
            for (auto& pair : m_fonts)
            {
                if (pair.second)
                {
                    pair.second->DeleteObject();
                    delete pair.second;
                }
            }
            m_fonts.clear();
            InitializeFonts();
            NotifyThemeChanged();
        }
    }

    void CMThemeManager::RegisterThemeChangedCallback(ThemeChangedCallback callback, void* context)
    {
        m_callbacks[context] = callback;
    }

    void CMThemeManager::UnregisterThemeChangedCallback(void* context)
    {
        m_callbacks.erase(context);
    }

    void CMThemeManager::NotifyThemeChanged()
    {
        for (const auto& pair : m_callbacks)
        {
            if (pair.second)
            {
                pair.second(pair.first);
            }
        }
    }
}
