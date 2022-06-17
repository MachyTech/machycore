#include "machyapi.h"

namespace machyapi
{
    void client::print()
    {
        nlohmann::json json;
        if (_controller->connected)
        {
            _controller->mtx_.lock();
            json["normalizedLX"] = _controller->normalizedLX;
            json["normalizedLY"] = _controller->normalizedLY;
            json["normalizedMagnitude"] = _controller->normalizedMagnitude;
            _controller->mtx_.unlock();
            std::cout<<"controller: " << json << std::endl;
        }
        
        printer_.expires_after(std::chrono::seconds(1));
        printer_.async_wait(std::bind(&client::print, this));
    }
    void client::start(tcp::resolver::results_type endpoints)
    {
        // Start the connect actor.
        stopped_ = false;
        endpoints_ = endpoints;
        start_connect(endpoints_.begin());

        // Start the deadline actor. You will note that we're not setting any
        // particular deadline here. Instead, the connect and input actors will
        // update the deadline prior to each asynchronous operation.
        printer_.async_wait(boost::bind(&client::print, this));
        deadline_.async_wait(std::bind(&client::check_deadline, this));
    }

    void client::stop()
    {
        stopped_ = true;
        _controller->connected = false;
        boost::system::error_code ignored_error;
        socket_.close(ignored_error);
        // deadline_.cancel();
        // heartbeat_timer_.cancel();

        starter_.expires_after(std::chrono::seconds(2));
        starter_.async_wait(boost::bind(&client::starter, this));
    }

    void client::starter()
    {
        client::start(resolver_.resolve(ip_, port_));
    }

    void client::start_connect(tcp::resolver::results_type::iterator endpoint_iter)
    {
        if (endpoint_iter != endpoints_.end())
        {
        std::cout << "Trying " << endpoint_iter->endpoint() << "...\n";

        // Set a deadline for the connect operation.
        deadline_.expires_after(std::chrono::seconds(2));

        // Start the asynchronous connect operation.
        socket_.async_connect(endpoint_iter->endpoint(),
            std::bind(&client::handle_connect,
                this, _1, endpoint_iter));
        }
        else
        {
        // There are no more endpoints to try. Shut down the client.
            client::stop();
        }
    }

    void client::handle_connect(const boost::system::error_code& error,
        tcp::resolver::results_type::iterator endpoint_iter)
    {
        if (stopped_)
        return;

        // The async_connect() function automatically opens the socket at the start
        // of the asynchronous operation. If the socket is closed at this time then
        // the timeout handler must have run first.
        if (!socket_.is_open())
        {
        std::cout << "Connect timed out\n";

        // Try the next available endpoint.
        start_connect(++endpoint_iter);
        }

        // Check if the connect operation failed before the deadline expired.
        else if (error)
        {
        std::cout << "Connect error: " << error.message() << "\n";

        // We need to close the socket used in the previous connection attempt
        // before starting a new one.
        socket_.close();

        // Try the next available endpoint.
        start_connect(++endpoint_iter);
        }

        // Otherwise we have successfully established a connection.
        else
        {
            std::cout << "Connected to " << endpoint_iter->endpoint() << "\n";
            _controller->connected = true;
            // Start the heartbeat actor.
            start_write();
        }
    }
    

    void client::start_read()
    {
        // Set a deadline for the read operation.
        deadline_.expires_after(std::chrono::seconds(30));

        // Start an asynchronous operation to read a newline-delimited message.
        boost::asio::async_read_until(socket_,
            boost::asio::dynamic_buffer(input_buffer_), '\n',
            std::bind(&client::handle_read, this, _1, _2));
    }

    void client::handle_read(const boost::system::error_code& error, std::size_t n)
    {
        if (stopped_)
        return;

        if (!error)
        {
            // Extract the newline-delimited message from the buffer.
            std::string line(input_buffer_.substr(0, n - 1));
            input_buffer_.erase(0, n);

            // Empty messages are heartbeats and so ignored.
            if (!line.empty())
            {
                nlohmann::json json = nlohmann::json::parse(line);
                if(_controller->mtx_.try_lock())
                {
                    _controller->normalizedLX = json["normalizedLX"];
                    _controller->normalizedLY = json["normalizedLY"];
                    _controller->normalizedMagnitude = json["normalizedMagnitude"];
                    _controller->mtx_.unlock();
                }
            }

            heartbeat_timer_.expires_after(std::chrono::milliseconds(10));
            heartbeat_timer_.async_wait(std::bind(&client::start_write, this));
        }
        else
        {
            std::cout << "Error on receive: " << error.message() << "\n";

            stop();
        }
    }

    void client::start_write()
    {
        if (stopped_)
        return;

        // Start an asynchronous operation to send a heartbeat message.
        boost::asio::async_write(socket_, boost::asio::buffer("\n", 1),
            std::bind(&client::handle_write, this, _1));
    }

    void client::handle_write(const boost::system::error_code& error)
    {
        if (stopped_)
        return;

        if (!error)
        {
            // Wait 10 seconds before sending the next heartbeat.
            start_read();
        }
        else
        {
        std::cout << "Error on heartbeat: " << error.message() << "\n";

        stop();
        }
    }

    void client::check_deadline()
    {
        if (stopped_)
        return;

        // Check whether the deadline has passed. We compare the deadline against
        // the current time since a new asynchronous operation may have moved the
        // deadline before this actor had a chance to run.
        if (deadline_.expiry() <= steady_timer::clock_type::now())
        {
        // The deadline has passed. The socket is closed so that any outstanding
        // asynchronous operations are cancelled.
        socket_.close();

        // There is no longer an active deadline. The expiry is set to the
        // maximum time point so that the actor takes no action until a new
        // deadline is set.
        deadline_.expires_at(steady_timer::time_point::max());
        }

        // Put the actor back to sleep.
        deadline_.async_wait(std::bind(&client::check_deadline, this));
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