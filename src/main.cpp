//
//  beame-websocket-tcp proxy 
//
//  Created with VIM by Zeev Glozman
//

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
#include <boost/archive/iterators/binary_from_base64.hpp>
#include <boost/archive/iterators/base64_from_binary.hpp>
#include <boost/archive/iterators/transform_width.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/program_options.hpp>


#include <string>
#ifdef WIN32
#define HIGHLIGHT(__O__) std::cout<<__O__<<std::endl
#define EM(__O__) std::cout<<__O__<<std::endl

#include <stdio.h>
#include <tchar.h>
#define MAIN_FUNC int _tmain(int argc, _TCHAR* argv[])
#else
#define HIGHLIGHT(__O__) std::cout<<"\e[1;31m"<<__O__<<"\e[0m"<<std::endl
#define EM(__O__) std::cout<<"\e[1;30;1m"<<__O__<<"\e[0m"<<std::endl

#define MAIN_FUNC int main(int argc ,const char* args[])
#endif

using namespace sio;
using namespace std;
using boost::asio::ip::tcp;
namespace ip = boost::asio::ip;

std::mutex _lock;

std::condition_variable_any _cond;
bool connect_finish = false;
socket::ptr current_socket;
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



std::map<string,string> extractParameterFromJsonFile(string fileName, vector<string> elementNames){
    using boost::property_tree::ptree;

    std::ifstream jsonFile(fileName);
    std::map<string,string> returnMap;

    ptree pt;
    read_json(jsonFile, pt);

    BOOST_FOREACH(string elementName, elementNames)
    {
        string value = pt.get<std::string>(elementName);
        returnMap[elementName] = value;
    }
    return returnMap;
}

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

sio::client h;
boost::shared_ptr<socketIoConnection> _socketIoConnection;

void startSocetIoConnection( boost::shared_ptr< boost::asio::io_service > ios){
std:map<string,string> values =  extractParameterFromJsonFile("/home/zglozman/.beame/v2/oj84r8hympcewe91.h65nc6ooxa8hup3g.v1.p.beameio.net/metadata.json", {"edge_fqdn", "fqdn"});
    connection_listener l(h);

    h.set_open_listener(std::bind(&connection_listener::on_connected, &l));
    h.set_close_listener(std::bind(&connection_listener::on_close, &l,std::placeholders::_1));
    h.set_fail_listener(std::bind(&connection_listener::on_fail, &l));

    std::stringstream url;
    url << "wss://" <<  values["edge_fqdn"];
    cout << " ********** " << url.str() << " our instance FQDN " <<  values["fqdn"];
    h.connect(url.str());

    _lock.lock();
    if(!connect_finish)
    {
        _cond.wait(_lock);
    }
    _lock.unlock();

    current_socket = h.socket("/control");
    _socketIoConnection = boost::shared_ptr<socketIoConnection>(new socketIoConnection(current_socket, ios));
    cout << "Connected to edge server @ " << url.str() << "\r\n";
    std::stringstream registratonMessage;
    registratonMessage << "{ \"socketId\": \"null\",\"payload\": { \"type\": \"HTTPS\", \"hostname\":\"" << values["fqdn"] <<"\"}}\r\n";


    current_socket->emit("register_server", registratonMessage.str());
    cout << "Running Thread " << pthread_self() <<  registratonMessage.str() << "\r\n";
}

bool shouldQuit = false;
void WorkerThread( boost::shared_ptr< boost::asio::io_service > io_service ){
    startSocetIoConnection(io_service);
    while(shouldQuit == false){
        cout << "**************************in************" "\r\n";
        io_service->run();
        cout << "**************************OUT************" "\r\n";
    }
    cout << "**************EVENT_LOOOP_EXISTED************" "\r\n";
}


namespace po = boost::program_options;

void
show_help(const po::options_description& desc, const std::string& topic = "")
{
    std::cout << desc << '\n';
    if (topic != "") {
        std::cout << "You asked for help on: " << topic << '\n';
    }
    exit( EXIT_SUCCESS );
}

using namespace boost::program_options;
void to_cout(const std::vector<std::string> &v)
{
    std::copy(v.begin(), v.end(),
            std::ostream_iterator<std::string>{std::cout, "\n"});
}


