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

class Splash;

class NewGeneMainWindow : public QMainWindow, public NewGeneWidget // do not reorder base classes; QWidget instance must be instantiated first
{
		Q_OBJECT

	public:
		explicit NewGeneMainWindow(QWidget * parent = 0);
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

		void SetTitle();
		void displaySplash(bool const);

	public slots:
		void displaySplashOpening();
		void displaySplashAbout();
		void doInitialize();
		void doDisable();
		void doEnable();
		void SignalMessageBox(STD_STRING);
		void ReceiveSignalStartProgressBar(int, STD_INT64 const, STD_INT64 const);
		void ReceiveSignalStopProgressBar(int);
		void ReceiveSignalUpdateProgressBarValue(int, STD_INT64 const);
		void ReceiveSignalUpdateStatusBarText(int, STD_STRING const);
		void UpdateInputConnections(NewGeneWidget::UPDATE_CONNECTIONS_TYPE connection_type, UIInputProject * project);
		void UpdateOutputConnections(NewGeneWidget::UPDATE_CONNECTIONS_TYPE connection_type, UIOutputProject * project);

		void Run();

	protected:
		void changeEvent(QEvent * e);
		void closeEvent(QCloseEvent * event);
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

		void on_actionAbout_NewGene_triggered();

	private:
		Ui::NewGeneMainWindow * ui;

		friend class NewGeneWidget;

		std::map<int, std::unique_ptr<QProgressBar>> status_bar_progress_bars;
		std::map<int, std::unique_ptr<QProgressBar>> main_pane_progress_bars;

	private:
		Splash * theSplash;

	public:
		bool newInputDataset; // kluge to indicate whether the "missing model settings" or "missing db file" message prompt should appear
		bool newOutputDataset; // kluge to indicate whether the "missing model settings" or "missing db file" message prompt should appear

};

#endif // NEWGENEMAINWINDOW_H
