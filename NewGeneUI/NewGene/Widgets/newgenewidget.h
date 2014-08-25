#ifndef NEWGENEWIDGET_H
#define NEWGENEWIDGET_H

#include "globals.h"
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

		enum UPDATE_CONNECTIONS_TYPE
		{
			  RELEASE_CONNECTIONS_INPUT_PROJECT
			, ESTABLISH_CONNECTIONS_INPUT_PROJECT
			, RELEASE_CONNECTIONS_OUTPUT_PROJECT
			, ESTABLISH_CONNECTIONS_OUTPUT_PROJECT
		};

	public:

		NewGeneWidget( WidgetCreationInfo const & creation_info );

		virtual ~NewGeneWidget();

		NewGeneMainWindow & mainWindow();

		bool IsTopLevel() { return top_level; }

		virtual void PrepareInputWidget(bool const also_link_output = true);
		virtual void PrepareOutputWidget();
		virtual void HandleChanges(DataChangeMessage const &) {}

		// Be sure to call this only in the context of the UI thread
		void ShowMessageBox(std::string msg);

	protected slots:

		// ****************************************************************************************************************************
		// Pseudo-slots.
		virtual void UpdateInputConnections(NewGeneWidget::UPDATE_CONNECTIONS_TYPE connection_type, UIInputProject * project);
		virtual void UpdateOutputConnections(NewGeneWidget::UPDATE_CONNECTIONS_TYPE connection_type, UIOutputProject * project);
		//
		/* Remove if unnecessary
		virtual void RefreshAllWidgets() {}
		*/
		//
	public:
		//
		// The following base class functions are not necessary.
		// However, they do assist in assuring that derived classes use the same naming scheme.
		// At some point, errors could be added to these otherwise empty functions to further
		// encourage the proper naming scheme in derived classes.
		//
		// Output-project related refreshes
		virtual void WidgetDataRefreshReceive(WidgetDataItem_VARIABLE_GROUPS_SCROLL_AREA) {}
		virtual void WidgetDataRefreshReceive(WidgetDataItem_VARIABLE_GROUPS_TOOLBOX) {}
		virtual void WidgetDataRefreshReceive(WidgetDataItem_VARIABLE_GROUP_VARIABLE_GROUP_INSTANCE) {}
		virtual void WidgetDataRefreshReceive(WidgetDataItem_VARIABLE_GROUPS_SUMMARY_SCROLL_AREA) {}
		virtual void WidgetDataRefreshReceive(WidgetDataItem_VARIABLE_GROUPS_SUMMARY_VARIABLE_GROUP_INSTANCE) {}
		virtual void WidgetDataRefreshReceive(WidgetDataItem_KAD_SPIN_CONTROLS_AREA) {}
		virtual void WidgetDataRefreshReceive(WidgetDataItem_KAD_SPIN_CONTROL_WIDGET) {}
		virtual void WidgetDataRefreshReceive(WidgetDataItem_TIMERANGE_REGION_WIDGET) {}
		virtual void WidgetDataRefreshReceive(WidgetDataItem_DATETIME_WIDGET) {}
		virtual void WidgetDataRefreshReceive(WidgetDataItem_GENERATE_OUTPUT_TAB) {}
		virtual void WidgetDataRefreshReceive(WidgetDataItem_LIMIT_DMUS_TAB) {}
		//
		// Input-project related refreshes
		virtual void WidgetDataRefreshReceive(WidgetDataItem_MANAGE_DMUS_WIDGET) {}
		virtual void WidgetDataRefreshReceive(WidgetDataItem_MANAGE_UOAS_WIDGET) {}
		virtual void WidgetDataRefreshReceive(WidgetDataItem_MANAGE_VGS_WIDGET) {}
		// ****************************************************************************************************************************

	protected:

		// ****************************************************************************************************************************
		// Pseudo-signals.
		// The following base class functions are not necessary.
		// However, they do assist in assuring that derived classes use the same naming scheme.
		// At some point, errors could be added to these otherwise empty functions to further
		// encourage the proper naming scheme in derived classes.
		//
		// Output-project related refreshes
		virtual void RefreshWidget(WidgetDataItemRequest_VARIABLE_GROUPS_SCROLL_AREA) {}
		virtual void RefreshWidget(WidgetDataItemRequest_VARIABLE_GROUPS_TOOLBOX) {}
		virtual void RefreshWidget(WidgetDataItemRequest_VARIABLE_GROUP_VARIABLE_GROUP_INSTANCE) {}
		virtual void RefreshWidget(WidgetDataItemRequest_VARIABLE_GROUPS_SUMMARY_SCROLL_AREA) {}
		virtual void RefreshWidget(WidgetDataItemRequest_VARIABLE_GROUPS_SUMMARY_VARIABLE_GROUP_INSTANCE) {}
		virtual void RefreshWidget(WidgetDataItemRequest_KAD_SPIN_CONTROLS_AREA) {}
		virtual void RefreshWidget(WidgetDataItemRequest_KAD_SPIN_CONTROL_WIDGET) {}
		virtual void RefreshWidget(WidgetDataItemRequest_TIMERANGE_REGION_WIDGET) {}
		virtual void RefreshWidget(WidgetDataItemRequest_DATETIME_WIDGET) {}
		virtual void RefreshWidget(WidgetDataItemRequest_GENERATE_OUTPUT_TAB) {}
		virtual void RefreshWidget(WidgetDataItemRequest_LIMIT_DMUS_TAB) {}
		//
		// Input-project related refreshes
		virtual void RefreshWidget(WidgetDataItemRequest_MANAGE_DMUS_WIDGET) {}
		virtual void RefreshWidget(WidgetDataItemRequest_MANAGE_UOAS_WIDGET) {}
		virtual void RefreshWidget(WidgetDataItemRequest_MANAGE_VGS_WIDGET) {}
		// ****************************************************************************************************************************


	public:

		QWidget * self;
		WIDGET_NATURE widget_nature;
		UUID uuid;
		UUID uuid_parent;
		UIInputProject * inp;
		UIOutputProject * outp;
		DATA_WIDGETS widget_type;
		WidgetInstanceIdentifier data_instance;
		bool top_level;
		static NewGeneMainWindow * theMainWindow;

	public:

		bool IsInputProjectWidget() const;
		bool IsOutputProjectWidget() const;

	protected:

		virtual void Empty() {}

    protected:

        bool inputConnected;
        bool outputConnected;

};

