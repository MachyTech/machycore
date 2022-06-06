#include "api.h"
#include "http_async_api.h"

namespace machyAPI
{
    namespace machysockets_aSync
    {
        int create_asynchronous_socket()
        {
            // size pending connection request
            const int BACKLOG_SIZE = 30;
            // define a port
            unsigned short port_num = 3333;
            // creating a server endpoint
            asio::ip::tcp::endpoint ep(asio::ip::address_v4::any(), port_num);
            // instance of io service class
            asio::io_service ios;
            try{
                // acceptor socket object
                asio::ip::tcp::acceptor acceptor(ios, ep.protocol());
                // binding socket to endpoint
                acceptor.bind(ep);
                // listen for incoming connection request
                acceptor.listen(BACKLOG_SIZE);
                // create active socket
                asio::ip::tcp::socket sock(ios);
                // connect to client
                acceptor.accept(sock);
                //acceptor.async_accept(sock, accept_handler);
                //readFromSocket(sock);
                ios.run();
            }
            catch(system::system_error &e) {
                std::cout<< "Error occured! Error code = "<<e.code()
                    << ". Message: "<<e.what();
                return e.code().value();
            }
            return 0;
        }

        int asynchronous_server()
        {
            unsigned short port_num = 3333;

            try {
                server srv;

                unsigned int thread_pool_size = std::thread::hardware_concurrency() * 2;
                if (thread_pool_size == 0)
                    thread_pool_size = DEFAULT_THREAD_POOL_SIZE;
                
                srv.start(port_num, thread_pool_size);
                std::this_thread::sleep_for(std::chrono::seconds(5));
                srv.stop();
            }
            catch (system::system_error&e) {
                std::cout << "Error occured! Error code = "<<e.code()
                    <<". Message: "<<e.what();
            }
            return 0;
        }

        int asynchronous_server(std::string port)
        {
            unsigned short port_num = std::stoi(port);

            try {
                server srv;

                unsigned int thread_pool_size = std::thread::hardware_concurrency() * 2;
                if (thread_pool_size == 0)
                    thread_pool_size = DEFAULT_THREAD_POOL_SIZE;
                
                srv.start(port_num, thread_pool_size);
                while(1){}
            }
            catch (system::system_error&e) {
                std::cout << "Error occured! Error code = "<<e.code()
                    <<". Message: "<<e.what();
            }
            return 0;
        }

        void service::StartHandling()
        {
            asio::async_read_until(*m_sock.get(), m_request, "\n\n", 
                [this]( const boost::system::error_code& ec, std::size_t bytes_transferred)
                {onRequestRecieved(ec, bytes_transferred);});
        }

        void service::onRequestRecieved(const boost::system::error_code& ec, std::size_t bytes_transferred )
        {
            if (ec != 0) {
                std::cout<< "Error occured! Error code = "<<ec.value()<<". Message: "<<ec.message();
                onFinish();
                return;
            }
            m_response = ProcessRequest(m_request);
            //machycore::print_vpos_data();
            asio::async_write(*m_sock.get(), asio::buffer(m_response),
                    [this]( const boost::system::error_code& ec, std::size_t bytes_transferred)
                        { onResponseSend(ec, bytes_transferred);});
        }   

        void service::onResponseSend(const boost::system::error_code& ec, std::size_t bytes_transferred)
        {
            if (ec != 0) {
                std::cout<< "Error occured! Error code = "<<ec.value()<<". Message: "<<ec.message();
            }
            onFinish();
        }

        std::string service::ProcessRequest(asio::streambuf& request)
        {
            std::istream is(&request);
            std::string request_line;

            // state machine (use first ten characters finished with ":")

            std::getline(is, request_line, ':');
            std::cout<<"recieved: "<<request_line<<std::endl;

            if(request_line.compare(0,10,"TEST000001")==0)
            {
                _state_test001(10000);
                std::string response("[TEST000001] OK\n");
                return response;
            }

            if(request_line.compare(0, 10, "TEST000002")==0)
            {
                std::string line;
                std::getline(is, line, '\n');
                std::cout<<std::stoi(line)<<std::endl;
                _state_test001(std::stoi(line));
                std::string response("[TEST000002] CPULOAD ");
                response+=line + ", OK\n";
                return response;
            }

            if(request_line.compare(0, 10, "VIRPOS0001")==0)
            {
                std::string line;
                while(is)
                {
                    while(std::getline(is, line, '\n'))
                    {
                        std::stringstream ss(line);
                        float value_vir[5];
                        for (int i=0; i<5; i++)
                        {
                            ss >> value_vir[i];
                            ss.ignore();
                        }
                        machycore::virposition->push_back( value_vir );
                    }
                }
                std::string response("[VIRPOS0001] OK\n");
                return response;
            }

            if(request_line.compare(0,10,"ECHO000001")==0)
            {
                std::string line;
                std::getline(is, line, '\n');
                std::string response("[ECHO000001] ");
                response += line+"\n";
                return response;
            }

            if(request_line.compare(0, 10, "TRAJSIM001")==0){
                std::string line;
                while (is)
                {
                    while(std::getline(is, line, '\n'))
                    {
                        std::stringstream ss(line);
                        float value[2];
                        float value_vir[5];
                        for (int i=0; i<2; i++)
                        {
                            ss >> value[i];
                            ss.ignore();
                        }
                        machycore::trajectory->push_back( value );
                        value_vir[0] = value[0];
                        value_vir[1] = value[1];
                        for (int i=0; i<3; i++)
                        {
                            ss >> value_vir[i+2];
                            ss.ignore();
                        }
                        machycore::virposition->push_back( value_vir );
                    }
                }
                std::string response("[TRAJSIM001] OK\n");
                return response;
            }

            if(request_line.compare(0, 10, "TRAJSIM002")==0){
                std::string line;
                while(is)
                {
                    while(std::getline(is, line, '\n'))
                    {
                        std::stringstream ss(line);
                        float value[2];
                        for (int i=0; i<2; i++)
                        {
                            ss >> value[i];
                            ss.ignore();
                        }
                        machycore::trajectory->push_back( value );
                    }
                }
                std::string response("[TRAJSIM002] OK\n");
                return response;
            }

            if(request_line.compare(0, 10, "START00001")==0){
                {
                    std::lock_guard<std::mutex> lk(machycore::m_machydata);
                    machycore::load_scene = true;
                    machycore::render_ready.notify_one();
                }

                // wait for success message
                {
                    std::unique_lock<std::mutex> lk(machycore::m_machydata);
                    machycore::render_ready.wait(lk, []{return machycore::scene_loaded;});
                }

                std::string response("[START00001] OK\n");
                return response;
            }

            else{
                std::string response("NOT RECOGNIZED\n");
                return response;
            }

            std::string response("NOT RECOGNIZED\n");
            return response;
        }
        
