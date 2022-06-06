#ifndef SYNC_API_H_
#define API_H_

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
    /*
     * machytech API
     */
    namespace machysockets_Sync
    {
        /*
         * machy synchronous sockets
         */
        int create_passive_synchronous_socket();
        /*
         * a passive synchronous socket that waits
         * for incoming connections
         */
        int connect_synchronous_socket();
        /*
         * function to connect to a synchronous socket
         */
        void writeToSocket(asio::ip::tcp::socket& sock);
        /*
         * function to write to socket using boost write_some method
         */
        void writeToSocketEnhanced(asio::ip::tcp::socket& sock);
        /*
         * function to write to socket using boost write method
         */
        std::string readFromSocket(asio::ip::tcp::socket& sock);
        /*
         * function to read from socket using boost read_some method
         */
        std::string readFromSocketEnhanced(asio::ip::tcp::socket& sock);
        /*
         * function to read from socket using boost read method
         */
        std::string readFromSocketDelim(asio::ip::tcp::socket& sock);
        /*
         * function to read from socket until character
         */
        int synchronous_client();
        /*
         * a synchronous client function
         */
        class SyncTCPClient {
            /* 
             * a synchronous tcp client
             */
            public:
                SyncTCPClient(const std::string& raw_ip_address, unsigned short port_num) :
                    m_ep(asio::ip::address::from_string(raw_ip_address), port_num), m_sock(m_ios)
                {
                    m_sock.open(m_ep.protocol());
                }
                void connect() {m_sock.connect(m_ep);}
                void close();
                std::string emulateLongComputationOp (unsigned int duration_sec);
            private:
                void sendRequest(const std::string& request)
                 {asio::write(m_sock, asio::buffer(request));}
                std::string receiveResponse();
                asio::io_service m_ios;               
                asio::ip::tcp::endpoint m_ep;
                asio::ip::tcp::socket m_sock;
        };
    }
}

#endif