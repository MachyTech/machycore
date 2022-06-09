#include "api.h"

namespace machyapi
{
    void client::print()
    {
        nlohmann::json json;
        json["normalizedLX"] = _controller->normalizedLX;
        json["normalizedLY"] = _controller->normalizedLY;
        json["normalizedMagnitude"] = _controller->normalizedMagnitude;
        std::cout<<"controller: " << json << std::endl;
        
        printer_.expires_after(std::chrono::seconds(1));
        printer_.async_wait(std::bind(&client::print, this));
    }

    void client::start(tcp::resolver::results_type endpoints)
    {
        // start the connect actor
        endpoints_ = endpoints;
        start_connect(endpoints_.begin());
        printer_.async_wait(std::bind(&client::print, this));
        deadline_.async_wait(std::bind(&client::check_deadline, this));
    }

    void client::stop()
    {
        stopped_ = true;
        boost::system::error_code ignored_error;
        socket_.close(ignored_error);
        deadline_.cancel();
        heartbeat_timer_.cancel();
        printer_.cancel();
    }

    void client::start_connect(tcp::resolver::results_type::iterator endpoint_iter)
    {
        if (endpoint_iter != endpoints_.end())
        {
            std::cout << "Trying " << endpoint_iter->endpoint() << "...\n";
            
            deadline_.expires_after(std::chrono::seconds(60));
            socket_.async_connect(endpoint_iter->endpoint(), std::bind(&client::handle_connect,
                this, _1, endpoint_iter));
        }
        else
        {
            stop();
        }
    }

    void client::handle_connect(const boost::system::error_code& error,
        tcp::resolver::results_type::iterator endpoint_iter)
    {
        if (stopped_)
            return;
        
        if(!socket_.is_open())
        {
            std::cout << "Connect timed out\n";
            start_connect(++endpoint_iter);
        }
        else if (error)
        {
            std::cout << "Connect error: " << error.message() << "\n";
            socket_.close();
            start_connect(++endpoint_iter);
        }
        else
        {
            std::cout << "Connected to " << endpoint_iter->endpoint() << "\n";
            start_write();
            start_read();
        }
    }
    
    void client::start_read()
    {
        deadline_.expires_after(std::chrono::seconds(30));

        boost::asio::async_read_until(socket_, 
            boost::asio::dynamic_buffer(input_buffer_), "\n",
            std::bind(&client::handle_read, this, _1, _2));
    }

    void client::handle_read(const boost::system::error_code& error, std::size_t n)
    {
        if (stopped_)
            return;
        
        if (!error)
        {
            std::string line(input_buffer_.substr(0, n - 1));
            input_buffer_.erase(0, n);

            if (!line.empty())
            {
                nlohmann::json json = nlohmann::json::parse(line);
                std::cout << "Received: " << json << "\n";
                _controller->normalizedLX = json["normalizedLX"].get<float>();
                _controller->normalizedLY = json["normalizedLY"].get<float>();
                _controller->normalizedMagnitude = json["normalizedMagnitude"].get<float>();
            }

            start_read();
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

        std::cout << "writing... " << std::endl;
        boost::asio::async_write(socket_, boost::asio::buffer("hello\n", 6),
            std::bind(&client::handle_write, this, _1));
    }
    
    void client::handle_write(const boost::system::error_code& error)
    {
        if (stopped_)
            return;
        if (!error)
        {
            heartbeat_timer_.expires_after(std::chrono::seconds(1));
            heartbeat_timer_.async_wait(std::bind(&client::start_write, this));
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
        
        if (deadline_.expiry() <= steady_timer::clock_type::now())
        {
            socket_.close();

            deadline_.expires_at(steady_timer::time_point::max());
        }

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