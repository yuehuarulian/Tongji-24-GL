#include <Basic/Timer.hpp>
#include <ctime>

using namespace Basic;
using namespace std;

Timer::Timer(size_t maxLogNum) : maxLogNum(maxLogNum)
{
    Reset();
}

bool Timer::Start()
{
    switch (state)
    {
    case Basic::Timer::ENUM_STATE_INIT:
        wholeTime = 0;
        logs.push_front(0);
    case Basic::Timer::ENUM_STATE_RUNNING:
        lastLogTime = GetCurTime();
        state = ENUM_STATE_RUNNING;
        return true;
    default:
        return false;
    }
}

bool Timer::Stop()
{
    double curTime = GetCurTime();
    double deltaTime = curTime - lastLogTime;
    switch (state)
    {
    case Basic::Timer::ENUM_STATE_RUNNING:
        wholeTime += deltaTime;
        logs.front() += deltaTime;
        state = ENUM_STATE_STOP;
        return true;
    default:
        return false;
    }
}

double Timer::Log()
{
    double curTime = GetCurTime();
    double deltaTime = curTime - lastLogTime;
    switch (state)
    {
    case Basic::Timer::ENUM_STATE_RUNNING:
        wholeTime += deltaTime;
        logs.front() += deltaTime;
        logs.push_front(0);
        if (logs.size() > maxLogNum)
            logs.pop_back();
        lastLogTime = curTime;
        return deltaTime;
    default:
        return -1.0;
    }
}

void Timer::Reset()
{
    wholeTime = 0;
    logs.clear();
    state = ENUM_STATE_INIT;
}

double Timer::GetCurTime() const
{
    clock_t curTime = clock();
    return static_cast<double>(curTime) / static_cast<double>(CLOCKS_PER_SEC);
}

double Timer::GetWholeTime() const
{
    double curTime = GetCurTime();
    double deltaTime = curTime - lastLogTime;
    switch (state)
    {
    case Basic::Timer::ENUM_STATE_INIT:
        return 0;
        break;
    case Basic::Timer::ENUM_STATE_RUNNING:
        return wholeTime + deltaTime;
        break;
    case Basic::Timer::ENUM_STATE_STOP:
        return wholeTime;
        break;
    default:
        return -1.0;
        break;
    }
}

std::ostream &Timer::operator<<(std::ostream &os)
{
    os << "Whole Time : " << wholeTime << endl;
    for (size_t i = 0; i < logs.size(); i++)
    {
        os << "Log[" << i << "] : " << logs[i] << endl;
    }
    return os;
}