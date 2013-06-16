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
		explicit NewGeneWidget( QWidget * self_ = 0 );

		NewGeneMainWindow & mainWindow();

		virtual void PrepareInputWidget();
		virtual void PrepareOutputWidget();

	protected:

		// ****************************************************************************************************************************
		// Pseudo-slots.
		//     There will be a compile-time error if the following functions are not overridden in every widget
		//     that calls PrepareInputWidget() or PrepareOutputWidget() during construction.
		virtual void UpdateInputConnections(UIProjectManager::UPDATE_CONNECTIONS_TYPE connection_type, UIInputProject * project);
		virtual void UpdateOutputConnections(UIProjectManager::UPDATE_CONNECTIONS_TYPE connection_type, UIOutputProject * project);
		// ****************************************************************************************************************************


		// ****************************************************************************************************************************
		// Pseudo-signals.
		//     There will be a compile-time error if the following functions are not given a declaration in every widget
		//     that calls PrepareInputWidget() or PrepareOutputWidget() during construction.
		virtual void RefreshWidget(DATA_WIDGETS) {};
		// ****************************************************************************************************************************


	public:

		QWidget * self;
		UIInputProject * inp;
		UIOutputProject * outp;
		static NewGeneMainWindow * theMainWindow;

};

#endif // NEWGENEWIDGET_H
