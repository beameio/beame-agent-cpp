//
// Created by zglozman on 5/3/17.
//

#ifndef PROJECT_PROXYCLIENT_H
#define PROJECT_PROXYCLIENT_H

#include <boost/asio.hpp>
#include "sio_client.h"
#include "boost/property_tree/ptree.hpp"
#include "boost/property_tree/json_parser.hpp"
#include <boost/foreach.hpp>
#include <stdlib.h>
#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <boost/asio.hpp>
#include "crypto.h"
#include <boost/enable_shared_from_this.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/make_shared.hpp>
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <functional>

using namespace sio;
using namespace std;
using boost::asio::ip::tcp;
namespace ip = boost::asio::ip;

class socketIoConnection;  // forward declaration

extern sio::client h;

extern boost::shared_ptr<socketIoConnection> _socketIoConnection;
extern std::mutex _lock;
extern std::condition_variable_any _cond;
extern bool connect_finish;
extern socket::ptr current_socket;

class connection_listener
{
    sio::client &handler;

public:

    connection_listener(sio::client& h):
            handler(h)
    {
    }

    void on_connected()
    {
        _lock.lock();
        _cond.notify_all();
        connect_finish = true;
        _lock.unlock();
    }
    void on_close(client::close_reason const& reason)
    {
        std::cout<<"sio closed "<<std::endl;
        exit(0);
    }

    void on_fail()
    {
        std::cout<<"sio failed "<<std::endl;
        exit(0);
    }
};

class tcpClientConnector;

class socketMap{
private:
    std::mutex socketMapCs;
    std::map<string, boost::shared_ptr<tcpClientConnector>> connectionMap;

public:
    socketMap(){}
    ~socketMap(){

    }

    boost::shared_ptr<tcpClientConnector> operator[](string key) {
        return connectionMap[key];
    }

    void addConnection(boost::shared_ptr<tcpClientConnector> connectedSocket, string remoteSocketId){
        connectionMap[remoteSocketId] = connectedSocket;
    }

    void remove(string key){
        connectionMap.erase(key);
    }
};



typedef std::function<void()> onDataNotification;

class tcpClientConnector : public boost::enable_shared_from_this<tcpClientConnector> {
    ip::tcp::socket downstream_socket_;
    enum { max_data_length = 8192 }; //8KB
    unsigned char downstream_data_[max_data_length];
    std::function<void (const std::shared_ptr<string>, string, size_t)> onData;
    std::function<void (std::string)> onDisconnect;
    string remoteSocketId;


public:

    void handle_upstream_connect(const boost::system::error_code& error)   {

    }

    tcpClientConnector(boost::shared_ptr< boost::asio::io_service > ios, string rSocketId, std::function<void (std::shared_ptr<string>, string,size_t)> _onData ,
                       std::function<void (std::string)> _onDisconnect)
            : onData(_onData), downstream_socket_(*ios), remoteSocketId(rSocketId), onDisconnect(_onDisconnect)
    {

    }

    ~tcpClientConnector() { cout << " ################################tcpClientConnector was just destructed \r\n"; };
    void start(string targetHost, int targetPort) {
        cout << "\r\n!!!!!!!!!!!!!!!!connecting to "  << " " << targetHost << " " << targetPort << "\r\n";
        downstream_socket_.async_connect(
                ip::tcp::endpoint(
                        boost::asio::ip::address::from_string(targetHost),
                        targetPort),

                boost::bind(&tcpClientConnector::handle_downstream_connect,
                            shared_from_this(),
                            boost::asio::placeholders::error));

    }
    void setupRead(){
        downstream_socket_.async_read_some(
                boost::asio::buffer(downstream_data_,max_data_length),
                boost::bind(&tcpClientConnector::handle_downstream_read,
                            shared_from_this(),
                            boost::asio::placeholders::error,
                            boost::asio::placeholders::bytes_transferred));
    }
    void handle_downstream_connect(const boost::system::error_code& error)
    {

        cout << "handle_downstream_connect "  << error <<  " \r\n";
        if (!error)
        {
            setupRead();


        }
        else{
            cout << "connection failed ";
        }

        //     close();
    }
    void handle_downstream_read(const boost::system::error_code& error,
                                const size_t& bytes_transferred)
    {
        cout <<   "####### handle_downstream_read \r\n" << error;
        if (!error)
        {
            shared_ptr<std::string> s = make_shared<std::string>((const char*)&downstream_data_[0], bytes_transferred);
            //onData(s, bytes_transferred);
            setupRead();
            auto f = onData;

            onData(s, remoteSocketId, bytes_transferred);
        }
        if ((boost::asio::error::eof == error) ||
            (boost::asio::error::connection_reset == error))
        {
            cout << "@@@@@@@DISCONNECT FROM DOWNSTREAM STREAM\r\n";

            // handle the disconnect.
        }
        //    close();
    }

    void sendDataOverTcp(std::shared_ptr<const string> data){
        auto handler = [=](const boost::system::error_code& ec, std::size_t bytes_transferred){
            cout << "TcpWriteHandle returned; \r\b";
        };
        boost::asio::async_write(downstream_socket_, boost::asio::buffer(*data), handler);

    }

    void close(){
        downstream_socket_.close();
    }

};

class socketIoConnection{
    const socket::ptr _sic;
    shared_ptr<socketMap> map;
    boost::shared_ptr< boost::asio::io_service > ios;

public:

