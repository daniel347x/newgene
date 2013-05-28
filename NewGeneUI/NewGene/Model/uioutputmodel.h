#ifndef UIOUTPUTMODEL_H
#define UIOUTPUTMODEL_H

#include "../../../NewGeneBackEnd/Model/OutputModel.h"
#include "uimodel.h"

class UIOutputModel : public QObject, public UIModel<OutputModel>
{

		Q_OBJECT

	signals:

	public slots:


	public:
		UIOutputModel(UIMessager & messager, OutputModel & model, boost::filesystem::path const path_to_model = boost::filesystem::path(), QObject * parent = NULL);
};

#endif // UIOUTPUTMODEL_H
