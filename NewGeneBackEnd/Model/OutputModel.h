#ifndef OUTPUTMODEL_H
#define OUTPUTMODEL_H

#include "Model.h"

class OutputModel : public Model
{

	public:

		OutputModel(Messager & messager, boost::filesystem::path const path_to_model)
			: Model(messager, path_to_model)
		{

		}

};

#endif
