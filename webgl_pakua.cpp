/*******************************************************************************
    Taichi - Physically based Computer Graphics Library

    Copyright (c) 2017 Yu Fang <squarefk@gmail.com>

    All rights reserved. Use of this source code is governed by
    the MIT license as written in the LICENSE file.
*******************************************************************************/

#define _WEBSOCKETPP_CPP11_THREAD_
#define ASIO_STANDALONE

#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/config/asio_no_tls_client.hpp>
#include <websocketpp/client.hpp>
#include <thread>
#include <set>
#include <mutex>
#include <vector>
#include <taichi/visualization/pakua.h>
#include <taichi/visualization/image_buffer.h>

using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;
using websocketpp::connection_hdl;

typedef websocketpp::client<websocketpp::config::asio_client> client;
typedef websocketpp::config::asio_client::message_type::ptr message_ptr;

TC_NAMESPACE_BEGIN

// implement websocket client
class WebglPakua : public Pakua {
    client pakua_client;
    websocketpp::lib::shared_ptr<websocketpp::lib::thread> client;
    websocketpp::lib::error_code echo_ec;

    std::vector<real> point_buffer;
    std::vector<real> line_buffer;
    std::vector<real> triangle_buffer;
    websocketpp::connection_hdl data_hdl;
    int frame_count;

#define CHECK_EC \
        if (echo_ec) { \
            printf("Connection initialization error : %s.\n", echo_ec.message().c_str()); \
            assert(false); \
        }

public:
    ~WebglPakua() {
        pakua_client.stop_perpetual();
        pakua_client.close(data_hdl, websocketpp::close::status::going_away, "", echo_ec);
        CHECK_EC
        client->join();
    }

    void initialize(const Config &config) {
        Pakua::initialize(config);
        frame_count = 0;
        int port = config.get_int("port");
        pakua_client.clear_access_channels(websocketpp::log::alevel::frame_header);
        pakua_client.clear_access_channels(websocketpp::log::alevel::frame_payload);
        pakua_client.init_asio();
        pakua_client.start_perpetual();
        auto th = new websocketpp::lib::thread(&client::run, &pakua_client);
        client.reset(th);
        printf("WebGL Pakua Client runs in new thread.\n");
        std::string uri = std::string("ws://localhost:") + std::to_string(port);

        // establish data connection
        client::connection_ptr data_con = pakua_client.get_connection(uri.c_str(), echo_ec);
        CHECK_EC
        data_hdl = data_con->get_handle();
        pakua_client.connect(data_con);
        pakua_client.set_message_handler(bind(&WebglPakua::on_message, this, ::_1, ::_2));
    }

    void on_message(connection_hdl hdl, message_ptr msg) {
        if (msg->get_payload() == std::string("screen")) {
            P("screen")
        } else if (msg->get_payload() == std::string("taichi")) {
        }
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
            pakua_client.send(data_hdl, kind, websocketpp::frame::opcode::text, echo_ec);
            CHECK_EC
            pakua_client.send(data_hdl, &buffer[0], buffer.size() * sizeof(real),
                              websocketpp::frame::opcode::binary, echo_ec);
            CHECK_EC
            buffer.clear();
        };
        send_single_kind("point", point_buffer);
        send_single_kind("line", line_buffer);
        send_single_kind("triangle", triangle_buffer);
        std::vector<real> frame_id_buffer;
        frame_id_buffer.push_back(frame_count);
        send_single_kind("frame_id", frame_id_buffer);

        frame_count += 1;
    }

    Array2D<Vector3> screenshot() {
        pakua_client.send(data_hdl, "screenshot", websocketpp::frame::opcode::text, echo_ec);
        CHECK_EC
    }

#undef CHECK_EC
};

TC_IMPLEMENTATION(Pakua, WebglPakua, "webgl");

TC_NAMESPACE_END

