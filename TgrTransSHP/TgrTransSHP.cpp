// ShapeFile.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <afx.h>
#include <afxdb.h>

#include <stdlib.h>
#include <iostream>
#include <string>
#include <assert.h>

#include "shapefil.h"
#include "TString.h"
#include "tigerdb.hpp"

using namespace std;

static short GetStateFips(const char*);
static short GetCountyFips(const char*);
static TigerDB::Classification MapMTFCC(const char* mtfcc);

const char* FILE_PREFIX = _T("tl_rd22_");

int main(int argc, char* argv[])
{
	char buffer[512];
	bool doNames = false,
		doLines = false;
	strcpy(buffer, argv[1]);
	if (argc > 2 && (argv[2][0] == '/' || argv[2][0] == '-'))
	{
		int i = 1;
		while (argv[2][i] != '\0')		// Can chain options
		{
			switch (argv[2][i])
			{
			default:
				printf("* TgrTransSHP - invalid argument\n");
				goto USAGE_ERROR;

			case 'L':
			case 'l':
				doLines = true;
				break;

			case 'N':
			case 'n':
				doNames = true;
				break;
			}
			i++;
		}
	}
	else
	{
		goto USAGE_ERROR;
	}

	try
	{
    DBFHandle dbf;
    SHPHandle shp;
    short countyFips = 0;
    int		stateFips = 0;

    if (doNames)
    {
      dbf = DBFOpen(argv[1], "rb");
      int fieldCount = DBFGetFieldCount(dbf);
      int recCount = DBFGetRecordCount(dbf);

      CString inputFileName = argv[1];

      int dirIdx = inputFileName.Find(FILE_PREFIX);
      int dotIdx = inputFileName.ReverseFind('.');
      if (dotIdx == -1)
        dotIdx = inputFileName.GetLength() - 1;
      CString rootName = inputFileName.Left(dotIdx);
      cout << "\nRoot name: " << rootName << endl;
      //printf("\nRoot name: %s\n", (const char*)rootName);

      CString baseName = rootName;
      countyFips = GetCountyFips(baseName);
      stateFips = GetStateFips(baseName);

      CString fName = rootName + "n.tab";
      FILE* tabNames;
      if ((tabNames = ::fopen(TString(fName), "w")) == 0)
      {
        printf("** Cannot create file %s\n", (const char*)fName);
        goto ERR_RETURN;
      }

      fprintf(tabNames, "StateFIPS\tCountyFIPS\t\TLID\tPrefixDir\tPrefixType\tPrefixQual\tBaseName\tSuffixDir\tSuffixType\tSuffixQual\tLineArid\tPAFlag\n");
/*
      for (int i = 0; i < fieldCount; i++)
      {
        char fieldName[80];
        int fWidth;
        int fDecimals;

        DBFFieldType fieldType = DBFGetFieldInfo(dbf, i, fieldName, &fWidth, &fDecimals);
        printf("Field %d, Name: %s, type: %d, width: %d\n", i, fieldName, fieldType, fWidth);
      }
*/
      for (int i = 0; i < recCount; i++)
      {
        char featName[256];
        char lineArid[23];
        char baseName[128];
        char prefixDir[20];
        char prefixType[51];
        char prefixQualifier[16];
        char suffixDir[20];
        char suffixType[60];
        char suffixQualifier[16];
        char mtfcc[6];
        char paFlag[2];
        int fWidth;
        int fDecimals;

        int tlid = DBFReadIntegerAttribute(dbf, i, 0);
        const char* str = DBFReadStringAttribute(dbf, i, 1);
        strcpy_s(featName, str);
        str = DBFReadStringAttribute(dbf, i, 2);
        strcpy_s(baseName, str);
        str = DBFReadStringAttribute(dbf, i, 3);    // Prefix direction abbreviation
        strcpy_s(prefixDir, str);
        str = DBFReadStringAttribute(dbf, i, 4);    // Prefix type description
        strcpy_s(prefixType, str);
        str = DBFReadStringAttribute(dbf, i, 5);    // Prefix qualifier description
        strcpy_s(prefixQualifier, str);
        str = DBFReadStringAttribute(dbf, i, 6);    // Suffix direction
        strcpy_s(suffixDir, str);
        str = DBFReadStringAttribute(dbf, i, 7);    // Suffix Type abbrev
        strcpy_s(suffixType, str);
        str = DBFReadStringAttribute(dbf, i, 8);   // Suffix qualifier abbrev
        strcpy_s(suffixQualifier, str);
        str = DBFReadStringAttribute(dbf, i, 15);
        strcpy_s(lineArid, str);
        str = DBFReadStringAttribute(dbf, i, 16);   // MTFCC
        strcpy_s(mtfcc, str);
        str = DBFReadStringAttribute(dbf, i, 17);   // PAFlag
        strcpy_s(paFlag, str);
        printf("  TLID: %d, Full: %s, Prefix: %s, Name: %s, Type: %s, Suffix: %s, MTFCC: %s, LinearId: %s\n", tlid, featName, prefixDir, baseName, suffixType, suffixDir, mtfcc, lineArid);
        fprintf(tabNames, "%d\t%d\t\%d\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\n", stateFips, countyFips, tlid, prefixDir, prefixType, prefixQualifier,
            baseName, suffixDir, suffixType, suffixQualifier, lineArid, paFlag);
      }
      /**/
      fclose(tabNames);
      DBFClose(dbf);
      return 0;
    }

    if (doLines)
    {
      int nEntities;
      int shapeType;
      double minBound,
        maxBound;

      shp = SHPOpen(argv[1], "rb");
      dbf = DBFOpen(argv[1], "rb");

      SHPGetInfo(shp, &nEntities, &shapeType, &minBound, &maxBound);
      std::cout << "Number of entities: " << nEntities << ", Shape Type: " << shapeType << ", Min/Max: " << minBound << ", " << maxBound << std::endl;
      int recCount = DBFGetRecordCount(dbf);
      assert(nEntities == recCount);
      for (int i = 0; i < nEntities; i++)
      {
        SHPObject* so = SHPReadObject(shp, i);
        printf("ID: %d, Parts: %d, Vertices: %d\n", so->nShapeId, so->nParts, so->nVertices);
        int tlid = DBFReadIntegerAttribute(dbf, i, 2);
        char mtfcc[6];
        char fullName[101];
        const char *str = DBFReadStringAttribute(dbf, i, 5);   // MTFCC
        strcpy_s(mtfcc, str);
        str = DBFReadStringAttribute(dbf, i, 6);  // FullName
        strcpy_s(fullName, str);
        str = DBFReadStringAttribute(dbf, i, 15);  // HydroFlag

				TigerDB::Classification cCode = MapMTFCC(mtfcc);
				if (cCode == TigerDB::NotClassified)
					printf("** Unclassifed MTFCC\n");

        SHPDestroyObject(so);
      }

      std::cout << "Closing shapefile" << std::endl;

      DBFClose(dbf);
      SHPClose(shp);
      return 0;
    }
	}
	catch (CDBException* e)
	{
		printf("* %s\n", (const char*)TString(e->m_strError));
		e->Delete();
	}
	catch (...)
	{
		printf(" *Unknown exception\n");
	}

	USAGE_ERROR:
		printf("Usage: TgrTransSHP...\n");
		printf("  <featNames> /n <.gdb filename> : Reads names from the Featue Names DBF file a <TGR*n.tab> with names in it\n");

	ERR_RETURN:
		return(-1);
}

