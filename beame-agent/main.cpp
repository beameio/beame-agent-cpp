//
//  beame-websocket-tcp proxy 
//
//  Created with VIM by Zeev Glozman
//

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
#include "agent.h"
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

MAIN_FUNC
{

    char *p = getenv("HOME");
    string beameDirPath = string(getenv("HOME"));
    beameDirPath += "/.beame/v2";
    cout << "Out beame_dir is: " <<  beameDirPath;
    boost::shared_ptr<BeameAgentService> service = BeameAgentService::getService(beameDirPath );
    service->decodeCommandLineOptions(argc, args, beameDirPath);
    service->startProxyThreads();

//    process_program_options(argc, args, beameDirPath);


    cout << "reeading stdin\r\n";
    string sq;
    std::string line;
    std::getline(std::cin, line);
    service->stopService();


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

