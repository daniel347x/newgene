#ifndef SETTING_H
#define SETTING_H

#ifndef Q_MOC_RUN
#	include <boost/filesystem.hpp>
#endif

class Setting
{
	public:
};

class GlobalSetting : virtual public Setting
{
	public:
};

class ProjectSetting : virtual public Setting
{
	public:
};

class BackendSetting : virtual public Setting
{
	public:
};

class BackendGlobalSetting : public GlobalSetting, public BackendSetting
{
	public:
};

class BackendProjectSetting : public ProjectSetting, public BackendSetting
{
	public:
};

#endif
