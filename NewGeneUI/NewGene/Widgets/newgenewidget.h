#ifndef NEWGENEWIDGET_H
#define NEWGENEWIDGET_H

#include "globals.h"
#include "uiprojectmanager.h"
#include "../../../NewGeneBackEnd/UIData/DataWidgets.h"

class QWidget;
class NewGeneMainWindow;
class UIInputProject;
class UIOutputProject;

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
		explicit NewGeneWidget( WIDGET_NATURE const widget_nature_, QWidget * self_ = 0 );

		NewGeneMainWindow & mainWindow();

		virtual void PrepareInputWidget(DATA_WIDGETS widget_type_);
		virtual void PrepareOutputWidget(DATA_WIDGETS widget_type_);

	protected:

		// ****************************************************************************************************************************
		// Pseudo-slots.
		//     There will be a compile-time error if the following functions are not overridden in every widget
		//     that calls PrepareInputWidget() or PrepareOutputWidget() during construction.
		virtual void UpdateInputConnections(UIProjectManager::UPDATE_CONNECTIONS_TYPE connection_type, UIInputProject * project);
		virtual void UpdateOutputConnections(UIProjectManager::UPDATE_CONNECTIONS_TYPE connection_type, UIOutputProject * project);
		virtual void RefreshAllWidgets() {};
		virtual void WidgetDataRefreshReceive(WidgetDataItem_VARIABLE_GROUPS_SCROLL_AREA) {};
		virtual void WidgetDataRefreshReceive(WidgetDataItem_VARIABLE_GROUPS_TOOLBOX) {};
		// ****************************************************************************************************************************


		// ****************************************************************************************************************************
		// Pseudo-signals.
		//     There will be a compile-time error if the following functions are not given a declaration in every widget
		//     that calls PrepareInputWidget() or PrepareOutputWidget() during construction.
		virtual void RefreshWidget(DATA_WIDGETS) {};
		// ****************************************************************************************************************************


	public:

		QWidget * self;
		WIDGET_NATURE widget_nature;
		UUID uuid;
		UIInputProject * inp;
		UIOutputProject * outp;
		DATA_WIDGETS widget_type;
		static NewGeneMainWindow * theMainWindow;

};

#endif // NEWGENEWIDGET_H
