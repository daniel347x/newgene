#ifndef TABLE_H
#define TABLE_H

#include <tuple>
#include <vector>

#include "TableTypes.h"
#include "../../globals.h"
#include "../../sqlite/sqlite-amalgamation-3071700/sqlite3.h"
#include <mutex>
#include "../TimeGranularity.h"
#include "Executor.h"
#include "Import/Import.h"
#include "Fields.h"
#include "Schema.h"

#ifndef Q_MOC_RUN
#	include <boost/algorithm/string.hpp>
#	include <boost/lexical_cast.hpp>
#endif

class InputModel;
class OutputModel;

class TableMetadata_base
{
	public:

};

template<TABLE_TYPES TABLE_TYPE>
class TableMetadata : public TableMetadata_base
{

};

enum TABLE_INSTANCE_IDENTIFIER_CONTAINER_TYPE
{
	  TABLE_INSTANCE_IDENTIFIER_CONTAINER_TYPE__NONE
	, TABLE_INSTANCE_IDENTIFIER_CONTAINER_TYPE__VECTOR
	, TABLE_INSTANCE_IDENTIFIER_CONTAINER_TYPE__MAP
	, TABLE_INSTANCE_IDENTIFIER_CONTAINER_TYPE__VECTOR_PLUS_INT
	, TABLE_INSTANCE_IDENTIFIER_CONTAINER_TYPE__VECTOR_PLUS_INT64

};

class Table_basemost
{

	public:

		enum TABLE_MODEL_TYPE
		{
			TABLE_MODEL_TYPE__INPUT_MODEL
			, TABLE_MODEL_TYPE__OUTPUT_MODEL
		};

		Table_basemost(TABLE_MODEL_TYPE const table_model_type_)
			: table_model_type(table_model_type_)
		{

		}

		std::string GetTableName() { return table_name; }
		std::string table_name;

		long badreadlines; // convenience: for data import
		long badwritelines; // convenience: for data import
		long goodreadlines; // convenience: for data import
		long goodwritelines; // convenience: for data import

		virtual void Load(sqlite3 *, InputModel * = nullptr) { };
		virtual void Load(sqlite3 *, OutputModel * = nullptr, InputModel * = nullptr) { };
		virtual bool ImportStart(sqlite3 *, WidgetInstanceIdentifier const & identifier, ImportDefinition const &, OutputModel * = nullptr, InputModel * = nullptr) { return true; };
		virtual void ImportBlockBulk(sqlite3 *, ImportDefinition const &, OutputModel *, InputModel *, DataBlock const &, int const, long & linenum, long & badwritelines, std::vector<std::string> & errors);
		virtual void ImportBlockUpdate(sqlite3 *, ImportDefinition const &, OutputModel *, InputModel *, DataBlock const &, int const, long & linenum, long & badwritelines, long & numlinesupdated, std::vector<std::string> & errors);

		int TryUpdateRow(DataBlock const & block, int row, bool & failed, ImportDefinition const &import_definition, sqlite3 * db, std::string & errorMsg);
		void TryInsertRow(DataBlock const & block, int row, bool & failed, ImportDefinition const &import_definition, sqlite3 * db, std::string & errorMsg);

		void FieldDataAsSqlText(std::shared_ptr<BaseField> const & field_data, std::string & sql_insert, bool const no_quotes = false);

		virtual bool ImportEnd(sqlite3 *, WidgetInstanceIdentifier const & identifier, ImportDefinition const &, OutputModel * = nullptr, InputModel * = nullptr) { return true; };

		std::recursive_mutex data_mutex;
		TABLE_MODEL_TYPE table_model_type;

		static std::string EscapeTicks(std::string const & s)
		{
			std::string out;
			char const * cs = s.c_str();
			while (*cs != '\0')
			{
				if (*cs == '\'')
				{
					out += '\'';
				}
				out += *cs;
				++cs;
			}
			return out;
		}

};

template<TABLE_INSTANCE_IDENTIFIER_CONTAINER_TYPE CONTAINER_TYPE>
class Table_base : public Table_basemost
{

