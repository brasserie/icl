#ifndef EVENT_LOOP_H
#define EVENT_LOOP_H

#include <functional>
#include <vector>
#include <thread>
#include <chrono>
#include <mutex>


class EventLoop
{
public:
    EventLoop();
    ~EventLoop();


    typedef std::function<void (void)> CallBack;

    void Run();
    void Stop();

    // From IEventLoop
    void AddTimer(std::uint32_t period, CallBack callBack);

private:
    bool mStopRequested;
    std::thread mThread;
    std::mutex mAccessGuard;

    struct Timer
    {
        std::uint32_t period;
        CallBack callBack;
        long next;
    };

    std::vector<Timer> mTimers;

    void UpdateTimers();
};

#endif // EVENT_LOOP_H
