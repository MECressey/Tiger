// PolyMake.cpp : This file contains the 'main' function. Program execution begins and ends there.
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
//#include "dac.hpp"

//#ifdef SAVE_FOR_NOW

class DbHash : public DbHashAccess {
public:
	long tlid;
	int is_equal(DbObject* dbo) { return this->tlid == ((TigerDB::Chain*)dbo)->GetTLID(); }
	long int hashKey(int nBits) { return HashTable::HashDK(nBits, tlid); }
};

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

int GetWaterLines(CDatabase& db, LPCTSTR blkTable, /*const char* lineTable,
	const char* equivTable,*/ CArray<DirLineId, DirLineId&>& ids)
{
	int nLines = 0;

	try
	{
		DbBlocks blks(&db);
//
//	Get the blocks that have water on either the left or the right
//
		blks.m_strFilter = "blkl like '%99%' OR blkr like '%99%'";
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
				DirLineId lineId;

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
//#endif

int FillTopoTables(
	TigerDB::Chain* line, //const CDbLines& line,
	LineTable* lTable,
	NodeTable* nTable
)
{
	long tlid = line->GetTLID();
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
	newLine->dfcc = line->GetCode(); // .GetDFCC();
	//strcpy(newLine->cfcc, line.GetCFCC());
	lTable->Link(newLine, tlid);
	line->GetNodes(&newLine->sPt, &newLine->ePt);
	newLine->mbr = line->GetMBR();
	if ((newLine->nPts = line->GetNumPts()) > 2)
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

	node->Insert(newLine/*tlid*/, angle);
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

	node->Insert(newLine/*-tlid*/, angle);
	newLine->eNode = node;

	return(0);
}

int FillPolyTables(
	std::map<int, int> &tlidMap,
	//DAC &dac,
	TigerDB &db,
	int nLines,
	CArray<DirLineId, DirLineId&>& lineIds,
	LineTable& lTable,
	NodeTable& nTable
)
{
	try
	{
		for (int i = 0; i < nLines; i++)
		{
			const DirLineId& lid = lineIds.GetAt(i);
			long tlid = lid.id;

			ObjHandle oh;
			DbHash dbHash;
			dbHash.tlid = tlid;
			int err = db.dacSearch(DB_TIGER_LINE, &dbHash, oh);

			if (err != 0)
			//std::map<int, int>::iterator it = tlidMap.find(tlid);
			//if (it == tlidMap.end())
			//if (!lineById.Requery() || lineById.IsEOF())
			{
				DirLineId lineId;

				lineId.id = 0;
				lineId.dir = lid.dir;
				lineIds.SetAt(i, lineId/*0*/);		// kludge for now
				if (tlid != 0)
					fprintf(stderr, "* FillPolyTables: cannot find line: %ld\n", tlid);
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
	catch(CDBException *e)
	{
		fprintf(stderr, "FillPolyTables: DB err: %s\n", e->m_strError);
	}
	catch(CMemoryException *e)
	{
		fprintf(stderr, "FillPolyTables: memory exception\n");
	}
	catch(CException *e)
	{
		fprintf(stderr, "FillPolyTables: C exception\n");
	}

	return(-1);
}

int AddClosureLines(
	std::map<int, int>& tlidMap,
	TigerDB &db, 
	CArray<DirLineId, DirLineId&>& lineIds,
	int &nLines
)
{
	int nCLines = 0;
	for (auto it = tlidMap.begin(); it != tlidMap.end(); it++)
	{
		ObjHandle oh;
		int err = db.Read(it->second, oh);
		TigerDB::Chain* line = (TigerDB::Chain*)oh.Lock();
		assert(it->first == line->GetTLID());
		if (line->GetCode() == TigerDB::NVF_USGSClosureLine)
		{
			int i;
			long tlid = it->first;

			for (i = 0; i < nLines; i++)
			{
				const DirLineId& lineId = lineIds[i];

				if (lineId.id == tlid /*|| lineIds[ i ] == -tlid*/)
				{
					break;
				}
			}

			if (i == nLines)
			{
				DirLineId lineId;

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
		db.SetQueryTimeout(60 * 10);
		db.Open(_T("TigerBase"), FALSE, TRUE, _T("ODBC;"), FALSE);
		CArray<DirLineId, DirLineId&> lineIds;
		CString baseName = "ME"/*argv[1]*/;
		TigerDB tDB(&db);

		int err = tDB.Open(TString(/*"C:\\Work\\Waldo2006poly.gdb")*/argv[1]), 1);

		std::map<int, int> tlidMap;
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
			err = tDB.dacSearch(5, &dbHash, oh);
			TigerDB::Chain* line = (TigerDB::Chain*)oh.Lock();
			int rec = line->dbAddress();
			assert(rec == recPtr);
			tlidMap.insert({ tlid, recPtr });
			oh.Unlock();
		}

#ifdef TESTING_POLYGON
		// Creating a polygon
		ObjHandle po;

		if ((err= tDB.NewObject(DB_TIGER_POLY, po/*, id*/)) != 0)
		{
			fprintf(stderr, "**dbOM.newObject failed\n");
		}

		TigerDB::Polygon* poly = (TigerDB::Polygon*)po.Lock();

		poly->SetCode(TigerDB::HYDRO_PerennialLakeOrPond);
		poly->SetArea(0.000005);
		Range2D range;
		range.y.min = 44.4465;
		range.x.min = -69.2176;
		range.y.max = 44.4488;
		range.x.max = -69.2134;
		poly->SetMBR(range);

		poly->write();
		//po.Unlock();
		tDB.Add(po);

		ObjHandle eh1;
		std::map<int, int>::iterator it = tlidMap.find(75629639);
		err = tDB.Read(it->second, eh1);

		err = poly->AddEdge(eh1, 1);

		ObjHandle eh2;
		it = tlidMap.find(75629638);
		err = tDB.Read(it->second, eh2);
		err = poly->AddEdge(eh2, 0);

		tDB.TrBegin();
		tDB.TrEnd();
#endif
		if (doHydro)
		{
			int nLines;
			long newId = 0;
			CString blockTable = baseName + "blocks";
			CString equivTable = baseName + "equiv";

			/*DbBlocks blks(&db);
			blks.m_strFilter = "blkl like '%99%' OR blkr like '%99%'";
			blks.Open(CRecordset::forwardOnly, blockTable, CRecordset::readOnly);
			while (!blks.IsEOF())
			{
				printf("TLID: %d\n", blks.m_tlid);
				blks.MoveNext();
			}*/

			LineTable lTable(1 << 16);
			NodeTable nTable;

			lineIds.SetSize(10000, 1000);

			if ((nLines = GetWaterLines(db, blockTable, lineIds)) <= 0)
			{
				fprintf(stderr, "Error getting Hydro lines: %s\n", (char *)(LPCTSTR)blockTable);
				return -1;
			}

			int nClosureLines = AddClosureLines(tlidMap, tDB, lineIds, nLines);
			fprintf(stderr, "\nNumber of Closure Lines: %d\n", nClosureLines);

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
			fprintf(stderr, "\nNumber of Lines: %d\n", nLines);

			FillPolyTables(tlidMap, tDB, nLines, lineIds, lTable, nTable);
/**/
			int stateCode = 23;
			int nPolys = BuildPoly2(tDB, tlidMap, "HYDRO", OPEN_WATER, 0, lineIds, nLines + count,
				lTable, nTable, stateCode, TString(baseName), &newId, nLines, "ISLAND", 121);
			fprintf(stderr, "Number of polygons: %ld\n", nPolys);
/**/
			long tSize,
				nEntries;
			int maxBucket;
			float average;

			tSize = lTable.GetStats(&nEntries, &maxBucket, &average);
			fprintf(stderr, "LineTable - Size:%ld, Buckets Used:%ld, Objects:%ld, Max chain:%d, Avg:%lf\n",
				tSize, nEntries, lTable.GetNumItems(), maxBucket, (double)average);

			tSize = nTable.GetStats(&nEntries, &maxBucket, &average);
			fprintf(stderr, "NodeTable - Size:%ld, Buckets Used:%ld, Objects:%ld, Max chain:%d, Avg:%lf\n",
				tSize, nEntries, nTable.GetNumItems(), maxBucket, (double)average);

			lTable.Flush();
			nTable.Flush();
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