	public:

		Table_base<CONTAINER_TYPE>(TABLE_MODEL_TYPE const table_model_type_)
			: Table_basemost(table_model_type_)
		{

		}

};

template<>
class Table_base<TABLE_INSTANCE_IDENTIFIER_CONTAINER_TYPE__NONE> : public Table_basemost
{

	public:

		Table_base(TABLE_MODEL_TYPE const table_model_type_)
			: Table_basemost(table_model_type_)
		{

		}

};

template<>
class Table_base<TABLE_INSTANCE_IDENTIFIER_CONTAINER_TYPE__VECTOR> : public Table_basemost
{

	public:

		Table_base(TABLE_MODEL_TYPE const table_model_type_)
			: Table_basemost(table_model_type_)
		{

		}

		WidgetInstanceIdentifiers getIdentifiers()
		{
			std::lock_guard<std::recursive_mutex> data_lock(data_mutex);
			return identifiers;
		}

		bool getIdentifierFromStringCode(std::string const code, WidgetInstanceIdentifier & the_identifier)
		{
			std::lock_guard<std::recursive_mutex> data_lock(data_mutex);
			bool found = false;
			std::for_each(identifiers.cbegin(), identifiers.cend(), [&code, &found, &the_identifier](WidgetInstanceIdentifier const & identifier)
			{
				if (found)
				{
					return;
				}

				if (identifier.code && boost::iequals(code, *identifier.code))
				{
					the_identifier = identifier;
					found = true;
					return;
				}
			});
			return found;
		}

		WidgetInstanceIdentifier getIdentifier(UUID const & uuid_)
		{
			std::lock_guard<std::recursive_mutex> data_lock(data_mutex);
			WidgetInstanceIdentifier the_identifier;
			bool found = false;
			std::for_each(identifiers.cbegin(), identifiers.cend(), [&uuid_, &found, &the_identifier](WidgetInstanceIdentifier const & identifier)
			{
				if (found)
				{
					return;
				}

				if (identifier.uuid && boost::iequals(uuid_, *identifier.uuid))
				{
					the_identifier = identifier;
					found = true;
					return;
				}
			});
			return the_identifier;
		}

		void Sort()
		{
			std::lock_guard<std::recursive_mutex> data_lock(data_mutex);
			std::sort(identifiers.begin(), identifiers.end());
		}

	protected:

		WidgetInstanceIdentifiers identifiers;

};

template<>
class Table_base<TABLE_INSTANCE_IDENTIFIER_CONTAINER_TYPE__MAP> : public Table_basemost
{

	public:

		Table_base(TABLE_MODEL_TYPE const table_model_type_)
			: Table_basemost(table_model_type_)
		{

		}

		WidgetInstanceIdentifiers getIdentifiers(UUID const & uuid)
		{
			std::lock_guard<std::recursive_mutex> data_lock(data_mutex);

			if (identifiers_map.find(uuid) == identifiers_map.end())
			{
				return WidgetInstanceIdentifiers();
			}

			return identifiers_map[uuid];
		}

		bool getIdentifierFromStringCodeAndParentUUID(std::string const code, UUID parent_uuid, WidgetInstanceIdentifier & the_identifier)
		{
			std::lock_guard<std::recursive_mutex> data_lock(data_mutex);
			bool found = false;
			std::for_each(identifiers_map.cbegin(), identifiers_map.cend(), [&code, &parent_uuid, &found, &the_identifier](std::pair<UUID, WidgetInstanceIdentifiers> const & identifiers_)
			{
				if (found)
				{
					return;
				}

				if (boost::iequals(parent_uuid, identifiers_.first))
				{

					// The category matches

					std::for_each(identifiers_.second.cbegin(), identifiers_.second.cend(), [&code, &parent_uuid, &found, &the_identifier](WidgetInstanceIdentifier const & identifier_)
					{
						if (identifier_.code && boost::iequals(code, *identifier_.code) && identifier_.identifier_parent && identifier_.identifier_parent->uuid
							&& boost::iequals(parent_uuid, *identifier_.identifier_parent->uuid))
						{
							the_identifier = identifier_;
							found = true;
							return;
						}
					});

				}
			});
			return found;
		}

