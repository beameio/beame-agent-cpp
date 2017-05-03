    //
// Created by zglozman on 5/3/17.
//

#ifndef PROJECT_AGENT_H
#define PROJECT_AGENT_H
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
#include <boost/program_options/variables_map.hpp>
#include "proxyClient.h"



namespace po = boost::program_options;

struct beameRegToken{
    string fqdn;
    string signature;
};


extern bool shouldQuit;

class BeameAgentService{
private:
    BeameAgentService();

    boost::shared_ptr< boost::asio::io_service > io_service;
    boost::shared_ptr< boost::asio::io_service::work > work;
    boost::thread_group worker_threads;
    static boost::shared_ptr<BeameAgentService> service;
    boost::shared_ptr<beameRegToken> proccessBeameRegToken(string json);

public:
    static boost::shared_ptr<BeameAgentService> getService(string beameHomeDir);
    po::variables_map decodeCommandLineOptions(const int argc, const char *argv[], string beameHomeDir);
    boost::shared_ptr<Credential> generateCredentials(string fqdn, string homeDir);
    void startProxyThreads();
    void stopService();
    ~BeameAgentService();


};
#endif //PROJECT_AGENT_H
