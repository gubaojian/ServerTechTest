//
//  common.h
//  websocketpp
//
//  Created by efurture on 2025/5/24.
//
#ifndef WSG_ROUTER_COMMON_H
#define WSG_ROUTER_COMMON_H

#include <string>
#include <iostream>
#include <uuid/uuid.h>
#include <chrono>
#include <map>
#include <set>
#include <cstdlib>
#include <unistd.h>
#include "simdjson/simdjson.h"

// This works around a bug in Boost <= 1.80.0 when using Clang >=18.
// See https://github.com/bitcoin/bitcoin/issues/30751.
// https://github.com/bitcoin/bitcoin/pull/30821/files
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wenum-constexpr-conversion"
#endif
#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <websocketpp/config/asio_client.hpp>
#include "websocketpp/client.hpp"

#if defined(__clang__)
#pragma clang diagnostic pop
#endif

// core_client.hpp中修改：
// enable_multithreading should be set to false, 两处

typedef websocketpp::client<websocketpp::config::asio_client> plain_client;
typedef websocketpp::client<websocketpp::config::asio_tls_client> tls_client;
typedef websocketpp::config::asio_client::message_type::ptr message_ptr;

struct WsgGateway {
    std::string wsgId;
    std::string url;
};

struct ws_connection_info{
    plain_client::connection_ptr conn = nullptr;
    std::shared_ptr<std::queue<std::string>> messageQueue = std::make_shared<std::queue<std::string>>();
    int64_t inMessageCount = 0;
    int64_t outMessageCount = 0;
    int32_t outRate = 0;
};

struct wss_connection_info{
    tls_client::connection_ptr conn = nullptr;
    std::shared_ptr<std::queue<std::string>> messageQueue = std::make_shared<std::queue<std::string>>();
    int64_t inMessageCount = 0;
    int64_t outMessageCount = 0;
    int32_t outRate = 0;
};




#endif
