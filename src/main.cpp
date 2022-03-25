#include "main.h"

ClimbingTimerCHOP::ClimbingTimerCHOP(const OP_NodeInfo* info) {}

ClimbingTimerCHOP::~ClimbingTimerCHOP() {
  this->stop();
  this->close();
}

bool ClimbingTimerCHOP::open() {
  if (!this->serial.open(("\\\\.\\" + this->portname).c_str())) {
    return false;
  }

  return true;
}

void ClimbingTimerCHOP::start() {
  std::cout << "thread start" << std::endl;

  recv_thread = thread([this]() { this->loop(); });
}

void ClimbingTimerCHOP::loop() {
  this->running = true;

  while (this->running) {
    auto vals = this->serial.read();
    for (auto c : vals) {
      if (c == 0x02) {
        pos = 0;
        this->minute = 0;
        this->second = 0;
        this->interval = false;
      } else if (c == 0x03) {
        this->createParams();
      } else {
        if (pos == 0) this->minute += (c - 0x30) * 10;
        if (pos == 1) this->minute += c - 0x30;
        if (pos == 2) this->second += (c - 0x30) * 10;
        if (pos == 3) this->second += c - 0x30;
        if (pos == 5) this->interval = c == 0x31;
        pos++;
      }
    }
  }
}

void ClimbingTimerCHOP::createParams() {
  this->chanValues[0] = (float)this->minute;
  this->chanValues[1] = (float)this->second;
  this->chanValues[2] = (float)this->interval ? 1 : 0;
}

void ClimbingTimerCHOP::stop() {
  std::cout << "thread stop" << std::endl;

  this->running = false;
  if (recv_thread.joinable()) recv_thread.join();
}

void ClimbingTimerCHOP::close() { this->serial.close(); }

void ClimbingTimerCHOP::getGeneralInfo(CHOP_GeneralInfo* ginfo,
                                       const OP_Inputs* inputs,
                                       void* reserved1) {
  ginfo->cookEveryFrameIfAsked = true;
  ginfo->timeslice = false;
  ginfo->inputMatchIndex = 0;
}

bool ClimbingTimerCHOP::getOutputInfo(CHOP_OutputInfo* info,
                                      const OP_Inputs* inputs,
                                      void* reserved1) {
  info->numSamples = 1;
  info->numChannels = (int32_t)this->chanNames.size();
  return true;
}

void ClimbingTimerCHOP::getChannelName(int32_t index, OP_String* name,
                                       const OP_Inputs* inputs,
                                       void* reserved1) {
  name->setString(this->chanNames.at(index).c_str());
}

void ClimbingTimerCHOP::execute(CHOP_Output* output, const OP_Inputs* inputs,
                                void* reserved) {
  std::string name = inputs->getParString("Portname");

  if (this->portname != name) {
    this->stop();
    this->close();
  }
  this->portname = name;

  if (!this->portname.size()) return;

  if (!this->running) {
    if (!this->open()) return;
    this->start();
  }

  for (int i = 0; i < this->chanNames.size(); i++) {
    for (int j = 0; j < output->numSamples; j++) {
      output->channels[i][j] = (float)this->chanValues.at(i);
    }
  }
}

void ClimbingTimerCHOP::setupParameters(OP_ParameterManager* manager,
                                        void* reserved1) {
  {
    OP_StringParameter sp;
    sp.name = "Portname";
    sp.label = "Port Name";
    sp.page = "Config";
    OP_ParAppendResult res = manager->appendString(sp);
    assert(res == OP_ParAppendResult::Success);
  }
}

extern "C" {
DLLEXPORT void FillCHOPPluginInfo(CHOP_PluginInfo* info) {
  info->apiVersion = CHOPCPlusPlusAPIVersion;
  info->customOPInfo.opType->setString("Climbingtimer");
  info->customOPInfo.opLabel->setString("Climbing Timer");
  info->customOPInfo.authorName->setString("Akira Kamikura");
  info->customOPInfo.authorEmail->setString("akira.kamikura@gmail.com");
}

DLLEXPORT CHOP_CPlusPlusBase* CreateCHOPInstance(const OP_NodeInfo* info) {
  return new ClimbingTimerCHOP(info);
}

DLLEXPORT void DestroyCHOPInstance(CHOP_CPlusPlusBase* instance) {
  delete (ClimbingTimerCHOP*)instance;
}
};
