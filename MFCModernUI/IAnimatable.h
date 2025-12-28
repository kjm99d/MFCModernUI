#pragma once

// MFC Modern UI - 애니메이션 인터페이스
// SOLID: 인터페이스 분리 원칙

namespace MFCModernUI
{
    // 애니메이션 가능한 객체 인터페이스
    class IAnimatable
    {
    public:
        virtual ~IAnimatable() = default;

        // 애니메이션 프레임 업데이트
        virtual void OnAnimationTick(float progress) = 0;

        // 애니메이션 완료
        virtual void OnAnimationComplete() = 0;
    };
}
