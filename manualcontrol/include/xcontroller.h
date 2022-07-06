#ifndef _XCONTROLLER_H_
#define _XCONTROLLER_H_

#ifdef _WIN32
    #ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
    #endif

    #include <windows.h>
    #include <XInput.h>
#endif //_WIN32

#if (__cplusplus >= 201703L)
    #include <semaphore>
    #define MAX_SEM_COUNT 10
#endif //C++20

#include <iostream>
#include <mutex>
#include <math.h>
#include <boost/asio/io_context.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/bind/bind.hpp>
#include <boost/thread.hpp>
#include <thread>

namespace manualcontrol
{
    struct xcontroller_data {
		// stored and filtered values
		float normalizedLX;
		float normalizedLY;
		float normalizedMagnitude;
#if (__cplusplus >= 201703L)
		// semaphore
		std::counting_semaphore<MAX_SEM_COUNT> _smph{ MAX_SEM_COUNT };
#endif //C++20
		// mutex
		std::mutex mtx_;
		// success
		int xSuccess;
	};

#ifdef _WIN32
    class xcontroller
    {
        private:
            XINPUT_STATE _controllerState;
            int _controllerNum;
            bool IsConnected();
            struct xcontroller_data* controller_;
		    XINPUT_STATE GetState();
        public:
        	xcontroller(boost::asio::io_context& io_context, int playernumber) :
			    _controllerNum(playernumber - 1),
			    vibration_val(32767),
			    controller_(new xcontroller_data),
			    connecter_(io_context)
		    {
			    std::thread* xcontroller_thread = new std::thread( &xcontroller::xcontroller_main, this );
		    }
            void xcontroller_main();
            int vibration_val;
            boost::asio::steady_timer connecter_;
            /* thread safe function to check connection */
            int getConnected() {
#if (_cplusplus >= 201703L)
			    controller_->mtx_.lock();
			    controller_->_smph.acquire();
			    controller_->mtx_.unlock();
			    int val = controller_->xSuccess;
			    controller_->_smph.release();
#elif (_cplusplus < 201703L)
                controller_->mtx_.lock();
                int val = controller_->xSuccess;
                controller_->mtx_.unlock();
#endif
			    return val;
		    }
            /* thread safe function to read the angle */
            float getAngle() {
#if (_cplusplus >= 201703L)
                controller_->mtx_.lock();
                controller_->_smph.acquire();
                controller_->mtx_.unlock();
                float val = atan2(controller_->normalizedLY, controller_->normalizedLX);
                controller_->_smph.release();
#elif (_cplusplus < 201703L)
                controller_->mtx_.lock();
                float val = atan2(controller_->normalizedLY, controller_->normalizedLX);
                controller_->mtx_.unlock();
#endif
                return val;
            }
            /* thread safe function to read the magnitude */
            float getMagnitude() {
#if (_cplusplus >= 201703L)
                controller_->mtx_.lock();
                controller_->_smph.acquire();
                controller_->mtx_.unlock();
                float val = controller_->normalizedMagnitude;
                controller_->_smph.release();
#elif (_cplusplus < 201703L)
                controller_->mtx_.lock();
                float val = controller_->normalizedMagnitude;
                controller_->mtx_.unlock(); 
#endif
                return val;
            }
            /* function to start virbrating the controller */
            void Vibrate(int leftVal = 0, int rightVal = 0);
    };
#endif // WIN32
}
#endif // _MACHY_GL_H_