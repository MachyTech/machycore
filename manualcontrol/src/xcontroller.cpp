#include <xcontroller.h>

namespace manualcontrol
{
#ifdef _WIN32
    void xcontroller::xcontroller_main() {
		bool was_connected = false;
		while (true) {
			float LY, LX = 0.0;

			if (IsConnected())
			{
				was_connected = true;
				LY = GetState().Gamepad.sThumbRY;
				LX = GetState().Gamepad.sThumbRX;
			}
			else {
				was_connected = false;
				controller_->mtx_.lock();
				controller_->xSuccess = 0;
				controller_->mtx_.unlock();
				std::cout << "XBOX Controller not connected...\n";
				std::this_thread::sleep_for(std::chrono::seconds(2));
				continue;
			}

			float magnitude = sqrt(LX * LX + LY * LY);

			float normalizedMagnitude = magnitude;

			if (normalizedMagnitude > XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE) {
				if (normalizedMagnitude > 32767) normalizedMagnitude = 32767;
				normalizedMagnitude -= XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE;
				normalizedMagnitude = normalizedMagnitude / (32767 - XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);
			}
			else {
				normalizedMagnitude = 0;
			}
			// critical section
			if (was_connected)
			{
				if (controller_->mtx_.try_lock())
				{
					controller_->xSuccess = 1;
					controller_->normalizedLX = LX / magnitude;
					controller_->normalizedLY = LY / magnitude;
					controller_->normalizedMagnitude = normalizedMagnitude;
					controller_->mtx_.unlock();
				}
			}
		}
	}

	XINPUT_STATE xcontroller::GetState()
	{
		ZeroMemory(&_controllerState, sizeof(XINPUT_STATE));

		XInputGetState(_controllerNum, &_controllerState);

		return _controllerState;
	}

	bool xcontroller::IsConnected()
	{
		ZeroMemory(&_controllerState, sizeof(XINPUT_STATE));
		//DWORD XInputGetState([in] DWORD dwUserIndex, [out] XINPUT_STATE* pState);
		DWORD Result = XInputGetState(_controllerNum, &_controllerState);

		if (Result == ERROR_SUCCESS)
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	void xcontroller::Vibrate(int leftVal, int rightVal)
	{
		XINPUT_VIBRATION Vibration;

		ZeroMemory(&Vibration, sizeof(XINPUT_VIBRATION));

		// Set the Vibration values
		// max is 65535
		Vibration.wLeftMotorSpeed = leftVal;
		Vibration.wRightMotorSpeed = rightVal;

		// Vibrate the controller
		XInputSetState(_controllerNum, &Vibration);
	}
#endif // WIN32
}