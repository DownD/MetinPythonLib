#include "stdafx.h"
#include "WebsocketHandler.h"
#include "../common/utils.h"

CWebSocketHandler::CConnectionMetadata::CConnectionMetadata(int id, websocketpp::connection_hdl hdl, std::string uri)
    : m_id(id)
    , m_hdl(hdl)
    , m_status("Connecting")
    , m_uri(uri)
    , m_server("N/A")
{}

void CWebSocketHandler::CConnectionMetadata::on_open(client* c, websocketpp::connection_hdl hdl) {
    m_status = "Open";

    DEBUG_INFO_LEVEL_3("Websocket on_open recived");
    client::connection_ptr con = c->get_con_from_hdl(hdl);
    m_server = con->get_response_header("Server");
}

void CWebSocketHandler::CConnectionMetadata::on_fail(client* c, websocketpp::connection_hdl hdl) {
    m_status = "Failed";

    DEBUG_INFO_LEVEL_3("Websocket on_fail recived");
    client::connection_ptr con = c->get_con_from_hdl(hdl);
    m_server = con->get_response_header("Server");
    m_error_reason = con->get_ec().message();
}

void CWebSocketHandler::CConnectionMetadata::on_close(client* c, websocketpp::connection_hdl hdl) {
    m_status = "Closed";
    client::connection_ptr con = c->get_con_from_hdl(hdl);

    DEBUG_INFO_LEVEL_3("Websocket on_close recived");
    std::stringstream s;
    s << "close code: " << con->get_remote_close_code() << " ("
        << websocketpp::close::status::get_string(con->get_remote_close_code())
        << "), close reason: " << con->get_remote_close_reason();
    m_error_reason = s.str();
}

void CWebSocketHandler::CConnectionMetadata::on_message(websocketpp::connection_hdl, client::message_ptr msg) {
    DEBUG_INFO_LEVEL_4("Websocket on_message recived");
    if (msg->get_opcode() == websocketpp::frame::opcode::text) {
        m_messages.push(msg->get_payload());
    }
    else {
        m_messages.push(websocketpp::utility::to_hex(msg->get_payload()));
    }
}

bool CWebSocketHandler::CConnectionMetadata::PopRecvMessage(std::string* buffer) {
    if (m_messages.size() > 0) {
        *buffer = m_messages.front();
        m_messages.pop();
        return true;
    }
    return false;
}

bool CWebSocketHandler::CConnectionMetadata::PeekRecvMessage(std::string* buffer) {
    if (m_messages.size() > 0) {
        *buffer = m_messages.front();
        return true;
    }
    return false;
}

CWebSocketHandler::CWebSocketHandler() : m_next_id(0) {
    m_endpoint.clear_access_channels(websocketpp::log::alevel::all);
    m_endpoint.clear_error_channels(websocketpp::log::elevel::all);

    m_endpoint.init_asio();
    m_endpoint.start_perpetual();

    m_thread = websocketpp::lib::make_shared<websocketpp::lib::thread>(&client::run, &m_endpoint);
}

CWebSocketHandler::~CWebSocketHandler() {
    m_endpoint.stop_perpetual();

    DEBUG_INFO_LEVEL_2("Destroying Websocket handler");
    for (con_list::const_iterator it = m_connection_list.begin(); it != m_connection_list.end(); ++it) {
        if (it->second->GetStatus() != "Open") {
            // Only close open connections
            continue;
        }

        DEBUG_INFO_LEVEL_2("Websocket Closing connection %d", it->second->GetID());

        websocketpp::lib::error_code ec;
        m_endpoint.close(it->second->GetHDL(), websocketpp::close::status::going_away, "", ec);
        if (ec) {
            DEBUG_INFO_LEVEL_1("Websocket error closing connection %d: %s", it->second->GetID(), ec.message().c_str());
        }
    }

    m_thread->join();
}

