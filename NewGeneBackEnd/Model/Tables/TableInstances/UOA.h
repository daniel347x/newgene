#ifndef UOA_H
#define UOA_H

#include "../Table.h"

class Table_UOA_Identifier : public Table<TABLE__UOA_IDENTIFIER, TABLE_INSTANCE_IDENTIFIER_CONTAINER_TYPE__VECTOR>
{

	public:

		static std::string const UOA_CATEGORY_UUID;
		static std::string const UOA_CATEGORY_STRING_CODE;
		static std::string const UOA_CATEGORY_STRING_LONGHAND;
		static std::string const UOA_CATEGORY_TIME_GRANULARITY;
		static std::string const UOA_CATEGORY_NOTES1;
		static std::string const UOA_CATEGORY_NOTES2;
		static std::string const UOA_CATEGORY_NOTES3;
		static std::string const UOA_CATEGORY_FLAGS;

		typedef std::pair<WidgetInstanceIdentifier, int> DMU_Plus_Count;
		typedef std::vector<DMU_Plus_Count> DMU_Counts;

		enum COLUMN_INDEX
		{
			  INDEX__UOA_CATEGORY_UUID = 0
			, INDEX__UOA_CATEGORY_STRING_CODE
			, INDEX__UOA_CATEGORY_STRING_LONGHAND
			, INDEX__UOA_CATEGORY_TIME_GRANULARITY
			, INDEX__UOA_CATEGORY_NOTES1
			, INDEX__UOA_CATEGORY_NOTES2
			, INDEX__UOA_CATEGORY_NOTES3
			, INDEX__UOA_CATEGORY_FLAGS
		};

		Table_UOA_Identifier()
			: Table<TABLE__UOA_IDENTIFIER, TABLE_INSTANCE_IDENTIFIER_CONTAINER_TYPE__VECTOR>(Table_basemost::TABLE_MODEL_TYPE__INPUT_MODEL)
		{

		}

		void Load(sqlite3 * db, InputModel * input_model_);

		// For a given UOA, retrieve its DMU categories
		WidgetInstanceIdentifiers RetrieveDMUCategories(sqlite3 const * db, InputModel * input_model_, UUID const & uuid);

		// For a given UOA, retrieve its DMU categories bucketed by DMU category
		// (i.e., if DMU's are Country-Country-MID, return result as
		// Country=2, MID=1
		DMU_Counts RetrieveDMUCounts(sqlite3 const * db, InputModel * input_model_, UUID const & uuid);

		bool Exists(sqlite3 * db, InputModel & input_model_, WidgetInstanceIdentifier const & uoa, bool const also_confirm_using_cache = true);
		bool ExistsByCode(sqlite3 * db, InputModel & input_model_, std::string const & uoa_code, bool const also_confirm_using_cache = true);
		bool DeleteUOA(sqlite3 * db, InputModel & input_model_, WidgetInstanceIdentifier const & uoa, DataChangeMessage & change_message);
		bool CreateNewUOA(sqlite3 * db, InputModel & input_model, std::string const & uoa_code, WidgetInstanceIdentifiers const & dmu_categories, TIME_GRANULARITY const & time_granularity);

		static std::string GetUoaCategoryDisplayText(WidgetInstanceIdentifier const & uoa_category, WidgetInstanceIdentifiers const & dmu_categories);

};

class Table_UOA_Member : public Table<TABLE__UOA_MEMBER, TABLE_INSTANCE_IDENTIFIER_CONTAINER_TYPE__MAP>
{

	public:

		static std::string const UOA_CATEGORY_LOOKUP_FK_UOA_CATEGORY_UUID;
		static std::string const UOA_CATEGORY_LOOKUP_SEQUENCE_NUMBER;
		static std::string const UOA_CATEGORY_LOOKUP_FK_DMU_CATEGORY_UUID;

		enum COLUMN_INDEX
		{
			  INDEX__UOA_CATEGORY_LOOKUP_FK_UOA_CATEGORY_UUID = 0
			, INDEX__UOA_CATEGORY_LOOKUP_SEQUENCE_NUMBER
			, INDEX__UOA_CATEGORY_LOOKUP_FK_DMU_CATEGORY_UUID
		};

		Table_UOA_Member()
			: Table<TABLE__UOA_MEMBER, TABLE_INSTANCE_IDENTIFIER_CONTAINER_TYPE__MAP>(Table_basemost::TABLE_MODEL_TYPE__INPUT_MODEL)
		{

		}

		void Load(sqlite3 * db, InputModel * input_model_);

		// For a given DMU, retrieve all UOA's that depend on it
		WidgetInstanceIdentifiers RetrieveUOAsGivenDMU(sqlite3 * db, InputModel * input_model_, WidgetInstanceIdentifier const & dmu);

		// Just delete from the cache
		bool DeleteUOA(sqlite3 * db, InputModel & input_model_, WidgetInstanceIdentifier const & uoa);

		// Create a new UOA
		bool CreateNewUOA(sqlite3 * db, InputModel & input_model, std::string const & uoa_uuid, WidgetInstanceIdentifiers dmu_categories);

};

#endif
