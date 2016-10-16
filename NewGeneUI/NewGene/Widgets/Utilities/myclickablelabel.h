#ifndef MYCLICKABLELABEL_H
#define MYCLICKABLELABEL_H

#include <QLabel>

class MyClickableLabel : public QLabel
{

	Q_OBJECT

	public:

		explicit MyClickableLabel( QWidget* parent = nullptr );
		~MyClickableLabel();

	signals:

		void clicked(QString const &);

	protected:

		void mousePressEvent(QMouseEvent* event);

	public slots:

	private:

};

#endif // MYCLICKABLELABEL_H
