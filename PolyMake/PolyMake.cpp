//
//	PolyMake.cpp - main program for building hydro polygons using Tiger/Line blocks.  The Tiger/Line files ended in 2006
//	and the Census Bureau started using ESRI shapefiles as the data exchange format.
//  Copyright(C) 2024 Michael E. Cressey
//
//	This program is free software : you can redistribute it and /or modify it under the terms of the
//	GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or
//	any later version.
//
//	This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the
//	implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License along with this program.
//  If not, see https://www.gnu.org/licenses/
//

#include <iostream>
#include <afx.h>
#include <afxdb.h>
#include <stdio.h>
#include <map>
#include <assert.h>

#include "TString.h"

#include "tigerdb.hpp"
#include "DbBlocks.h"
#include "LineTable.hpp"
#include "NODETABL.HPP"
#include "polynode.HPP"
#include "bldpoly2.h"

using namespace NodeEdgePoly;

static int BlkFind(const CString& blk)
{
	int pos;

	if ((pos = blk.Find(_T("99"))) != -1)
	{
		char cc;

		if (pos == 0 && (cc = blk[2]) != ' ')
		{
			if (cc >= '0' && cc <= '8')
				pos = -1;
		}
	}

	return(pos);
}

static int GetWaterLines(CDatabase& db, LPCTSTR blkTable, int county, CArray<GeoDB::DirLineId, GeoDB::DirLineId&>& ids)
{
	int nLines = 0;

	try
	{
		DbBlocks blks(&db);
//
//	Get the blocks that have water on either the left or the right
//
		blks.m_strFilter.Format(_T("county = %d AND "), county);
		blks.m_strFilter += _T("(blkl like '%99%' OR blkr like '%99%')");
		blks.Open(CRecordset::forwardOnly, blkTable, CRecordset::readOnly);
		while (!blks.IsEOF())
		{
			int side = 0;

			if (blks.IsFieldNull(&blks.m_blkl))
			{
				if (BlkFind(blks.m_blkr) != -1)
					side = 1;
			}
			else if (blks.IsFieldNull(&blks.m_blkr))
			{
				if (BlkFind(blks.m_blkl) != -1)
					side = -1;
			}
			else
			{
				int left = BlkFind(blks.m_blkl),
						right = BlkFind(blks.m_blkr);
				//	water block on both sides
				if (left != -1 && right != -1)
					side = 0;
				else if (left != -1)
					side = -1;
				else if (right != -1)
					side = 1;
				else
				{
#if defined( _CONSOLE )
					printf("* GetWaterLines: Invalid block definition: %ld\n", blks.m_tlid);
#endif
				}
			}

			if (side != 0)
			{
				GeoDB::DirLineId lineId;

				lineId.id = blks.m_tlid;
				lineId.dir = (signed char)side;

				ids.SetAtGrow(nLines, lineId);
				nLines++;
			}

			blks.MoveNext();
		}

		blks.Close();
	}
	catch (CDBException* e)
	{
		//	THROW_LAST();
		printf("GetWaterLines: %s\n", e->m_strError);
		nLines = -1;
	}
	catch(CException *e)
	{
		printf("GetWaterLines: C exception\n");
		nLines = -1;
	}

	return(nLines);
}