int CWebSocketHandler::Connect(std::string const& uri) {
    websocketpp::lib::error_code ec;

    client::connection_ptr con = m_endpoint.get_connection(uri, ec);

    if (ec) {
        DEBUG_INFO_LEVEL_1("Websocket Connect initialization error: %s", ec.message().c_str());
        return -1;
    }

    int new_id = m_next_id++;
    CConnectionMetadata::ptr metadata_ptr = websocketpp::lib::make_shared<CConnectionMetadata>(new_id, con->get_handle(), uri);
    m_connection_list[new_id] = metadata_ptr;

    con->set_open_handler(websocketpp::lib::bind(
        &CConnectionMetadata::on_open,
        metadata_ptr,
        &m_endpoint,
        websocketpp::lib::placeholders::_1
    ));
    con->set_fail_handler(websocketpp::lib::bind(
        &CConnectionMetadata::on_fail,
        metadata_ptr,
        &m_endpoint,
        websocketpp::lib::placeholders::_1
    ));
    con->set_close_handler(websocketpp::lib::bind(
        &CConnectionMetadata::on_close,
        metadata_ptr,
        &m_endpoint,
        websocketpp::lib::placeholders::_1
    ));
    con->set_message_handler(websocketpp::lib::bind(
        &CConnectionMetadata::on_message,
        metadata_ptr,
        websocketpp::lib::placeholders::_1,
        websocketpp::lib::placeholders::_2
    ));


    m_endpoint.connect(con);

    return new_id;
}

bool CWebSocketHandler::Close(int id, std::string reason, websocketpp::close::status::value code) {
    websocketpp::lib::error_code ec;

    con_list::iterator metadata_it = m_connection_list.find(id);
    if (metadata_it == m_connection_list.end()) {
        DEBUG_INFO_LEVEL_1("Websocket no connection found with id %d", id);
        return false;
    }

    m_endpoint.close(metadata_it->second->GetHDL(), code, reason, ec);
    if (ec) {
        DEBUG_INFO_LEVEL_1("Websocket error initiating close: %s", ec.message().c_str());
        return false;
    }
    m_connection_list.erase(metadata_it);
    return true;
}

bool CWebSocketHandler::Send(int id, std::string message) {
    websocketpp::lib::error_code ec;

    con_list::iterator metadata_it = m_connection_list.find(id);
    if (metadata_it == m_connection_list.end()) {
        DEBUG_INFO_LEVEL_1("Websocket send no connection found with id %d", id);
        return false;
    }

    m_endpoint.send(metadata_it->second->GetHDL(), message, websocketpp::frame::opcode::text, ec);
    if (ec) {
        DEBUG_INFO_LEVEL_1("Websocket error Sending Message: %s", ec.message().c_str());
        return false;
    }

    return true;
}

bool CWebSocketHandler::Recv(int id, std::string* buffer) {
    con_list::iterator metadata_it = m_connection_list.find(id);
    if (metadata_it == m_connection_list.end()) {
        DEBUG_INFO_LEVEL_1("Websocket recv no connection found with id %d", id);
        return false;
    }
    auto metadata = metadata_it->second;
    return metadata_it->second->PopRecvMessage(buffer);
}

bool CWebSocketHandler::IsConnected(int id)
{
    con_list::iterator metadata_it = m_connection_list.find(id);
    if (metadata_it == m_connection_list.end()) {
        return false;
    }
    std::string s= metadata_it->second->GetStatus();
    if (s != "Open" &&  s != "Connecting") {
        m_connection_list.erase(metadata_it);
        return false;
    }

    return true;
}

CWebSocketHandler::CConnectionMetadata::ptr CWebSocketHandler::GetMetadata(int id)
{
    con_list::const_iterator metadata_it = m_connection_list.find(id);
    if (metadata_it == m_connection_list.end()) {
        return CConnectionMetadata::ptr();
    }
    else {
        return metadata_it->second;
    }
}
