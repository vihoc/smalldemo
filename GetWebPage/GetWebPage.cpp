// GetWebPage.cpp : This file contains the 'main' function. Program execution begins and ends there.
// author : vihoc

//-D_WIN32_WINNT=0x0601 
// #ifdef _WIN32
// #define _WIN32_WINNT 0X0A00
// #endif

#define ASIO_STANDALONE
#include <asio.hpp>
#include <asio/ts/buffer.hpp>
#include <asio/ts/internet.hpp>
#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <memory>


class Semaphore {
private:
    int n_;
    mutable std::mutex mu_;
    std::condition_variable cv_;

public:
    Semaphore(int n) : n_{ n } {}

public:
    void wait() {
        std::unique_lock<std::mutex> lock(mu_);
        if (!n_) {
            cv_.wait(lock, [this] {return n_; });
        }
        --n_;
    }
    template <class _Predicate>
    void wait(_Predicate pred) {
        std::unique_lock<std::mutex> lock(mu_);
        if (!(std::invoke(pred) || n_)) {
            cv_.wait(lock, [&pred, this] {return std::invoke(pred) || n_; });
        }
        --n_;
    }

    void signal() {
        std::unique_lock<std::mutex> lock(mu_);
        ++n_;
        cv_.notify_one();
    }
    void signalToall() {
        std::unique_lock<std::mutex> lock(mu_);
        ++n_;
        cv_.notify_all();
    }
};


void GrabData(asio::ip::tcp::socket& socket, std::vector<char>& Buffer, Semaphore& singal)
{
    constexpr size_t bufferlength = 1024;
    std::shared_ptr<std::vector<char>> tempbuffer = std::make_shared<std::vector<char>>(1024);
    socket.async_read_some(asio::buffer(tempbuffer->data(), tempbuffer->size()),
         [tempbuffer, &socket, &Buffer, &singal, bufferlength](asio::error_code ec, std::size_t length)
        {
            //if not use shared_ptr, then need delete pointer in lambda
            if (ec)
            {
                std::cout << "fucked:\n" << ec.message() << std::endl;
                return;
            }
            std::cout << " \n\n READ " << length << "bytes.\n\n"; 
            Buffer.reserve(Buffer.size() + length);
            for (/*auto& c : tempbuffer*/int index = 0; index < length; ++index)
            {
                Buffer.emplace_back((*tempbuffer)[index]);
            }
            if(length == bufferlength)
            {
                GrabData(socket, Buffer, singal);
            }
            else
            {
                std::cout << " \n\n READ Finished.\n\n";
                singal.signal();
            }

        }
    );
}


int main()
{
    asio::error_code ec;

    asio::io_context context;

    asio::io_context::work idle_work(context);

    std::thread threadContext([&context]() {context.run(); });

    asio::ip::tcp::endpoint endpoint(asio::ip::make_address(std::move("93.184.216.34"), ec), 80);

    asio::ip::tcp::socket socket(context);
    
    std::vector<char> vBuffer;

    Semaphore singal(0);

    socket.connect(endpoint, ec);

    if (!ec)
    {
        std::cout << "connected" << std::endl;
    }
    else
    {
        std::cout << "Failed to connect to Address:\n" << ec.message() << std::endl;
    }
    if (socket.is_open())
    {
        GrabData(socket, vBuffer, singal);
        std::string sRequest =
            "GET /index.html HTTP/1.1\r\n"
            "Host:example.com\r\n"
            "Connection:close\r\n\r\n";

        socket.write_some(asio::buffer(sRequest.data(), sRequest.size()), ec);
        using namespace std::chrono_literals;
/*        std::this_thread::sleep_for(20000ms);*/
        singal.wait();
        context.stop();
        if (threadContext.joinable()) threadContext.join();
       
       
        //the following code may read not enough data
//         socket.wait(socket.wait_read);
// 
//         size_t bytes = socket.available();
//      std::cout << "Get Byte: " << bytes << "Avaliable" << std::endl;
// 
//         if (bytes > 0)
//         {
//             vBuffer.resize(bytes);
//             socket.read_some(asio::buffer(vBuffer.data(), vBuffer.size()), ec);
//          if (ec)
//          {
//              std::cout << "Fucked:\n" << ec.message() << std::endl;
//          }
//         }
// 
        for (auto& c : vBuffer)
        {
            std::cout << c;
        }
    }

    std::cout << "Hello World!\n";
}

