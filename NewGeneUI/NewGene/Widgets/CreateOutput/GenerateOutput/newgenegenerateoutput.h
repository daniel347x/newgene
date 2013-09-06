#ifndef NEWGENEGENERATEOUTPUT_H
#define NEWGENEGENERATEOUTPUT_H

#include "..\..\..\newgenewidget.h"
#include <QWidget>

namespace Ui {
	class NewGeneGenerateOutput;
}

class NewGeneGenerateOutput : public QWidget, public NewGeneWidget
{

		Q_OBJECT

	public:

		explicit NewGeneGenerateOutput(QWidget *parent = 0);
		~NewGeneGenerateOutput();

	private:

		Ui::NewGeneGenerateOutput *ui;

	public:

	signals:

		void GenerateOutputSignal(WidgetActionItemRequest_ACTION_GENERATE_OUTPUT);
		void RefreshWidget(WidgetDataItemRequest_GENERATE_OUTPUT_TAB);

	public slots:

		void UpdateOutputConnections(UIProjectManager::UPDATE_CONNECTIONS_TYPE connection_type, UIOutputProject * project);
		void on_pushButtonGenerateOutput_clicked();
		void WidgetDataRefreshReceive(WidgetDataItem_GENERATE_OUTPUT_TAB);
		void RefreshAllWidgets();
		void ReceiveSignalAppendKadStatusText(int, STD_STRING const);
		void ReceiveSignalSetPerformanceLabel(int, STD_STRING const);
		void on_pushButton_clicked();

	private slots:
		void on_lineEditFilePathToKadOutput_lostFocus();
		void on_lineEditFilePathToKadOutput_editingFinished();
};

#endif // NEWGENEGENERATEOUTPUT_H