static int FillTopoTables(
	TigerDB::Chain* line,
	LineTable* lTable,
	NodeTable* nTable
)
{
	long tlid = line->userId;
	PolyNode* node;
	double angle;
	GeoLine* newLine;
	//
	//	Check first to see if line already in list: some lines are in the list twice
	//	  (positive & negative directions)
	//
	if ((newLine = (GeoLine*)lTable->Find(&tlid, (unsigned long)tlid)) != 0)
	{
		return(1);
	}

	newLine = new GeoLine;
	ASSERT(newLine != 0);

	newLine->tlid = tlid;
	newLine->dfcc = line->userCode;
	lTable->Link(newLine, tlid);
	line->getNodes(&newLine->sPt, &newLine->ePt);
	newLine->mbr = line->getMBR();
	if ((newLine->nPts = line->getNumPts()) > 2)
	{
		if (newLine->pts != 0)
			delete[] newLine->pts;

		newLine->pts = new XY_t[newLine->nPts];
		ASSERT(newLine->pts != 0);
		line->Get(newLine->pts);
	}

	if (newLine->nPts > 2)
		angle = newLine->sPt.Angle(newLine->pts[1]);
	else
		angle = newLine->sPt.Angle(newLine->ePt);

	unsigned long hashVal = newLine->sPt.Hash();
	if ((node = (PolyNode*)nTable->Find(&newLine->sPt, hashVal)) == 0)
	{
		node = new PolyNode(newLine->sPt);
		ASSERT(node != 0);
		nTable->Link(node, hashVal);
	}

	node->Insert(newLine, angle);
	newLine->sNode = node;

	hashVal = newLine->ePt.Hash();
	if ((node = (PolyNode*)nTable->Find(&newLine->ePt, hashVal)) == 0)
	{
		node = new PolyNode(newLine->ePt);
		ASSERT(node != 0);
		nTable->Link(node, hashVal);
	}

	if (newLine->nPts > 2)
		angle = newLine->ePt.Angle((XY_t&)newLine->pts[newLine->nPts - 2]);
	else
		angle = newLine->ePt.Angle(newLine->sPt);

	node->Insert(newLine, angle);
	newLine->eNode = node;

	return(0);
}

int AddClosureLines(
	std::map<int, int>& tlidMap,
	TigerDB &db, 
	CArray<GeoDB::DirLineId, GeoDB::DirLineId&>& lineIds,
	int &nLines
)
{
	int nCLines = 0;
	for (auto it = tlidMap.begin(); it != tlidMap.end(); it++)
	{
		ObjHandle oh;
		int err = db.Read(it->second, oh);
		TigerDB::Chain* line = (TigerDB::Chain*)oh.Lock();
		assert(it->first == line->userId/*GetTLID()*/);
		if (line->userCode == TigerDB::HYDRO_USGSClosureLine)
		{
			int i;
			long tlid = it->first;

			for (i = 0; i < nLines; i++)
			{
				const GeoDB::DirLineId& lineId = lineIds[i];

				if (lineId.id == tlid /*|| lineIds[ i ] == -tlid*/)
				{
					break;
				}
			}

			if (i == nLines)
			{
				GeoDB::DirLineId lineId;

				lineId.id = tlid;
				lineId.dir = 1;
				lineIds.SetAtGrow(nLines, lineId/*tlid*/);
				nLines++;
				lineId.dir = -1;
				lineIds.SetAtGrow(nLines, lineId/*-tlid*/);
				nLines++;
				printf("* USGS Closure line: %ld\n", tlid);
				nCLines++;
			}
		}
		oh.Unlock();
	}
	return nCLines;
}