void
process_program_options(const int argc, const char *argv[], string beameHomeDir)
{
    try {
        options_description desc{"Options"};
        desc.add_options()
            ("help,h", "Help screen")
            ("fqdn", value<string>()->notifier([&](string a) {
                                               cout << "huj" << a;
                                               }), "fqdn of the local install credential")
        ("authToken", value<string>()->notifier([&](string a) {
                                                cout << "Initiating with a  secure token ..... \r\n";
                                                }), "authorization token from beame-gatekeeper")
        ("credDir", value<string>()->notifier([&](string a) {
                                              cout << "authToken" << a;
                                              }), "specify a different credential folder other than ~/.beame");
        ("enableSXG", value<string>(), "enables intel SGX ");

        variables_map vm;
        store(parse_command_line(argc, argv, desc), vm);
        notify(vm);

        if (vm.count("help"))
            std::cout << desc << '\n';

        if (vm.count("fqdn")) {
            std::cout << "fqdn " << vm["fqdn"].as<string>() << '\n';
        } else if (vm.count("authToken")) {

            string json = decode64(vm["authToken"].as<string>());
            //          std::cout << "authToken: " << json;
            boost::property_tree::ptree pt;
            std::stringstream ss;
            ss << json;
            boost::property_tree::read_json(ss, pt);

            boost::property_tree::ptree embeddedToken;
            std::stringstream tokenStream;

            tokenStream << pt.get<string>("authToken");
            //            cout  << pt.get<string>("authToken") << "\r\n";
            boost::property_tree::read_json(tokenStream, embeddedToken);

            string emdeddedDataSection = embeddedToken.get<string>("signedData.data");
            cout << "Signed Data Segmenet: " << emdeddedDataSection  << "\r\n";
            std::stringstream embeddedTokenStream;
            embeddedTokenStream << emdeddedDataSection;
            boost::property_tree::ptree embeddedDataSection;
            boost::property_tree::read_json(embeddedTokenStream, embeddedDataSection);

            string fqdn =embeddedDataSection.get<string>("fqdn");
            cout << "Finally fqdn " << fqdn << "\r\n";
            Credential c(fqdn, beameHomeDir);
            c.testPublicKeyBytes();
            shared_ptr<string> returnPtr;
            unsigned char *huj = (unsigned char *)" huj";
            //        c.RSASign(huj , 3, returnPtr);
            //         cout << "Returned signature " << returnPtr->length();


        }
    }

    catch (const error &ex)
    {
        std::cerr << ex.what() << '\n';
    }
}



MAIN_FUNC
{

    char *p = getenv("HOME");
    string beameDirPath = string(getenv("HOME"));
    beameDirPath += "/.beame/v2";
    cout << "Out beame_dir is: " <<  beameDirPath;

    process_program_options(argc, args, beameDirPath);

    return 0;


    boost::shared_ptr< boost::asio::io_service > io_service(
            new boost::asio::io_service
            );
    boost::shared_ptr< boost::asio::io_service::work > work(
            new boost::asio::io_service::work( *io_service )
            );


    boost::thread_group worker_threads;
    for( int x = 0; x < 1; ++x )
    {
        worker_threads.create_thread( boost::bind( &WorkerThread, io_service ) );
    }
    cout << "reeading stdin\r\n";
    string sq;
    std::string line; std::getline(std::cin, line);
    cout << "**************************************read" <<sq << "\r\n";
    shouldQuit = true;
    io_service->stop();

    worker_threads.join_all();

    //boost::shared_ptr<tcpClientConnector> connector = boost::make_shared<tcpClientConnector>(io_service, dataRecived);
    //auto f = boost::bind(boost::mem_fn(&tcpClientConnector::start), connector, "127.0.0.1", 8080);


    //io_service->post( f );
    //io_service->run();
    //ios.run();
    //boost::bind(&boost::asio::io_service::run, &ios);
    string s;
    //cin >>s;





    HIGHLIGHT("Closing...");
    //  h.sync_close();
    //    h.clear_con_listeners();
    return 0;
}

