#pragma once
class SettingsBase
{
public:
	virtual void Save() = 0;
	virtual void Load() = 0;
	virtual void Default() = 0;
};