int main(int argc, char* argv[])
{
	bool doHydro = true;
	try
	{
		CDatabase db;
		db.Open(_T("TigerBase"), FALSE, TRUE, _T("ODBC;"), FALSE);
		CArray<GeoDB::DirLineId, GeoDB::DirLineId&> lineIds;
		CString baseName = "";
		TigerDB tDB(&db);

		if (argc < 3)
		{
			printf("* Invalid arguments: %d\n", argc);
			return -1;
		}

		int stateFips = ::atoi(argv[2]);
		switch (stateFips)
		{
		case 23:
			baseName = "ME";
			break;
		default:
			printf("* State %d not supported at this time\n", stateFips);
			return -1;
		}
		std::string version;
		int err = tDB.Open(TString(argv[1]), version, 1);
		if (err != 0)
		{
			printf("* Cannot open: %s\n", argv[1]);
			return -1;
		}

		int countyFips = ::atoi(argv[3]);

		std::map<int, int> tlidMap;
#if defined(SAVE_FOR_NOW)
		FILE* file = ::fopen("C:\\Work\\Census\\Data\\Maine-2006\\Waldo\\TGR23027Map.tab", "r");
		short s;
		//DAC dac;
		//err = dac.open(TString("C:\\Work\\Waldo2006.dac"), 0, 1, &s);

		char record[100];
		while (::fgets(record, sizeof(record) - 1, file) != NULL)
		{
			char* pos = strchr(record, '\t');
			*pos = '\0';
			int tlid = atoi(record);
			int recPtr = atoi(++pos);

			ObjHandle oh;
			DbHash dbHash;
			dbHash.tlid = tlid;
			err = tDB.dacSearch(DB_EDGE, &dbHash, oh);
			assert(err == 0);
			TigerDB::Chain* line = (TigerDB::Chain*)oh.Lock();
			int rec = line->dbAddress();
			assert(rec == recPtr);
			tlidMap.insert({ tlid, recPtr });
			oh.Unlock();
		}
#endif

		if (doHydro)
		{
			int nLines;
			long newId = 0;
			CString blockTable = baseName + "blocks";
			CString equivTable = baseName + "equiv";
/*	Tables used to create polygons before topology was built into GeoDB
			LineTable lTable(1 << 16);
			NodeTable nTable;
*/
			lineIds.SetSize(10000, 1000);
			
			if ((nLines = GetWaterLines(db, blockTable, countyFips, lineIds)) <= 0)
			{
				fprintf(stdout, "Error getting Hydro lines: %s\n", (char *)(LPCTSTR)blockTable);
				return -1;
			}

			//int nClosureLines = AddClosureLines(tlidMap, tDB, lineIds, nLines);
			//fprintf(stdout, "\nNumber of Closure Lines: %d\n", nClosureLines);

			int count = 0;

#ifdef TEST_HYDRO
			CDbLines lines(&db, 0);
			lines.m_strFilter = "(DFCC = 111)";
			lines.Open(CRecordset::forwardOnly, lineTable, CRecordset::readOnly);
			while (!lines.IsEOF())
			{
				CDbLines::Id tlid = lines.GetTLID();
				int i;

				for (i = 0; i < nLines; i++)
				{
					long id;
					if ((id = lineIds[i]) == tlid || id == -tlid)
					{
						break;
					}
				}

				if (i == nLines)
				{
					count++;
					lineIds.Add(tlid);
				}

				lines.MoveNext();
			}
			lines.Close();
#endif
			fprintf(stdout, "\nNumber of Lines: %d\n", nLines);
/*	This is the old way of building polygons before topology was built into GeoDB
			FillPolyTables(tlidMap, tDB, nLines, lineIds, lTable, nTable);
			fprintf(stderr, "\nNumber of Nodes: %d\n", nTable.GetNumItems());
*/
			//int stateCode = 23;
			int nPolys = BuildPoly(tDB, "HYDRO", OPEN_WATER, 0, lineIds, nLines + count,
				 stateFips, TString(baseName), &newId, nLines, "ISLAND", 121);
			fprintf(stdout, "Number of polygons: %ld\n", nPolys);

			long tSize,
				nEntries;
			int maxBucket;
			float average;
/*	Old way of building polygons
			tSize = lTable.GetStats(&nEntries, &maxBucket, &average);
			fprintf(stderr, "LineTable - Size:%ld, Buckets Used:%ld, Objects:%ld, Max chain:%d, Avg:%lf\n",
				tSize, nEntries, lTable.GetNumItems(), maxBucket, (double)average);

			tSize = nTable.GetStats(&nEntries, &maxBucket, &average);
			fprintf(stderr, "NodeTable - Size:%ld, Buckets Used:%ld, Objects:%ld, Max chain:%d, Avg:%lf\n",
				tSize, nEntries, nTable.GetNumItems(), maxBucket, (double)average);

			lTable.Flush();
			nTable.Flush();
*/
		}
		tDB.Close();
		db.Close();
	}
	catch (CDBException* e)
	{
		fprintf(stderr, "* PolyMake: DB err: %s\n", e->m_strError);
		fprintf(stderr, "%s\n", e->m_strStateNativeOrigin);
		e->Delete();
	}
	catch (CMemoryException* e)
	{
		fprintf(stderr, "* PolyMake: memory exception\n");
		e->Delete();
	}
	catch (CException* e)
	{
		fprintf(stderr, "* PolyMake: C exception\n");
		e->Delete();
	}
	catch (...)
	{
		fprintf(stderr, "* PolyMake: other exception\n");
	}

	return(0);
}


