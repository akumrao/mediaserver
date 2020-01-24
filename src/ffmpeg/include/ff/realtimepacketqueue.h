

#ifndef BASE_RealtimePacketQueue_H
#define BASE_RealtimePacketQueue_H


#include "ff/packet.h"
#include "base/base.h"
#include "base/queue.h"
//#include "base/packetqueue.h"


namespace base {
namespace ff {

    template <class T = IPacket>
class AsyncPacketQueue : public AsyncQueue<T>, public PacketProcessor
{
public:
    typedef AsyncQueue<T> Queue;
    typedef PacketProcessor Processor;

    AsyncPacketQueue(int maxSize = 1024)
        : Queue(maxSize)
        , Processor(this->emitter)
    {
    }

    virtual ~AsyncPacketQueue() {}

    virtual void close();

    virtual void process(IPacket& packet) override;
    virtual bool accepts(IPacket* packet) override;

//    PacketSignal emitter;

protected:
    virtual void dispatch(T& packet);

//    virtual void onStreamStateChange(const PacketStreamState&);
};


template <class T>
inline void AsyncPacketQueue<T>::close()
{
    // Flush queued items, some protocols can't afford dropped packets
    Queue::flush();
    assert(Queue::empty());
    Queue::cancel();
    Queue::_thread.join();
}


template <class T> inline void
AsyncPacketQueue<T>::dispatch(T& packet)
{
    if (Queue::cancelled()) {
        LWarn("Dispatch late packet")
        assert(0);
        return;
    }

    Processor::emit(packet);
}


template <class T>
inline void AsyncPacketQueue<T>::process(IPacket& packet)
{
    if (Queue::cancelled()) {
        LWarn("Process late packet")
        assert(0);
        return;
    }

    this->push(reinterpret_cast<T*>(packet.clone()));
}


template <class T>
inline bool AsyncPacketQueue<T>::accepts(IPacket* packet)
{
    return dynamic_cast<T*>(packet) != 0;
}


  

/// This class emits media packets based on their realtime pts value.
template <class PacketT>
class RealtimePacketQueue : public AsyncPacketQueue<PacketT>
{
public:
    typedef AsyncPacketQueue<PacketT> BaseQueue;

    RealtimePacketQueue(int maxSize = 1024)
        : BaseQueue(maxSize)
    {
    }

    virtual ~RealtimePacketQueue()
    {
    }

    // Add an item to the queue
    virtual void push(PacketT* item) override
    {
        BaseQueue::push(item);
        BaseQueue::template sort<MediaPacketTimeCompare>();
    }

    // Return the current duration from stream start in microseconds
    int64_t realTime()
    {
        return (time::hrtime() - _startTime) / 1000;
    }

protected:
    virtual PacketT* popNext() override
    {
        if (BaseQueue::empty())
            return nullptr;

        auto next = BaseQueue::front();
        if (next->time > realTime())
            return nullptr;
        BaseQueue::pop();

        STrace << "Pop next: " << BaseQueue::size() << ": "
            << realTime() << " > " << next->time << std::endl;
        return next;
    }



    struct MediaPacketTimeCompare
    {
        bool operator()(const MediaPacket* a, const MediaPacket* b) {
            return a->time < b->time;
        }
    };

    int64_t _startTime;
};


} // namespace ff
} // namespace base


#endif // BASE_RealtimePacketQueue_H

