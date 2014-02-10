/**************************/
/*                        */
/* OutputModelDefault.SQL */
/*                        */
/**************************/

/* Disable Foreign Keys */
pragma foreign_keys = off;
/* Begin Transaction */
begin transaction;

/* Database [DefaultOutputModel.newgene.out] */
pragma auto_vacuum=0;
pragma encoding='UTF-8';
pragma page_size=1024;

/* Drop table [main].[KAD_COUNT] */
drop table if exists [main].[KAD_COUNT];

/* Table structure [main].[KAD_COUNT] */
CREATE TABLE [main].[KAD_COUNT] (
  [DMU_CATEGORY_STRING_CODE] VARCHAR(128) NOT NULL ON CONFLICT FAIL, 
  [COUNT] INT NOT NULL ON CONFLICT FAIL, 
  [FLAGS] CHAR(16), 
  CONSTRAINT [sqlite_autoindex_KAD_COUNT_1] PRIMARY KEY ([DMU_CATEGORY_STRING_CODE]) ON CONFLICT FAIL);

/* Drop table [main].[TIMERANGE_SELECTED] */
drop table if exists [main].[TIMERANGE_SELECTED];

/* Table structure [main].[TIMERANGE_SELECTED] */
CREATE TABLE [main].[TIMERANGE_SELECTED] (
  [TIMERANGE_START] INT64, 
  [TIMERANGE_END] INT64);

/* Data [main].[TIMERANGE_SELECTED] */
insert into [main].[TIMERANGE_SELECTED] values(-2208988799912, -1577923199024);


/* Drop table [main].[VG_SET_MEMBERS_SELECTED] */
drop table if exists [main].[VG_SET_MEMBERS_SELECTED];

/* Table structure [main].[VG_SET_MEMBERS_SELECTED] */
CREATE TABLE [main].[VG_SET_MEMBERS_SELECTED] (
  [VG_SET_MEMBER_STRING_CODE] VARCHAR(128), 
  [VG_CATEGORY_STRING_CODE] VARCHAR(128));
CREATE UNIQUE INDEX [main].[VG_SET_MEMBER_STRING_CODE__VG_CATEGORY_STRING_CODE] ON [VG_SET_MEMBERS_SELECTED] ([VG_SET_MEMBER_STRING_CODE], [VG_CATEGORY_STRING_CODE]);

/* Commit Transaction */
commit transaction;

/* Enable Foreign Keys */
pragma foreign_keys = on;