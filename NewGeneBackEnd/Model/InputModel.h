#ifndef INPUTMODEL_H
#define INPUTMODEL_H

#include "Model.h"

class InputModel : public Model
{

	public:

		InputModel(Messager & messager, boost::filesystem::path const path_to_model)
			: Model(messager, path_to_model)
		{

		}

};

#endif
