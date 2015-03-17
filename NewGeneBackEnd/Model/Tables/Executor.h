#ifndef EXECUTOR_H
#define EXECUTOR_H

class Executor
{
	public:

		Executor(sqlite3 * db_, bool const begin_transaction = true)
			: db(db_)
			, transaction_begun(false)
			, good(false)
		{
			if (db && begin_transaction)
			{
				sqlite3_exec(db, "BEGIN TRANSACTION;", NULL, NULL, NULL);
				transaction_begun = true;
			}
		}

		~Executor()
		{
			EndTransaction();
		}

		void success(bool const good_ = true)
		{
			good = good_;
		}

		bool succeeded()
		{
			return good;
		}

		void BeginTransaction()
		{
			if (db && !transaction_begun)
			{
				sqlite3_exec(db, "BEGIN TRANSACTION;", NULL, NULL, NULL);
				transaction_begun = true;
			}
		}

		void EndTransaction(bool const rollback = false)
		{
			if (db && transaction_begun)
			{
				if (rollback)
				{
					sqlite3_exec(db, "ROLLBACK TRANSACTION;", NULL, NULL, NULL);
				}
				else
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

				transaction_begun = false;
			}
		}

		sqlite3 * db;
		bool transaction_begun;
		bool good;
};


#endif
