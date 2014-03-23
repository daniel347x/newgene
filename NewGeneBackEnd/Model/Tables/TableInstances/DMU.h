#ifndef DMU_H
#define DMU_H

#include "../Table.h"
#ifndef Q_MOC_RUN
#	include <boost/algorithm/string.hpp>
#endif
#include "../../../Utilities/TimeRangeHelper.h"

class Table_DMU_Identifier : public Table<TABLE__DMU_IDENTIFIER, TABLE_INSTANCE_IDENTIFIER_CONTAINER_TYPE__VECTOR>
{

	public:

		static std::string const DMU_CATEGORY_UUID;
		static std::string const DMU_CATEGORY_STRING_CODE;
		static std::string const DMU_CATEGORY_STRING_LONGHAND;
		static std::string const DMU_CATEGORY_NOTES1;
		static std::string const DMU_CATEGORY_NOTES2;
		static std::string const DMU_CATEGORY_NOTES3;
		static std::string const DMU_CATEGORY_FLAGS;

		enum COLUMN_INDEX
		{
			  INDEX__DMU_CATEGORY_UUID = 0
			, INDEX__DMU_CATEGORY_STRING_CODE
			, INDEX__DMU_CATEGORY_STRING_LONGHAND
			, INDEX__DMU_CATEGORY_NOTES1
			, INDEX__DMU_CATEGORY_NOTES2
			, INDEX__DMU_CATEGORY_NOTES3
			, INDEX__DMU_CATEGORY_FLAGS
		};

		Table_DMU_Identifier()
			: Table<TABLE__DMU_IDENTIFIER, TABLE_INSTANCE_IDENTIFIER_CONTAINER_TYPE__VECTOR>(Table_basemost::TABLE_MODEL_TYPE__INPUT_MODEL)
		{
			table_name = "DMU_CATEGORY";
		}

		void Load(sqlite3 * db, InputModel * input_model_);

		bool Exists(sqlite3 * db, InputModel & input_model_, std::string const & dmu, bool const also_confirm_using_cache = true);
		bool CreateNewDMU(sqlite3 * db, InputModel & input_model_, std::string const & dmu, std::string const & dmu_description);
		bool DeleteDMU(sqlite3 * db, InputModel & input_model_, WidgetInstanceIdentifier & dmu, DataChangeMessage & change_message);

		static std::string GetDmuCategoryDisplayText(WidgetInstanceIdentifier const & dmu_category);

};

class Table_DMU_Instance : public Table<TABLE__DMU_INSTANCE, TABLE_INSTANCE_IDENTIFIER_CONTAINER_TYPE__MAP>
{

	public:

		static std::string const DMU_SET_MEMBER_UUID;
		static std::string const DMU_SET_MEMBER_STRING_CODE;
		static std::string const DMU_SET_MEMBER_STRING_LONGHAND;
		static std::string const DMU_SET_MEMBER_NOTES1;
		static std::string const DMU_SET_MEMBER_NOTES2;
		static std::string const DMU_SET_MEMBER_NOTES3;
		static std::string const DMU_SET_MEMBER_FK_DMU_CATEGORY_UUID;
		static std::string const DMU_SET_MEMBER_FLAGS;

		enum COLUMN_INDEX
		{
			  INDEX__DMU_SET_MEMBER_UUID = 0
			, INDEX__DMU_SET_MEMBER_STRING_CODE
			, INDEX__DMU_SET_MEMBER_STRING_LONGHAND
			, INDEX__DMU_SET_MEMBER_NOTES1
			, INDEX__DMU_SET_MEMBER_NOTES2
			, INDEX__DMU_SET_MEMBER_NOTES3
			, INDEX__DMU_SET_MEMBER_FK_DMU_CATEGORY_UUID
			, INDEX__DMU_SET_MEMBER_FLAGS
		};

		Table_DMU_Instance()
			: Table<TABLE__DMU_INSTANCE, TABLE_INSTANCE_IDENTIFIER_CONTAINER_TYPE__MAP>(Table_basemost::TABLE_MODEL_TYPE__INPUT_MODEL)
		{
				table_name = "DMU_SET_MEMBER";
		}

		void Load(sqlite3 * db, InputModel * input_model_);

		bool Exists(sqlite3 * db, InputModel & input_model_, WidgetInstanceIdentifier const & dmu, std::string const & dmu_member, bool const also_confirm_using_cache = true);
		WidgetInstanceIdentifier CreateNewDmuMember(sqlite3 * db, InputModel & input_model_, WidgetInstanceIdentifier const & dmu_category, std::string const & dmu_member_uuid, std::string const & dmu_member_code, std::string const & dmu_member_description);
		bool DeleteDmuMember(sqlite3 * db, InputModel & input_model_, WidgetInstanceIdentifier const & dmu_member);
		bool RefreshFromFile(sqlite3 * db, InputModel & input_model_, WidgetInstanceIdentifier const & dmu_category, boost::filesystem::path const & dmu_refresh_file_pathname, std::vector<std::string> const & dmu_column_labels, Messager & messager, bool const do_refresh_not_plain_insert);

		static std::string GetDmuMemberDisplayText(WidgetInstanceIdentifier const & dmu_member);

};

#endif
