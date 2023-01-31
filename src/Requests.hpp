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
    std::string mURL;
public:
    Requests();
    bool sendPOST(std::string &data);
    ~Requests();
};


#endif //TATSU_REQUESTS_HPP