static short GetStateFips(const char* string)
{
  wchar_t state[3];

  const char* pos;

  if ((pos = strstr(string, FILE_PREFIX)) == 0)
    return(0);

  int size = strlen(FILE_PREFIX);
  state[0] = pos[size];
  state[1] = pos[size + 1];
  state[2] = '\0';

  return((short)_wtoi(state));
}

static short GetCountyFips(const char* string)
{
  const char* pos;

  if ((pos = ::strstr(string, FILE_PREFIX)) == 0)
    return(0);

  pos += strlen(FILE_PREFIX) + 2;

  return((short)atoi(pos));
}

// Census Feature Code Class - based on 2006 classification codes
static TigerDB::Classification MapMTFCC(const char* mtfcc)
{
	int mtfccNum = atoi(&mtfcc[1]);
	TigerDB::Classification code = TigerDB::NotClassified;

	switch (mtfcc[0])
	{
	default:
		fprintf(stderr, "*MapMTFCC - invalid MTFCC Code: %c\n", mtfcc[0]);
		break;

	case 'C':
		switch (mtfccNum)
		{
		default:
			fprintf(stderr, "*MapMTFCC - Unknown MTFCC C: %d\n", mtfccNum);
			break;
		case 3022:
			code = TigerDB::PF_MountainPeak;
			break;
		case 3023:
			code = TigerDB::PF_Island;
			break;
		case 3024:
			code = TigerDB::PF_Levee;
			break;
		case 3026:
			code = TigerDB::PF_QuarryMine;
			break;
		case 3027:
			code = TigerDB::PF_Dam;
			break;
		case 3061:
			code = TigerDB::ROAD_Cul_de_sac;
			break;
		case 3062:
			code = TigerDB::ROAD_TrafficCircle;
			break;
		case 3066:
		case 3067:
			code = TigerDB::ROAD_BarrierToTravel;
			break;
		case 3071:
			code = TigerDB::LM_Tower;
			break;
		case 3074:
			code = TigerDB::LM_LighthouseBeacon;
			break;
		case 3075:
			code = TigerDB::LM_Tank;
			break;
		case 3076:
			code = TigerDB::LM_WindmillFarm;
			break;
		case 3077:
			code = TigerDB::LM_SolarFarm;
			break;
		case 3078:
			code = TigerDB::LM_MonumentMemorial;
			break;
		case 3079:
		case 3080:
			code = TigerDB::LM_SurveyBoundaryMemorial;
			break;
		}
		break;

	case 'G':
		switch (mtfccNum)
		{
		default:
			fprintf(stderr, "*MapMTFCC - Unknown MTFCC G: %d\n", mtfccNum);
			break;
		case 6350:
			code = TigerDB::NVF_ZIPCodeBoundary;
			break;
		}
		break;

	case 'H':
		switch (mtfccNum)
		{
		default:
			fprintf(stderr, "*MapMTFCC - Unknown MTFCC H: %d\n", mtfccNum);
			break;
		case 2030:
			code = TigerDB::HYDRO_PerennialLakeOrPond;
			break;
		case 2040:
			code = TigerDB::HYDRO_PerennialReservoir;
			break;
		case 2041:
			code = TigerDB::HYDRO_TreatmentPond;
			break;
		case 2051:
			code = TigerDB::HYDRO_BayEstuaryGulfOrSound;
			break;
		case 2053:
			code = TigerDB::HYDRO_SeaOrOcean;
			break;
		case 3010:
			code = TigerDB::HYDRO_PerennialStream;
			break;
		case 3013:
			code = TigerDB::HYDRO_BraidedStream;
			break;
		case 3020:
			code = TigerDB::HYDRO_PerennialCanalDitchOrAqueduct;
			break;
		}
		break;

	case 'K':
		switch (mtfccNum)
		{
		default:
			fprintf(stderr, "*MapMTFCC - Unknown MTFCC K: %d\n", mtfccNum);
			break;
		case 1225:
			code = TigerDB::LM_CrewOfVessel;
			break;
		case 1227:
			code = TigerDB::LM_HotelOrMotel;
			break;
		case 1228:
			code = TigerDB::LM_Campground;
			break;
		case 1229:
			code = TigerDB::LM_ShelterOrMission;
			break;
		case 1231:
			code = TigerDB::LM_Hospital;
			break;
		case 1233:
			code = TigerDB::LM_NursingHome;
			break;
		case 1236:
			code = TigerDB::LM_Jail;
			break;
		case 1237:
			code = TigerDB::LM_FederalOrStatePrison;
			break;
		case 1239:
			code = TigerDB::LM_ConventOrMonastery;
			break;
		case 2165:
			code = TigerDB::LM_GovernmentCenter;
			break;
		case 2167:
			code = TigerDB::LM_ConventionCenter;
			break;
		case 2180:		// Park
			code = TigerDB::LM_OpenSpace;
			break;
		case 2181:
			code = TigerDB::LM_NationalParkService;
			break;
		case 2182:
			code = TigerDB::LM_NationalForestOrOther;
			break;
		case 2184:
			code = TigerDB::LM_StateOrLocalPark_Forest;
			break;
		case 2195:
			code = TigerDB::LM_Library;
			break;
		case 2361:
			code = TigerDB::LM_ShoppingOrRetailCenter;
			break;
		case 2362:
			code = TigerDB::LM_IndustrialBuildingOrPark;
			break;
		case 2363:
			code = TigerDB::LM_OfficebuildingOrPark;
			break;
		case 2364:
			code = TigerDB::LM_VineyardWineryOrchard;
			break;
		case 2366:
			code = TigerDB::LM_Employmentcenter;
			break;
		case 2400:
			code = TigerDB::LM_TransportationTerminal;
			break;
		case 2451:
			code = TigerDB::LM_Airport;
			break;
		case 2452:
			code = TigerDB::LM_TrainStation;
			break;
		case 2453:
			code = TigerDB::LM_BusTerminal;
			break;
		case 2454:
			code = TigerDB::LM_MarineTerminal;
			break;
		case 2455:
			code = TigerDB::LM_SeaplaneAnchorage;
			break;
		case 2457:
			code = TigerDB::LM_AirportStatisticalRepresentation;
			break;
		case 2459:
			code = TigerDB::ROAD_Runway_Taxiway;
			break;
		case 2545:
			code = TigerDB::LM_Museum;
			break;
		case 2561:
			code = TigerDB::LM_GolfCourse;
			break;
		case 2582:
			code = TigerDB::LM_Cemetery;
			break;
		case 2586:
			code = TigerDB::LM_Zoo;
			break;
		}
		break;

	case 'L':
		switch (mtfccNum)
		{
		default:
			fprintf(stderr, "*MapMTFCC - Unknown MTFCC L: %d\n", mtfccNum);
			break;
		case 4010:
			code = TigerDB::MGT_Pipeline;
			break;
		case 4020:
			code = TigerDB::MGT_PowerLine;
			break;
		case 4031:
			code = TigerDB::MGT_AerialTramway;
			break;
		case 4110:
			code = TigerDB::PF_Fenceline;
			break;
		case 4121:
			code = TigerDB::PF_RidgeLine;
			break;
		case 4140:
			code = TigerDB::NVF_PropertyLine;
			break;
		case 4150:	// Coastline
			code = TigerDB::HYDRO_WaterBoundaryInlandVsCoastal;
			break;
		case 4165:
			code = TigerDB::ROAD_FerryCrossing;
			break;
		}
		break;

	case 'P':
		switch (mtfccNum)
		{
		default:
			fprintf(stderr, "*MapMTFCC - Unknown MTFCC P: %d\n", mtfccNum);
			break;
		case 1:
			code = TigerDB::NVF_LegalOrAdministrativeBoundary;
			break;
		case 2:
			code = TigerDB::HYDRO_PerennialShoreline;
			break;
		case 3:
			code = TigerDB::HYDRO_IntermittentShoreline;
			break;
		case 4:  // Other non-visible edge (need a new code)
			code = TigerDB::HYDRO_USGSClosureLine;
			break;
		}
		break;

	case 'R':
		switch (mtfccNum)
		{
		default:
			fprintf(stderr, "*MapMTFCC - Unknown MTFCC R: %d\n", mtfccNum);
			break;
		case 1011:
			code = TigerDB::RR_MainLine;
			break;
		}
		break;

	case 'S': // Street Type
		switch (mtfccNum)
		{
		default:
			fprintf(stderr, "*MapMTFCC - Unknown MTFCC S: %d\n", mtfccNum);
			break;
		case 1100:
			code = TigerDB::ROAD_PrimaryLimitedAccess;
			break;
		case 1200:
			code = TigerDB::ROAD_SecondaryAndConnecting;
			break;
		case 1400:
			code = TigerDB::ROAD_SecondaryAndConnecting;
			break;
		case 1500:
			code = TigerDB::ROAD_VehicularTrail;
			break;
		case 1630:
			code = TigerDB::ROAD_AccessRamp;
			break;
		case 1640:
			code = TigerDB::ROAD_ServiceDrive;
			break;
		case 1710:
		case 1720:
		case 1730:
		case 1740:
			code = TigerDB::ROAD_OtherThoroughfare;
			break;
		case 1750:
			code = TigerDB::ROAD_InternalUSCensusBureau;
			break;
		case 1780:
			code = TigerDB::LM_ParkAndRide;
			break;
		case 1820:
			code = TigerDB::ROAD_VehicularTrail;
			break;
		}
		break;
	}

	return(code);
}
