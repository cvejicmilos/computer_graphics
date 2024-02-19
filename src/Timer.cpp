#include "Timer.h"

void Timer::Pause() {
    if (!m_IsPaused) {
        m_PausedTime += std::chrono::high_resolution_clock::now() - m_Start;
        m_IsPaused = true;
    }
}
void Timer::Resume() {
    if (m_IsPaused) {
        m_Start = std::chrono::high_resolution_clock::now() - m_PausedTime;
        m_IsPaused = false;
    }
}
void Timer::Stop() {
    m_PausedTime = std::chrono::nanoseconds(0);
    m_IsPaused = true;
}

void Timer::Reset() {
    m_Start = std::chrono::high_resolution_clock::now();
    m_PausedTime = std::chrono::nanoseconds(0);
    m_IsPaused = false;
}

void Timer::SetTime(double seconds) {
    m_Start = std::chrono::high_resolution_clock::now() - std::chrono::duration_cast<std::chrono::high_resolution_clock::duration>(std::chrono::duration<double>(seconds));
}

Duration Timer::Record() const {
    if (m_IsPaused) {
        return Duration(m_Start, m_Start + m_PausedTime);
    } else {
        return Duration(m_Start, std::chrono::high_resolution_clock::now() - m_PausedTime);
    }
}