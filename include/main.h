#include <serial.h>

#include <chrono>
#include <cstring>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

#include "CHOP_CPlusPlusBase.h"
#include "CPlusPlus_Common.h"

using namespace std;

class ClimbingTimerCHOP : public CHOP_CPlusPlusBase {
 public:
  string portname = "";

  vector<string> chanNames{"minute", "second", "interval"};
  vector<double> chanValues{0.0f, 0.0f, 0.0f};

  thread recv_thread;
  bool running = false;
  bool interval = false;

  unsigned int pos = 0, minute = 0, second = 0;

  Serial serial;

  ClimbingTimerCHOP(const OP_NodeInfo* info);
  ~ClimbingTimerCHOP();

  bool open();
  void start();
  void stop();
  void close();
  void loop();

  void createParams();

  virtual void getGeneralInfo(CHOP_GeneralInfo* ginfo, const OP_Inputs* inputs,
                              void* reserved1);
  virtual bool getOutputInfo(CHOP_OutputInfo* info, const OP_Inputs* inputs,
                             void* reserved1);
  virtual void getChannelName(int32_t index, OP_String* name,
                              const OP_Inputs* inputs, void* reserved1);
  virtual void execute(CHOP_Output* output, const OP_Inputs* inputs,
                       void* reserved);
  virtual void setupParameters(OP_ParameterManager* manager, void* reserved1);
};
