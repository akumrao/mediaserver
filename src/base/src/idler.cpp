/* This file is part of mediaserver. A webrtc sfu server.
 * Copyright (C) 2018 Arvind Umrao <akumrao@yahoo.com> & Herman Umrao<hermanumrao@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 */


#include "base/logger.h"
#include "base/application.h"
#include "base/idler.h"

#include <assert.h>
#include <iostream>


using std::cout;
using std::cerr;
using std::endl;


namespace base {

    static void runit(uv_idle_t* handle) {

        Idler *idler = (Idler*) handle->data;
        idler->run();
    }

    Idler::Idler(std::function<void() > cbfun) : cbfun(cbfun) {
        start();
    }

    Idler::Idler() {
    }

    void Idler::start() {
        uv_idle_init(Application::uvGetLoop(), &idler);
        idler.data = this;
        uv_idle_start(&idler, runit);
    }

    Idler::~Idler() {
        stop();
    }

    void Idler::run() {
        if (cbfun)
            cbfun();
    }

    void Idler::stop() {
        uv_idle_stop(&idler);
    }


} // namespace base


