#ifndef MACHYCONTROL_H_
#define MACHYCONTROL_H_

#include <iostream>
#include <math.h>
#include <mutex>
#include <chrono>

#include <boost/asio/io_context.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/bind/bind.hpp>

#include "machycore.h"

#define XboxGain 100
namespace machycontrol
{
    struct mass{
        // mass
        int m_n;
        // internal time
        long t;
        // state
        float v_x, v_y;
        float s_x, s_y;
        // mutex
        std::mutex mtx_;
        // constructor
        mass(int M) : m_n(M*9.81),
            v_x(0.0),
            v_y(0.0),
            s_x(0.0),
            s_y(0.0)
        {}
    };

    void mass_no_moment(int angle, float mag, machycontrol::mass* m);
}
#endif