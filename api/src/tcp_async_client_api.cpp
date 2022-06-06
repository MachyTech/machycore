#include "tcp_async_client_api.h"

namespace machyAPI
{
    namespace machysockets_aSync
    {
        void handler(unsigned int request_id, const std::string& response, const system::error_code& ec)
        {
            if (ec == 0) {
                std::cout << "Request #"<<request_id<<" has completed. Response: "
                    << response << std::endl;
            } else if (ec == asio::error::operation_aborted) {
                std::cout << "Request #" << request_id << " has been cancelled by the user.\n";
            } else {
                std::cout << "Request #"<< request_id << " failed! Error code = " << ec.value()
                <<". Error message = "<<ec.message()<<std::endl;
            }

            return;
        }

        void read_callback(const boost::system::error_code& ec, std::size_t bytes_transferred, std::shared_ptr<read_Session> s)
        {
            if (ec != 0) {
                std::cout << "Error occured! Error code = "<< ec.value() << ". Message: "<< ec.message();
                return;
            }

            s->total_bytes_read += bytes_transferred;

            if(s->total_bytes_read == s->buf_size)
            {
                return;
            }

            s->sock->async_read_some(
                asio::buffer(
                s->buf.get() +
                s->total_bytes_read,
                s->buf_size -
                s->total_bytes_read),
                std::bind(read_callback, std::placeholders::_1,
                std::placeholders::_2, s));
        }

        void write_callback(const boost::system::error_code& ec, std::size_t bytes_transferred, std::shared_ptr<write_Session> s)
        {
            if (ec != 0) {
                std::cout << "Error occured! Error code = "<< ec.value() << ". Message: "<< ec.message();
                return;
            }

            s->total_bytes_written += bytes_transferred;
            if (s->total_bytes_written == s->buf.length()) {
                return;
            }
            s->sock->async_write_some(
                asio::buffer(s->buf.c_str() + s->total_bytes_written,
                                s->buf.length() - s->total_bytes_written),
                std::bind(write_callback, std::placeholders::_1,
                            std::placeholders::_2, s)
            );
        }

        void readFromSocket(std::shared_ptr<asio::ip::tcp::socket> sock)
        {
            std::shared_ptr<read_Session> s(new read_Session);

            const unsigned int MESSAGE_SIZE = 9;

            s->buf.reset(new char[MESSAGE_SIZE]);
            s->total_bytes_read = 0;
            s->sock = sock;
            s->buf_size = MESSAGE_SIZE;
            
            s->sock->async_read_some(
                asio::buffer(s->buf.get(), s->buf_size),
                std::bind(read_callback,
                        std::placeholders::_1,
                        std::placeholders::_2,
                        s)
            );
        }

        void writeToSocket(std::shared_ptr<asio::ip::tcp::socket> sock)
        {
            std::shared_ptr<write_Session> s(new write_Session);

            s->buf = std::string("Hello");
            s->total_bytes_written  = 0;
            s->sock = sock;

            s->sock->async_write_some(
                asio::buffer(s->buf),
                std::bind(write_callback, std::placeholders::_1, std::placeholders::_2, s)
            );
            std::cout<<"i am doing something while writing"<<std::endl;
        }

        int connect_asynchronous_socket()
        {
            std::string raw_ip_address = "127.0.0.1";
            unsigned short port_num = 3333;

            try{
                asio::ip::tcp::endpoint ep(asio::ip::address::from_string(raw_ip_address), port_num);
                asio::io_service ios;
                std::shared_ptr<asio::ip::tcp::socket> sock(new asio::ip::tcp::socket(ios, ep.protocol()));
                new asio::ip::tcp::socket(ios, ep.protocol());

                sock->connect(ep);

                readFromSocket(sock);

                ios.run();
            }
            catch(system::system_error &e) {
                std::cout<< "Error occured! Error code = "<<e.code()
                    << ". Message: "<<e.what();
                return e.code().value(); 
            }
            return 0;
        }

