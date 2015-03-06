#ifndef NEWGENEMAINWINDOW_H
#define NEWGENEMAINWINDOW_H

#include "globals.h"
#include <QMainWindow>
#include <QProgressBar>
#include <QStatusBar>
#include "newgenewidget.h"
#include "../Infrastructure/Messager/uimessager.h"

#include <memory>
#include <map>

namespace Ui
{
	class NewGeneMainWindow;
}

enum NEWGENE_ACTIONS
{
    NEWGENE_ACTION__CLOSE_INPUT_DATASET
    , NEWGENE_ACTION__CLOSE_OUTPUT_DATASET
};

class NewGeneMainWindow : public QMainWindow, public NewGeneWidget // do not reorder base classes; QWidget instance must be instantiated first
{
		Q_OBJECT

	public:
		explicit NewGeneMainWindow( QWidget * parent = 0 );
		~NewGeneMainWindow();

		UIMessager messager;

	signals:
		void SignalOpenOutputDataset(STD_STRING, QObject *);
		void SignalCloseCurrentOutputDataset();
		void SignalOpenInputDataset(STD_STRING, QObject *);
		void SignalCloseCurrentInputDataset();
		void SignalSaveCurrentInputDatasetAs(STD_STRING, QObject *);
		void SignalSaveCurrentOutputDatasetAs(STD_STRING, QObject *);

	public:

		void SetInputDatasetText();
		void SetOutputDatasetText();
        void SetTitle();
        void EnableAction(NEWGENE_ACTIONS const theAction, bool const enable = true);

	public slots:
		void doInitialize();
		void SignalMessageBox(STD_STRING);
		void ReceiveSignalStartProgressBar(int, STD_INT64 const, STD_INT64 const);
		void ReceiveSignalStopProgressBar(int);
		void ReceiveSignalUpdateProgressBarValue(int, STD_INT64 const);
		void ReceiveSignalUpdateStatusBarText(int, STD_STRING const);
		void UpdateInputConnections(NewGeneWidget::UPDATE_CONNECTIONS_TYPE connection_type, UIInputProject * project);
		void UpdateOutputConnections(NewGeneWidget::UPDATE_CONNECTIONS_TYPE connection_type, UIOutputProject * project);

	protected:
		void changeEvent( QEvent * e );
		void closeEvent(QCloseEvent *event);
        void PrepareGlobalConnections();

	private slots:
		void on_actionOpen_Input_Dataset_triggered();
		void on_actionOpen_Output_Dataset_triggered();
		void on_actionClose_Current_Input_Dataset_triggered();
		void on_actionClose_Current_Output_Dataset_triggered();
		void on_actionNew_Input_Dataset_triggered();
		void on_actionNew_Output_Dataset_triggered();
		void on_actionSave_Input_Dataset_As_triggered();
		void on_actionSave_Output_Dataset_As_triggered();
        void on_actionDisplay_input_dataset_path_triggered();
        void on_actionDisplay_output_dataset_path_triggered();

private:
		Ui::NewGeneMainWindow * ui;

		friend class NewGeneWidget;

		std::map<int, std::unique_ptr<QProgressBar>> status_bar_progress_bars;
		std::map<int, std::unique_ptr<QProgressBar>> main_pane_progress_bars;

};

#endif // NEWGENEMAINWINDOW_H
