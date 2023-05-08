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
FROM 'c:\Work\TIGER\tigercnv\tgr23027t.tab'
WITH (
  FIRSTROW = 1,
  FIELDTERMINATOR = '\t',
  ROWTERMINATOR = '\n',
  TABLOCK
);

BULK INSERT [23Names]
FROM 'c:\Work\Census\Data\Maine-2006\Waldo\tgr23027n.tab'
WITH (
  FIRSTROW = 1,
  FIELDTERMINATOR = '\t',
  ROWTERMINATOR = '\n',
  TABLOCK
);

INSERT INTO DistNames (name)
SELECT DISTINCT name FROM TgrNames;