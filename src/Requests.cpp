//
// Created by cryptic on 1/31/23.
//

#include <Requests.hpp>
#include <Timer.hpp>
#include <fmt/core.h>
#include <fmt/color.h>

Requests::Requests() {
    this->mHttpClient = new ix::HttpClient(true);
    this->mURL = std::string("http://gs.apple.com/TSS/controller?action=2");
    this->setupTLS();
    this->setupHeaders();
    fmt::print(fg((fmt::color)0x00c200), "{0}: url: {1}\n", __PRETTY_FUNCTION__, this->mURL);
}

std::string Requests::sendPOST(std::string &body) {
    if(!this->mHttpClient) {
        return "";
    }
    this->setupArgs(ix::HttpClient::kPost, body);
    std::atomic<bool> requestCompleted(false);
    std::atomic<int> statusCode(0);
    std::string bodyResponse;
    auto responseFunc = [&requestCompleted, &statusCode, &bodyResponse](const ix::HttpResponsePtr& response) {
        bodyResponse = response->body;
        statusCode = response->statusCode;
//        fmt::print(fg(fmt::color::forest_green), "{0}: statusCode: {1}\n", __PRETTY_FUNCTION__, statusCode);
        requestCompleted = true;
    };
    TIMER();
    this->mHttpClient->performRequest(this->mArgs, responseFunc);

    int wait = 0;
    while (wait < 5000)
    {
        if (requestCompleted) break;

        std::chrono::duration<double, std::milli> requestDuration(1);
        std::this_thread::sleep_for(requestDuration);
        wait += 10;
    }
    if(wait >= 5000) {
        fmt::print(fg(fmt::color::crimson), "{0}: Request timed out!\n", __PRETTY_FUNCTION__);
        return "";
    }
    TIMER1();
    auto tmp = bodyResponse;
    tmp.resize(40);
    fmt::print(fg((fmt::color)0x00c200), "{0}: {1}\nDone({2})\n", __PRETTY_FUNCTION__, tmp, statusCode);
    return bodyResponse;
}

void Requests::setupTLS() {
    if(!this->mHttpClient) {
        return;
    }
    this->mTLS = new ix::SocketTLSOptions{};
    this->mTLS->certFile = "";
    this->mTLS->keyFile = "";
    this->mTLS->caFile = "";
    this->mTLS->tls = false;
    this->mHttpClient->setTLSOptions(*this->mTLS);
}

void Requests::setupHeaders() {
    this->mHeaders = new ix::WebSocketHttpHeaders{};
    this->mHeaders->insert(this->mHeaders->begin(), std::make_pair<std::string, std::string>("User-Agent", "InetURL/1.0"));
    this->mHeaders->insert(this->mHeaders->begin(), std::make_pair<std::string, std::string>("Content-Type", "text/xml; charset=\"utf-8\""));
    this->mHeaders->insert(this->mHeaders->begin(), std::make_pair<std::string, std::string>("Cache-Control", "no-cache"));
}

void Requests::setupArgs(const std::string &requestType, const std::string &body) {
    if(!this->mHttpClient) {
        return;
    }
    if(!this->mHeaders) {
        return;
    }
    this->mArgs = this->mHttpClient->createRequest(this->mURL, requestType);
    this->mArgs->body = body;
    this->mArgs->extraHeaders = *this->mHeaders;
    this->mArgs->connectTimeout = 60;
    this->mArgs->transferTimeout = 60;
    this->mArgs->followRedirects = true;
    this->mArgs->maxRedirects = 10;
    this->mArgs->verbose = false;
    this->mArgs->compress = false;
//    this->mArgs->logger = [](const std::string& msg) { fmt::print(fg(fmt::color::forest_green), "{0}: msg: {1}\n", __PRETTY_FUNCTION__, msg); };
}

Requests::~Requests() = default;
