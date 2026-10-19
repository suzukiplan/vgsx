#pragma once
#include <chrono>

namespace frame_timing {

class FramePacer
{
  public:
    using Clock = std::chrono::steady_clock;
    using TimePoint = Clock::time_point;
    static constexpr auto period = std::chrono::nanoseconds(16666667);
    static constexpr auto lagTolerance = std::chrono::milliseconds(1);

    explicit FramePacer(TimePoint start) : nextStart_(start) {}

    TimePoint nextFrame(TimePoint finished)
    {
        // Advance the schedule, not the completion time: present/V-SYNC and
        // sleep overshoot must not add a second delay to every frame.
        nextStart_ += period;
        // After a stall, resume without a burst of catch-up frames.
        if (nextStart_ < finished) nextStart_ = finished;
        return nextStart_;
    }

    static bool isLate(Clock::duration elapsed)
    {
        return elapsed > period + lagTolerance;
    }

  private:
    TimePoint nextStart_;
};

} // namespace frame_timing
