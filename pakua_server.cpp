/*******************************************************************************
    Taichi - Physically based Computer Graphics Library

    Copyright (c) 2017 Yu Fang <squarefk@gmail.com>

    All rights reserved. Use of this source code is governed by
    the MIT license as written in the LICENSE file.
*******************************************************************************/

#define ASIO_STANDALONE

#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#include <thread>
#include <set>
#include <mutex>
#include <vector>
#include <taichi/visualization/pakua.h>

using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;
using websocketpp::connection_hdl;

typedef websocketpp::server<websocketpp::config::asio> server;
typedef std::set<connection_hdl,std::owner_less<connection_hdl> > con_list;

// pull out the type of messages sent by our config
typedef server::message_ptr message_ptr;

TC_NAMESPACE_BEGIN

class WebglPakua : public Pakua {
    server echo_server;
    std::vector<float> echo_buffer;
    std::vector<float> echo_buffer_cache;
    std::mutex echo_mutex;

    void on_message(connection_hdl hdl, message_ptr msg) {
        std::cout << "on_message called with hdl: " << hdl.lock().get()
                  << " and message: " << msg->get_payload()
                  << std::endl;
//        if (msg->get_payload() == "stop-listening") {
//            echo_server.stop_listening();
//            return;
//        }
        try {
            echo_mutex.lock();
            echo_server.send(hdl, &echo_buffer[0], echo_buffer.size() * sizeof(float), websocketpp::frame::opcode::binary);
            echo_mutex.unlock();
        } catch (const websocketpp::lib::error_code &e) {
            std::cout << "Echo failed because: " << e
                      << "(" << e.message() << ")" << std::endl;
        }

    }

    void loop_body(int port) {
        try {
            // Set logging settings
            echo_server.set_access_channels(websocketpp::log::alevel::all);
            echo_server.clear_access_channels(websocketpp::log::alevel::frame_payload);
            echo_server.init_asio();
            echo_server.set_message_handler(bind(&WebglPakua::on_message, this, ::_1, ::_2));
            echo_server.listen(port);
            echo_server.start_accept();
            std::cout << "Running ..." << std::endl;
            echo_server.run();
        } catch (websocketpp::exception const &e) {
            std::cout << e.what() << std::endl;
        } catch (...) {
            std::cout << "other exception" << std::endl;
        }
    }


public:
    void initialize(const Config &config) {
        Pakua::initialize(config);
        std::thread t(&WebglPakua::loop_body, this, config.get_int("port"));
        t.detach();
        printf("WebGL Pakua Server start.\n");
    }

    void add_particle(Vector pos, Vector color) {
        // Add a particle to buffer
        echo_buffer_cache.push_back(pos.x);
        echo_buffer_cache.push_back(pos.y);
        echo_buffer_cache.push_back(pos.z);
        echo_buffer_cache.push_back(color.x);
        echo_buffer_cache.push_back(color.y);
        echo_buffer_cache.push_back(color.z);
    }

    void start() {
    }

    void finish() {
        echo_mutex.lock();
        echo_buffer.swap(echo_buffer_cache);
        echo_mutex.unlock();
        echo_buffer_cache.clear();
    }
};

TC_IMPLEMENTATION(Pakua, WebglPakua, "webgl");

TC_NAMESPACE_END

