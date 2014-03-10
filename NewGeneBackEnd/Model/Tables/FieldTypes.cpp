#include "FieldTypes.h"

// Handle string default initialziations
FieldTypeTraits<FIELD_TYPE_STRING_FIXED>::type const FieldTypeTraits<FIELD_TYPE_STRING_FIXED>::default_;
FieldTypeTraits<FIELD_TYPE_STRING_VAR>::type const FieldTypeTraits<FIELD_TYPE_STRING_VAR>::default_;
FieldTypeTraits<FIELD_TYPE_FLOAT>::type const FieldTypeTraits<FIELD_TYPE_FLOAT>::default_ = 0.0;
FieldTypeTraits<FIELD_TYPE_UUID>::type const FieldTypeTraits<FIELD_TYPE_UUID>::default_;
FieldTypeTraits<FIELD_TYPE_UUID_FOREIGN>::type const FieldTypeTraits<FIELD_TYPE_UUID_FOREIGN>::default_;
FieldTypeTraits<FIELD_TYPE_STRING_CODE>::type const FieldTypeTraits<FIELD_TYPE_STRING_CODE>::default_;
FieldTypeTraits<FIELD_TYPE_STRING_LONGHAND>::type const FieldTypeTraits<FIELD_TYPE_STRING_LONGHAND>::default_;
FieldTypeTraits<FIELD_TYPE_NOTES_1>::type const FieldTypeTraits<FIELD_TYPE_NOTES_1>::default_;
FieldTypeTraits<FIELD_TYPE_NOTES_2>::type const FieldTypeTraits<FIELD_TYPE_NOTES_2>::default_;
FieldTypeTraits<FIELD_TYPE_NOTES_3>::type const FieldTypeTraits<FIELD_TYPE_NOTES_3>::default_;
FieldTypeTraits<FIELD_TYPE_DMU_MEMBER_UUID>::type const FieldTypeTraits<FIELD_TYPE_DMU_MEMBER_UUID>::default_;
FieldTypeTraits<FIELD_TYPE_DMU_MEMBER_UUID_STRING>::type const FieldTypeTraits<FIELD_TYPE_DMU_MEMBER_UUID_STRING>::default_;
FieldTypeTraits<FIELD_TYPE_DMU_MEMBER_CODE>::type const FieldTypeTraits<FIELD_TYPE_DMU_MEMBER_CODE>::default_;
FieldTypeTraits<FIELD_TYPE_DMU_MEMBER_DESCRIPTION>::type const FieldTypeTraits<FIELD_TYPE_DMU_MEMBER_DESCRIPTION>::default_;
FieldTypeTraits<FIELD_TYPE_FK_TO_DMU_CATEGORY_UUID>::type const FieldTypeTraits<FIELD_TYPE_FK_TO_DMU_CATEGORY_UUID>::default_;
FieldTypeTraits<FIELD_TYPE_TIMERANGE_STRING>::type const FieldTypeTraits<FIELD_TYPE_TIMERANGE_STRING>::default_;
FieldTypeTraits<FIELD_TYPE_DMU_PRIMARY_KEY_AND_TIMERANGE_STRING>::type const FieldTypeTraits<FIELD_TYPE_DMU_PRIMARY_KEY_AND_TIMERANGE_STRING>::default_;
