#ifndef NEWGENEWEBENGINEVIEW
#define NEWGENEWEBENGINEVIEW

#include <QWebEngineView>
#include <QDesktopServices>

class NewGeneWebEngineView : public QWebEngineView
{
	Q_OBJECT
public:
	NewGeneWebEngineView(QWidget * parent) : QWebEngineView(parent) {}
protected:
	QWebEngineView * createWindow(QWebEnginePage::WebWindowType type)
	{
		QWebEngineView * webView = new QWebEngineView();
		connect(webView, SIGNAL(urlChanged(const QUrl &)), this, SLOT(receiveLinkClicked(const QUrl &)));
		return webView;
	}
private slots:
	void receiveLinkClicked(const QUrl & url)
	{
		QDesktopServices::openUrl(url);
	}
};

#endif // NEWGENEWEBENGINEVIEW
