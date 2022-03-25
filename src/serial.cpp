#include "serial.h"

Serial::Serial() {}

Serial::~Serial() { this->close(); }

bool Serial::open(const char* portname) {
  handle = CreateFile(portname, GENERIC_READ | GENERIC_WRITE, 0, NULL,
                      OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

  if (handle == INVALID_HANDLE_VALUE) {
    if (GetLastError() == ERROR_FILE_NOT_FOUND) {
      cout << "ERROR: Handle was not attached. Reason: " << portname
           << " not available" << endl;
    } else {
      cout << "ERROR!!!" << endl;
    }
  } else {
    DCB dcb = {0};
    if (!GetCommState(handle, &dcb)) {
      cout << "failed to get current serial parameters!" << endl;
    } else {
      dcb.BaudRate = CBR_9600;
      dcb.ByteSize = 8;
      dcb.StopBits = ONESTOPBIT;
      dcb.Parity = NOPARITY;
      dcb.fDtrControl = DTR_CONTROL_ENABLE;

      if (!SetCommState(handle, &dcb)) {
        cout << "ALERT: Could not set Serial Port parameters" << endl;
      } else {
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

void Serial::close() {
  if (connected) {
    connected = false;
    CloseHandle(handle);
  }
}

int available(void* handle) {
  unsigned long error;
  COMSTAT stat;
  ClearCommError(handle, &error, &stat);
  return stat.cbInQue;
}

vector<unsigned char> Serial::read() {
  vector<unsigned char> vals;
  char buffer[256] = "";
  unsigned int nbChar = 255;

  DWORD bytesRead;
  unsigned int toRead;
  ClearCommError(handle, &errors, &status);

  if (status.cbInQue > 0) {
    if (status.cbInQue > nbChar) {
      toRead = nbChar;
    } else {
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
