/*******************************************************************************
    Taichi - Physically based Computer Graphics Library

    Copyright (c) 2016 Yuanming Hu <yuanmhu@gmail.com>

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
#include <taichi/util.h>
#include <taichi/visualization/pakua.h>

using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;
using websocketpp::connection_hdl;

typedef websocketpp::server<websocketpp::config::asio> server;
typedef std::set<connection_hdl,std::owner_less<connection_hdl> > con_list;

// pull out the type of messages sent by our config
typedef server::message_ptr message_ptr;
typedef std::set<connection_hdl,std::owner_less<connection_hdl> > con_list;

TC_NAMESPACE_BEGIN

using Vector = VectorND<3, real>;

class WebglPakuaServer : public Task {
    server echo_server;
    con_list echo_connections;

    void on_open(connection_hdl hdl) {
    }

    void on_close(connection_hdl hdl) {
        echo_connections.erase(hdl);
        printf("There is %lu monitors.\n", echo_connections.size());
    }

    void on_message(connection_hdl hdl, message_ptr msg) {
        if (msg->get_payload() == std::string("monitor")) {
            echo_connections.insert(hdl);
            printf("There is %lu monitors.\n", echo_connections.size());
        }
        else {
//            std::string ack_msg = "ack";
//            echo_server.send(hdl, ack_msg, websocketpp::frame::opcode::text);
            for (auto &connection : echo_connections) {
                echo_server.send(connection, msg->get_payload(), msg->get_opcode());
            }
        }
    }

    void run() override {
        int port = 9563;
        try {
            // Set logging settings
            echo_server.set_access_channels(websocketpp::log::alevel::all);
            echo_server.clear_access_channels(websocketpp::log::alevel::frame_payload);
            echo_server.init_asio();
            echo_server.set_open_handler(bind(&WebglPakuaServer::on_open,this,::_1));
            echo_server.set_close_handler(bind(&WebglPakuaServer::on_close,this,::_1));
            echo_server.set_message_handler(bind(&WebglPakuaServer::on_message, this, ::_1, ::_2));
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
};

TC_IMPLEMENTATION(Task, WebglPakuaServer, "pakua_server");

TC_NAMESPACE_END