		WidgetInstanceIdentifier getIdentifier(UUID const & uuid_, UUID parent_uuid, bool const use_code_for_parent = false)
		{
			std::lock_guard<std::recursive_mutex> data_lock(data_mutex);
			WidgetInstanceIdentifier the_identifier;
			bool found = false;
			std::for_each(identifiers_map.cbegin(), identifiers_map.cend(), [&uuid_, &parent_uuid, &found, &the_identifier, &use_code_for_parent](std::pair<UUID, WidgetInstanceIdentifiers> const & identifiers_)
			{
				if (found)
				{
					return;
				}

				if (boost::iequals(parent_uuid, identifiers_.first))
				{

					// The category matches

					std::for_each(identifiers_.second.cbegin(), identifiers_.second.cend(), [&uuid_, &parent_uuid, &found, &the_identifier, &use_code_for_parent](WidgetInstanceIdentifier const & identifier_)
					{

						if (!use_code_for_parent)
						{
							if (identifier_.uuid && boost::iequals(uuid_, *identifier_.uuid) && identifier_.identifier_parent && identifier_.identifier_parent->uuid
								&& boost::iequals(parent_uuid, *identifier_.identifier_parent->uuid))
							{
								the_identifier = identifier_;
								found = true;
								return;
							}
						}
						else
						{
							if (identifier_.uuid && boost::iequals(uuid_, *identifier_.uuid) && identifier_.identifier_parent && identifier_.identifier_parent->code
								&& boost::iequals(parent_uuid, *identifier_.identifier_parent->code))
							{
								the_identifier = identifier_;
								found = true;
								return;
							}
						}
					});

				}
			});
			return the_identifier;
		}

		void Sort()
		{
			std::lock_guard<std::recursive_mutex> data_lock(data_mutex);
			std::for_each(identifiers_map.begin(), identifiers_map.end(), [](std::pair<UUID const, WidgetInstanceIdentifiers> & map_entry)
			{
				WidgetInstanceIdentifiers & the_identifiers = map_entry.second;
				std::sort(the_identifiers.begin(), the_identifiers.end());
			});
		}

	protected:

		std::map<UUID, WidgetInstanceIdentifiers> identifiers_map;

};

template<TABLE_TYPES TABLE_TYPE, TABLE_INSTANCE_IDENTIFIER_CONTAINER_TYPE CONTAINER_TYPE>
class Table : public Table_base<CONTAINER_TYPE>
{

	public:

		Table<TABLE_TYPE, CONTAINER_TYPE>(Table_basemost::TABLE_MODEL_TYPE const table_model_type_)
			: Table_base<CONTAINER_TYPE>(table_model_type_)
			, table_type(TABLE_TYPE)
		{

		}

		TableMetadata<TABLE_TYPE> metadata;
		TABLE_TYPES table_type;

};

template<>
class Table_base<TABLE_INSTANCE_IDENTIFIER_CONTAINER_TYPE__VECTOR_PLUS_INT> : public Table_basemost
{

	public:

		Table_base(Table_basemost::TABLE_MODEL_TYPE const table_model_type_)
			: Table_basemost(table_model_type_)
		{

		}

		WidgetInstanceIdentifiers_WithInts getIdentifiers()
		{
			std::lock_guard<std::recursive_mutex> data_lock(data_mutex);
			return identifiers;
		}

		bool getIdentifierFromStringCode(std::string const code, WidgetInstanceIdentifier_Int_Pair & the_identifier)
		{
			std::lock_guard<std::recursive_mutex> data_lock(data_mutex);
			bool found = false;
			std::for_each(identifiers.cbegin(), identifiers.cend(), [&code, &found, &the_identifier](WidgetInstanceIdentifier_Int_Pair const & identifier)
			{
				if (found)
				{
					return;
				}

				if (identifier.first.code && boost::iequals(code, *identifier.first.code))
				{
					the_identifier = identifier;
					found = true;
					return;
				}
			});
			return found;
		}