        void service::_state_test001(int cycles)
        {
            int i=0;
            std::cout<<"starting cpu process emulation\n";
            while (i != cycles)
                i++;
                std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            std::cout<<"finished...\n";
        }

        void acceptor::InitAccept()
        {
            std::shared_ptr<asio::ip::tcp::socket> 
                sock(new asio::ip::tcp::socket(m_ios));
            m_acceptor.async_accept(*sock.get(), 
                [this, sock](const boost::system::error_code& error)
                {onAccept(error, sock);});
        }

        void acceptor::onAccept(const boost::system::error_code& ec,
            std::shared_ptr<asio::ip::tcp::socket> sock)
        {
            if (ec == 0) {
                (new service(sock))->StartHandling();
            }
            else {
                std::cout<< "Error occured! Error code = "<<ec.value()<< ". Message: "<<ec.message();
            }
            // init next async accept operation
            if (!m_isStopped.load()) {
                InitAccept();
            }
            else {
                // stop accepting incoming connections
                m_acceptor.close();
            }
        }

        void server::start(unsigned short port_num, unsigned int thread_pool_size)
        {
            assert(thread_pool_size > 0);
            // create and start acceptor
            acc.reset(new acceptor(m_ios, port_num));
            acc->start();
            // create specified number of threads and add them to the pool
            for (unsigned int i = 0; i < thread_pool_size; i++)
            {
                std::unique_ptr<std::thread> th(
                    new std::thread([this]()
                    {
                        m_ios.run();
                    }));
                m_thread_pool.push_back(std::move(th));
            }
        }

        void server::stop()
        {
            acc->stop();
            m_ios.stop();

            for (auto& th : m_thread_pool)
            {
                th->join();
            }
        }
    }
#ifdef USE_CURL
    void read_remote_csv(char* weburl, std::vector<Data> &position)
    {
        CURL *curl_handle;
        CURLcode res;
        std::string buffer;

        curl_global_init(CURL_GLOBAL_ALL);
        curl_handle = curl_easy_init();
        if(curl_handle)
        {
            std::cout<<"using weburl: "<<weburl<<std::endl;
            curl_easy_setopt(curl_handle, CURLOPT_URL, weburl);
            curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteCallback);
            curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, &buffer);
            res = curl_easy_perform(curl_handle);
            std::stringstream ssline(buffer);
            if(res != CURLE_OK){
                fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
            }
            else{
                std::string line;
                float value[7];
                while(std::getline(ssline, line, '\n'))
                {
                    std::stringstream ss(line);
                    ss >> value[0];
                    ss.ignore();
                    ss >> value[1];
                    position.push_back({value[0], value[1]});
                }
            }
        }
    }


    void read_remote_csv(char* weburl, std::vector<Sim> &virposition)
    {
        CURL *curl_handle;
        CURLcode res;
        std::string buffer;
        
        curl_global_init(CURL_GLOBAL_ALL);
        curl_handle = curl_easy_init();
        if(curl_handle)
        {
            std::cout<<"using weburl: "<<weburl<<std::endl;
            curl_easy_setopt(curl_handle, CURLOPT_URL, weburl);
            curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteCallback);
            curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, &buffer);
            res = curl_easy_perform(curl_handle);
            std::stringstream ssline(buffer);
            if(res != CURLE_OK){
                fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
                exit(-1);
            }
            else{
                std::string line;
                float value[7];
                while(std::getline(ssline, line, '\n'))
                {
                    std::stringstream ss(line);
                    for(int i=0; i<7; i++)
                    {
                        ss >> value[i];
                        ss.ignore();
                    }
                    virposition.push_back({value[0], value[1], value[2], value[3], value[6]+1.57});
                }
            }
        }
    }
#endif
}