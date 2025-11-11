#pragma once
#include "SettingsBase.h"
#include <string.h>
#include <vector>
#include <stdio.h>
#include <DiskDrive.h>
#include <SystemFlash.h>
#include "FlashTranslator2.h"

struct ZaboyInfo
{
	char name[50];
};

class Settings : public SettingsBase
{
public:
	Settings();

	virtual void Save();
	virtual void Load();
	virtual void Default();
	void Init();
	void format();
	/*DATA BEGIN*/
	std::vector<ZaboyInfo> zaboy_list;
	float q_scale;
	float q_offset;
	float t_scale;
	float t_offset;
	/*DATA END*/
private:

	SystemFlash<0x08040000, 100, 512, 2048> _flash;
	FlashTranslator2<4> _setup_fs;
};


