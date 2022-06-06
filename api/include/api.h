#ifndef API_H_
#define API_H_

#include <boost/asio.hpp>
//#include <boost/filesystem.hpp>

#include <iostream>
#include <string>
#include <memory>
#include <thread>
#include <atomic>
#include <mutex>

#include <fstream>

#include <machycore.h>

const unsigned int DEFAULT_THREAD_POOL_SIZE = 2;

using namespace boost;

struct GLdata
{
    float x, y;
    GLdata(float a[2]) : x(a[0]/10), y(a[1]/10)
    {}
};

namespace machyAPI
{
    /*
     * machytech API
     */
    namespace machysockets_aSync
     {
        /*
         * machy asynchronous sockets
         */
        int asynchronous_server();
        /*
         * create an asynchronous server
         */
        int asynchronous_server(std::string port);
        /*
         * overload when virtual position data is passed
         */
        class service
        {
            /*
            * the service class implements the function
            * provided by the server to the clients.
            * One instance of this class is intended
            * to handle a single client by reading the
            * request, processing it, and then sending
            * back the response message.
            */
            public:
                service(std::shared_ptr<asio::ip::tcp::socket> sock) : m_sock(sock)
                {}
                void StartHandling();
                /*
                * this method starts handling the client by initiating
                * the asynchronous reading operation to read the request
                * message from the client specifying the onRequestReceived()
                * method as a callback.
                */
            private:
                void onRequestRecieved(const boost::system::error_code& ec, std::size_t bytes_transferred);
                /*
                * checks for succesfull request then processrequest is
                * called to prepare response message and the asynchronous
                * writing operation is initiated
                */
                void onResponseSend(const boost::system::error_code& ec, std::size_t bytes_transferred);
                /*
                * check request operation succeeded then cleanup
                */
                void onFinish(){ delete this; };
                /*
                * cleanup
                */
                std::string ProcessRequest(asio::streambuf& request);
                /*
                 * state machine implementation
                 */
                void _state_test001(int cycles);
                /*
                 * this state implements a cpu load
                 */
                void _state_trajsim001();
                /*
                 * this state implements a trajectory simulator
                 */
                std::shared_ptr<asio::ip::tcp::socket> m_sock;
                std::string m_response;
                asio::streambuf m_request;
        };
        
        class acceptor
        {
            /*
            * the acceptor class accepts the connection
            * request from clients and instantiating 
            * objects of the service class
            */
            public:
                acceptor(asio::io_service&ios, unsigned short port_num) :
                    m_ios(ios), 
                    m_acceptor(m_ios, asio::ip::tcp::endpoint(asio::ip::address_v4::any(), port_num)), 
                    m_isStopped(false)
                {}
                void start(){
                    std::cout<<"start acceptor\n";
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

        class server
        {
            /*
            * the server class represents the server
            * itself, using the acceptor and service
            * classes.
            */
            public:
                server()
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
                std::unique_ptr<acceptor>acc;
                std::vector<std::unique_ptr<std::thread>>m_thread_pool;
        };
    }
}
#endif