class WidgetCreationInfo
{

	public:

		WidgetCreationInfo(
							   QWidget * const self_,
							   NewGeneWidget::WIDGET_NATURE const widget_nature_ = NewGeneWidget::WIDGET_NATURE::WIDGET_NATURE_UNKNOWN,
							   DATA_WIDGETS const widget_type_ = WIDGET_TYPE_NONE,
							   UUID const & uuid_parent_ = "",
							   bool const top_level_ = false,
							   WidgetInstanceIdentifier data_instance_ = WidgetInstanceIdentifier()
						   )
			: self(self_)
			, widget_nature(widget_nature_)
			, widget_type(widget_type_)
			, uuid_parent(uuid_parent_)
			, top_level(top_level_)
			, data_instance(data_instance_)
		{

		}

		WidgetCreationInfo(
							   QWidget * const self_,
							   QWidget * parent,
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
			if (parent)
			{
				try
				{
					NewGeneWidget * newgene_parent_widget = dynamic_cast<NewGeneWidget*>(parent);
					if (newgene_parent_widget)
					{
						uuid_parent = newgene_parent_widget->uuid;
					}
				}
				catch (std::bad_cast & bc)
				{
					// nothing to do
				}
			}
		}

		QWidget * self;
		NewGeneWidget::WIDGET_NATURE widget_nature;
		DATA_WIDGETS widget_type;
		UUID uuid_parent;
		bool top_level; // Is this widget class tied into a UI Form, or is it created dynamically in code via "new" from some other widget?
		WidgetInstanceIdentifier data_instance;

};

#include "../q_declare_metatype.h"

#endif // NEWGENEWIDGET_H
