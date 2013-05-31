#ifndef MODEL_H
#define MODEL_H

#ifndef Q_MOC_RUN
#	include <boost/filesystem.hpp>
#	include <boost/format.hpp>
#endif
#include "..\Messager\Messager.h"
#ifndef Q_MOC_RUN
#	include <boost/property_tree/ptree.hpp>
#	include <boost/property_tree/xml_parser.hpp>
#endif

class Model
{

	public:
		
		Model(Messager & messager, boost::filesystem::path const path_to_model)
			: _path_to_model(path_to_model)
		{

		}

		virtual void LoadModel(Messager & messager)
		{

		}

	
	protected:

		boost::filesystem::path const _path_to_model;

};

template<typename MODEL_CLASS>
class ModelFactory
{

	public:

		MODEL_CLASS * operator()(Messager & messager, boost::filesystem::path const path_to_model)
		{
			MODEL_CLASS * new_model = new MODEL_CLASS(messager, path_to_model);
			new_model->LoadModel(messager);
			return new_model;
		}

};

#endif
