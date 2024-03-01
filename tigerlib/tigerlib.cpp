//
//	tigerlib.cpp - has some general purposed methods for processing Tiger/Line files (2006).
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
#include <string.h>
#include "tigerlib.h"

static const char *CFCCcodes[] = { "",
  "A00", "A01", "A02", "A03", "A04", "A05", "A06", "A07", "A08", "A10",
  "A11", "A12", "A13", "A14", "A15", "A16", "A17", "A18", "A20", "A21",
  "A22", "A23", "A24", "A25", "A26", "A27", "A28", "A30", "A31", "A32",
  "A33", "A34", "A35", "A36", "A37", "A38", "A40", "A41", "A42", "A43",
  "A44", "A45", "A46", "A47", "A48", "A50", "A51", "A52", "A53", "A60",
  "A61", "A62", "A63", "A64", "A65", "A70", "A71", "A72", "A73", "B00",
  "B01", "B02", "B03", "B10", "B11", "B12", "B13", "B20", "B21", "B22",
  "B23", "B30", "B31", "B32", "B33", "B40", "B50", "B51", "B52", "C00",
  "C10", "C20", "C30", "C31", "D00", "D50", "D51", "E00", "E10", "E20",
  "E21", "F00", "F10", "F11", "F12", "F13", "F20", "F21", "F22", "F23",
  "F30", "F40", "F50", "F60", "F70", "F71", "F72", "F73", "F74", "H00",
  "H01", "H02", "H10", "H11", "H12", "H13", "H20", "H21", "H22", "H70",
  "H71", "H72", "H73", "H74", "H75", "P00", "P01", "P02", "X00", "D10",
  "D20", "D21", "D22", "D23", "D24", "D25", "D26", "D27", "D28", "D29",
  "D30", "D31", "D32", "D33", "D34", "D35",	"D36", "D37", "D40", "D41",
  "D42", "D43", "D44", "D52", "D53", "D54", "D55", "D60", "D61", "D62",
  "D63", "D64", "D65", "D66", "D70", "D71", "D80", "D81", "D82", "D83",
  "D84", "D85", "D90", "D91", "E22", "F14", "F15", "F16", "F24", "F25",
  "F80", "F81", "F82", "H30", "H31", "H32", "H40", "H41", "H42", "H50",
  "H51", "H53", "H60", "H80", "H81" 
};

int CfccToDfcc( const char *cfcc )
{
  for( int i = 1; i < sizeof( CFCCcodes ) / sizeof( CFCCcodes[0] ); i++ )
  {
    if( strcmp( CFCCcodes[ i ], cfcc ) == 0 )
    {
	  return( i );
    }
  }
  return( 0 );
}

void TigerToGeoPoint(	long tlon, long tlat, double *x, double *y )
{
  *x = (double)tlon / 1000000.0,
	*y = (double)tlat / 1000000.0;

//  Nad27ToWGS84( &dlat, &dlon );
}


struct FieldAbbrev {
	char *field;
	char *abbrev;
};

static FieldAbbrev directionals[] =
{
	{ "EAST",				"E" },
	{ "E",					"E" },
	{ "NORTH",			"N" },
	{ "N",					"N" },
	{ "NORTHEAST",	"NE" },
	{ "NE",					"NE" },
	{ "NORTHWEST",	"NW" },
	{ "NW",					"NW" },
	{ "SOUTH",			"S" },
	{ "S",					"S" },
	{ "SOUTHEAST",	"SE" },
	{ "SE",					"SE" },
	{ "SOUTHWEST",	"SW" },
	{ "SW",					"SW" },
	{ "W",					"W" },
	{ "WEST",				"W" }
};
 

static FieldAbbrev TgrAbbrevs[] =
{
	{ "AL",					"Aly" },
	{ "BEND",				"Bnd" },
	{ "BRDG",				"Brg" },
	{ "COVE",				"Cv" },
	{ "CRSG",				"Xing" },
	{ "EXWY",				"Expy" },
	{ "FRWY",				"Fwy" },
	{ "PKWY",				"Pky" },
	{ "LANE",				"Ln" },
	{ "CAMP",				"Cp" },
	{ "CAPE",				"Cpe" },
	{ "FORK",				"Frk" },
	{ "LAKE",				"Lk" },
	{ "ROAD",				"Rd" }
};

