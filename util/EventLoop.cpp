/**
 * MIT License
 * Copyright (c) 2019 Anthony Rabine
 */

#include "EventLoop.h"

static long long Next(long long p)
{
    long long tp = std::chrono::steady_clock::now().time_since_epoch().count();
    return ((tp / p) * p) + p;
}

/*****************************************************************************/
EventLoop::EventLoop()
    : mStopRequested(false)
{

}
/*****************************************************************************/
EventLoop::~EventLoop()
{
    Stop();
}
/*****************************************************************************/
void EventLoop::Stop()
{
    mAccessGuard.lock();
    mStopRequested = true;
    mAccessGuard.unlock();

    if (mThread.joinable())
    {
        mThread.join();
    }
}
/*****************************************************************************/
void EventLoop::AddTimer(uint32_t period, CallBack callBack)
{
    Timer tm;

    tm.period = period;
    tm.callBack = callBack;
    tm.next = Next(period);

    mAccessGuard.lock();
    mTimers.push_back(tm);
    mAccessGuard.unlock();
}
/*****************************************************************************/
void EventLoop::Start()
{
    mThread = std::thread(&EventLoop::Loop, this);
}
/*****************************************************************************/
void EventLoop::Loop()
{
    mStopRequested = false;
    bool stop = false;

    while(!stop)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));

        mAccessGuard.lock();
        UpdateTimers();
        stop = mStopRequested;
        mAccessGuard.unlock();
    }
}
/*****************************************************************************/
void EventLoop::UpdateTimers()
{
    std::chrono::system_clock::time_point time_now = std::chrono::system_clock::now();
    time_t epoch_time = std::chrono::system_clock::to_time_t(time_now);

    int i = 0;
    for (auto &t : mTimers)
    {
        if (!(epoch_time % t.period))
        {
            t.next = Next(t.period);
            t.callBack();
        }
        i++;
    }
}
