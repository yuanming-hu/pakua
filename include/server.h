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
#include <vector>

using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::connection_hdl;
using websocketpp::lib::bind;

typedef websocketpp::server<websocketpp::config::asio> server;
typedef std::set<connection_hdl,std::owner_less<connection_hdl> > con_list;

// pull out the type of messages sent by our config
typedef server::message_ptr message_ptr;

class PakuaServer {

    server echo_server;
    con_list echo_connections;

    void on_open(connection_hdl hdl) {
        echo_connections.insert(hdl);
    }

    void on_close(connection_hdl hdl) {
        echo_connections.erase(hdl);
    }

    // Define a callback to handle incoming messages
    void on_message(connection_hdl hdl, message_ptr msg) {
        std::cout << "on_message called with hdl: " << hdl.lock().get()
                  << " and message: " << msg->get_payload()
                  << std::endl;

        // check for a special command to instruct the server to stop listening so
        // it can be cleanly exited.
        if (msg->get_payload() == "stop-listening") {
            echo_server.stop_listening();
            return;
        }

        try {
            echo_server.send(hdl, msg->get_payload(), msg->get_opcode());
        } catch (const websocketpp::lib::error_code &e) {
            std::cout << "Echo failed because: " << e
                      << "(" << e.message() << ")" << std::endl;
        }
    }

public:
    void init(int port) {
        try {
            // Set logging settings
            echo_server.set_access_channels(websocketpp::log::alevel::all);
            echo_server.clear_access_channels(websocketpp::log::alevel::frame_payload);

            std::cout << "Initializing ASIO..." << std::endl;
            echo_server.init_asio();

            echo_server.set_open_handler(bind(&PakuaServer::on_open, this, ::_1));
            echo_server.set_close_handler(bind(&PakuaServer::on_close, this, ::_1));
            echo_server.set_message_handler(bind(&PakuaServer::on_message, this, ::_1, ::_2));
            echo_server.listen(port);

            echo_server.start_accept();

            std::cout << "Running ..." << std::endl;

            echo_server.run();

            std::cout << "Exit." << std::endl;

        } catch (websocketpp::exception const &e) {
            std::cout << e.what() << std::endl;
        } catch (...) {
            std::cout << "other exception" << std::endl;
        }

    }

    void send(std::vector<float> buffer) {
        auto send_lambda = [&](const std::vector<float>& buffer) {
            for (auto it = echo_connections.begin(); it != echo_connections.end(); ++it) {
                echo_server.send(*it, &buffer, buffer.size() * sizeof(float), websocketpp::frame::opcode::binary);
            }
        };
        std::thread t(send_lambda, buffer);
        t.join();
    }
};
