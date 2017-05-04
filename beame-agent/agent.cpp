//
// Created by zglozman on 5/3/17.
//

#include "agent.h"
bool shouldQuit = false;
void WorkerThread( boost::shared_ptr< boost::asio::io_service > io_service){
    while(shouldQuit == false){
        cout << "**************************in************" "\r\n";
        io_service->run();
        cout << "**************************OUT************" "\r\n";
    }
    cout << "**************EVENT_LOOOP_EXISTED************" "\r\n";
}

BeameAgentService::BeameAgentService(){
    io_service = boost::shared_ptr<boost::asio::io_service>(
            new boost::asio::io_service
    );
    new boost::asio::io_service::work((*io_service));
    //work = new boost::shared_ptr<boost::asio::io_service::work>(
      //  new boost::asio::io_service::work(io_service->));


}

BeameAgentService::~BeameAgentService(){


}

void
BeameAgentService::startProxyThreads() {
    for (int x = 0; x < 1; ++x) {
        this->worker_threads.create_thread(boost::bind(&WorkerThread, io_service));
    }
}

void
BeameAgentService::stopService(){

    shouldQuit = true;
    io_service->stop();
    worker_threads.join_all();
}
boost::shared_ptr<BeameAgentService> BeameAgentService::service(0);
boost::shared_ptr<BeameAgentService> BeameAgentService::getService(string beameHomeDir) {
    if(BeameAgentService::service == nullptr){
        BeameAgentService::service = boost::shared_ptr<BeameAgentService>( new BeameAgentService());
    }
    return service;
}

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

variables_map
BeameAgentService::decodeCommandLineOptions(const int argc, const char *argv[], string beameHomeDir)
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
            boost::shared_ptr<beameRegToken> token = proccessBeameRegToken(json);

            //          std::cout << "authToken: " << json;

            //        c.RSASign(huj , 3, returnPtr);
            //         cout << "Returned signature " << returnPtr->length();
            boost::shared_ptr<Credential> cred = generateCredentials(token->fqdn, beameHomeDir) ;
            shared_ptr<pt::ptree> tree = cred->getRequestJSON();
            boost::property_tree::write_json(std::cout, *tree);
//

        }
        return vm;
    }

    catch (const error &ex)
    {
        std::cerr << ex.what() << '\n';
    }
}


boost::shared_ptr<beameRegToken>
BeameAgentService::proccessBeameRegToken(string json) { // return
    boost::property_tree::ptree pt;
    std::stringstream ss;
    ss << json;
    boost::property_tree::read_json(ss, pt);

    boost::property_tree::ptree embeddedToken;
    std::stringstream tokenStream;

    tokenStream << pt.get<string>("authToken");
    cout << pt.get<string>("authToken") << "\r\n";
    boost::property_tree::read_json(tokenStream, embeddedToken);

    string emdeddedDataSection = embeddedToken.get<string>("signedData.data");
    string signedData = embeddedToken.get<string>("signedData");
    cout << "Signed Data Segmenet: " << signedData << "\r\n";
    std::stringstream embeddedTokenStream;
    embeddedTokenStream << emdeddedDataSection;
    boost::property_tree::ptree embeddedDataSection;
    boost::property_tree::read_json(embeddedTokenStream, embeddedDataSection);

    string fqdn = embeddedDataSection.get<string>("fqdn");
    boost::shared_ptr<beameRegToken> token(new beameRegToken());
    token->signature = signedData;
    token->fqdn = fqdn;
    cout << "Finally fqdn " << fqdn << "\r\n";
    return token;
}

boost::shared_ptr<Credential>
BeameAgentService::generateCredentials(string fqdn, string beameHomeDir){
    boost::shared_ptr<Credential> cred =  boost::shared_ptr<Credential>(new Credential(fqdn, beameHomeDir));
    shared_ptr <pt::ptree> req = cred->getRequestJSON();
    pt::write_json(std::cout, *req);
    return cred;
}