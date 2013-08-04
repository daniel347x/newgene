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
#include "Import\Import.h"
#include "Fields.h"
#include "Schema.h"

#ifndef Q_MOC_RUN
#	include <boost/algorithm/string.hpp>
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

		virtual void Load(sqlite3 * db, InputModel * input_model_ = nullptr) {};
		virtual void Load(sqlite3 * db, OutputModel * output_model_ = nullptr, InputModel * input_model_ = nullptr) {};
		virtual bool ImportStart(sqlite3 * db, std::string code, ImportDefinition const & import_definition, OutputModel * output_model_ = nullptr, InputModel * input_model_ = nullptr) { return true; };
		virtual bool ImportBlock(sqlite3 * db, ImportDefinition const & import_definition, OutputModel * output_model_, InputModel * input_model_, DataBlock const & block, int const number_rows_in_block) { return true; };
		virtual bool ImportEnd(sqlite3 * db, ImportDefinition const & import_definition, OutputModel * output_model_ = nullptr, InputModel * input_model_ = nullptr) { return true; };

		std::recursive_mutex data_mutex;
		TABLE_MODEL_TYPE table_model_type;

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
						if (identifier_.code && boost::iequals(code, *identifier_.code) && identifier_.identifier_parent && identifier_.identifier_parent->uuid && boost::iequals(parent_uuid, *identifier_.identifier_parent->uuid))
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

		WidgetInstanceIdentifier getIdentifier(UUID const & uuid_, UUID parent_uuid)
		{
			std::lock_guard<std::recursive_mutex> data_lock(data_mutex);
			WidgetInstanceIdentifier the_identifier;
			bool found = false;
			std::for_each(identifiers_map.cbegin(), identifiers_map.cend(), [&uuid_, &parent_uuid, &found, &the_identifier](std::pair<UUID, WidgetInstanceIdentifiers> const & identifiers_)
			{
				if (found)
				{
					return;
				}
				if (boost::iequals(parent_uuid, identifiers_.first))
				{

					// The category matches

					std::for_each(identifiers_.second.cbegin(), identifiers_.second.cend(), [&uuid_, &parent_uuid, &found, &the_identifier](WidgetInstanceIdentifier const & identifier_)
					{
						if (identifier_.uuid && boost::iequals(uuid_, *identifier_.uuid) && identifier_.identifier_parent && identifier_.identifier_parent->uuid && boost::iequals(parent_uuid, *identifier_.identifier_parent->uuid))
						{
							the_identifier = identifier_;
							found = true;
							return;
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

		Table<TABLE_TYPE, CONTAINER_TYPE>(TABLE_MODEL_TYPE const table_model_type_)
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

		Table_base(TABLE_MODEL_TYPE const table_model_type_)
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

#endif
