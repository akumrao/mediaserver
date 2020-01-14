#ifndef TIMER_H
#define TIMER_H
#include <uv.h>
#include <functional>


namespace base
{

    class Timer
    {
    public:

        class Listener
        {
        public:
            virtual ~Listener() = default;

        public:
            virtual void OnTimer(Timer* timer, int timerID) = 0;
        };

    public:
        explicit Timer(Listener* listener,int timerID=-1);
        Timer& operator=(const Timer&) = delete;
        Timer(const Timer&) = delete;
        ~Timer();

    public:
        void Close();
        void Start(uint64_t timeout, uint64_t repeat = 0);
        void Stop();
        void Reset();
        void Restart();
        bool IsActive() const;

        /* Callbacks fired by UV events. */
    public:
        void OnUvTimer(int timerID);
        int timerID{-1}; //
        std::function<void(void) > cb_timeout;
    private:
        // Passed by argument.
        Listener* listener{ nullptr};
        // Allocated by this.
        uv_timer_t* uvHandle{ nullptr};
        // Others.
        bool closed{ false};
        uint64_t timeout{ 0};
        uint64_t repeat{ 0};
    };

    /* Inline methods. */

    inline bool Timer::IsActive() const {
        return uv_is_active(reinterpret_cast<uv_handle_t*> (this->uvHandle)) != 0;
    }

} // namespace base
#endif //TIMER_H
