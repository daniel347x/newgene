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

	public slots:

		void UpdateOutputConnections(UIProjectManager::UPDATE_CONNECTIONS_TYPE connection_type, UIOutputProject * project);
		void on_pushButtonGenerateOutput_clicked();

};

#endif // NEWGENEGENERATEOUTPUT_H
