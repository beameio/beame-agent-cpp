//
// Created by zglozman on 5/5/17.
//

#ifndef PROJECT_HTTPS_CLIENT_H
#define PROJECT_HTTPS_CLIENT_H


//#define BOOST_ASIO_ENABLE_HANDLER_TRACKING

#include <iostream>
#include <iomanip>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <functional>
using namespace std;

class client
{
public:
    client(boost::asio::io_service& io_service,
           string fqdn,
           boost::asio::ssl::context& context,
           boost::asio::ip::tcp::resolver::iterator endpoint_iterator,
           string path,
           shared_ptr<string> request,
           string token,
           std::function<void (string, bool)> callback);

    bool verify_certificate(bool preverified, boost::asio::ssl::verify_context& ctx);
    void handle_connect(const boost::system::error_code& error);
    void handle_handshake(const boost::system::error_code& error);
    void handle_write(const boost::system::error_code& error, size_t /*bytes_transferred*/);
    void handle_read(const boost::system::error_code& error, size_t /*bytes_transferred*/);

private:
    boost::asio::ssl::stream<boost::asio::ip::tcp::socket> socket_;
    char request_[2048];
    boost::asio::streambuf reply_;
    shared_ptr<string> request;
    string path;
    string token;
    string fqdn;
};

#endif //PROJECT_HTTPS_CLIENT_H