static FieldAbbrev types[] =
{
	{ "ACCESS",			"Acss" },		// MEC
	{ "ACRE",				"Acre" },
	{ "ACRES",			"Acrs" },		// MEC
	{ "ACRS",				"Acrs" },		// MEC
	{ "AIRFIELD",		"Arfd" },		// MEC
	{ "AIRSTRIP",		"Arsp" },		// MEC
	{ "AIRPORT",		"Arpt" },		// MEC
	{ "ALLEE",			"Aly" },
	{ "ALLEY",			"Aly" },
	{ "ALY",				"Aly" },
	{ "ANEX",				"Anx" },
	{ "ANNEX",			"Anx" },
	{ "ANX",				"Anx" },
	{ "ANNX",				"Anx" },		// MEC
	{ "APARTMENTS",	"Apts" },		// MEC
	{ "APTS",				"Apts" },		// MEC
	{ "ARCADE",			"Arc" },
	{ "ARC",				"Arc" },
	{ "AVENUE",			"Ave" },
	{ "AVENIDA"			"Ave" },		// Tiger
	{ "AV",					"Ave" },
	{ "AVE",				"Ave" },
	{ "AVN",				"Ave" },
	{ "AVEN",				"Ave" },
	{ "AVENU",			"Ave" },
	{ "AVNUE",			"Ave" },
	{ "BASIN",			"Bsn" },		// MEC
	{ "BAY",				"Bay" },		// MEC
	{ "BAYOO",			"Byu" },
	{ "BAYOU",			"Byu" },
	{ "BYU",				"Byu" },
	{ "BEACH",			"Bch" },
	{ "BCH",				"Bch" },
	{ "BEND",				"Bnd" },
	{ "BND",				"Bnd" },
	{ "BLUFF",			"Blf" },
	{ "BLUF",				"Blf" },
	{ "BLF",				"Blf" },
	{ "BLFF",				"Blf" },
	{ "BLUFFS",			"Blfs" },
	{ "BOG",				"Bog" },		// MEc
	{ "BOTTOM",			"Btm" },
	{ "BOTTM",			"Btm" },
	{ "BTM",				"Btm" },
	{ "BOUNDARY",		"Bdry" },		// MEC
	{ "BOULEVARD",	"Blvd" },
	{ "BLVD",				"Blvd" },
	{ "BOUL",				"Blvd" },
	{ "BOULV",			"Blvd" },
	{ "BRANCH",			"Br" },
	{ "BR",					"Br" },
	{ "BRNCH",			"Br" },
	{ "BRDGE",			"Brg" },
	{ "BRIDGE",			"Brg" },
	{ "BRG",				"Brg" },
	{ "BROOK",			"Br"/*"Brk"*/ },		// Tiger 97 has this type wrong
	{ "BROO",				"Brk" },
	{ "BRK",				"Brk" },
	{ "BROOKS",			"Brks" },
	{ "BURG",				"Bg" },
	{ "BURGS",			"Bgs" },
	{ "BG",					"Bg" },
	{ "BYPASS",			"Byp" },
	{ "BYPA",				"Byp" },
	{ "BYPAS",			"Byp" },
	{ "BYP",				"Byp" },
	{ "BYPS",				"Byp" },
	{ "CAMP",				"Cp" },
	{ "CMP",				"Cp" },
	{ "CP",					"Cp" },
	{ "CANAL",			"Cnl" },		// MEC
	{ "CANYN",			"Cyn" },
	{ "CANYON",			"Cyn" },
	{ "CYN",				"Cyn" },
	{ "CNYN",				"Cyn" },
	{ "CAPE",				"Cpe" },
	{ "CPE",				"Cpe" },
	{ "CAUSEWAY",		"Cswy" },
	{ "CAUSWAY",		"Cswy" },
	{ "CSWY",				"Cswy" },
	{ "CSW",				"Cswy" },
	{ "CEN",				"Ctr" },
	{ "CENT",				"Ctr" },
 	{ "CENTER",			"Ctr" },
 	{ "CENTR",			"Ctr" },
 	{ "CENTRE",			"Ctr" },
	{ "CTR",				"Ctr" },
	{ "CNTR",				"Ctr" },
	{ "CNTER",			"Ctr" },
 	{ "CENTERS",		"Ctrs" },
	{ "CIR",				"Cir" },
	{ "CIRC",				"Cir" },
	{ "CIRCL",			"Cir" },
	{ "CRCL",				"Cir" },
	{ "CRCLE",			"Cir" },
	{ "CIRCLE",			"Cir" },
	{ "CIRCLES",		"Cirs" },
	{ "CLF",				"Clf" },
	{ "CLIFF",			"Clf" },
	{ "CLIFFS",			"Clfs" },
	{ "CLFS",				"Clfs" },
	{ "CLUB",				"Clb" },
	{ "CLB",				"Clb" },
	{ "COMMON",			"Cmn" },			// MEC
	{ "COMMONS",		"Cmns" },			// MEC
	{ "CONDOMINIUM",		"Cndo" },	// MEC
	{ "CONDOMINIUMS",		"Cnds" },	// MEC
	{ "CONNECTOR",		"Cntr" },		// MEC
	{ "CORNER",			"Cor" },
	{ "COR",				"Cor" },
	{ "CORNERS",		"Cors" },
	{ "CORS",				"Cors" },
	{ "COTTAGES",		"Ctgs" },		// MEC
	{ "COURSE",			"Crse" },
	{ "CRSE",				"Crse" },
	{ "COURT",			"Ct" },
	{ "CRT",				"Ct" },
	{ "CT",					"Ct" },
	{ "COURTS",			"Cts" },
	{ "CTS",				"Cts" },
	{ "COVE",				"Cv" },
	{ "CV",					"Cv" },
	{ "COVES",			"Cvs" },
	{ "CREEK",			"Crk" },
	{ "CRK",				"Crk" },
	{ "CR",					"Crk" },
	{ "CK",					"Crk" },
	{ "CRECENT",		"Cres" },
	{ "CRESCENT",		"Cres" },
	{ "CRES",				"Cres" },
	{ "CRESENT",		"Cres" },
	{ "CREST",			"Crst" },		// MEC
	{ "CRSCNT",			"Cres" },
	{ "CRSENT",			"Cres" },
	{ "CRSNT",			"Cres" },
	{ "CREST",			"Crst" },
	{ "CROSSING",		"Xing" },
	{ "CRSSING",		"Xing" },
	{ "CRSSNG",			"Xing" },
	{ "XING",				"Xing" },
	{ "CROSSROAD",	"Xrd" },
	{ "CURVE",			"Curv" },
	{ "DALE",				"Dl" },
	{ "DL",					"Dl" },
	{ "DAM",				"Dm" },
	{ "DM",					"Dm" },
	{ "DIV",				"Dv" },
	{ "DIVIDE",			"Dv" },
	{ "DV",					"Dv" },
	{ "DVD",				"Dv" },
	{ "DRIVE",			"Dr" },
	{ "DR",					"Dr" },
	{ "DRIV",				"Dr" },
	{ "DRV",				"Dr" },
	{ "DRIVES",			"Drs" },
	{ "DRIVEWAY",		"Drwy" },		// MEC

	{ "ENTRANCE",		"Entr" },		// MEC
	{ "ESTATES",		"Ests" },
	{ "ESTS",				"Ests" },
	{ "ESTATE",			"Est" },
	{ "EST",				"Est" },
	{ "EXP",				"Expy" },
	{ "EXPR",				"Expy" },
	{ "EXPRESS",		"Expy" },
	{ "EXPRESSWAY",	"Expy" },
	{ "EXPY",				"Expy" },
	{ "EXTENSION",	"Ext" },
	{ "EXTENDED",		"Ext" },
	{ "EXT",				"Ext" },
	{ "EXTN",				"Ext" },
	{ "EXTNSN",			"Ext" },
	{ "EXTENSIONS",	"Exts" },
	{ "EXTS",				"Exts" },
	{ "FAIRGROUNDS",	"Frgd" },		// MEC

	{ "FALL",				"Fall" },
	{ "FALLS",			"Fls" },
	{ "FALS",				"Fls" },
	{ "FLS",				"Fls" },
	{ "FERRY",			"Fry" },
	{ "FRRY",				"Fry" },
	{ "FRY",				"Fry" },
	{ "FIELD",			"Fld" },
	{ "FLD",				"Fld" },
	{ "FID",				"Fld" },
	{ "FIELDS",			"Flds" },
	{ "FLDS",				"Flds" },
	{ "FLAT",				"Flt" },
	{ "FLT",				"Flt" },
	{ "FLATS",			"Flts" },
	{ "FLTS",				"Flts" },
	{ "FLOWAGE",		"Flwg" },		// MEC
	{ "FORD",				"Frd" },
	{ "FRD",				"Frd" },
	{ "FORDS",			"Frds" },
	{ "FOREST",			"Frst" },
	{ "FORESTS",		"Frst" },
	{ "FRST",				"Frst" },
	{ "FORG",				"Frg" },
	{ "FORGE",			"Frg" },
	{ "FRG",				"Frg" },
	{ "FORGES",			"Frgs" },
	{ "FORK",				"Frk" },
	{ "FRK",				"Frk" },
	{ "FORKS",			"Frks" },
	{ "FRKS",				"Frks" },
	{ "FORT",				"Ft" },
	{ "FRT",				"Ft" },
	{ "FT",					"Ft" },
	{ "FREEWAY",		"Fwy" },
	{ "FREEWY",			"Fwy" },
	{ "FRWAY",			"Fwy" },
	{ "FWY",				"Fwy" },

	{ "GARDEN",			"Gdn" },
	{ "GARDN",			"Gdn" },
	{ "GDN",				"Gdn" },
	{ "GRDEN",			"Gdn" },
	{ "GRDN",				"Gdn" },
	{ "GARDENS",		"Gdns" },
	{ "GDNS",				"Gdns" },
	{ "GRDNS",			"Gdns" },
	{ "GATEWAY",		"Gtwy" },
	{ "GATEWY",			"Gtwy" },
	{ "GATWAY",			"Gtwy" },
	{ "GTWY",				"Gtwy" },
	{ "GLADE",			"Gld" },		// MEC
	{ "GLEN",				"Gln" },
	{ "GLN",				"Gln" },
	{ "GLENS",			"Glns" },
	{ "GREEN",			"Grn" },
	{ "GRADE",			"Grd" },		// MEC 2/27/00
	{ "GRN",				"Grn" },
	{ "GREENS",			"Grns" },
	{ "GROVE",			"Grv" },
	{ "GROV",				"Grv" },
	{ "GRV",				"Grv" },
	{ "GROVES",			"Grvs" },

	{ "HARBOR",			"Hbr" },
	{ "HARB",				"Hbr" },
	{ "HARBR",			"Hbr" },
	{ "HBR",				"Hbr" },
	{ "HRBOR",			"Hbr" },
	{ "HARBORS",		"Hbrs" },
	{ "HAVEN",			"Hvn" },
	{ "HAVN",				"Hvn" },
	{ "HVN",				"Hvn" },
	{ "HEATH",			"Hth" },		// MEC
	{ "HEIGHT",			"Hts" },
	{ "HEIGHTS",		"Hts" },
	{ "HGTS",				"Hts" },
	{ "HTS",				"Hts" },
	{ "HT",					"Hts" },
	{ "HIGHWAY",		"Hwy" },
	{ "HIGHWY",			"Hwy" },
	{ "HIWAY",			"Hwy" },
	{ "HIWY",				"Hwy" },
	{ "HWAY",				"Hwy" },
	{ "HWY",				"Hwy" },
	{ "HILL",				"Hl" },
	{ "HL",					"Hl" },
	{ "HILLS",			"Hls" },
	{ "HLS",				"Hls" },
	{ "HOLLOW",			"Holw" },
	{ "HOLLOWS",		"Holw" },
	{ "HLLW",				"Holw" },
	{ "HOLW",				"Holw" },
	{ "HOLWS",			"Holw" },

	{ "INLET",			"Inlt" },
	{ "INLT",				"Inlt" },
	{ "ISLAND",			"Is" },
	{ "IS",					"Is" },
	{ "ISLND",			"Is" },
	{ "ISLANDS",		"Iss" },
	{ "ISLNDS",			"Iss" },
	{ "ISS",				"Iss" },
	{ "ISLE",				"Isle" },
	{ "ISLES",			"Isle" },

	{ "JUNCTION",		"Jct" },
	{ "JCTION",			"Jct" },
	{ "JCTN",				"Jct" },
	{ "JUNCTN",			"Jct" },
	{ "JUNCTON"			"Jct" },
	{ "JCT",				"Jct" },
	{ "JUNCTIONS",	"Jcts" },
	{ "JCTNS",			"Jcts" },
	{ "JCTS",				"Jcts" },

	{ "KEY",				"Ky" },
	{ "KY",					"Ky" },
	{ "KEYS",				"Kys" },
	{ "KYS",				"Kys" },
	{ "KNOLL",			"Knl" },
	{ "KNL",				"Knl" },
	{ "KNOL",				"Knl" },
	{ "KNOLLS",			"Knls" },
	{ "KNLS",				"Knls" },

	{ "LAKE",				"Lk" },
	{ "LK",					"Lk" },
	{ "LAKES",			"Lks" },
	{ "LKS",				"Lks" },
	{ "LAND",				"Land" },
	{ "LANDING",		"Lndg" },
	{ "LNDG",				"Lndg" },
	{ "LNDNG",			"Lndg" },
	{ "LA",					"Ln" },
	{ "LANE",				"Ln" },
	{ "LANES",			"Ln" },
	{ "LN",					"Ln" },
	{ "LEDGES",			"Ldgs" },		// MEC
	{ "LIGHT",			"Lgt" },
	{ "LGT",				"Lgt" },
	{ "LIGHTS",			"Lgts" },
	{ "LINE",				"Line" },		// MEC
	{ "LOAF",				"Lf" },
	{ "LF",					"Lf" },
	{ "LOCK",				"Lck" },
	{ "LCK",				"Lck" },
	{ "LOCKS",			"Lcks" },
	{ "LCKS",				"Lcks" },
	{ "LODGE",			"Ldg" },
	{ "LDGE",				"Ldg" },
	{ "LODG",				"Ldg" },
	{ "LDG",				"Ldg" },
	{ "LOOP",				"Loop" },
	{ "LOOPS",			"Loop" },

	{ "MALL",				"Mall" },
	{ "MANOR",			"Mnr" },
	{ "MNR",				"Mnr" },
	{ "MANORS",			"Mnrs" },
	{ "MNRS",				"Mnrs" },
	{ "MARSH",			"Mrsh" },		// MEC
	{ "MEADOW",			"Mdw" },
	{ "MDW",				"Mdw" },
	{ "MEADOWS",		"Mdws" },
	{ "MDWS",				"Mdws" },
	{ "MEDOWS",			"Mdws" },
	{ "MEWS",				"Mews" },
	{ "MILL",				"Ml" },
	{ "ML",					"Ml" },
	{ "MILLS",			"Mls" },
	{ "MLS",				"Mls" },
	{ "MISSION",		"Msn" },
	{ "MISSN",			"Msn" },
	{ "MSN",				"Msn" },
	{ "MSSN",				"Msn" },
	{ "MOTORWAY",		"Mtwy" },		// Tiger
	{ "MTWY",				"Mtwy" },		// Tiger
	{ "MOUNT",			"Mt" },
	{ "MT",					"Mt" },
	{ "MNT",				"Mt" },
	{ "MOUNTAIN",		"Mtn" },
	{ "MNTAIN",			"Mtn" },
	{ "MNTN",				"Mtn" },
	{ "MOUNTIN",		"Mtn" },
	{ "MTIN",				"Mtn" },
	{ "MTN",				"Mtn" },
	{ "MOUNTAINS",	"Mtns" },
	{ "MNTNS",			"Mtns" },

	{ "NECK",				"Nck" },
	{ "NCK",				"Nck" },

	{ "ORCHARD",		"Orch" },
	{ "ORCH",				"Orch" },
	{ "ORCHRD",			"Orch" },
	{ "OUTLET",			"Otlt" },		// MEC
	{ "OVAL",				"Oval" },
	{ "OVL",				"Oval" },
	{ "OVERLOOK",		"Ovlk" },		// MEC
	{ "OVERPASS",		"Opas" },
	{ "OVPS",				"Opas" },		//Tiger

	{ "PARK",				"Park" },
	{ "PK",					"Park" },
	{ "PRK",				"Park" },
	{ "PARKWAY",		"Pkwy" },
	{ "PARKWY",			"Pkwy" },
	{ "PKWAY",			"Pkwy" },
	{ "PKWY",				"Pkwy" },
	{ "PKY",				"Pkwy" },
	{ "PARKWAYS",		"Pkwy" },
	{ "PKWYS",			"Pkwy" },
	{ "PASS",				"Pass" },
	{ "PASSAGE",		"Psge" },
	{ "PATH",				"Path" },
	{ "PATHS",			"Path" },
	{ "PIER",				"Pier" },		// MEC
	{ "PIKE",				"Pike" },
	{ "PIKES",			"Pike" },
	{ "PINE",				"Pne" },
	{ "PINES",			"Pnes" },
	{ "PNES",				"Pnes" },
	{ "PLACE",			"Pl" },
	{ "PL",					"Pl" },
	{ "PLAIN",			"Pln" },
	{ "PLN",				"Pln" },
	{ "PLAINES",		"Plns" },
	{ "PLAINS",			"Plns" },
	{ "PLNS",				"Plns" },
	{ "PLAZA",			"Plz" },
	{ "PLZ",				"Plz" },
	{ "PLZA",				"Plz" },
	{ "POINT",			"Pt" },
	{ "PT",					"Pt" },
	{ "POINTS",			"Pts" },
	{ "PTS",				"Pts" },
	{ "POND",				"Pnd" },		// MEC
	{ "PON",				"Pnd" },		// MEC
	{ "PND",				"Pnd" },		// MEC
	{ "PONDS",			"Pnds" },		// MEC
	{ "PORT",				"Prt" },
	{ "PRT",				"Prt" },
	{ "PORTS",			"Prts" },
	{ "PRTS",				"Prts" },
	{ "PRESERVE",		"Prsv" },		// MEC
	{ "PROMENADE",	"Prmd" },		// MEC
	{ "PRAIRIE",		"Pr" },
	{ "PRARIE",			"Pr" },
	{ "PPR",				"Pr" },
	{ "PR",					"Pr" },

	{ "RAD",				"Radl" },
	{ "RADIAL",			"Radl" },
	{ "RADIEL",			"Radl" },
	{ "RADL",				"Radl" },
	{ "RAILROAD",		"RR" },			// MEC
	{ "RAILWAY",		"RR" },			// MEC
	{ "RR",					"RR" },			// MEC
	{ "RAMP",				"Ramp" },
	{ "RANCH",			"Rnch" },
	{ "RNCH",				"Rnch" },
	{ "RANCHES",		"Rnch" },
	{ "RNCHS",			"Rnch" },
	{ "RAPID",			"Rpd" },
	{ "RPD",				"Rpd" },
	{ "RAPIDS",			"Rpds" },
	{ "RPDS",				"Rpds" },
	{ "REST",				"Rst" },
	{ "RST",				"Rst" },
	{ "RESERVOIR",	"Rsvr" },		// MEC
	{ "RIDGE",			"Rdg" },
	{ "RDGE",				"Rdg" },
	{ "RDG",				"Rdg" },
	{ "RIDGES",			"Rdgs" },
	{ "RDGS",				"Rdgs" },
	{ "RIVER",			"Riv" },
	{ "RIVR",				"Riv" },
	{ "RIV",				"Riv" },
	{ "RVR",				"Riv" },
	{ "RIVE",				"Riv" },		// Mis-spelling
	{ "ROAD",				"Rd" },
	{ "RD",					"Rd" },
	{ "ROADS",			"Rds" },
	{ "RDS",				"Rds" },
	{ "ROUTE",			"Rte" },
	{ "ROW",				"Row" },
	{ "RUN",				"Run" },
	{ "RUE",				"Rue" },

	{ "SCHOOL",			"Schl" },		// MEC
	{ "SHOAL",			"Shl" },
	{ "SHL",				"Shl" },
	{ "SHOALS",			"Shls" },
	{ "SHLS",				"Shls" },
	{ "SHOAR",			"Shr" },
	{ "SHORE",			"Shr" },
	{ "SHR",				"Shr" },
	{ "SHOARS",			"Shrs" },
	{ "SHORES",			"Shrs" },
	{ "SHRS",				"Shrs" },
	{ "SIDING",			"Sdg" },		// MEC
	{ "SKYWAY",			"Skwy" },
	{ "SKYWY",			"Skwy" },		// Tiger
	{ "SPRING",			"Spg" },
	{ "SPRNG",			"Spg" },
	{ "SPNG",				"Spg" },
	{ "SPG",				"Spg" },
	{ "SPRINGS",		"Spgs" },
	{ "SPRNGS",			"Spgs" },
	{ "SPGS",				"Spgs" },
	{ "SPNGS",			"Spgs" },
	{ "SPUR",				"Spur" },
	{ "SPURS",			"Spur" },
	{ "STRIP",			"Strp" },		// MEC
	{ "SQUARE",			"Sq" },
	{ "SQ",					"Sq" },
	{ "SQR",				"Sq" },
	{ "SQRE",				"Sq" },
	{ "SQU",				"Sq" },
	{ "SQUARES",		"Sqs" },
	{ "SQRS",				"Sqs" },
	{ "STATION",		"Sta" },
	{ "STATN",			"Sta" },
	{ "STA",				"Sta" },
	{ "STN",				"Sta" },
	{ "STRA",				"Stra" },
	{ "STRAV",			"Stra" },
	{ "STRAVE",			"Stra" },
	{ "STRAVEN",		"Stra" },
	{ "STRAVENUE",	"Stra" },
	{ "STRAVN",			"Stra" },
	{ "STRVN",			"Stra" },
	{ "STRVNUE",		"Stra" },
	{ "STREAM",			"St"/*"Strm"*/ },		// Tiger 97 has this type wrong
	{ "STRM",				"Strm" },
	{ "STREME",			"Strm" },
	{ "STREA",			"Strm" },
	{ "SREAM",			"Strm" },		// spp.
	{ "STR",				"Strm" },
	{ "STREET",			"St" },
	{ "STR",				"St" },
	{ "ST",					"St" },
	{ "STRT",				"St" },
	{ "STREETS",		"Sts" },
//	{ "STR",				"Strm" },
	{ "SUMMIT",			"Smt" },
	{ "SUMIT",			"Smt" },
	{ "SUMITT",			"Smt" },
	{ "SMT",				"Smt" },
	{ "SWAMP",			"Swmp" },		// MEC

	{ "TERRACE",		"Ter" },
	{ "TERR",				"Ter" },
	{ "TER",				"Ter" },
	{ "THROUGHWAY",	"Trwy" },
	{ "THWY",				"Trwy" },		// Tiger
	{ "TRAFFICWAY",	"Tfwy" },
	{ "TFWY",				"Tfwy" },
	{ "TRFY",				"Tfwy" },
	{ "TRACE",			"Trce" },
	{ "TRACES",			"Trce" },
	{ "TRCE",				"Trce" },
	{ "TRACK",			"Trak" },
	{ "TRACKS",			"Trak" },
	{ "TRKS",				"Trak" },
	{ "TRAK",				"Trak" },
	{ "TRAIL",			"Trl" },
	{ "TRL",				"Trl" },
	{ "TR",					"Trl" },
	{ "TRAILS",			"Trl" },
	{ "TRLS",				"Trl" },
	{ "TRAILER",		"Trlr" },		// ??
	{ "TRLR",				"Trlr" },		// ??
	{ "TUNEL",			"Tunl" },
	{ "TUNNEL",			"Tunl" },
	{ "TUNNL",			"Tunl" },
	{ "TUNL",				"Tunl" },
	{ "TUNLS",			"Tunl" },
	{ "TUNNELS",		"Tunl" },
	{ "TURNPIKE",		"Tpke" },
	{ "TPKE",				"Tpke" },
	{ "TPK",				"Tpke" },
	{ "TRNPK",			"Tpke" },
	{ "TRPK",				"Tpke" },
	{ "TURNPK",			"Tpke" },

	{ "UNION",			"Un" },
	{ "UN",					"Un" },
	{ "UNIONS",			"Uns" },
	{ "UNDERPASS",	"Upas" },
	{ "UNP",				"Unp" },		// Tiger

	{ "VALLEY",			"Vly" },
	{ "VALLY",			"Vly" },
	{ "VLLY",				"Vly" },
	{ "VLY",				"Vly" },
	{ "VALLEYS",		"Vlys" },
	{ "VLYS",				"Vlys" },
	{ "VIADUCT",		"Via" },
	{ "VIA",				"Via" },
	{ "VDCT",				"Via" },
	{ "VIADCT",			"Via" },
	{ "VIEW",				"Vw" },
	{ "VW",					"Vw" },
	{ "VIEWS",			"Vws" },
	{ "VWS",				"Vws" },
	{ "VILL",				"Vlg" },
	{ "VILLAG",			"Vlg" },
	{ "VILLG",			"Vlg" },
	{ "VILLAGE",		"Vlg" },
	{ "VILLIAGE",		"Vlg" },
	{ "VLG",				"Vlg" },
	{ "VILLAGES",		"Vlgs" },
	{ "VLGS",				"Vlgs" },
	{ "VILLE",			"Vl" },
	{ "VL",					"Vl" },
	{ "VISTA",			"Vis" },
	{ "VIS",				"Vis" },
	{ "VST",				"Vis" },
	{ "VSTA",				"Vis" },
	{ "VIST",				"Vis" },

	{ "WALK",				"Walk" },
	{ "WALKS",			"Walk" },
	{ "WALL",				"Wall" },		// Tiger
	{ "WAY",				"Way" },
	{ "WY",					"Way" },
	{ "WAYS",				"Ways" },
	{ "WELL",				"Wl" },
	{ "WELLS",			"Wls" },
	{ "WLS",				"Wls" },
	{ "WHARF",			"Whrf" },	// MEC
	{ "WOODS",			"Wds" }		// MEC
};

