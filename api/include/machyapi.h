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
#include <xcontroller.h>

#include <functional>
#include <iostream>
#include <string>
#include <mutex>

// server types
#define GENERIC_SERVER 0
#define XCONTROLLER_SERVER 1

using boost::asio::steady_timer;
using boost::asio::ip::tcp;

namespace machyapi
{
    using boost::asio::ip::tcp;
    
    struct data_xcontroller
    {
        float normalizedAngle;
        float normalizedMagnitude;
        // json serialization information
        NLOHMANN_DEFINE_TYPE_INTRUSIVE(data_xcontroller, normalizedAngle, normalizedMagnitude);
    };

    class session
    {
        public:
            session(boost::asio::io_service& io_service)
		        : socket_(io_service),
                session_type(GENERIC_SERVER)
            {}
#ifdef _WIN32
            session(boost::asio::io_service& io_service, manualcontrol::xcontroller* controller)
                : socket_(io_service),
                controller_(controller),
                session_type(XCONTROLLER_SERVER)
            {}
#endif
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
                switch (session_type)
                {
                    case GENERIC_SERVER :
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
#ifdef _WIN32
                    case XCONTROLLER_SERVER :
                        if (!error && controller_->getConnected() != 0)
                        { 
                            machyapi::data_xcontroller* c_data = new machyapi::data_xcontroller;

                            c_data->normalizedAngle = controller_->getAngle();
                            c_data->normalizedMagnitude = controller_->getMagnitude();
                            
                            nlohmann::json json = *c_data;
                            std::string json_as_string = json.dump() + "\n";
                            boost::asio::async_write(socket_,
                                boost::asio::buffer(json_as_string.c_str(), json_as_string.length()),
                                boost::bind(&session::handle_write, this,
                                    boost::asio::placeholders::error));
                        }
                        else
                        {
                            delete this;
                        }
#endif
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
#ifdef _WIN32
            manualcontrol::xcontroller* controller_;
#endif
            int session_type;
    };

    class server
    {
    public:
        server(boost::asio::io_service& io_service, short port)
            :io_service_(io_service),
            server_type(GENERIC_SERVER),
            acceptor_(io_service, tcp::endpoint(tcp::v4(), port))
        {
            start_accept();
        }
#ifdef _WIN32
        server(boost::asio::io_service& io_service, short port, manualcontrol::xcontroller* controller)
            :io_service_(io_service),
            server_type(XCONTROLLER_SERVER),
            controller_(controller),
            acceptor_(io_service, tcp::endpoint(tcp::v4(), port))
        {
            start_accept();
        }
#endif
    private:
        void start_accept()
        {
            session* new_session;
            switch(server_type)
            {
                case GENERIC_SERVER:
                    new_session = new session(io_service_);
#ifdef _WIN32
                case XCONTROLLER_SERVER:
                    new_session = new session(io_service_, controller_);
#endif
            }
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

        int server_type;
#ifdef _WIN32
        manualcontrol::xcontroller* controller_;
#endif
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