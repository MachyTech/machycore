#ifndef API_H_
#define API_H_

#include <iostream>
#include <boost/asio/io_context.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/read_until.hpp>
#include <boost/asio/write.hpp>
#include <boost/asio/steady_timer.hpp>

#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>

#include <nlohmann/json.hpp>
#include <machycore.h>

#include <functional>
#include <iostream>
#include <string>
#include <mutex>

using boost::asio::steady_timer;
using boost::asio::ip::tcp;
using std::placeholders::_1;
using std::placeholders::_2;

namespace machyapi
{
    using boost::asio::ip::tcp;
    
    class session
    {
        public:
            session(boost::asio::io_service& io_service)
		        : socket_(io_service)
            {}

            tcp::socket& socket()
            {
                return socket_;
            }

            void start()
	        {
		        socket_.async_read_some(boost::asio::buffer(data_, max_length),
			        boost::bind(&session::handle_read, this,
			        boost::asio::placeholders::error,
			        boost::asio::placeholders::bytes_transferred));
	        }

        private:
            	void handle_read(const boost::system::error_code& error, size_t bytes_transferred)
                {
                    if(!error)
                    {
                        boost::asio::async_write(socket_,
                        boost::asio::buffer(data_, max_length),
                        boost::bind(&session::handle_write, this,
                            boost::asio::placeholders::error));
                    }
                    else
                    {
                        delete this;
                    }
                }

            void handle_write(const boost::system::error_code& error)
            {
                if (!error)
                {
                    socket_.async_read_some(boost::asio::buffer(data_, max_length),
                        boost::bind(&session::handle_read, this,
                            boost::asio::placeholders::error,
                            boost::asio::placeholders::bytes_transferred));
                }
            }

            tcp::socket socket_;
	        enum { max_length = 1024 };
	        char data_[max_length];
    };

    class server
    {
    public:
        server(boost::asio::io_service& io_service, short port)
            :io_service_(io_service),
            acceptor_(io_service, tcp::endpoint(tcp::v4(), port))
        {
            start_accept();
        }
    private:
        void start_accept()
        {
            session* new_session = new session(io_service_);

            acceptor_.async_accept(new_session->socket(),
                boost::bind(&server::handle_accept, this, 
                    new_session, boost::asio::placeholders::error));
        }

        void handle_accept(session* new_session, const boost::system::error_code& error)
        {
            if (!error)
                new_session->start();
            else
                delete new_session;
            
            start_accept();
        }

        boost::asio::io_service& io_service_;
        tcp::acceptor acceptor_;
    };

    class client
    {
        public:
            client(boost::asio::io_context& io_context, std::string ip, std::string port, machycore::controller_data* controller_data)
                : socket_(io_context),
                    deadline_(io_context),
                    heartbeat_timer_(io_context),
                    ip_(ip),
                    resolver_(io_context),
                    port_(port),
                    printer_(io_context),
                    starter_(io_context),
                    _controller(controller_data)
            {
                start(resolver_.resolve(ip_, port_));
            }
            void start(tcp::resolver::results_type);
            void stop();
            machycore::controller_data* _controller;
        private:
            void start_connect(tcp::resolver::results_type::iterator);
            void handle_connect(const boost::system::error_code&,
                tcp::resolver::results_type::iterator);
            void start_read();
            void start_write();
            void handle_read(const boost::system::error_code& error, std::size_t n);
            void handle_write(const boost::system::error_code& error);
            void check_deadline();
            void print();
            void starter();
        private:
            tcp::resolver resolver_;
            std::string ip_, port_;
            bool stopped_ = false;
            tcp::resolver::results_type endpoints_;
            tcp::socket socket_;
            std::string input_buffer_;
            steady_timer deadline_;
            steady_timer heartbeat_timer_;
            steady_timer printer_;
            steady_timer starter_;
    };
}
#endif