#include "splash.h"
#include "ui_splash.h"
#include <QCloseEvent>
#include <QDesktopServices>
#include <QMovie>
#include <QWebEngineSettings>
#include "newgenewebengineview.h"

Splash::Splash(QWidget * parent, NewGeneMainWindow * mainWindow_, bool const opened_as_about_box_) :
	QDialog{parent},
	mainWindow{mainWindow_},
	closed_via_click{false},
	opened_as_about_box{opened_as_about_box_},
	webView{nullptr},
	ui{new Ui::Splash}
{
	this->setAttribute(Qt::WA_DeleteOnClose, true);
	ui->setupUi(this);
	QTimer::singleShot(0, mainWindow, SLOT(hide()));
	QTimer::singleShot(10, this, SLOT(execMyself()));
}

Splash::~Splash()
{
	delete ui;
}

void Splash::execMyself()
{
	showMyself();
	exec();
}

void Splash::showEvent(QShowEvent* event)
{
	QDialog::showEvent(event);
	this->setFocus();
}

void Splash::showMyself()
{
	webView = new NewGeneWebEngineView(this);
	ui->verticalLayoutWeb->addWidget(webView);

	webView->setVisible(false);

	QMovie * movie = new QMovie(":/ajax-loader.gif");
	ui->labelSpinner->setMovie(movie);
	movie->start();

	// So that links that open external pages trigger the 'CreateWindow' function in our derived NewGeneWebEngineView class
	webView->page()->settings()->setAttribute(QWebEngineSettings::JavascriptEnabled, true);
	webView->page()->settings()->setAttribute(QWebEngineSettings::JavascriptCanOpenWindows, true);
	webView->page()->settings()->setAttribute(QWebEngineSettings::LocalContentCanAccessRemoteUrls, true);

	connect(webView, SIGNAL(loadFinished(bool)), this, SLOT(receiveLoadFinished(bool)));
	webView->setUrl(QUrl("https://d1ce36c5f50a052d14358de742b69156b6345b37-www.googledrive.com/host/0B0q-yvic3PFIfjBBUUxObVIwSmMtdE9EaDR5YS03cXRHem5KQXZHNTFncHdjNU4tdWVTd3c/splash.html"));
	webView->show();
}

void Splash::receiveLoadFinished(bool)
{
	ui->labelSpinner->setVisible(false);
	ui->labelLatest->setVisible(false);
	webView->setVisible(true);
	webView->setEnabled(true);
}

bool Splash::eventFilter(QObject * obj, QEvent * event)
{
	bool ret = QWidget::eventFilter(obj, event);
	return ret;
}

void Splash::closeEvent(QCloseEvent * event)
{
	event->accept();
}

void Splash::closeAndRefreshSequence()
{
	// Deal with visual artifacts - I'm not sure why the window needs to be repainted,
	// but repainting to remove artifacts after the splash screen has been removed
	// only works after the splash screen has been deleted, and there is a safety delay
	// prior to deleting it.
	this->done(0);
	QTimer::singleShot(100, mainWindow, SLOT(Run()));
}

void Splash::deleteMe()
{
}

void Splash::on_pushButton_clicked()
{
	closeMyself();
}

void Splash::closeMyself()
{
	QTimer::singleShot(0, this, SLOT(closeAndRefreshSequence()));
}
