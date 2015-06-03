#include "splash.h"
#include "ui_splash.h"
#include <QCloseEvent>
#include <QDesktopServices>
#include <QMovie>
#include <QWebEngineSettings>
#include "newgenewebengineview.h"

Splash::Splash(QWidget * parent, NewGeneMainWindow * mainWindow_, bool const opened_as_about_box_) :
	QWidget{parent},
	mainWindow{mainWindow_},
	closed_via_click{false},
	opened_as_about_box{opened_as_about_box_},
	ui{new Ui::Splash}
{
	ui->setupUi(this);
	//QTimer::singleShot(10, mainWindow, SLOT(doDisable()));
	//QWebSettings::globalSettings()->setAttribute(QWebSettings::PluginsEnabled, true);
	//ui->webView->settings()->setAttribute(QWebSettings::PluginsEnabled, true);
	//ui->webView->page()->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);
	//ui->webView->setVisible(false);
	//QMovie * movie = new QMovie(":/spinner.gif");
	//ui->label->setMovie(movie);
	//movie->start();
	//connect(ui->webView, SIGNAL(linkClicked(const QUrl &)), this, SLOT(receiveLinkClicked(const QUrl &)));
	//connect(ui->webView, SIGNAL(loadFinished(bool)), this, SLOT(receiveLoadFinished(bool)));

	QWebEngineView * webView = new NewGeneWebEngineView(this);
	ui->verticalLayoutWeb->addWidget(webView);
	//webView->page()->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);
	//ui->webView->setVisible(false);
	//QMovie * movie = new QMovie(":/spinner.gif");
	//ui->label->setMovie(movie);
	//movie->start();
	webView->page()->settings()->setAttribute(QWebEngineSettings::JavascriptEnabled, true);
	webView->page()->settings()->setAttribute(QWebEngineSettings::JavascriptCanOpenWindows, true);
	webView->page()->settings()->setAttribute(QWebEngineSettings::LocalContentCanAccessRemoteUrls, true);
	//connect(webView, SIGNAL(linkClicked(const QUrl &)), this, SLOT(receiveLinkClicked(const QUrl &)));
	//connect(webView, SIGNAL(urlChanged(const QUrl &)), this, SLOT(receiveLinkClicked(const QUrl &)));
	//connect(ui->webView, SIGNAL(loadFinished(bool)), this, SLOT(receiveLoadFinished(bool)));
	webView->setUrl(QUrl("https://d1ce36c5f50a052d14358de742b69156b6345b37-www.googledrive.com/host/0B0q-yvic3PFIfjBBUUxObVIwSmMtdE9EaDR5YS03cXRHem5KQXZHNTFncHdjNU4tdWVTd3c/splash.html"));
	webView->show();
}

Splash::~Splash()
{
	delete ui;
}

void Splash::receiveLinkClicked(const QUrl & url)
{
	//QMessageBox::information(this, "Title", "Something");
	QDesktopServices::openUrl(url);
	//closeMyself();
}

void Splash::receiveLoadFinished(bool)
{
	//ui->label->setVisible(false);
	//ui->webView->setVisible(true);
	//ui->webView->setEnabled(true);
}

bool Splash::eventFilter(QObject * obj, QEvent * event)
{
	bool ret = QWidget::eventFilter(obj, event);
	return ret;
}

void Splash::closeEvent(QCloseEvent * event)
{
	if (!closed_via_click)
	{
		closeAndRefreshSequence();
	}

	event->accept();
}

void Splash::closeAndRefreshSequence()
{
	// Deal with visual artifacts - I'm not sure why the window needs to be repainted,
	// but repainting to remove artifacts after the splash screen has been removed
	// only works after the splash screen has been deleted, and there is a safety delay
	// prior to deleting it.
	QTimer::singleShot(100, mainWindow, SLOT(show()));
	QTimer::singleShot(0, this, SLOT(deleteMe()));
	QTimer::singleShot(100, mainWindow, SLOT(doEnable()));
	QTimer::singleShot(1000, mainWindow, SLOT(update()));
}

void Splash::deleteMe()
{
	deleteLater();
}

void Splash::on_pushButton_clicked()
{
	closeMyself();
}

void Splash::closeMyself()
{
	QTimer::singleShot(10, this, SLOT(close()));
	QTimer::singleShot(1000, this, SLOT(closeAndRefreshSequence()));
}
