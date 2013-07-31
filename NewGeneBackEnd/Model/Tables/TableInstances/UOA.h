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

};

#endif
