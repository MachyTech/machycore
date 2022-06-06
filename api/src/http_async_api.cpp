#include "api.h"
#include "http_async_api.h"

namespace machyAPI
{
    namespace machysockets_aSync
    {
        int asynchronous_httpserver()
        {
            unsigned short port_num = 3333;

            try {
                http_server srv;

                unsigned int thread_pool_size = std::thread::hardware_concurrency() * 2;
                if (thread_pool_size == 0)
                    thread_pool_size = DEFAULT_THREAD_POOL_SIZE;
                
                srv.start(port_num, thread_pool_size);
                std::this_thread::sleep_for(std::chrono::seconds(60));
                srv.stop();
            }
            catch (system::system_error &e) {
                std::cout << "Error occured! Error code = "<<e.code()
                    <<". Message: "<<e.what();
            }
            return 0;
        }

        void http_service::StartHandling()
        {
            asio::async_read_until(*m_sock.get(), m_request, "\r\n",
                [this](
                    const boost::system::error_code& ec,
                    std::size_t bytes_transferred)
                {on_request_line_recieved(ec, bytes_transferred);}); //async_read_until
        }

        void http_service::on_request_line_recieved(const boost::system::error_code& ec,
                    std::size_t bytes_transferred)
        {
            if(ec != 0) {
                std::cout<< "Error occured! Error code = "<<ec.value()
                << ". Message: " <<ec.message();

                if(ec == asio::error::not_found){
                    m_response_status_code = 413;
                    send_response();

                    return;
                }
                else {
                    on_finish();
                    return;
                }
            }
            std::string request_line;
            std::istream request_stream(&m_request);
            std::getline(request_stream, request_line, '\r');
            request_stream.get();

            std::string request_method;
            std::istringstream request_line_stream(request_line);
            request_line_stream >> request_method;

            if (request_method.compare("GET") != 0) {
                m_response_status_code = 501;
                send_response();

                return;
            }

            std::cout<<"[GET]\n";
            request_line_stream >> m_requested_resource;

            std::string request_http_version;
            request_line_stream >> request_http_version;

            if (request_http_version.compare("HTTP/1.1") != 0) {
                m_response_status_code = 505;
                send_response();
                
                return;
            }
            asio::async_read_until(*m_sock.get(), m_request, "\r\n\r\n",
            [this]( const boost::system::error_code& ec,
                            std::size_t bytes_transferred)
                {
                    on_headers_received(ec, bytes_transferred);
                });

            return;
        }

        void http_service::on_headers_received(const boost::system::error_code& ec,
            std::size_t bytes_transferred)
        {
            if( ec != 0){
                std::cout<< "Error occured! Error code = "<<ec.value()<<". Message: "<<ec.message();

                if(ec == asio::error::not_found){
                    m_response_status_code = 413;
                    send_response();
                    return;
                }
                else {
                    on_finish();
                    return;
                }
            }

            std::istream request_stream(&m_request);
            std::string header_name, header_value;

            while(!request_stream.eof()) {
                std::getline(request_stream, header_name, ':');
                if (!request_stream.eof()){
                    std::getline(request_stream, header_value, 'r');
                    request_stream.get();
                    m_request_headers[header_name] = header_value;
                }
            }

            process_request();
            send_response();

            return;
        }

        void http_service::process_request()
        {
            std::string resource_file_path = std::string("/home/timo/" + m_requested_resource);
/* if (!boost::filesystem::exists(resource_file_path)) {
                m_response_status_code = 404;
                return;
            } */
            std::ifstream ifs;
            ifs.open(resource_file_path, std::ifstream::binary);
            if(ifs.fail())
            {
                std::cout<<"file not found\n";
                m_response_status_code = 404;
                return;
            }
            if (!ifs.good())
            {
                std::cout<<"file not opened ifstream\n";
                m_response_status_code = 500;
                return;
            }
            //std::ifstream resource_fstream(resource_file_path, std::ifstream::binary);
            // get length of file
            ifs.seekg(0, std::ifstream::end);
            m_resource_size_bytes = static_cast<std::size_t>(ifs.tellg());
            m_resource_buffer.reset(new char[m_resource_size_bytes]);

            ifs.seekg(std::ifstream::beg);
            ifs.read(m_resource_buffer.get(), m_resource_size_bytes);

            m_response_headers += std::string("content-length") + ": " 
            + std::to_string(m_resource_size_bytes) + "\r\n";
        }

        void http_service::send_response() {
            m_sock->shutdown(asio::ip::tcp::socket::shutdown_receive);
            auto status_line = http_status_table.at(m_response_status_code);
            m_response_status_line = std::string(" HTTP/1.1 ") + status_line + "\r\n";
            m_response_headers += "\r\n";

            std::vector<asio::const_buffer> response_buffers;
            response_buffers.push_back(asio::buffer(m_response_status_line));
            if (m_response_headers.length() > 0) {
                response_buffers.push_back(asio::buffer(m_response_headers));
            }

            if (m_resource_size_bytes > 0) {
                response_buffers.push_back(asio::buffer(m_resource_buffer.get(), m_resource_size_bytes));
            }

            asio::async_write(*m_sock.get(), response_buffers,
                [this](
                    const boost::system::error_code& ec,
                    std::size_t bytes_transferred)
                {
                    on_response_sent(ec, bytes_transferred);
            }); //async_write
        }

        void http_service::on_response_sent(const boost::system::error_code& ec,std::size_t bytes_transferred)
        {
            if (ec != 0) {
                std::cout<<"Error occured! Error code = "<<ec.value()<<". Message: "<<ec.message();
            }
            m_sock->shutdown(asio::ip::tcp::socket::shutdown_both);
            on_finish();
        }

        void http_acceptor::InitAccept()
        {
            std::shared_ptr<asio::ip::tcp::socket>
                sock(new asio::ip::tcp::socket(m_ios));
            m_acceptor.async_accept(*sock.get(),
                [this, sock](const boost::system::error_code& error)
                {onAccept(error, sock);});
        }

        void http_acceptor::onAccept(const boost::system::error_code& ec,
            std::shared_ptr<asio::ip::tcp::socket> sock)
        {
            if (ec == 0) {
                (new http_service(sock))->StartHandling();
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

        void http_server::start(unsigned short port_num, unsigned int thread_pool_size)
        {
            std::cout<<"start server\n";
            assert(thread_pool_size > 0);
            // create and start acceptor
            acc.reset(new http_acceptor(m_ios, port_num));
            acc->start();
            for (unsigned int i = 0; i<thread_pool_size; i++)
            {
                std::unique_ptr<std::thread> th(
                    new std::thread([this]()
                    {
                        m_ios.run();
                    }));
                m_thread_pool.push_back(std::move(th));
            }
        }

        void http_server::stop()
        {
            acc->stop();
            m_ios.stop();

            for (auto& th : m_thread_pool)
            {
                th->join();
            }
        }
    }
}