static int addNode(
	TigerDB& db,
	GeoDB::Edge* line,
	XY_t& nodePt,
	double angle,
	unsigned char dir,
	NodeTable* nTable
)
{
	unsigned long hashVal = nodePt.Hash();
	PolyNode* pNode = (PolyNode*)nTable->Find(&nodePt, hashVal);
	if (pNode != 0)
	{
		assert(pNode->pt == nodePt);
	}

	Range2D mbr;
	mbr.Add(nodePt);
	ObjHandle fo;
	GeoDB::Search ss;
	Range2D range = mbr;
	range.ExtendXY(0.0001);
	bool found = false;
	GeoDB::searchClasses_t classFilter;
	classFilter.set(DB_NODE);
	db.InitSearch(&ss, range, classFilter);
	while (db.getNext(&ss, &fo) == 0)
	{
		GeoDB::SpatialObj* spatialObj = (GeoDB::SpatialObj*)fo.Lock();
		GeoDB::SpatialClass sc = spatialObj->IsA();
		if (sc == GeoDB::POINT)
		{
			GeoDB::Node* node = (GeoDB::Node*)spatialObj;
			XY_t pt = node->getPt();
			fo.Unlock();
			if (pt == nodePt)
			{
				found = true;
				break;
			}
		}
		fo.Unlock();
	}

	int err = 0;
	ObjHandle no;
	if (!found)
	{
		if ((err = db.NewObject(DB_NODE, no/*, id*/)) != 0)
		{
			fprintf(stderr, "**addNode: dbOM.newObject failed\n");
		}

		GeoDB::Node* node = (GeoDB::Node*)no.Lock();
		node->Set(nodePt);
		node->setMBR(mbr);

		if ((err = node->Write()) == 0)
			err = db.addToSpatialTree(no);
		if (pNode != 0 && pNode->pt == nodePt)
			fprintf(stderr, "* addNode: Node (%d) not found but but existed in hash table\n", node->dbAddress());
		no.Unlock();
		if (err != 0)
			fprintf(stderr, "**addNode: write() or Add() failed: %d\n", err);
	}
	else
	{
		no = fo;
		assert(pNode != 0);
	}

	GeoDB::Node* node = (GeoDB::Node*)no.Lock();
	err = node->Insert(line, angle, dir);
	no.Unlock();

	if (pNode == 0)
	{
		pNode = new PolyNode(nodePt);
		nTable->Link(pNode, hashVal);
	}

	return err;
}

static int FillTopoTables2(
	TigerDB& db,
	GeoDB::Edge* line,
	LineTable* lTable,
	NodeTable* nTable
)
{
	long tlid = line->userId;
	double angle;

	XY_t sPt,
		ePt;
	XY_t* pts = 0;
	line->getNodes(&sPt, &ePt);
	int nPts = line->getNumPts();

	if (nPts > 2)
	{
		pts = new XY_t[nPts];
		line->Get(pts);
	}

	if (nPts > 2)
		angle = sPt.Angle(pts[1]);
	else
		angle = sPt.Angle(ePt);

	int err = addNode(db, line, sPt, angle, 0, nTable);
	if (err != 0)
		fprintf(stderr, "* addNode failed (%d) for line: %d\n", err, tlid);

	if (nPts > 2)
		angle = ePt.Angle((XY_t&)pts[nPts - 2]);
	else
		angle = ePt.Angle(sPt);

	if ((err = addNode(db, line, ePt, angle, 1, nTable)) != 0)
		fprintf(stderr, "* addNode failed (%d) for line: %d\n", err, tlid);

	delete[] pts;
	if (err == 0)
	{
		if ((err = db.TrBegin()) == 0)
			err = db.TrEnd();
	}
	return err;
}

