/*******************************************************************************
    Taichi - Physically based Computer Graphics Library

    Copyright (c) 2016 Yuanming Hu <yuanmhu@gmail.com>

    All rights reserved. Use of this source code is governed by
    the MIT license as written in the LICENSE file.
*******************************************************************************/

#pragma once

#include <server.h>
#include <taichi/visualization/pakua.h>

TC_NAMESPACE_BEGIN

class WebglPakua : public Pakua{
    std::vector<float> pakua_buffer;
    PakuaServer pakua_server;
public:
    void add_particle(Vector pos, Vector color) {
        // Add a particle to buffer
        pakua_buffer.push_back(pos.x);
        pakua_buffer.push_back(pos.y);
        pakua_buffer.push_back(pos.z);
        pakua_buffer.push_back(color.x);
        pakua_buffer.push_back(color.y);
        pakua_buffer.push_back(color.z);
    }

    void start() {
    }

    void finish() {
        pakua_server.send(pakua_buffer);
        pakua_buffer.clear();
    }
};

TC_NAMESPACE_END

