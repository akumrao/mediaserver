/* This file is part of mediaserver. A webrtc sfu server.
 * Copyright (C) 2018 Arvind Umrao <akumrao@yahoo.com> & Herman Umrao<hermanumrao@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 */


#ifndef base_Base_Tests_H
#define base_Base_Tests_H


#include "base/test.h"
#include "base/logger.h"
#include "base/filesystem.h"
#include "base/platform.h"
#include "base/queue.h"

using std::cout;
using std::cerr;
using std::endl;
using base::test::Test;


namespace base {

template <class T = int>
class mySyncQueue : public SyncQueue<T>
{
public:
    typedef SyncQueue<T> Queue;
     //typedef SyncQueue<T> Queue;
    mySyncQueue(int maxSize = 1024);
      
    // virtual ~Thread2(void);

    void dispatch(T& item);

};

}

#endif // base_Base_Tests_H

