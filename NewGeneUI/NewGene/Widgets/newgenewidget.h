#ifndef NEWGENEWIDGET_H
#define NEWGENEWIDGET_H

#include "globals.h"
#include "uiprojectmanager.h"
#include "../../../NewGeneBackEnd/UIData/DataWidgets.h"

class QWidget;
class NewGeneMainWindow;
class UIInputProject;
class UIOutputProject;

class WidgetCreationInfo;

class NewGeneWidget
{

	public:

		enum WIDGET_NATURE
		{
			  WIDGET_NATURE_UNKNOWN
			, WIDGET_NATURE_GENERAL
			, WIDGET_NATURE_INPUT_WIDGET
			, WIDGET_NATURE_OUTPUT_WIDGET
		};

	public:
		NewGeneWidget( WidgetCreationInfo const & creation_info );

		virtual ~NewGeneWidget();

		NewGeneMainWindow & mainWindow();

		bool IsTopLevel() { return top_level; }

		virtual void PrepareInputWidget();
		virtual void PrepareOutputWidget();
		virtual void HandleChanges(DataChangeMessage const &) {}

		// Be sure to call this only in the context of the UI thread
		void ShowMessageBox(std::string msg);

	protected:

		// ****************************************************************************************************************************
		// Pseudo-slots.
		//     There will be a compile-time error if the following functions are not overridden in every widget
		//     that calls PrepareInputWidget() or PrepareOutputWidget() during construction.
		virtual void UpdateInputConnections(UIProjectManager::UPDATE_CONNECTIONS_TYPE connection_type, UIInputProject * project);
		virtual void UpdateOutputConnections(UIProjectManager::UPDATE_CONNECTIONS_TYPE connection_type, UIOutputProject * project);
		virtual void RefreshAllWidgets() {};
		//
	public:
		//
		virtual void WidgetDataRefreshReceive(WidgetDataItem_VARIABLE_GROUPS_SCROLL_AREA) {};
		virtual void WidgetDataRefreshReceive(WidgetDataItem_VARIABLE_GROUPS_TOOLBOX) {};
		virtual void WidgetDataRefreshReceive(WidgetDataItem_VARIABLE_GROUP_VARIABLE_GROUP_INSTANCE) {};
		virtual void WidgetDataRefreshReceive(WidgetDataItem_VARIABLE_GROUPS_SUMMARY) {};
		virtual void WidgetDataRefreshReceive(WidgetDataItem_VARIABLE_GROUPS_SUMMARY_VARIABLE_GROUP_INSTANCE) {};
		// ****************************************************************************************************************************

	protected:

		// ****************************************************************************************************************************
		// Pseudo-signals.
		//     There will be a compile-time error if the following functions are not given a declaration in every widget
		//     that calls PrepareInputWidget() or PrepareOutputWidget() during construction.
		virtual void RefreshWidget(WidgetDataItemRequest_VARIABLE_GROUPS_SCROLL_AREA) {};
		virtual void RefreshWidget(WidgetDataItemRequest_VARIABLE_GROUPS_TOOLBOX) {};
		virtual void RefreshWidget(WidgetDataItem_VARIABLE_GROUP_VARIABLE_GROUP_INSTANCE) {};
		virtual void RefreshWidget(WidgetDataItemRequest_VARIABLE_GROUPS_SUMMARY) {};
		virtual void RefreshWidget(WidgetDataItem_VARIABLE_GROUPS_SUMMARY_VARIABLE_GROUP_INSTANCE) {};
		// ****************************************************************************************************************************


	public:

		QWidget * self;
		WIDGET_NATURE widget_nature;
		UUID uuid;
		UIInputProject * inp;
		UIOutputProject * outp;
		DATA_WIDGETS widget_type;
		WidgetInstanceIdentifier data_instance;
		bool top_level;
		static NewGeneMainWindow * theMainWindow;

	public:

		bool IsInputProjectWidget() const;
		bool IsOutputProjectWidget() const;

};

class WidgetCreationInfo
{
	public:
		WidgetCreationInfo(
							   QWidget * const self_,
							   NewGeneWidget::WIDGET_NATURE const widget_nature_ = NewGeneWidget::WIDGET_NATURE::WIDGET_NATURE_UNKNOWN,
							   DATA_WIDGETS const widget_type_ = WIDGET_TYPE_NONE,
							   bool const top_level_ = false,
							   WidgetInstanceIdentifier data_instance_ = WidgetInstanceIdentifier()
						   )
			: self(self_)
			, widget_nature(widget_nature_)
			, widget_type(widget_type_)
			, top_level(top_level_)
			, data_instance(data_instance_)
		{

		}

		QWidget * self;
		NewGeneWidget::WIDGET_NATURE widget_nature;
		DATA_WIDGETS widget_type;
		bool top_level;
		WidgetInstanceIdentifier data_instance;
};

#endif // NEWGENEWIDGET_H
