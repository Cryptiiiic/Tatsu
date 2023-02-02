//
// Created by cryptic on 1/31/23.
//

#ifndef TATSU_REQUESTS_HPP
#define TATSU_REQUESTS_HPP

#pragma once
#include <ixwebsocket/IXHttpClient.h>


class Requests {
private:
    ix::HttpClient *mHttpClient = nullptr;
    ix::SocketTLSOptions *mTLS = nullptr;
    ix::WebSocketHttpHeaders *mHeaders = nullptr;
    ix::HttpRequestArgsPtr mArgs = nullptr;
    std::string mURL;
public:
    Requests();
    void setupTLS();
    void setupHeaders();
    void setupArgs(const std::string &requestType, const std::string &body);
    bool sendPOST(std::string &body);
    ~Requests();
};


#endif //TATSU_REQUESTS_HPP
