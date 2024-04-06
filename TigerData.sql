-- Tiger 2023
CREATE TABLE MEFeatureNames (
  stateFips tinyint NOT NULL,
  countyFips smallint NOT NULL,
  tlid int NOT NULL,
  prefixDir varchar(15) NULL,
  prefixType varchar(50) NULL,
  prefixQual varchar(15) NULL,
  baseName varchar(100) NULL,
  suffixDir varchar(15) NULL,
  suffixType varchar(50) NULL,
  suffixQual varchar(15) NULL,
  lineArid varchar(22) NULL,
  PAFlag char(1) NOT NULL
);

--ALTER TABLE MEFeatureNames Don't have a unique primary key
--  ADD CONSTRAINT [MEFeatureNames_PK] PRIMARY KEY(stateFips,countyFips,tlid,PAFlag);

CREATE INDEX MEFeatureNamesIdx ON MEFeatureNames (stateFips,countyFips,tlid);

BULK INSERT MEFeatureNames
FROM 'C:\Work\Census\Data\Maine-2023\23027-Waldo\Featnames\tl_rd22_23027_featnamen.tab'
WITH (
  FIRSTROW = 2,
  ROWTERMINATOR = '\n',
  FIELDTERMINATOR = '\t',
  TABLOCK
);

CREATE TABLE [MEblocks] (
  county smallint NOT NULL,
  tlid int NOT NULL,
  blkl varchar(6) NULL,
  blkr varchar(6) NULL,
  trackl int NULL,
  trackr int NULL,
  countyl smallint NULL,
  countyr smallint NULL,
  CONSTRAINT [MEBlocks_PK] PRIMARY KEY(tlid,county)
);

select * from MEblocks
where blkl like '%99%' OR blkr like '%99%';

BULK INSERT [MEblocks]
FROM 'c:\Work\Census\Data\Maine-2006\Waldo\tgr23027b.tab'
WITH (
  FIRSTROW = 1,
  FIELDTERMINATOR = '\t',
  ROWTERMINATOR = '\n',
  TABLOCK
);

CREATE TABLE [23Names] (
  state tinyint NOT NULL,
  county smallint NOT NULL,
  tlid int NOT NULL,
  feat int NOT NULL,
  rtsq smallint NOT NULL,
  CONSTRAINT [23Names_PK] PRIMARY KEY(state,county,tlid,rtsq)
);

CREATE TABLE DistNames (
  Id INT IDENTITY(1,1) NOT NULL,
  name varchar(40) NOT NULL,
  CONSTRAINT DistNames_PK PRIMARY KEY (Id)
);

CREATE UNIQUE INDEX NameIdx ON DistNames (name);

CREATE TABLE TgrNames (
  state tinyint NOT NULL,
  county smallint NOT NULL,
  feat int NOT NULL,
  dirp char(2) NULL,
  name varchar(30) NOT NULL,
  type char(4) NULL,
  dirs char(2) NULL,
  CONSTRAINT TgrNames_PK PRIMARY KEY(state,county,feat)
);

BULK INSERT TgrNames
FROM 'c:\Work\TIGER\tigercnv\tgr23013t.tab'
WITH (
  FIRSTROW = 1,
  FIELDTERMINATOR = '\t',
  ROWTERMINATOR = '\n',
  TABLOCK
);

BULK INSERT [23Names]
FROM 'c:\Work\Census\Data\Maine-2006\Knox\tgr23013n.tab'
WITH (
  FIRSTROW = 1,
  FIELDTERMINATOR = '\t',
  ROWTERMINATOR = '\n',
  TABLOCK
);
-- query to link 23Names and TgrNames together (used by NameLook.cpp)
select tlid, t2.feat, rtsq, dirp, name, type, dirs
from [23Names] t1, tgrnames t2
where t1.feat = t2.feat
and tlid = 75631525
and t1.state = t2.state and t1.county = t2.county
and t1.state = 23 and t2.county = 27;

INSERT INTO DistNames (name)
SELECT DISTINCT name FROM TgrNames;

SELECT DISTINCT name FROM TgrNames
where county = 13;