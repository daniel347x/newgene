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
#include "../Threads/ThreadPool.h"
#include "../Threads/WorkerThread.h"
#include "../Settings/ModelSettings.h"
#include "../sqlite/sqlite-amalgamation-3071700/sqlite3.h"

class InputModel;
class InputModelSettings;

template<typename MODEL_SETTINGS_ENUM>
class Model
{

	public:

		static int const number_worker_threads = 1; // For now, single thread only in pool

		Model(Messager & messager, boost::filesystem::path const path_to_model_database)
			: _path_to_model_database(path_to_model_database)
			, db(NULL)
		{

		}

		~Model()
		{
			UnloadDatabase();
		}

		boost::filesystem::path getPathToDatabaseFile()
		{
			return _path_to_model_database;
		}

		void LoadDatabase()
		{
			if (db == nullptr)
			{
				sqlite3 * db_ = nullptr;
				int err = sqlite3_open(getPathToDatabaseFile().string().c_str(), &db_);
				if (err)
				{
					return;
				}
				db = db_;
			}
		}

		void UnloadDatabase()
		{
			if (db != nullptr)
			{
				sqlite3_close(db);
			}
		}

		virtual void LoadTables()
		{
			if (db == nullptr)
			{
				LoadDatabase();
			}
		}

	protected:

		boost::filesystem::path _path_to_model_database;
		sqlite3 * db;

};

template<typename MODEL_CLASS>
class ModelFactory
{

	public:

		MODEL_CLASS * operator()(Messager & messager, boost::filesystem::path const path_to_model_database)
		{
			MODEL_CLASS * new_model = new MODEL_CLASS(messager, path_to_model_database);
			return new_model;
		}

		MODEL_CLASS * operator()(Messager & messager, boost::filesystem::path const path_to_model_database, std::shared_ptr<InputModelSettings> const input_model_settings_, std::shared_ptr<InputModel> const input_model_)
		{
			MODEL_CLASS * new_model = new MODEL_CLASS(messager, path_to_model_database, input_model_settings_, input_model_);
			return new_model;
		}

};

#endif
