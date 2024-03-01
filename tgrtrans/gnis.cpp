//
//	gnis.cc - is the method that maps features to TigerDB::GNISFeatures.
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
#include "gnis.h"

#include <assert.h>

static const char* featureCodes[] = {
	"Arch",
	"Area",
	"Arroyo",
	"Bar",
	"Basin",
	"Bay",
	"Beach",
	"Bench",
	"Bend",
	"Canal",
	"Cape",
	"Census",
	"Channel",
	"Civil",
	"Cliff",
	"Crater",
	"Crossing",
	"Falls",
	"Flat",
	"Gap",
	"Glacier",
	"Gut",
	"Island",
	"Isthmus",
	"Lake",
	"Lava",
	"Levee",
	"Military",
	"Pillar",
	"Plain",
	"Populated Place",
	"Range",
	"Rapids",
	"Reservoir",
	"Ridge",
	"Sea",
	"Slope",
	"Spring",
	"Stream",
	"Summit",
	"Swamp",
	"Valley",
	"Woods"
};

TigerDB::GNISFeatures MapFeatureClassToCode(const std::string& feature)
{

	int i;
	for (i = 0; i < sizeof(featureCodes) / sizeof(featureCodes[0]); i++)
	{
		if (strcmp(feature.c_str(), featureCodes[i]) == 0)
		{
			break;
		}
	}
	assert(i < sizeof(featureCodes) / sizeof(featureCodes[0]));
	TigerDB::GNISFeatures fc = (TigerDB::GNISFeatures)i;

	return fc;
}