        int asynchronous_client()
        {
            try {
                aSyncTCPClient client;

                client.emulateLongComputationOp(10, "127.0.0.1", 3333, handler, 1);
                std::this_thread::sleep_for(std::chrono::seconds(2));
                client.emulateLongComputationOp(11, "127.0.0.1", 3333, handler, 2);
                client.cancelRequest(1);
                std::this_thread::sleep_for(std::chrono::seconds(6));
                client.emulateLongComputationOp(12, "127.0.0.1", 3333, handler, 3);
                std::this_thread::sleep_for(std::chrono::seconds(15));
                client.close();
            }
            catch (system::system_error &e) {
                std::cout << "Error occured! Error code = " << e.code() << ". Message: "<< e.what();
                return e.code().value();
            }
            return 0;
        }

        void aSyncTCPClient::emulateLongComputationOp(unsigned int duration_sec, 
            const std::string& raw_ip_address,
            unsigned short port_num,
            Callback callback,
            unsigned int request_id)
        {
            std::string request = "EMULATE_LONG_CALC_OP " + std::to_string(duration_sec) + "\n";
            std::shared_ptr<aSync_Session> session =
            std::shared_ptr<aSync_Session>(new aSync_Session(m_ios, 
                raw_ip_address, port_num, request, request_id, callback));
            session->m_sock.open(session->m_ep.protocol());

            std::unique_lock<std::mutex> lock(m_active_sessions_guard);
            m_active_sessions[request_id] = session;
            lock.unlock();
            session->m_sock.async_connect(session->m_ep,
                [this, session](const system::error_code& ec)
                {
                    if (ec != 0) {
                        session->m_ec = ec;
                        onRequestComplete(session);
                        return;
                    }
                    std::unique_lock<std::mutex>cancel_lock(session->m_cancel_guard);
                    if(session->m_was_cancelled){
                        onRequestComplete(session);
                        return;
                    }
                    asio::async_write(session->m_sock, asio::buffer(session->m_request),
                    [this, session](const boost::system::error_code& ec,
                                        std::size_t bytes_transferred)
                        {
                            if(ec != 0) {
                                session->m_ec = ec;
                                onRequestComplete(session);
                                return;
                            }
                            std::unique_lock<std::mutex>cancel_lock(session->m_cancel_guard);
                            if (session->m_was_cancelled) {
                                onRequestComplete(session);
                                return;
                            }
                            asio::async_read_until(session->m_sock, session->m_response_buf,'\n',
                            [this, session](const boost::system::error_code& ec,
                                std::size_t bytes_transferred)
                                {
                                    if (ec != 0) {
                                        session->m_ec = ec;
                                    } else {
                                        std::istream strm(&session->m_response_buf);
                                        std::getline(strm, session->m_response);
                                    }

                                    onRequestComplete(session);
                                });//async_read_until
                        });// async_write
                });// async_connect
        }

        void aSyncTCPClient::cancelRequest(unsigned int request_id) {
            std::unique_lock<std::mutex> lock(m_active_sessions_guard);
            auto it = m_active_sessions.find(request_id);
            if (it != m_active_sessions.end()) {
                std::unique_lock<std::mutex>cancel_lock(it->second->m_cancel_guard);
                it->second->m_was_cancelled = true;
                it->second->m_sock.cancel();
            }
        }

        void aSyncTCPClient::onRequestComplete(std::shared_ptr<aSync_Session> session)
        {
            boost::system::error_code ignored_ec;
            session->m_sock.shutdown(asio::ip::tcp::socket::shutdown_both, ignored_ec);
            std::unique_lock<std::mutex> lock(m_active_sessions_guard);
            auto it = m_active_sessions.find(session->m_id);
            if (it != m_active_sessions.end())
                m_active_sessions.erase(it);
            lock.unlock();
            boost::system::error_code ec;
            if (session->m_ec == 0 && session->m_was_cancelled)
                ec = asio::error::operation_aborted;
            else
                ec = session->m_ec;
            session->m_callback(session->m_id, session->m_response, ec);
        }
    }
}