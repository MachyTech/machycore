#ifndef TCP_ASYNC_CLIENT_API_H_
#define TCP_ASYNC_CLIENT_API_H_

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
        struct read_Session {
            std::shared_ptr<asio::ip::tcp::socket> sock;
            std::unique_ptr<char[]> buf;
            std::size_t total_bytes_read;
            unsigned int buf_size;
        };
        /*
         * store objects for callback function
         * and identify al data has been read
         */
        struct write_Session {
            std::shared_ptr<asio::ip::tcp::socket> sock;
            std::string buf;
            std::size_t total_bytes_written;
        };
        /*
         * store objects for callback function
         * and identify al data has been written
         */
        typedef void(*Callback) (unsigned int request_id, const std::string& response, const system::error_code& ec);
        /*
         * data type representing a pointer to a callback function.
         * used by the asynchronous client application
         */
        int asynchronous_client();
        /*
         * connect to a server in asynchronous manner
         */
        int connect_asynchronous_socket();
        /*
         * connect to asynchronous socket
         */
        void writeToSocket(std::shared_ptr<asio::ip::tcp::socket> sock);
        /*
         * asynchronous writing operation using boosts async_write_some
         */
        void readFromSocket(std::shared_ptr<asio::ip::tcp::socket> sock);
        /*
         * asynchronous reading operation using boosts async_read_some
         */
        void handler(unsigned int request_id, const std::string& response, const system::error_code& ec);

        void read_callback(const boost::system::error_code& ec, std::size_t bytes_transferred, std::shared_ptr<read_Session> s);
        /*
         * callback function for asynchronous reading
         */
        void write_callback(const boost::system::error_code& ec, std::size_t bytes_transferred, std::shared_ptr<write_Session> s);
        /*
         * callback function for asynchronous writing
         */
        struct aSync_Session {
            aSync_Session(asio::io_service& ios,
            const std::string& raw_ip_address,
            unsigned short port_num,
            const std::string& request,
            unsigned int id,
            Callback callback) :
            m_sock(ios),
            m_ep(asio::ip::address::from_string(raw_ip_address),port_num),
            m_request(request),
            m_id(id),
            m_callback(callback),
            m_was_cancelled(false) {}

            asio::ip::tcp::socket m_sock;
            asio::ip::tcp::endpoint m_ep;
            std::string m_request;

            asio::streambuf m_response_buf;
            std::string m_response;

            system::error_code m_ec;

            unsigned int m_id;

            Callback m_callback;

            bool m_was_cancelled;
            std::mutex m_cancel_guard;
        };
        /*
         * keep data related to a particular request
         * while it is being executed
         */
        class aSyncTCPClient : public boost::noncopyable
        {
            /*
             * all the functionality related to communication
             * with the server application. It spawns a thread
             * that plays the role of I/O thread in the application.
             * In the context of this thread, the callbacks assigned
             * asynchronous operations will be invoked.
             */
            public:
                aSyncTCPClient() {
                    m_work.reset(new asio::io_service::work(m_ios));
                    m_thread.reset(new std::thread([this] (){
                        m_ios.run();
                    }));
                }
                void emulateLongComputationOp(unsigned int duration_sec, 
                    const std::string& raw_ip_address,
                    unsigned short port_num, Callback callback, unsigned int request_id);

                void cancelRequest(unsigned int request_id);
                void close(){
                    m_work.reset(NULL);
                    m_thread->join();
                }
            private:
                void onRequestComplete(std::shared_ptr<aSync_Session> session);
                asio::io_service m_ios;
                std::map<int, std::shared_ptr<aSync_Session>> m_active_sessions;
                std::mutex m_active_sessions_guard;
                std::unique_ptr<boost::asio::io_service::work> m_work;
                std::unique_ptr<std::thread> m_thread;
        };
    }
}

#endif