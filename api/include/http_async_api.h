#ifndef HTTP_ASYNC_API_H_
#define HTTP_ASYNC_API_H_

#include <boost/asio.hpp>

#include <iostream>
#include <string>
#include <memory>
#include <thread>
#include <atomic>
#include <mutex>

#include <fstream>

using namespace boost;

namespace machyAPI
{
    namespace machysockets_aSync
    {
        int asynchronous_httpserver();
        /*
         * http server
         */
        class http_service
        {
            /*
             * service class that provides the implementation of the 
             * http protocol.
             */
            static const std::map<unsigned int, std::string>http_status_stable;
            public:
                http_service(std::shared_ptr<boost::asio::ip::tcp::socket> sock) :
                    m_sock(sock), m_request(4096), m_response_status_code(200), m_resource_size_bytes(0)
                {};
                void StartHandling();
                /*
                 * initiate asynchronous communication session with the
                 * client connected to the socket
                 */
            private:
                void on_request_line_recieved(const boost::system::error_code& ec,
                    std::size_t bytes_transferred);
                /*
                 * process the http request line 
                 * supported: GET
                 */
                void on_headers_received(const boost::system::error_code& ec,
                    std::size_t bytes_transferred);
                /*
                 * process and store the request header block
                 */
                void process_request();
                /*
                 * read the contents of the requested resource from
                 * the file system and store it in a buffer
                 */
                void send_response();
                /*
                 * response message and send it to the client
                 */
                void on_response_sent(const boost::system::error_code& ec,std::size_t bytes_transferred);
                /*
                 * after response sent kill the socket and cleanup
                 */
                void on_finish(){ delete this; }
                std::shared_ptr<boost::asio::ip::tcp::socket> m_sock;
                boost::asio::streambuf m_request;
                std::map<std::string, std::string> m_request_headers;
                std::string m_requested_resource;

                std::unique_ptr<char[]> m_resource_buffer;
                unsigned int m_response_status_code;
                std::size_t m_resource_size_bytes;
                std::string m_response_headers;
                std::string m_response_status_line;
        };
        
        const std::map<unsigned int, std::string>
        http_status_table = 
        {
            { 200, "200 OK" },
            { 404, "404 Not Found" },
            { 413, "413 Request Entity Too Large" },
            { 500, "500 Server Error" },
            { 501, "501 Not Implemented" },
            { 505, "505 HTTP Version Not Supported" }
        };
        
        class http_acceptor
        {
            /*
            * the acceptor class accepts the connection
            * request from clients and instantiating 
            * objects of the service class
            */
            public:
                http_acceptor(asio::io_service&ios, unsigned short port_num) :
                    m_ios(ios), 
                    m_acceptor(m_ios, asio::ip::tcp::endpoint(asio::ip::address_v4::any(), port_num)), 
                    m_isStopped(false)
                {}
                void start(){
                    std::cout<<"start http acceptor\n";
                    m_acceptor.listen();
                    InitAccept();
                }
                /*
                * start accepting incoming connection requests
                */
                void stop(){ m_isStopped.store(true);}
                /*
                * stop accepting incoming connection requests
                */
            private:
                void InitAccept();
                /*
                * constructs an active socket object and initiates
                * the asynchronous accept operation.
                */
                void onAccept(const boost::system::error_code& ec,
                    std::shared_ptr<asio::ip::tcp::socket> sock);
                /*
                * if asynchronous operation was succesfull an instance
                * of the service class is created.
                */
                asio::io_service&m_ios;
                asio::ip::tcp::acceptor m_acceptor;
                std::atomic<bool>m_isStopped;
        };

        class http_server
        {
            /*
            * the server class represents the server
            * itself, using the acceptor and service
            * classes.
            */
            public:
                http_server()
                {
                    m_work.reset(new asio::io_service::work(m_ios));
                }
                void start(unsigned short port_num, unsigned int thread_pool_size);
                /*
                * starts a server on specified port and listens for asynchronous
                * operations on defined number of threads.
                */
                void stop();
                /*
                 * stops the acceptor instance and stops all trheads
                 * defined in start.
                */
            private:
                asio::io_service m_ios;
                std::unique_ptr<asio::io_service::work>m_work;
                std::unique_ptr<http_acceptor>acc;
                std::vector<std::unique_ptr<std::thread>>m_thread_pool;
        };
    }
}

#endif