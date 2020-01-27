
#ifndef base_PacketQueue_H
#define base_PacketQueue_H


#include "base/base.h"
#include "base/queue.h"
#include "base/application.h"
namespace base {


//
// Synchronized Packet Queue
//
/*
 * Syncqueue is fifo. So do it with pipe of libuv
template <class T = IPacket>
class SyncPacketQueue : public SyncQueue<T>, public PacketProcessor
{
public:
    typedef SyncQueue<T> Queue;
    typedef PacketProcessor Processor;

    SyncPacketQueue(uv::Loop* loop, int maxSize = 1024)
        : Queue(loop, maxSize)
        , Processor(this->emitter)
    {
    }

    SyncPacketQueue(int maxSize = 1024)
        : Queue(uv::defaultLoop(), maxSize)
        , Processor(this->emitter)
    {
    }

    virtual ~SyncPacketQueue() {}

    virtual void process(IPacket& packet) override;
    virtual bool accepts(IPacket* packet) override;

    PacketSignal emitter;

protected:
    virtual void dispatch(T& packet);

    virtual void onStreamStateChange(const PacketStreamState&);
};


template <class T>
inline void SyncPacketQueue<T>::process(IPacket& packet)
{
    if (Queue::cancelled()) {
        LWarn("Process late packet")
        assert(0);
        return;
    }

    Queue::push(reinterpret_cast<T*>(packet.clone()));
}


template <class T>
inline void SyncPacketQueue<T>::dispatch(T& packet)
{
    if (Queue::cancelled()) {
        LWarn("Dispatch late packet")
        assert(0);
        return;
    }

    // Note: Emit should never be called after closure.
    // Any late packets should have been dealt with
    // and dropped by the run() function.
    Processor::emit(packet);
}


template <class T>
inline bool SyncPacketQueue<T>::accepts(IPacket* packet)
{
    return dynamic_cast<T*>(packet) != 0;
}


*/

//
// Asynchronous Packet Queue
//

template <class T = IPacket>
class AsyncPacketQueue : public AsyncQueue<T>, public PacketProcessor
{
public:
    typedef AsyncQueue<T> Queue;
    typedef PacketProcessor Processor;

    AsyncPacketQueue(int maxSize = 1024)
        : Queue(maxSize)
        , Processor()
    {
    }

    virtual ~AsyncPacketQueue() {}

    virtual void close();

    virtual void process(IPacket& packet) override;
    virtual bool accepts(IPacket* packet) override;


protected:
    virtual void dispatch(T& packet);


};


template <class T>
inline void AsyncPacketQueue<T>::close()
{
    // Flush queued items, some protocols can't afford dropped packets
    Queue::flush();
    assert(Queue::empty());
    Queue::stop();
    Queue::join();
}


template <class T> inline void
AsyncPacketQueue<T>::dispatch(T& packet)
{
    if (Queue::stopped()) {
        LWarn("Dispatch late packet")
        assert(0);
        return;
    }

    LTrace("emit " , packet.data())
    Processor::emit(packet);
}


template <class T>
inline void AsyncPacketQueue<T>::process(IPacket& packet)
{
    if (Queue::stopped()) {
        LWarn("Process late packet")
        assert(0);
        return;
    }

  //  this->push(reinterpret_cast<T*>(packet.clone()));
    this->push(reinterpret_cast<T*>(&packet));
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
       _startTime = Application::GetTime(); // time in milliscond
    }

    virtual ~RealtimePacketQueue()
    {
    }

    // Add an item to the queue
    virtual void push(PacketT* item) override
    {
         STrace << "push size: " << BaseQueue::size() << ": "
            << realTime() << " > " << item->time << std::endl;
         
        BaseQueue::push(item);
        BaseQueue::template sort<MediaPacketTimeCompare>();
    }

    // Return the current duration from stream start in microseconds
    int64_t realTime()
    {
      //  return (time::hrtime()/*nanosecond time*/ - _startTime) / 1000;
        return (Application::GetTime() - _startTime);
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

        STrace << "Pop size: " << BaseQueue::size() << ": "
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



} // namespace base


#endif // base_PacketQueue_H


