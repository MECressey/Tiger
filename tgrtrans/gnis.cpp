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