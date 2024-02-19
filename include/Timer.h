#pragma once

#include <chrono>

class Duration {
private:
    std::chrono::nanoseconds m_Elapsed;

public:

    Duration() : m_Elapsed(0) {}
    Duration(std::chrono::high_resolution_clock::time_point start, std::chrono::high_resolution_clock::time_point end) {
        m_Elapsed = end - start;
    }


    double GetSeconds()      const { return std::chrono::duration<double>(m_Elapsed).count(); }
    double GetMilliseconds() const { return std::chrono::duration<double, std::milli>(m_Elapsed).count(); }
    double GetMicroseconds() const { return std::chrono::duration<double, std::micro>(m_Elapsed).count(); }
    double GetNanoseconds()  const { return std::chrono::duration<double, std::nano>(m_Elapsed).count(); }

    float GetSecondsF()      const { return std::chrono::duration<float>(m_Elapsed).count(); }
    float GetMillisecondsF() const { return std::chrono::duration<float, std::milli>(m_Elapsed).count(); }
    float GetMicroSecondsF() const { return std::chrono::duration<float, std::micro>(m_Elapsed).count(); }
    float GetNanosecondsF()  const { return std::chrono::duration<float, std::nano>(m_Elapsed).count(); }
};


class Timer {
private:
    std::chrono::high_resolution_clock::time_point m_Start;
    std::chrono::nanoseconds m_PausedTime;
    bool m_IsPaused;

public:

    Timer() : m_PausedTime(0), m_IsPaused(false) {
        Reset();
    }

    void Pause();
    void Resume();
    void Stop();
    void Reset();
    void SetTime(double seconds);

    Duration Record() const;
};