    socketIoConnection(socket::ptr sic, boost::shared_ptr< boost::asio::io_service > _ios) : _sic(sic), ios(_ios), map(new socketMap()){

        std::function<void(event& event)>  onSocketErrorBound(boost::bind(&socketIoConnection::onError, this, _1));
        current_socket->on("socket_error", onSocketErrorBound);;

        std::function<void(event& event)>  onData(boost::bind(&socketIoConnection::onDataFromEdge, this, _1));
        current_socket->on("data", onData);;

        std::function<void(event& event)>  onDisconnetBound(boost::bind(&socketIoConnection::onDisconnect, this, _1));
        current_socket->on("_end", onDisconnetBound);;

        std::function<void(event& event)>  onCreateConnectionBound(boost::bind(&socketIoConnection::onCreateConnection, this, _1));
        current_socket->on("create_connection", onCreateConnectionBound);;

        std::function<void(event& event)>  onHostRegistered(boost::bind(&socketIoConnection::onHostRegistered, this, _1));
        current_socket->on("hostRegistered", onHostRegistered);;


    }

    ~socketIoConnection() {
        _sic->close();
        cout << "!!!!!!!!!!!!!!!!!!!!!!!!!! DESTRUCTED !!!!!!!!!!!!!!!!!!!!!!!1";
    }

    socketIoConnection(const socketIoConnection &src); //  Make it fail on linkage if this is called by misstake
    socketIoConnection(const socketIoConnection *src); //  Make it fail on linkage if this is called by misstake

    const socketIoConnection& operator=(const socketIoConnection &src);

private:
    void onHostRegistered(sio::event &ev)
    {
        string message = ev.get_message()->get_map()["Hostname"]->get_string();
        auto tes =ev.get_message();
        cout << "Running Thread " << pthread_self() << " onHostRegistered Arrived " << message  << "\r\n";
    }

    void onDataFromEdge(sio::event &event){
        std::map<std::string,message::ptr> &msg = event.get_message()->get_map();
        std::shared_ptr<const string> bin = msg["payload"]->get_binary();
        auto socketId = msg["socketId"]->get_string();
        cout << "Running Thread " << pthread_self()  << "Receiving data from " << socketId << " " << bin->length() << "\r\n";
        boost::shared_ptr<tcpClientConnector> tcpConnection = map->operator[](socketId);
        tcpConnection->sendDataOverTcp(bin);
    };

    void onDisconnect(sio::event &ev)
    {
        std::map<std::string,message::ptr> &messageMap  = ev.get_message()->get_map();
        auto socketId = messageMap["socketId"]->get_string();

        boost::shared_ptr<tcpClientConnector> tcpConnection = map->operator[](socketId);
        if(tcpConnection) {
            tcpConnection->close();
            map->remove(socketId);
        }
        cout << "Running Thread " << pthread_self() << " OnDisconnect \r\n";

    };

    void onError(sio::event &event) {
        cout << "Running Thread " << pthread_self() << " onError\r\n";

        std::map<std::string,message::ptr> &messageMap  = event.get_message()->get_map();
        auto socketId = messageMap["socketId"]->get_string();
        onTcpDisconnect(socketId);
    }

    void onEnd(sio::event &event) {
        cout << "Running Thread " << pthread_self() << " onEnd\r\n";

    }

    void onCreateConnection(sio::event &ev){
        cout << "Running Thread " << pthread_self() << " onCreateConnection  ";
        std::map<std::string,message::ptr> &messageMap  = ev.get_message()->get_map();

        string clientIp = messageMap["clientIp"]->get_string();
        string socketId = messageMap["socketId"]->get_string();

        cout << "create_connection " <<  pthread_self() << "Incoming connection from " << clientIp << " " << socketId << " \r\n";



        std::function<void (const std::shared_ptr<string>, string, size_t)>  onTcpDataCallback = bind(&socketIoConnection::tcpDataReicived, this, _1, _2, _3);
        std::function<void (std::string)>  onDisconnectCallback = bind(&socketIoConnection::onTcpDisconnect, this, _1);

        boost::shared_ptr<tcpClientConnector> connector = boost::make_shared<tcpClientConnector>(ios, socketId, onTcpDataCallback, onDisconnectCallback);
        connector->start("127.0.0.1", 8080);
        map->addConnection(connector, socketId);
        // connector->
    }

    void tcpDataReicived(const std::shared_ptr<string> &binaryString, string assosiatedEdgeSocketId, size_t payload) {
        auto s = sio::object_message::create();
        static_cast<object_message*>(s.get())->insert("socketId",assosiatedEdgeSocketId);
        static_cast<object_message*>(s.get())->insert("payload", binaryString);
        _sic->emit("data", s);
        cout << "transmiting data back to edge  \r\n";

    }
    void onTcpDisconnect(string edgeSocketId){
        boost::shared_ptr<tcpClientConnector> tcpConnection = map->operator[](edgeSocketId);
        if(tcpConnection) {
            tcpConnection->close();
            map->remove(edgeSocketId);
            auto s = sio::object_message::create();
            static_cast<object_message *>(s.get())->insert("socketId", edgeSocketId);
            _sic->emit("disconnect_client", s);
        }
        //  This should never be called either;
    };

};

extern void startSocetIoConnection( boost::shared_ptr< boost::asio::io_service > ios, string edgeFqdn, string fqdn);

#endif //PROJECT_PROXYCLIENT_H