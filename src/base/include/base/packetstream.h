
#ifndef PacketStream_H
#define PacketStream_H


#include "base/packet.h"

#include <stdint.h>
#include <vector>
#include <mutex>
#include <functional>
namespace base {

    class PacketProcessor {
    public:

        PacketProcessor() {
        }

        /// This method performs processing on the given
        /// packet and emits the result.
        ///
        /// Note: If packet processing is async (the packet is not in
        /// the current thread scope) then packet data must be copied.
        /// Copied data can be freed directly aFter the async call to
        /// emit() the outgoing packet.
        virtual void process(IPacket& packet) {};

        /// This method ensures compatibility with the given
        /// packet type. Return false to reject the packet.

        virtual bool accepts(IPacket*) {
            return true;
        };

        virtual void emit(IPacket& packet) {

            if (cbProcess)
                cbProcess(packet);
        };
        /// This method ensures compatibility with the given
        /// packet type. Return false to reject the packet.

        std::function<void(IPacket&) > cbProcess;
        /// Stream operator alias for process()
        // virtual void operator<<(IPacket& packet) { process(packet); };

    };



    typedef std::vector<PacketProcessor*> PacketStreamVec;

    class PacketStream {
    public:

        PacketStream();

        virtual ~PacketStream();

        virtual void attachSource(PacketProcessor* source, bool freePointer = true);

        virtual void attach(PacketProcessor* proc, bool freePointer = true);


        PacketStreamVec sources() const;

        /// Returns a list of all stream processors.
        PacketStreamVec processors() const;


        void process(IPacket& packet);

        void start();
        void stop();
        
        
        void emit(IPacket& packet) {

            if (cbProcess)
                cbProcess(packet);
        };
        /// This method ensures compatibility with the given
        /// packet type. Return false to reject the packet.

        std::function<void(IPacket&) > cbProcess;
        

    private:
        mutable std::mutex _mutex;
        mutable std::mutex _procMutex;

        PacketStreamVec _sources;
        PacketStreamVec _processors;
        
        bool freeSources{false};
        bool freeProcessors{false};
        
        };


} // namespace base





#endif // PacketStream_H

