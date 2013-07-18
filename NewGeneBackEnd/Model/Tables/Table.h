#ifndef TABLE_H
#define TABLE_H

#include <tuple>
#include <vector>

#include "TableTypes.h"
#include "../../globals.h"
#include "../../sqlite/sqlite-amalgamation-3071700/sqlite3.h"
#include <mutex>
#include "../TimeGranularity.h"

#ifndef Q_MOC_RUN
#	include <boost/algorithm/string.hpp>
#endif

class InputModel;
class OutputModel;

template<FIELD_TYPE THE_FIELD_TYPE>
class FieldValue
{
public:
	FieldValue()
	{

	}
	FieldValue(typename FieldTypeTraits<THE_FIELD_TYPE>::type & value_)
		: value(value_)
	{

	}
	typename FieldTypeTraits<THE_FIELD_TYPE>::type value;
};

template <FIELD_TYPE THE_FIELD_TYPE>
struct FieldData
{
	typedef std::tuple<FIELD_TYPE, std::string, FieldValue<THE_FIELD_TYPE>> type;
};

class BaseField
{
public:
	virtual FIELD_TYPE GetType() = 0;
	virtual std::string GetName() = 0;
};

template <FIELD_TYPE THE_FIELD_TYPE>
class Field : public BaseField
{

public:

	Field(std::string const field_name, FieldValue<THE_FIELD_TYPE> const & field_value)
		: BaseField()
		, data(std::make_tuple(THE_FIELD_TYPE, field_name, field_value))

	Field(FieldData<field_type>::type const & data_)
		: BaseField()
		, data_(data_)
	{

	}

	FIELD_TYPE GetType()
	{
		return THE_FIELD_TYPE;
	}

	std::string GetName()
	{
		return data.get<1>(data);
	}

	typename FieldTypeTraits<THE_FIELD_TYPE>::type GetValue()
	{
		return data.get<2>(data);
	}

	typename FieldData<THE_FIELD_TYPE>::type const data;
};

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

	  TABLE_INSTANCE_IDENTIFIER_CONTAINER_TYPE__VECTOR
	, TABLE_INSTANCE_IDENTIFIER_CONTAINER_TYPE__MAP
	, TABLE_INSTANCE_IDENTIFIER_CONTAINER_TYPE__VECTOR_PLUS_INT

};

class Table_basemost
{
	 
	public:

		Table_basemost()
		{

		}

		virtual void Load(sqlite3 * db, InputModel * input_model_ = nullptr) {};
		virtual void Load(sqlite3 * db, OutputModel * output_model_ = nullptr, InputModel * input_model_ = nullptr) {};

		std::recursive_mutex data_mutex;

};

template<TABLE_INSTANCE_IDENTIFIER_CONTAINER_TYPE CONTAINER_TYPE>
class Table_base : public Table_basemost
{

	public:

		Table_base<CONTAINER_TYPE>()
			: Table_basemost()
		{

		}

};

template<>
class Table_base<TABLE_INSTANCE_IDENTIFIER_CONTAINER_TYPE__VECTOR> : public Table_basemost
{

	public:

		Table_base()
			: Table_basemost()
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

	protected:

		WidgetInstanceIdentifiers identifiers;

};

template<>
class Table_base<TABLE_INSTANCE_IDENTIFIER_CONTAINER_TYPE__MAP> : public Table_basemost
{

	public:

		Table_base()
			: Table_basemost()
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

	protected:

		std::map<UUID, WidgetInstanceIdentifiers> identifiers_map;

};

template<TABLE_TYPES TABLE_TYPE, TABLE_INSTANCE_IDENTIFIER_CONTAINER_TYPE CONTAINER_TYPE>
class Table : public Table_base<CONTAINER_TYPE>
{

	public:

		Table<TABLE_TYPE, CONTAINER_TYPE>()
			: Table_base<CONTAINER_TYPE>()
		{

		}

		TableMetadata<TABLE_TYPE> metadata;

};

template<>
class Table_base<TABLE_INSTANCE_IDENTIFIER_CONTAINER_TYPE__VECTOR_PLUS_INT> : public Table_basemost
{

	public:

		Table_base()
			: Table_basemost()
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

	protected:

		WidgetInstanceIdentifiers_WithInts identifiers;

};

#endif
