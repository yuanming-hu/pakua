/*******************************************************************************
    Taichi - Physically based Computer Graphics Library

    Copyright (c) 2016 Yuanming Hu <yuanmhu@gmail.com>

    All rights reserved. Use of this source code is governed by
    the MIT license as written in the LICENSE file.
*******************************************************************************/

#include <webgl_pakua.h>
#include <taichi/util.h>

TC_NAMESPACE_BEGIN

using Vector = VectorND<3, real>;

class PakuaDemo : public Task {
    void run() override {
        while (1) {
            WebglPakua p;
            p.start();
            for (int i = 0; i < 10; i++) {
                p.add_particle(Vector::rand(), Vector::rand());
            }
            p.finish();
        }
    }
};

TC_IMPLEMENTATION(Task, PakuaDemo, "pakua_demo");

TC_NAMESPACE_END