#pragma once

#include "CMTypes.h"
#include <vector>
#include <functional>

// MFC Modern UI - 애니메이션 매니저
// SOLID: 단일 책임 원칙 - 애니메이션 관리만 담당

namespace MFCModernUI
{
    // 이징 함수 타입
    enum class EasingType
    {
        Linear,
        EaseOutQuad,
        EaseOutCubic,
        EaseInOutQuad
    };

    // 애니메이션 정보
    struct AnimationInfo
    {
        HWND targetHwnd;
        int animationId;
        DWORD startTime;
        int duration;
        EasingType easing;
        float startValue;
        float endValue;
        float currentValue;
        std::function<void(float)> updateCallback;
        std::function<void()> completeCallback;
        bool isRunning;

        AnimationInfo()
            : targetHwnd(nullptr)
            , animationId(0)
            , startTime(0)
            , duration(0)
            , easing(EasingType::Linear)
            , startValue(0.0f)
            , endValue(0.0f)
            , currentValue(0.0f)
            , isRunning(false)
        {}
    };

    class CMAnimationManager
    {
    public:
        static CMAnimationManager& GetInstance();

        CMAnimationManager(const CMAnimationManager&) = delete;
        CMAnimationManager& operator=(const CMAnimationManager&) = delete;

        // 애니메이션 시작
        int StartAnimation(
            HWND hwnd,
            float startValue,
            float endValue,
            int duration,
            EasingType easing,
            std::function<void(float)> updateCallback,
            std::function<void()> completeCallback = nullptr
        );

        // 애니메이션 중지
        void StopAnimation(int animationId);
        void StopAllAnimations(HWND hwnd);

        // 색상 보간
        static COLORREF InterpolateColor(COLORREF from, COLORREF to, float progress);

        // 이징 함수
        static float ApplyEasing(EasingType type, float t);

    private:
        CMAnimationManager();
        ~CMAnimationManager();

        void ProcessAnimations();
        static void CALLBACK TimerProc(HWND hwnd, UINT msg, UINT_PTR idEvent, DWORD dwTime);

        std::vector<AnimationInfo> m_animations;
        int m_nextAnimationId;
        UINT_PTR m_timerId;
        bool m_timerRunning;

        static CMAnimationManager* s_instance;
    };
}
