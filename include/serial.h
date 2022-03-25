#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

#include <iostream>
#include <string>
#include <vector>

using namespace std;

using Tstring = string;
using Tchar = char;

class Serial {
 private:
  string port;

  HANDLE handle = nullptr;
  bool connected = false;
  COMSTAT status;
  DWORD errors = 0;

 public:
  Serial();
  ~Serial();

  bool open(const char *portname);
  void close();
  vector<unsigned char> read();
};
