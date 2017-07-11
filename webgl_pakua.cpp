/*******************************************************************************
    Taichi - Physically based Computer Graphics Library

    Copyright (c) 2017 Yu Fang <squarefk@gmail.com>

    All rights reserved. Use of this source code is governed by
    the MIT license as written in the LICENSE file.
*******************************************************************************/

#define ASIO_STANDALONE

#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/config/asio_no_tls_client.hpp>
#include <websocketpp/client.hpp>
#include <thread>
#include <set>
#include <mutex>
#include <vector>
#include <taichi/visualization/pakua.h>

using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;
using websocketpp::connection_hdl;

typedef websocketpp::client<websocketpp::config::asio_client> client;
typedef websocketpp::config::asio_client::message_type::ptr message_ptr;

TC_NAMESPACE_BEGIN

// implement websocket client
class WebglPakua : public Pakua {
    client echo_client;
    websocketpp::lib::shared_ptr<websocketpp::lib::thread> echo_thread;
    websocketpp::lib::error_code echo_ec;

    std::vector<real> point_buffer;
    std::vector<real> line_buffer;
    std::vector<real> triangle_buffer;
    websocketpp::connection_hdl data_hdl;

#define CHECK_EC \
        if (echo_ec) { \
            printf("Connection initialization error : %s.\n", echo_ec.message().c_str()); \
            assert(false); \
        }

public:
    ~WebglPakua() {
        echo_client.stop_perpetual();
        echo_client.close(data_hdl, websocketpp::close::status::going_away, "", echo_ec);
        CHECK_EC
        echo_thread->join();
    }

    void initialize(const Config &config) {
        Pakua::initialize(config);
        int port = config.get_int("port");
        echo_client.clear_access_channels(websocketpp::log::alevel::frame_header);
        echo_client.clear_access_channels(websocketpp::log::alevel::frame_payload);
        echo_client.init_asio();
        echo_client.start_perpetual();
        auto th = new websocketpp::lib::thread(&client::run, &echo_client);
        echo_thread.reset(th);
        printf("WebGL Pakua Client runs in new thread.\n");
        std::string uri = std::string("ws://localhost:") + std::to_string(port);

        // establish data connection
        client::connection_ptr data_con = echo_client.get_connection(uri.c_str(), echo_ec);
        CHECK_EC
        data_hdl = data_con->get_handle();
        echo_client.connect(data_con);
    }

    void add_point(Vector pos, Vector color) {
        for (int i = 0; i < 3; i++)
            point_buffer.push_back(pos[i]);
        for (int i = 0; i < 3; i++)
            point_buffer.push_back(color[i]);
    }

    void add_line(const std::vector<Vector> &pos_v, const std::vector<Vector> &color_v) {
        int number = (int)pos_v.size();
        for (int i = 0; i < number; i++) {
            for (int j = 0; j < 3; j++)
                line_buffer.push_back(pos_v[i][j]);
            for (int j = 0; j < 3; j++)
                line_buffer.push_back(color_v[i][j]);
        }
    }

    void add_triangle(const std::vector<Vector> &pos_v, const std::vector<Vector> &color_v) {
        int number = (int)pos_v.size();
        for (int i = 0; i < number; i++) {
            for (int j = 0; j < 3; j++)
                triangle_buffer.push_back(pos_v[i][j]);
            for (int j = 0; j < 3; j++)
                triangle_buffer.push_back(color_v[i][j]);
        }
    }

    void start() {
    }

    void finish() {
        auto send_single_kind = [&](const std::string &kind, std::vector<float> &buffer) {
            echo_client.send(data_hdl, kind, websocketpp::frame::opcode::text, echo_ec);
            CHECK_EC
            echo_client.send(data_hdl, &buffer[0], buffer.size() * sizeof(real),
                             websocketpp::frame::opcode::binary, echo_ec);
            CHECK_EC
            buffer.clear();
        };
        send_single_kind("point", point_buffer);
        send_single_kind("line", line_buffer);
        send_single_kind("triangle", triangle_buffer);
    }

#undef CHECK_EC
};

TC_IMPLEMENTATION(Pakua, WebglPakua, "webgl");

TC_NAMESPACE_END

