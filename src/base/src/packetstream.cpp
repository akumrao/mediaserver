/* This file is part of mediaserver. A webrtc sfu server.
 * Copyright (C) 2018 Arvind Umrao <akumrao@yahoo.com> & Herman Umrao<hermanumrao@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 */

#include "base/packetstream.h"

#include "base/logger.h"

#include <assert.h>

namespace base {

    PacketStream::PacketStream() {
    }

    PacketStream::~PacketStream() {


        for (auto source : _sources) {
            std::lock_guard<std::mutex> guard(_procMutex);
            if (freeSources)
                delete source;
        }


        for (auto processor : _processors) {
            std::lock_guard<std::mutex> guard(_procMutex);
            if (freeProcessors)
                delete processor;

        }
        
        
        _sources.clear();
        _processors.clear();

    }
    
    
    void PacketStream::attachSource(PacketProcessor* source, bool freePointer) {
        LTrace("Attach source: ", source)

        freeSources = freePointer;                 
        std::lock_guard<std::mutex> guard(_mutex);
        _sources.push_back(source);

    }

   void PacketStream::attach(PacketProcessor* proc, bool freePointer) {
        // LTrace("Attach processor: ", proc)
        freeProcessors = freePointer; 
        std::lock_guard<std::mutex> guard(_mutex);
        _processors.push_back(proc);


    }
    void PacketStream::start() {
        using std::placeholders::_1;

        for (auto source : _sources) {
            std::lock_guard<std::mutex> guard(_procMutex);

            source->cbProcess = std::bind(&PacketStream::process, this, _1);

            base::Thread* startable = (base::Thread*) (source);
            if (startable) {
                startable->start();
            } else
                assert(0 && "unknown synchronizable");

        }

    }

    void PacketStream::stop() {

        for (auto source : _sources) {
            std::lock_guard<std::mutex> guard(_procMutex);

            base::Thread* startable = (base::Thread*) (source);
            if (startable) {
                startable->stop();
            } else
                assert(0 && "unknown synchronizable");

        }
    }



    void PacketStream::process(IPacket& packet) {
       // STrace << "Processing packet: " << ": " << packet.className();
        // assert(Thread::currentID() == _runner->tid());
         
        LTrace(packet.className());

        for (auto processor : _processors) {
            std::lock_guard<std::mutex> guard(_procMutex);

            if (processor->accepts(&packet))
                processor->process(packet);
            else
                processor->emit(packet);

        }
        
      // emit(packet);

    }

} // namespace base