static FieldAbbrev specials[] =
{
	{ "FIRE LANE ",			"Fire Ln " },
	{ "FIRE ROAD ",			"Fire Ln " },
	{ "FIREROAD ",			"Fire Ln " },
	{ "FIRE RD ",				"Fire Ln " },
	{ "STATE HIGHWAY ",	"State Hwy " },
	{ "STATE ROUTE ",		"State Hwy " },
	{ "STATE RTE ",			"State Hwy " },
	{ "STATE RT ",			"State Hwy " },
	{ "ST RTE ",				"State Hwy " },
	{ "ST RT ",					"State Hwy " },
	{ "ST ROUTE ",			"State Hwy " },
	{ "US HIGHWAY ",		"US Hwy " },
	{ "US RT ",					"US Hwy " },
	{ "US RTE ",				"US Hwy " },
	{ "US ROUTE ",			"US Hwy " },
	{ "OLD ROUTE ",			"Old Rte " },
	{ "OLD RT ",				"Old Rte " },
	{ "ROAD NUMBER ",		"Road No " },
	{ " (BYPASS)",			" BYPASS" },
	{ " (ALT)",					"A" }
};

bool FixFeatureName(FeatureName &fn)
{
	int i;
	bool changed = false;
  CString name = fn.fename,
				  work;
	int start,
			length = name.GetLength();
	long nNames = 0,
			 nPrefixes = 0,
			 nSuffixes = 0,
			 nTypes = 0;

	work = name;
	work.MakeUpper();
//
//	Look for specials in the name
	for( i = 0; i < sizeof( specials ) / sizeof( specials[ 0 ]  ); i++ )
	{
		int pos;

		if( ( pos = work.Find( specials[ i ].field ) ) != -1 )
		{
			int size = ::strlen( specials[ i ].field );

			if( pos > 0 )
				work = name.Left( pos /*+ 1*/ );
			else
				work = _T("");

			work += specials[ i ].abbrev;
			work += name.Right( length - size - pos );
			name = work;
			length = name.GetLength();
			work.MakeUpper();
			nNames++;
			changed = true;
		}
	}

//
//	Look for suffixes
	if( fn.fedirs[ 0 ] == '\0' )
	{
		if( ( start = name.ReverseFind( ' ' ) ) != -1 )
		{
			work = name.Right( length - start - 1 );
			work.MakeUpper();
			for( i = 0; i < sizeof( directionals ) / sizeof( directionals[ 0 ]  ); i++ )
			{
				if( ::strcmp( work, directionals[ i ].field ) == 0 )
				{
					::strcpy( fn.fedirs, directionals[ i ].abbrev );
					name = name.Left( start );
					length = name.GetLength();
					changed = true;
					nSuffixes++;
					break;
				}
			}
		}
	}		
//
//	Correct any types that are wrong
	if( fn.fetype[ 0 ] != '\0' )
	{
		work = fn.fetype;
		work.MakeUpper();
		for( i = 0; i < sizeof( TgrAbbrevs ) / sizeof( TgrAbbrevs[ 0 ]  ); i++ )
		{
			if( ::strcmp( work, TgrAbbrevs[ i ].field ) == 0 )
			{
				::strcpy( fn.fetype, TgrAbbrevs[ i ].abbrev );
				nTypes++;
				break;
			}
		}
	}
//
//	If type is in the name field, move it to type
	else
	{
		if( ( start = name.ReverseFind( ' ' ) ) != -1 )
		{
			work = name.Right( length - start - 1 );
			work.MakeUpper();
			for( int i = 0; i < sizeof( types ) / sizeof( types[ 0 ]  ); i++ )
			{
				if( ::strcmp( work, types[ i ].field ) == 0 )
				{
					::strcpy( fn.fetype, types[ i ].abbrev );
					name = name.Left( start );
					changed = true;
					length = start;
					nTypes++;
					break;
				}
			}
		}
		length = name.GetLength();
	}
//
//	Look for prefixes
	if( fn.fedirp[ 0 ] == '\0' )
	{
		if( ( start = name.Find( ' ' ) ) != -1 )
		{
			work = name.Left( start );
			work.MakeUpper();
			for( i = 0; i < sizeof( directionals ) / sizeof( directionals[ 0 ]  ); i++ )
			{
				if( ::strcmp( work, directionals[ i ].field ) == 0 )
				{
					::strcpy( fn.fedirp, directionals[ i ].abbrev );
					name = name.Right( length - start );
					changed = true;
					nPrefixes++;
					break;
				}
			}
		}
	}

  name.TrimLeft();
	name.TrimRight();
	if (name.CompareNoCase(fn.fename) != 0)
//	if (name.GetLength() != ::strlen(fn.fename))
	{
		ASSERT(name.GetLength() < sizeof(fn.fename));
		::strcpy( fn.fename, (const char *)name );
	}
	return changed;
}