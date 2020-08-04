/* Shared Use License: This file is owned by Derivative Inc. (Derivative)
* and can only be used, and/or modified for use, in conjunction with
* Derivative's TouchDesigner software, and only if you are a licensee who has
* accepted Derivative's TouchDesigner license or assignment agreement
* (which also govern the use of this file). You may share or redistribute
* a modified version of this file provided the following conditions are met:
*
* 1. The shared file or redistribution must retain the information set out
* above and this list of conditions.
* 2. Derivative's name (Derivative Inc.) or its trademarks may not be used
* to endorse or promote products derived from this file without specific
* prior written permission from Derivative.
*/

#include "CPlusPlus_Common.h"
#include "CHOP_CPlusPlusBase.h"

#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include <thread>
#include <chrono>

#include "Serial.hpp"

using namespace std;

class ClimbingTimerCHOP : public CHOP_CPlusPlusBase
{
public:
	std::string portname = "";

	std::vector<std::string> chanNames{ "minute", "second" };
	std::vector<double> chanValues{ 0.0f, 0.0f };

	std::thread recv_thread;
	bool running;

	Serial serial;

	ClimbingTimerCHOP(const OP_NodeInfo* info)
	{
		this->running = false;
	}

	virtual ~ClimbingTimerCHOP()
	{
		this->stop();
		this->close();
	}

	bool open()
	{
		// open error
		if (!this->serial.Open(this->portname)) {
			return false;
		}

		cout << "open success" << endl;

		return true;
	}

	void start()
	{
		std::cout << "thread start" << std::endl;

		recv_thread = std::thread([this]() {
			this->loop();
		});
	}

	void loop()
	{
		this->running = true;

		char data[9] = { 0 };
		int pos = 0;

		while (this->running)
		{

			auto vals = this->serial.Read();
			for (auto c : vals) {
				if (c == 0x46) { // F
					pos = 0;
				}
				data[pos++] = c;
				if (pos == 8) {
					this->createParams(data);
				}
			}
		}
	}

	void createParams(char* data)
	{
		std::string time_str(data, 8);

		this->chanValues[0] = std::atof(time_str.substr(3, 2).c_str());
		this->chanValues[1] = std::atof(time_str.substr(5, 2).c_str());
	}

	void stop()
	{
		std::cout << "thread stop" << std::endl;

		this->running = false;
		if (recv_thread.joinable())
			recv_thread.join();
	}

	void close()
	{
		this->serial.Close();
	}

	void getGeneralInfo(CHOP_GeneralInfo* ginfo, const OP_Inputs* inputs, void* reserved1)
	{
		ginfo->cookEveryFrameIfAsked = true;
		ginfo->timeslice = false;
		ginfo->inputMatchIndex = 0;
	}

	bool getOutputInfo(CHOP_OutputInfo* info, const OP_Inputs* inputs, void* reserved1)
	{
		info->numSamples = 1;
		info->numChannels = (int32_t)this->chanNames.size();
		return true;
	}

	void getChannelName(int32_t index, OP_String *name, const OP_Inputs* inputs, void* reserved1)
	{
		name->setString(this->chanNames.at(index).c_str());
	}

	void execute(CHOP_Output* output, const OP_Inputs* inputs, void* reserved)
	{
		std::string name = inputs->getParString("Portname");

		if (this->portname != name) {
			this->stop();
			this->close();
		}
		this->portname = name;

		if (!this->portname.size())
			return;

		if (!this->running) {
			if (!this->open())
				return;
			this->start();
		}

		for (int i = 0; i < this->chanNames.size(); i++) {
			for (int j = 0; j < output->numSamples; j++) {
				output->channels[i][j] = (float)this->chanValues.at(i);
			}
		}
	}

	void setupParameters(OP_ParameterManager* manager, void *reserved1)
	{
		{
			OP_StringParameter	sp;
			sp.name = "Portname";
			sp.label = "Port Name";
			sp.page = "Config";
			OP_ParAppendResult res = manager->appendString(sp);
			assert(res == OP_ParAppendResult::Success);
		}
	}
};

extern "C"
{
	DLLEXPORT void FillCHOPPluginInfo(CHOP_PluginInfo* info)
	{
		info->apiVersion = CHOPCPlusPlusAPIVersion;
		info->customOPInfo.opType->setString("Climbingtimer");
		info->customOPInfo.opLabel->setString("Climbing Timer");
		info->customOPInfo.authorName->setString("Akira Kamikura");
		info->customOPInfo.authorEmail->setString("akira.kamikura@gmail.com");
	}

	DLLEXPORT CHOP_CPlusPlusBase* CreateCHOPInstance(const OP_NodeInfo* info)
	{
		return new ClimbingTimerCHOP(info);
	}

	DLLEXPORT void DestroyCHOPInstance(CHOP_CPlusPlusBase* instance)
	{
		delete (ClimbingTimerCHOP*)instance;
	}

};
