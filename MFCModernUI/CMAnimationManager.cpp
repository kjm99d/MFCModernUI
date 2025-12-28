#include "pch.h"
#include "CMAnimationManager.h"
#include <algorithm>

namespace MFCModernUI
{
    CMAnimationManager* CMAnimationManager::s_instance = nullptr;

    CMAnimationManager& CMAnimationManager::GetInstance()
    {
        static CMAnimationManager instance;
        return instance;
    }

    CMAnimationManager::CMAnimationManager()
        : m_nextAnimationId(1)
        , m_timerId(0)
        , m_timerRunning(false)
    {
        s_instance = this;
    }

    CMAnimationManager::~CMAnimationManager()
    {
        if (m_timerRunning)
        {
            KillTimer(nullptr, m_timerId);
        }
        s_instance = nullptr;
    }

    int CMAnimationManager::StartAnimation(
        HWND hwnd,
        float startValue,
        float endValue,
        int duration,
        EasingType easing,
        std::function<void(float)> updateCallback,
        std::function<void()> completeCallback)
    {
        AnimationInfo info;
        info.targetHwnd = hwnd;
        info.animationId = m_nextAnimationId++;
        info.startTime = GetTickCount();
        info.duration = duration;
        info.easing = easing;
        info.startValue = startValue;
        info.endValue = endValue;
        info.currentValue = startValue;
        info.updateCallback = updateCallback;
        info.completeCallback = completeCallback;
        info.isRunning = true;

        m_animations.push_back(info);

        // 타이머 시작 (16ms ≈ 60fps)
        if (!m_timerRunning)
        {
            m_timerId = SetTimer(nullptr, 0, 16, TimerProc);
            m_timerRunning = true;
        }

        return info.animationId;
    }

    void CMAnimationManager::StopAnimation(int animationId)
    {
        auto it = std::find_if(m_animations.begin(), m_animations.end(),
            [animationId](const AnimationInfo& info) {
                return info.animationId == animationId;
            });

        if (it != m_animations.end())
        {
            it->isRunning = false;
        }
    }

    void CMAnimationManager::StopAllAnimations(HWND hwnd)
    {
        for (auto& anim : m_animations)
        {
            if (anim.targetHwnd == hwnd)
            {
                anim.isRunning = false;
            }
        }
    }

    void CALLBACK CMAnimationManager::TimerProc(HWND hwnd, UINT msg, UINT_PTR idEvent, DWORD dwTime)
    {
        if (s_instance)
        {
            s_instance->ProcessAnimations();
        }
    }

    void CMAnimationManager::ProcessAnimations()
    {
        DWORD currentTime = GetTickCount();
        bool hasActiveAnimations = false;

        for (auto& anim : m_animations)
        {
            if (!anim.isRunning)
                continue;

            hasActiveAnimations = true;

            DWORD elapsed = currentTime - anim.startTime;
            float t = min(1.0f, static_cast<float>(elapsed) / static_cast<float>(anim.duration));

            // 이징 적용
            float easedT = ApplyEasing(anim.easing, t);

            // 값 보간
            anim.currentValue = anim.startValue + (anim.endValue - anim.startValue) * easedT;

            // 업데이트 콜백
            if (anim.updateCallback)
            {
                anim.updateCallback(anim.currentValue);
            }

            // 완료 체크
            if (t >= 1.0f)
            {
                anim.isRunning = false;
                if (anim.completeCallback)
                {
                    anim.completeCallback();
                }
            }
        }

        // 완료된 애니메이션 제거
        m_animations.erase(
            std::remove_if(m_animations.begin(), m_animations.end(),
                [](const AnimationInfo& info) { return !info.isRunning; }),
            m_animations.end()
        );

        // 활성 애니메이션이 없으면 타이머 중지
        if (!hasActiveAnimations && m_timerRunning)
        {
            KillTimer(nullptr, m_timerId);
            m_timerRunning = false;
        }
    }

    COLORREF CMAnimationManager::InterpolateColor(COLORREF from, COLORREF to, float progress)
    {
        int r = GetRValue(from) + static_cast<int>((GetRValue(to) - GetRValue(from)) * progress);
        int g = GetGValue(from) + static_cast<int>((GetGValue(to) - GetGValue(from)) * progress);
        int b = GetBValue(from) + static_cast<int>((GetBValue(to) - GetBValue(from)) * progress);

        r = max(0, min(255, r));
        g = max(0, min(255, g));
        b = max(0, min(255, b));

        return RGB(r, g, b);
    }

    float CMAnimationManager::ApplyEasing(EasingType type, float t)
    {
        switch (type)
        {
        case EasingType::Linear:
            return t;

        case EasingType::EaseOutQuad:
            return 1.0f - (1.0f - t) * (1.0f - t);

        case EasingType::EaseOutCubic:
            return 1.0f - pow(1.0f - t, 3);

        case EasingType::EaseInOutQuad:
            return t < 0.5f
                ? 2.0f * t * t
                : 1.0f - pow(-2.0f * t + 2.0f, 2) / 2.0f;

        default:
            return t;
        }
    }
}
