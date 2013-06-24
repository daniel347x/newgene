#ifndef VARIABLEGROUP_H
#define VARIABLEGROUP_H

#include "../../../globals.h"
#include "../Table.h"

class Table_VG_CATEGORY : public Table<TABLE__VG_CATEGORY>
{

	public:

		static std::string const VG_CATEGORY_UUID;
		static std::string const VG_CATEGORY_STRING_CODE;
		static std::string const VG_CATEGORY_STRING_LONGHAND;
		static std::string const VG_CATEGORY_NOTES1;
		static std::string const VG_CATEGORY_NOTES2;
		static std::string const VG_CATEGORY_NOTES3;
		static std::string const VG_CATEGORY_FK_UOA_CATEGORY_UUID;
		static std::string const VG_CATEGORY_FLAGS;

		enum COLUMN_INDEX
		{
			  INDEX__VG_CATEGORY_UUID = 0
			, INDEX__VG_CATEGORY_STRING_CODE
			, INDEX__VG_CATEGORY_STRING_LONGHAND
			, INDEX__VG_CATEGORY_NOTES1
			, INDEX__VG_CATEGORY_NOTES2
			, INDEX__VG_CATEGORY_NOTES3
			, INDEX__VG_CATEGORY_FK_UOA_CATEGORY_UUID
			, INDEX__VG_CATEGORY_FLAGS
		};

	public:

		Table_VG_CATEGORY()
			: Table<TABLE__VG_CATEGORY>()
		{

		}

		void Load(sqlite3 * db);

		DataInstanceIdentifiers getIdentifiers()
		{
			std::lock_guard<std::recursive_mutex> data_lock(data_mutex);
			return identifiers;
		}

	protected:

		DataInstanceIdentifiers identifiers;

};

class Table_VG_SET_MEMBER : public Table<TABLE__VG_SET_MEMBER>
{

	public:

		static std::string const VG_SET_MEMBER_UUID;
		static std::string const VG_SET_MEMBER_STRING_CODE;
		static std::string const VG_SET_MEMBER_STRING_LONGHAND;
		static std::string const VG_SET_MEMBER_NOTES1;
		static std::string const VG_SET_MEMBER_NOTES2;
		static std::string const VG_SET_MEMBER_NOTES3;
		static std::string const VG_SET_MEMBER_FK_VG_CATEGORY_UUID;
		static std::string const VG_SET_MEMBER_FLAGS;

		enum COLUMN_INDEX
		{
			  INDEX__VG_SET_MEMBER_UUID = 0
			, INDEX__VG_SET_MEMBER_STRING_CODE
			, INDEX__VG_SET_MEMBER_STRING_LONGHAND
			, INDEX__VG_SET_MEMBER_NOTES1
			, INDEX__VG_SET_MEMBER_NOTES2
			, INDEX__VG_SET_MEMBER_NOTES3
			, INDEX__VG_SET_MEMBER_FK_VG_CATEGORY_UUID
			, INDEX__VG_SET_MEMBER_FLAGS
		};

	public:

		Table_VG_SET_MEMBER()
			: Table<TABLE__VG_SET_MEMBER>()
		{

		}

		void Load(sqlite3 * db);

		DataInstanceIdentifiers getIdentifiers(UUID const & uuid)
		{
			std::lock_guard<std::recursive_mutex> data_lock(data_mutex);
			return identifiers_map[uuid];
		}

	protected:

		std::map<UUID, DataInstanceIdentifiers> identifiers_map;

};

#endif
