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
        printf("Pakua demo starts.\n");
        WebglPakua p;
        Config config;
        config.set("port", 9563);
        p.initialize(config);
        while (1) {
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