		WidgetInstanceIdentifier_Int_Pair getIdentifier(UUID const & uuid_)
		{
			std::lock_guard<std::recursive_mutex> data_lock(data_mutex);
			WidgetInstanceIdentifier_Int_Pair the_identifier;
			bool found = false;
			std::for_each(identifiers.cbegin(), identifiers.cend(), [&uuid_, &found, &the_identifier](WidgetInstanceIdentifier_Int_Pair const & identifier)
			{
				if (found)
				{
					return;
				}

				if (identifier.first.uuid && boost::iequals(uuid_, *identifier.first.uuid))
				{
					the_identifier = identifier;
					found = true;
					return;
				}
			});
			return the_identifier;
		}

		void Sort()
		{
			std::lock_guard<std::recursive_mutex> data_lock(data_mutex);
			std::sort(identifiers.begin(), identifiers.end(), [](WidgetInstanceIdentifier_Int_Pair const & lhs, WidgetInstanceIdentifier_Int_Pair const & rhs)
			{
				return lhs.first > rhs.first;
			});
		}

	protected:

		WidgetInstanceIdentifiers_WithInts identifiers;

};

template<>
class Table_base<TABLE_INSTANCE_IDENTIFIER_CONTAINER_TYPE__VECTOR_PLUS_INT64> : public Table_basemost
{

	public:

		Table_base(TABLE_MODEL_TYPE const table_model_type_)
			: Table_basemost(table_model_type_)
		{

		}

		WidgetInstanceIdentifiers_WithInt64s getIdentifiers()
		{
			std::lock_guard<std::recursive_mutex> data_lock(data_mutex);
			return identifiers;
		}

		bool getIdentifierFromStringCodeAndFlags(std::string const code, std::string const & flags, WidgetInstanceIdentifier_Int64_Pair & the_identifier)
		{
			std::lock_guard<std::recursive_mutex> data_lock(data_mutex);
			bool found = false;
			std::for_each(identifiers.cbegin(), identifiers.cend(), [&code, &flags, &found, &the_identifier](WidgetInstanceIdentifier_Int64_Pair const & identifier)
			{
				if (found)
				{
					return;
				}

				if (identifier.first.code && boost::iequals(code, *identifier.first.code) && identifier.first.flags == flags)
				{
					the_identifier = identifier;
					found = true;
					return;
				}
			});
			return found;
		}

		bool getIdentifierFromStringCode(std::string const code, WidgetInstanceIdentifier_Int64_Pair & the_identifier)
		{
			std::lock_guard<std::recursive_mutex> data_lock(data_mutex);
			bool found = false;
			std::for_each(identifiers.cbegin(), identifiers.cend(), [&code, &found, &the_identifier](WidgetInstanceIdentifier_Int64_Pair const & identifier)
			{
				if (found)
				{
					return;
				}

				if (identifier.first.code && boost::iequals(code, *identifier.first.code))
				{
					the_identifier = identifier;
					found = true;
					return;
				}
			});
			return found;
		}

		WidgetInstanceIdentifier_Int64_Pair getIdentifier(UUID const & uuid_)
		{
			std::lock_guard<std::recursive_mutex> data_lock(data_mutex);
			WidgetInstanceIdentifier_Int64_Pair the_identifier;
			bool found = false;
			std::for_each(identifiers.cbegin(), identifiers.cend(), [&uuid_, &found, &the_identifier](WidgetInstanceIdentifier_Int64_Pair const & identifier)
			{
				if (found)
				{
					return;
				}

				if (identifier.first.uuid && boost::iequals(uuid_, *identifier.first.uuid))
				{
					the_identifier = identifier;
					found = true;
					return;
				}
			});
			return the_identifier;
		}

	protected:

		WidgetInstanceIdentifiers_WithInt64s identifiers;

};

#endif
