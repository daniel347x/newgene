#ifndef MODEL_H
#define MODEL_H

#ifndef Q_MOC_RUN
#	include <boost/filesystem.hpp>
#	include <boost/format.hpp>
#endif
#include "../Messager/Messager.h"
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

class Model_basemost
{

	public:

		Model_basemost(Messager &, boost::filesystem::path const & path_to_model_database)
			: _path_to_model_database(path_to_model_database)
			, db(NULL)
		{

		}

		virtual ~Model_basemost()
		{
		}

		boost::filesystem::path getPathToDatabaseFile()
		{
			return _path_to_model_database;
		}

		void setPathToDatabaseFile(boost::filesystem::path const & path_to_model_databas)
		{
			_path_to_model_database = path_to_model_databas;
		}

		void LoadDatabase()
		{
			if (db == nullptr)
			{
				bool exists = true;
				if (!boost::filesystem::exists(getPathToDatabaseFile()))
				{
					exists = false;
				}

				sqlite3 * db_ = nullptr;
				int err = sqlite3_open(getPathToDatabaseFile().string().c_str(), &db_);
				if (err)
				{
					// TODO: Parse the error codes from https://www.sqlite.org/c3ref/c_abort.html
					boost::format msg("Error opening database file");
					throw NewGeneException() << newgene_error_description(msg.str());
				}
				db = db_;

				if (!exists)
				{
					// Create the new database
					std::string err;
					std::string sql = this->GetCreationSQL();
					bool success = this->RunSQL(sql, err);
					if (!success)
					{
						boost::format msg("Error creating new database: %1%");
						msg % err.c_str();
						throw NewGeneException() << newgene_error_description(msg.str());
					}
				}
			}
		}

		void UnloadDatabase()
		{
			if (db != nullptr)
			{
				sqlite3_close(db);
			}
		}

		virtual std::string GetCreationSQL() = 0;

		void SaveDatabaseAs(Messager & messager, boost::filesystem::path new_database_path)
		{
			boost::filesystem::path current_path = getPathToDatabaseFile();
			if (current_path == new_database_path)
			{
				return;
			}
			UnloadDatabase();
			try
			{
				boost::filesystem::copy_file(current_path, new_database_path, boost::filesystem::copy_option::overwrite_if_exists);
			}
			catch (boost::filesystem::filesystem_error & ex)
			{
				boost::format msg("Error copying model database file \"%1%\" to \"%2%\"): %3%");
				msg % current_path.string() % new_database_path.string() % ex.what();
				messager.AppendMessage(new MessagerWarningMessage(MESSAGER_MESSAGE__FILE_DOES_NOT_EXIST, msg.str()));
				LoadDatabase();
				return;
			}
			UnloadDatabase();
			setPathToDatabaseFile(new_database_path);
			LoadDatabase();
		}

		sqlite3 * getDb()
		{
			return db;
		}

		sqlite3 const * getDb() const
		{
			return db;
		}

		void VacuumDatabase()
		{
			char * errmsg = nullptr;
			sqlite3_exec(db, "VACUUM", NULL, NULL, &errmsg);
			if (errmsg != nullptr)
			{
				// todo - handle very rare and mostly harmless error at some point in the future
				sqlite3_free(errmsg);
			}
		}

		bool RunSQL(std::string const sql, std::string & err)
		{
			char * errmsg = nullptr;
			sqlite3_exec(db, sql.c_str(), NULL, NULL, &errmsg);
			err.clear();
			if (errmsg != nullptr)
			{
				err = errmsg;
				sqlite3_free(errmsg);
				return false;
			}
			return true;
		}

		void ClearRemnantTemporaryTables()
		{
			sqlite3_stmt * stmt = NULL;
			std::string sql("SELECT name FROM sqlite_master WHERE type='table'");
			sqlite3_prepare_v2(db, sql.c_str(), sql.size() + 1, &stmt, NULL);
			if (stmt == NULL)
			{
				return;
			}
			int step_result = 0;

			// must store table names in a vector,
			// because if we try to delete them
			// while iterating through the table names themselves,
			// the entire database is locked because we're still executing
			// the SQL that returns the table names and so the attempt to delete fails due to locked database
			std::vector<std::string> tables_to_delete;

			while ((step_result = sqlite3_step(stmt)) == SQLITE_ROW)
			{
				char const * table_name_ = reinterpret_cast<char const *>(sqlite3_column_text(stmt, 0));
				std::string table_name(table_name_);
				std::string prefix("NGTEMP_");
				if (table_name.size() >= prefix.size())
				{
					if (boost::iequals(table_name.substr(0, prefix.size()), prefix))
					{
						tables_to_delete.push_back(table_name);
					}
				}
			}

			if (stmt)
			{
				sqlite3_finalize(stmt);
				stmt = nullptr;
			}

			std::for_each(tables_to_delete.cbegin(), tables_to_delete.cend(), [this](std::string const & table_to_delete)
			{
				boost::format drop_stmt("DROP TABLE IF EXISTS %1%");
				drop_stmt % table_to_delete;
				char * errmsg = nullptr;
				sqlite3_exec(db, drop_stmt.str().c_str(), NULL, NULL, &errmsg);
				if (errmsg != nullptr)
				{
					// todo - handle very rare and mostly harmless error at some point in the future
					sqlite3_free(errmsg);
				}
			});
		}

	protected:

		boost::filesystem::path _path_to_model_database;
		sqlite3 * db;

};

template<typename MODEL_SETTINGS_ENUM>
class Model : public Model_basemost
{

	public:

		static int const number_worker_threads = 1; // For now, single thread only in pool

		Model(Messager & messager, boost::filesystem::path const & path_to_model_database)
			: Model_basemost(messager, path_to_model_database)
		{

		}

		~Model()
		{
			UnloadDatabase();
		}

};

template<typename MODEL_CLASS>
class ModelFactory
{

	// ****************************************************************************** //
	// Empty non-specialized class.
	// See InputModel.h and OutputModel.h for SPECIALIZATIONS of this class
	// ****************************************************************************** //

};

#endif
