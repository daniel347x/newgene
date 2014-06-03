#ifndef LIMIT_DMUS_H
#define LIMIT_DMUS_H

#include "../Table.h"
#ifndef Q_MOC_RUN
#	include <boost/algorithm/string.hpp>
#endif

class Table__Limit_DMUS__Categories : public Table<TABLE__LIMIT_DMUS__CATEGORIES, TABLE_INSTANCE_IDENTIFIER_CONTAINER_TYPE__VECTOR>
{

public:

	static std::string const LIMIT_DMUS__DMU_CATEGORY_STRING_CODE;

	enum COLUMN_INDEX
	{
		INDEX__LIMIT_DMUS__DMU_CATEGORY_STRING_CODE = 0
	};

	Table__Limit_DMUS__Categories()
		: Table<TABLE__LIMIT_DMUS__CATEGORIES, TABLE_INSTANCE_IDENTIFIER_CONTAINER_TYPE__VECTOR>(Table_basemost::TABLE_MODEL_TYPE__OUTPUT_MODEL)
	{
		table_name = "LIMIT_DMUS__CATEGORIES";
	}

	void Load(sqlite3 * db, OutputModel * output_model_, InputModel * input_model_);

	bool Exists(sqlite3 * db, OutputModel & output_model_, InputModel & input_model_, std::string const & dmu_category_code, bool const also_confirm_using_cache = true);
	bool ExistsInCache(sqlite3 * db, OutputModel & output_model_, InputModel & input_model_, std::string const & dmu_category_code);
	bool AddDMU(sqlite3 * db, OutputModel & output_model_, InputModel & input_model_, std::string const & dmu_category_code);
	bool RemoveDMU(sqlite3 * db, OutputModel & output_model_, InputModel & input_model_, WidgetInstanceIdentifier const & dmu_category);

};

class Table__Limit_DMUs__Elements : public Table<Table__Limit_DMUS__ELEMENTS, TABLE_INSTANCE_IDENTIFIER_CONTAINER_TYPE__MAP>
{

public:

	static std::string const LIMIT_DMUS__DMU_CATEGORY_STRING_CODE;
	static std::string const LIMIT_DMUS__DMU_SET_MEMBER_UUID;

	enum COLUMN_INDEX
	{
		INDEX__LIMIT_DMUS__DMU_CATEGORY_STRING_CODE = 0
		, INDEX__LIMIT_DMUS__DMU_SET_MEMBER_UUID
	};

	Table__Limit_DMUs__Elements()
		: Table<Table__Limit_DMUS__ELEMENTS, TABLE_INSTANCE_IDENTIFIER_CONTAINER_TYPE__MAP>(Table_basemost::TABLE_MODEL_TYPE__OUTPUT_MODEL)
	{
		table_name = "LIMIT_DMUS__ELEMENTS";
	}

	void Load(sqlite3 * db, OutputModel * output_model_, InputModel * input_model_);

	bool Exists(sqlite3 * db, OutputModel & output_model_, InputModel & input_model_, WidgetInstanceIdentifier const & dmu_category, std::string const & dmu_member_uuid, bool const also_confirm_using_cache = true);
	bool ExistsInCache(sqlite3 * db, OutputModel & output_model_, InputModel & input_model_, WidgetInstanceIdentifier const & dmu_category, std::string const & dmu_member_uuid);
	bool ExistsInCache(sqlite3 * db, OutputModel & output_model_, InputModel & input_model_, WidgetInstanceIdentifier const & dmu_category, std::int64_t const & dmu_member_uuid);
	WidgetInstanceIdentifier AddDmuMember(sqlite3 * db, OutputModel & output_model_, InputModel & input_model_, WidgetInstanceIdentifier const & dmu_category, std::string const & dmu_member_uuid);
	bool RemoveDmuMember(sqlite3 * db, OutputModel & output_model_, InputModel & input_model_, WidgetInstanceIdentifier const & dmu_category, std::string const & dmu_member_uuid);

};

#endif