static int FillPolyTables(
	std::map<int, int>& tlidMap,
	TigerDB& db,
	int nLines,
	CArray<GeoDB::DirLineId, GeoDB::DirLineId&>& lineIds,
	LineTable& lTable,
	NodeTable& nTable
)
{
	try
	{
		for (int i = 0; i < nLines; i++)
		{
			const GeoDB::DirLineId& lid = lineIds.GetAt(i);
			long tlid = lid.id;

			ObjHandle oh;
			DbHash dbHash;
			dbHash.tlid = tlid;
			int err = db.dacSearch(DB_EDGE, &dbHash, oh);

			if (err != 0)
				//std::map<int, int>::iterator it = tlidMap.find(tlid);
				//if (it == tlidMap.end())
				//if (!lineById.Requery() || lineById.IsEOF())
			{
				GeoDB::DirLineId lineId;

				lineId.id = 0;
				lineId.dir = lid.dir;
				lineIds.SetAt(i, lineId/*0*/);		// kludge for now
				if (tlid != 0)
					fprintf(stderr, "* FillPolyTables: cannot find line: %ld\n", tlid);
				continue;
			}
			//ObjHandle oh;
			//int err = db.Read(it->second, oh);
			//TigerDB::Chain* line = (TigerDB::Chain*)oh.Lock();
			GeoDB::Edge* line = (GeoDB::Edge*)oh.Lock();
			//if ((err = FillTopoTables2(db, line, &lTable, &nTable)) != 0)
			//	fprintf(stderr, "** FillTopoTables2 failed: %d\n", err);

			//FillTopoTables(line, &lTable, &nTable);
			oh.Unlock();
			//
			//	Check first to see if line already in list: some lineById are in the list twice
			//	  (positive & negative directions)
			//
		}

		return(0);
	}
	catch (CDBException* e)
	{
		fprintf(stderr, "FillPolyTables: DB err: %s\n", e->m_strError);
	}
	catch (CMemoryException* e)
	{
		fprintf(stderr, "FillPolyTables: memory exception\n");
	}
	catch (CException* e)
	{
		fprintf(stderr, "FillPolyTables: C exception\n");
	}

	return(-1);
}

static int FillPolyTables2(
	std::map<int, int>& tlidMap,
	TigerDB& db,
	int nLines,
	CArray<GeoDB::DirLineId, GeoDB::DirLineId&>& lineIds,
	LineTable& lTable,
	NodeTable& nTable
)
{
	try
	{
		for (int i = 0; i < nLines; i++)
		{
			const GeoDB::DirLineId& lid = lineIds.GetAt(i);
			long tlid = lid.id;

			ObjHandle oh;
			DbHash dbHash;
			dbHash.tlid = tlid;
			int err = db.dacSearch(DB_EDGE, &dbHash, oh);

			if (err != 0)
				//std::map<int, int>::iterator it = tlidMap.find(tlid);
				//if (it == tlidMap.end())
				//if (!lineById.Requery() || lineById.IsEOF())
			{
				GeoDB::DirLineId lineId;

				lineId.id = 0;
				lineId.dir = lid.dir;
				lineIds.SetAt(i, lineId/*0*/);		// kludge for now
				if (tlid != 0)
					fprintf(stderr, "* FillPolyTables2: cannot find line: %ld\n", tlid);
				continue;
			}
			//ObjHandle oh;
			//int err = db.Read(it->second, oh);
			TigerDB::Chain* line = (TigerDB::Chain*)oh.Lock();
			FillTopoTables(line, &lTable, &nTable);
			oh.Unlock();
			//
			//	Check first to see if line already in list: some lineById are in the list twice
			//	  (positive & negative directions)
			//
		}

		return(0);
	}
	catch (CDBException* e)
	{
		fprintf(stderr, "FillPolyTables: DB err: %s\n", e->m_strError);
	}
	catch (CMemoryException* e)
	{
		fprintf(stderr, "FillPolyTables: memory exception\n");
	}
	catch (CException* e)
	{
		fprintf(stderr, "FillPolyTables: C exception\n");
	}

	return(-1);
}
