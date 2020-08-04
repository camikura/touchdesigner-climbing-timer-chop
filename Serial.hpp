#pragma once

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

#include <string>
#include <vector>
#include <iostream>

using Tstring = std::string;
using Tchar = char;

class Serial {

private:
	std::string port;

	HANDLE handle;
	bool connected;
	COMSTAT status;
	DWORD errors;

public:
	Serial();
	~Serial();

	bool Open(const std::string port);
	void Close();
	bool IsConnected();

	std::vector<unsigned char> Read();
};
