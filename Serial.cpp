#include "Serial.hpp"

Serial::Serial()
{
	connected = false;
	handle = nullptr;
	errors = 0;
}

Serial::~Serial()
{
	Close();
}

bool Serial::Open(const std::string port)
{
	Tstring path = "\\\\.\\" + port;
	const char* portName = path.c_str();
	handle = CreateFile(portName,
		GENERIC_READ | GENERIC_WRITE,
		0,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);

	if (handle == INVALID_HANDLE_VALUE) {
		if (GetLastError() == ERROR_FILE_NOT_FOUND) {
			std::cout << "ERROR: Handle was not attached. Reason: " << portName << " not available" << std::endl;
		}
		else {
			std::cout << "ERROR!!!" << std::endl;
		}
	}
	else {
		DCB dcb = { 0 };
		if (!GetCommState(handle, &dcb))
		{
			std::cout << "failed to get current serial parameters!" << std::endl;
		}
		else {
			dcb.BaudRate = CBR_9600;
			dcb.ByteSize = 8;
			dcb.StopBits = ONESTOPBIT;
			dcb.Parity = NOPARITY;
			dcb.fDtrControl = DTR_CONTROL_ENABLE;

			if (!SetCommState(handle, &dcb)) {
				std::cout << "ALERT: Could not set Serial Port parameters" << std::endl;
			}
			else {
				PurgeComm(handle, PURGE_RXCLEAR | PURGE_TXCLEAR);
				Sleep(200);

				connected = true;
				return true;
			}
		}
	}

	connected = false;
	handle = nullptr;
	return false;
}

void Serial::Close()
{
	if (connected) {
		connected = false;
		CloseHandle(handle);
	}
}

bool Serial::IsConnected()
{
	return this->connected;
}

int available(void* handle)
{
	unsigned long error;
	COMSTAT stat;
	ClearCommError(handle, &error, &stat);
	return stat.cbInQue;
}

/*
std::vector<unsigned char> Serial::Read()
{
	std::vector<unsigned char> vals;

	unsigned long readSize;
	int read_size = available(handle);
	if (read_size == 0) {
		read_size = 1;
	}

	unsigned char* data = new unsigned char[read_size];
	bool status = ReadFile(handle, data, read_size, &readSize, NULL);
	if (!status)
		return vals;

	for (int i = 0; i < read_size; i++) {
		vals.push_back(data[i]);
	}
	delete[] data;
	return vals;
}
*/

std::vector<unsigned char> Serial::Read()
{
	std::vector<unsigned char> vals;
	char buffer[256] = "";
	unsigned int nbChar = 255;

	DWORD bytesRead;
	unsigned int toRead;
	ClearCommError(handle, &errors, &status);

	if (status.cbInQue > 0) {
		if (status.cbInQue > nbChar) {
			toRead = nbChar;
		}
		else {
			toRead = status.cbInQue;
		}

		if (ReadFile(handle, buffer, toRead, &bytesRead, NULL)) {
			for (int i = 0; i < bytesRead; i++) {
				vals.push_back(buffer[i]);
			}
		}
	}

	return vals;
}
