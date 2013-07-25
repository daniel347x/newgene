#ifndef EXECUTOR_H
#define EXECUTOR_H

class Executor
{
	public:
	
		Executor(sqlite3 * db_)
			: db(db_)
			, good(false)
		{
			if (db)
			{
				sqlite3_exec(db, "BEGIN TRANSACTION;", NULL, NULL, NULL);
			}
		}

		~Executor()
		{
			if (db)
			{
				if (good)
				{
					sqlite3_exec(db, "END TRANSACTION;", NULL, NULL, NULL);
				}
				else
				{
					sqlite3_exec(db, "ROLLBACK TRANSACTION;", NULL, NULL, NULL);
				}
			}
		}

		void success(bool const good_ = true)
		{
			good = good_;
		}

		bool succeeded()
		{
			return good;
		}

		sqlite3 * db;
		bool good;
};


#endif
