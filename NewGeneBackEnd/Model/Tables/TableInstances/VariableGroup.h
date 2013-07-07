#ifndef VARIABLEGROUP_H
#define VARIABLEGROUP_H

#include "../../../globals.h"
#include "../Table.h"
#ifndef Q_MOC_RUN
#	include <boost/algorithm/string.hpp>
#endif

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

	protected:

		WidgetInstanceIdentifiers identifiers;

};

class Table_VG_SET_MEMBER : public Table<TABLE__VG_SET_MEMBER>
{

	public:

		static std::string const VG_SET_MEMBER_UUID;
		static std::string const VG_SET_MEMBER_STRING_CODE;
		static std::string const VG_SET_MEMBER_STRING_LONGHAND;
		static std::string const VG_SET_MEMBER_SEQUENCE_NUMBER;
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
			, INDEX__VG_SET_MEMBER_SEQUENCE_NUMBER
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

		void Load(sqlite3 * db, InputModel * input_model_ = nullptr);

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

					// The variable group matches
					
					std::for_each(identifiers_.second.cbegin(), identifiers_.second.cend(), [&code, &parent_uuid, &found, &the_identifier](WidgetInstanceIdentifier const & identifier_)
					{
						if (identifier_.code && boost::iequals(code, *identifier_.code) && identifier_.uuid_parent && boost::iequals(parent_uuid, *identifier_.uuid_parent))
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

	protected:

		std::map<UUID, WidgetInstanceIdentifiers> identifiers_map;

};

#endif
