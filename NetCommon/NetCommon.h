#ifndef __NETCOMMON_H__
#define __NETCOMMON_H__



//-D_WIN32_WINNT=0x0601 
// #ifdef _WIN32
// #define _WIN32_WINNT 0X0A00
// #endif


#include <iostream>
#include <string>
#include <vector>
#include <deque>
#include <thread>
#include <mutex>
#include <chrono>
#include <memory>
#include <algorithm>
#include <functional>

#define ASIO_STANDALONE
//#define ASIO_HAS_BOOST_DATE_TIME
#include <asio.hpp>
#include <asio/ts/buffer.hpp>
#include <asio/ts/internet.hpp>
#include "asio/steady_timer.hpp"
//#include "asio/deadline_timer.hpp"


#include "CommonConst.h"

#endif//end of __NETCOMMON_H__
