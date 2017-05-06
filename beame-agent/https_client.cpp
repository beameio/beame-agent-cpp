//
// Created by zglozman on 5/5/17.
//

#include "https_client.h"
#include <iostream>
#include <stdlib.h>

client::client(boost::asio::io_service& io_service,
               string _fqdn,
               boost::asio::ssl::context& context,
               boost::asio::ip::tcp::resolver::iterator endpoint_iterator, string path, shared_ptr<string> request, string t, std::function<void (string, bool)> callback)

        : socket_(io_service
        , context), token(t), fqdn(_fqdn)
{
    socket_.set_verify_mode(boost::asio::ssl::verify_peer);
    socket_.set_verify_callback(
            boost::bind(&client::verify_certificate, this, _1, _2));

    this->request = request;
    this->path = path;
    boost::asio::async_connect(socket_.lowest_layer(), endpoint_iterator,
                               boost::bind(&client::handle_connect, this,
                                           boost::asio::placeholders::error));
}

bool client::verify_certificate(bool preverified,
                                boost::asio::ssl::verify_context& ctx)
{
    // The verify callback can be used to check whether the certificate that is
    // being presented is valid for the peer. For example, RFC 2818 describes
    // the steps involved in doing this for HTTPS. Consult the OpenSSL
    // documentation for more details. Note that the callback is called once
    // for each certificate in the certificate chain, starting from the root
    // certificate authority.

    // In this example we will simply print the certificate's subject name.
    char subject_name[256];
    X509* cert = X509_STORE_CTX_get_current_cert(ctx.native_handle());
    X509_NAME_oneline(X509_get_subject_name(cert), subject_name, 256);
    std::cout << "Verifying " << subject_name << "\n";

    return true;
}

void client::handle_connect(const boost::system::error_code& error)
{
    if (!error)
    {
        socket_.async_handshake(boost::asio::ssl::stream_base::client,
                                boost::bind(&client::handle_handshake, this,
                                            boost::asio::placeholders::error));
    }
    else
    {
        std::cout << "Connect failed: " << error.message() << "\n";
    }
}

void client::handle_handshake(const boost::system::error_code& error)
{
    if (!error)
    {
        std::cout << "Enter message: ";
        std::stringstream raw;
        raw << "POST " << path  << " HTTP/1.1\r\n Host: " << fqdn << "\r\n"  << "X-BeameAuthToken: " << token  << "\r\n"  << "Content-Length: " << request->length() << "\r\n" << "Content-Type: application/json" << "\r\nConnection: close\r\n\r\n" << (*request);

        /*static_assert(sizeof(raw)<=sizeof(request_), "too large");

        size_t request_length = strlen(raw);
        std::copy(raw, raw+request_length, request_);

        {
            // used this for debugging:
            std::ostream hexos(std::cout.rdbuf());
            for(auto it = raw; it != raw+request_length; ++it)
                hexos << std::hex << std::setw(2) << std::setfill('0') << std::showbase << ((short unsigned) *it) << " ";
            std::cout << "\n";
        }*/
        cout << raw.str();
        boost::asio::async_write(socket_,
                                 boost::asio::buffer(raw.str(), raw.str().length()),
                                 boost::bind(&client::handle_write, this,
                                             boost::asio::placeholders::error,
                                             boost::asio::placeholders::bytes_transferred));
    }
    else
    {
        std::cout << "Handshake failed: " << error.message() << "\n";
    }
}

void client::handle_write(const boost::system::error_code& error,
                          size_t /*bytes_transferred*/)
{
    if (!error)
    {
        std::cout << "starting read loop\n";
        boost::asio::async_read_until(socket_,
                //boost::asio::buffer(reply_, sizeof(reply_)),
                                      reply_, '\n',
                                      boost::bind(&client::handle_read, this,
                                                  boost::asio::placeholders::error,
                                                  boost::asio::placeholders::bytes_transferred));
    }
    else
    {
        std::cout << "Write failed: " << error.message() << "\n";
    }
}

void client::handle_read(const boost::system::error_code& error, size_t /*bytes_transferred*/)
{
    if (!error)
    {
        std::cout << "Reply: " << &reply_ << "\n";
        boost::asio::async_read_until(socket_,
                //boost::asio::buffer(reply_, sizeof(reply_)),
                                      reply_, '\n',
                                      boost::bind(&client::handle_read, this,
                                                  boost::asio::placeholders::error,
                                                  boost::asio::placeholders::bytes_transferred));
    }
    else
    {
        socket_.shutdown();
        std::cout << "Read failed: " << error.message() << "\n";
    }
}


int testClient (string host, string port, shared_ptr<string> requestJson, string path,  std::function<void (string, bool)> callback, string token)
{
    try
    {

        boost::asio::io_service io_service;
        boost::asio::io_service::work w(io_service);
        boost::asio::ip::tcp::resolver resolver(io_service);
        boost::asio::ip::tcp::resolver::query query(host,port);
        boost::asio::ip::tcp::resolver::iterator iterator = resolver.resolve(query);

        boost::asio::ssl::context ctx(boost::asio::ssl::context::sslv23);
        ctx.set_default_verify_paths();

        client c(io_service, host, ctx, iterator, path, requestJson, token, callback);

        io_service.run();
    }
    catch (std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}