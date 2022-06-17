#include "machycontrol.h"

namespace machycontrol
{
    void cartesian_to_diamond(float norm_x, float norm_y, float mag, float* force_y, float* force_x)
    {
        if(mag!=0)
        {
            int angle = atan2(norm_x, norm_y);
            switch((angle/90)%4+1)
            {
                case(1):
                    std::cout<<"q1"<<std::endl;
                case(2):
                    std::cout<<"q2"<<std::endl;
                case(3):
                    std::cout<<"q3"<<std::endl;
                case(4):
                    std::cout<<"q4"<<std::endl;
            }
        }
    }

    void mass_no_moment(int angle, float mag, machycontrol::mass* m)
    {
        long current_time = machycore::current_time_ms();
        if(m->mtx_.try_lock())
        {
            float a = mag/(m->m_n*XboxGain);
            float dt;
            if(m->t==0)
                dt = 0;
            else
                dt = current_time - m->t;
            m->v_x -= a*cos(angle)*dt;
            m->v_y -= a*sin(angle)*dt;
            m->s_x -= a*cos(angle)*dt*dt/2;
            m->s_y -= a*sin(angle)*dt*dt/2;
            m->t = current_time;
            m->mtx_.unlock();
        }
    }
}