#pragma once
#include "stdafx.h"
#include <websocketpp/config/asio_no_tls_client.hpp>
#include <websocketpp/client.hpp>

#include <websocketpp/common/thread.hpp>
#include <websocketpp/common/memory.hpp>


#include <map>
#include <string>
#include <queue>

typedef websocketpp::client<websocketpp::config::asio_client> client;


class CWebSocketHandler {
public:

    class CConnectionMetadata {
    public:
        typedef websocketpp::lib::shared_ptr<CConnectionMetadata> ptr;
        CConnectionMetadata(int id, websocketpp::connection_hdl hdl, std::string uri);

        void on_open(client* c, websocketpp::connection_hdl hdl);
        void on_fail(client* c, websocketpp::connection_hdl hdl);
        void on_close(client* c, websocketpp::connection_hdl hdl);
        void on_message(websocketpp::connection_hdl, client::message_ptr msg);

    public:
        inline websocketpp::connection_hdl GetHDL() const {return m_hdl;}
        inline int GetID() const {return m_id;}
        inline std::string GetStatus() const {return m_status;}
        inline int GetMessageBufferLength() {return m_messages.size();}

        bool PopRecvMessage(std::string* buffer);
        bool PeekRecvMessage(std::string* buffer);

    private:
        int m_id;
        websocketpp::connection_hdl m_hdl;
        std::string m_status;
        std::string m_uri;
        std::string m_server;
        std::string m_error_reason;
        std::queue<std::string> m_messages;
    };


public:
    typedef std::map<int, CConnectionMetadata::ptr> con_list;

    CWebSocketHandler();
    ~CWebSocketHandler();

    int Connect(std::string const& uri);
    bool Close(int id, std::string reason = "", websocketpp::close::status::value code = websocketpp::close::status::normal);
    bool Send(int id, std::string message);
    bool Recv(int id, std::string* buffer);
    bool IsConnected(int id);

    CConnectionMetadata::ptr GetMetadata(int id);

private:
    client m_endpoint;
    websocketpp::lib::shared_ptr<websocketpp::lib::thread> m_thread;

    con_list m_connection_list;
    int m_next_id;
};

