//
// Created by cryptic on 1/31/23.
//

#include <Requests.hpp>
#include <Lib.hpp>
#include <fmt/core.h>
#include <fmt/color.h>

Requests::Requests() {
    this->mHttpClient = new ix::HttpClient(true);
    ix::SocketTLSOptions tls;
    tls.certFile = "";
    tls.keyFile = "";
    tls.caFile = "";
    tls.tls = false;
    this->mHttpClient->setTLSOptions(tls);
    this->mURL = std::string("http://gs.apple.com/TSS/controller?action=2");
    fmt::print(fg(fmt::color::forest_green), "{0}: url: {1}\n", __PRETTY_FUNCTION__, this->mURL);
}

bool Requests::sendPOST(std::string &data) {
    auto args = this->mHttpClient->createRequest(this->mURL, ix::HttpClient::kPost);
    ix::WebSocketHttpHeaders headers;
    headers.insert(headers.begin(), std::make_pair<std::string, std::string>("User-Agent", "InetURL/1.0"));
    headers.insert(headers.begin(), std::make_pair<std::string, std::string>("Content-Type", "text/xml; charset=\"utf-8\""));
    headers.insert(headers.begin(), std::make_pair<std::string, std::string>("Cache-Control", "no-cache"));
    args->body = data;
    args->multipartBoundary = "";
//    args->multipartBoundary = std::string("----").append(generateUUID()).append("----");
    args->extraHeaders = headers;
    args->connectTimeout = 60;
    args->transferTimeout = 60;
    args->followRedirects = true;
    args->maxRedirects = 10;
    args->verbose = false;
    args->compress = false;
    args->logger = nullptr;
//    args->logger = [](const std::string& msg) { fmt::print(fg(fmt::color::forest_green), "{0}: msg: {1}\n", __PRETTY_FUNCTION__, msg); };
    args->onProgressCallback = nullptr;
//    args->onProgressCallback = [](int current, int total) -> bool {
//        fmt::print(fg(fmt::color::forest_green), "{0}: Downloaded ({1}/{2})\n", __PRETTY_FUNCTION__, current, total);
//        return true;
//    };
    args->onChunkCallback = nullptr;
//    args->onChunkCallback = [](const std::string& data) {
//        fmt::print(fg(fmt::color::forest_green), "{0}: data: {1}\n", __PRETTY_FUNCTION__, data);
//    };
    std::atomic<bool> requestCompleted(false);
    std::atomic<int> statusCode(0);
    std::string bodyResponse;
    auto responseFunc = [&requestCompleted, &statusCode, &bodyResponse](const ix::HttpResponsePtr& response) {
        bodyResponse = response->body;
        statusCode = response->statusCode;
//        fmt::print(fg(fmt::color::forest_green), "{0}: statusCode: {1}\n", __PRETTY_FUNCTION__, statusCode);
        requestCompleted = true;
    };
    args->cancel = false;
    this->mHttpClient->performRequest(args, responseFunc);

    int wait = 0;
    while (wait < 5000)
    {
        if (requestCompleted) break;

        std::chrono::duration<double, std::milli> duration(10);
        std::this_thread::sleep_for(duration);
        wait += 10;
    }
    if(wait >= 5000) {
        fmt::print(fg(fmt::color::crimson), "{0}: Request timed out!\n", __PRETTY_FUNCTION__);
        return false;
    }
    bodyResponse.resize(40);
    fmt::print(fg(fmt::color::forest_green), "{0}: {1}\nDone({2})\n", __PRETTY_FUNCTION__, bodyResponse, statusCode);
    return true;
}

Requests::~Requests() = default;
