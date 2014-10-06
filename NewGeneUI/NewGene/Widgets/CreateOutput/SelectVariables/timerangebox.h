#ifndef TIMERANGEBOX_H
#define TIMERANGEBOX_H

#include <QFrame>
#include "../../newgenewidget.h"

namespace Ui
{
	class TimeRangeBox;
}

class TimeRangeBox : public QFrame, public NewGeneWidget // do not reorder base classes; QWidget instance must be instantiated first
{

		Q_OBJECT

	public:

		explicit TimeRangeBox( QWidget * parent = 0 );
		~TimeRangeBox();

	protected:

		void changeEvent( QEvent * e );

	private:

		Ui::TimeRangeBox * ui;

	public:

	signals:
		void UpdateDoKadSampler(WidgetActionItemRequest_ACTION_DO_RANDOM_SAMPLING_CHANGE);
		void UpdateKadSamplerCount(WidgetActionItemRequest_ACTION_RANDOM_SAMPLING_COUNT_PER_STAGE_CHANGE);
		void UpdateConsolidateRows(WidgetActionItemRequest_ACTION_CONSOLIDATE_ROWS_CHANGE);
		void UpdateDisplayAbsoluteTimeColumns(WidgetActionItemRequest_ACTION_DISPLAY_ABSOLUTE_TIME_COLUMNS_CHANGE);
		void RefreshWidget(WidgetDataItemRequest_TIMERANGE_REGION_WIDGET);

	public slots:
		void UpdateOutputConnections(NewGeneWidget::UPDATE_CONNECTIONS_TYPE connection_type, UIOutputProject * project);
		void UpdateInputConnections(NewGeneWidget::UPDATE_CONNECTIONS_TYPE connection_type, UIInputProject * project);
		void RefreshAllWidgets();
		void WidgetDataRefreshReceive(WidgetDataItem_TIMERANGE_REGION_WIDGET);

	private slots:
		void on_doKadSampler_stateChanged(int arg1);
		void on_randomSamplingHowManyRows_textChanged(const QString &arg1);
		void on_dateTimeEdit_start_editingFinished();
		void on_mergeIdenticalRows_stateChanged(int arg1);
        void on_displayAbsoluteTimeColumns_stateChanged(int arg1);
};

#endif // TIMERANGEBOX_H
