#include "proxyClient.h"

boost::shared_ptr<socketIoConnection> _socketIoConnection;
std::mutex _lock;
std::condition_variable_any _cond;
bool connect_finish = false;
socket::ptr current_socket;
sio::client h;

void startSocetIoConnection( boost::shared_ptr< boost::asio::io_service > ios, string edgeFqdn, string fqdn){
    connection_listener l(h);

    h.set_open_listener(std::bind(&connection_listener::on_connected, &l));
    h.set_close_listener(std::bind(&connection_listener::on_close, &l,std::placeholders::_1));
    h.set_fail_listener(std::bind(&connection_listener::on_fail, &l));

    std::stringstream url;
    url << "wss://" <<  edgeFqdn;
    cout << " ********** " << url.str() << " our instance FQDN " <<  fqdn;
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
    registratonMessage << "{ \"socketId\": \"null\",\"payload\": { \"type\": \"HTTPS\", \"hostname\":\"" << fqdn  <<"\"}}\r\n";


    current_socket->emit("register_server", registratonMessage.str());
    cout << "Running Thread " << pthread_self() <<  registratonMessage.str() << "\r\n";
}