/**
 * @file	map_resize.c
 *
 * @date Jan 12, 2014
 * @author Andrey Belomutskiy, (c) 2012-2018
 */

#include "engine_configuration.h"
#include "fuel_math.h"
#include "interpolation.h"

#if 1

#define AD_LOAD_COUNT 16
#define AD_RPM_COUNT 16


static float ad_rpm_table[] = {/*0*/ 800.000000,
		/*1*/ 1213.333374,
		/*2*/ 1626.666748,
		/*3*/ 2040.000000,
		/*4*/ 2453.333496,
		/*5*/ 2866.666748,
		/*6*/ 3280.000000,
		/*7*/ 3693.333496,
		/*8*/ 4106.666992,
		/*9*/ 4520.000000,
		/*10*/ 4933.333496,
		/*11*/ 5346.666992,
		/*12*/ 5760.000000,
		/*13*/ 6173.333496,
		/*14*/ 6586.666992,
		/*15*/ 7000.000000,
		};;

static float ad_maf_table[] =
{/*0*/ 1.200000,
/*1*/ 1.413333,
/*2*/ 1.626667,
/*3*/ 1.840000,
/*4*/ 2.053333,
/*5*/ 2.266667,
/*6*/ 2.480000,
/*7*/ 2.693333,
/*8*/ 2.906667,
/*9*/ 3.120000,
/*10*/ 3.333333,
/*11*/ 3.546667,
/*12*/ 3.760000,
/*13*/ 3.973333,
/*14*/ 4.186667,
/*15*/ 4.400000,
};

static float ad_table[AD_LOAD_COUNT][AD_RPM_COUNT] = {
		/* RPM					800.000000	1213.333374	1626.666748	2040.000000	2453.333496	2866.666748	3280.000000	3693.333496	4106.666992	4520.000000	4933.333496	5346.666992	5760.000000	6173.333496	6586.666992	7000.000000*/
		/* Load 1.200000 */{	0.662000,	-7.730000,	-16.722000,	-23.139999,	-29.398001,	-31.268000,	-32.108002,	-30.436001,	-30.896000,	-26.656000,	-24.704000,	-25.108000,	-25.132000,	-25.459999,	-25.459999,	-25.459999},
		/* Load 1.413333 */{	0.546000,	-7.662000,	-16.882000,	-23.482000,	-29.520000,	-31.323999,	-32.108002,	-30.656000,	-30.468000,	-26.879999,	-24.746000,	-24.742001,	-29.032000,	-25.562000,	-25.562000,	-25.562000},
		/* Load 1.626667 */{	0.584000,	-7.870000,	-16.714001,	-23.025999,	-29.542000,	-31.166000,	-32.175999,	-30.540001,	-30.268000,	-26.416000,	-24.134001,	-25.007999,	-24.698000,	-26.167999,	-26.167999,	-26.167999},
		/* Load 1.840000 */{	0.584000,	-7.658000,	-16.714001,	-23.254000,	-29.351999,	-30.978001,	-32.141998,	-30.874001,	-30.896000,	-26.507999,	-24.558001,	-24.389999,	-25.761999,	-35.492001,	-35.492001,	-35.492001},
		/* Load 2.053333 */{	0.584000,	-7.862000,	-16.538000,	-23.254000,	-29.232000,	-31.296000,	-32.520000,	-30.142000,	-30.388000,	-25.903999,	-24.370001,	-24.082001,	-24.792000,	-24.351999,	-24.351999,	-24.351999},
		/* Load 2.266667 */{	-1.364000,	-7.726000,	-16.806000,	-23.254000,	-29.639999,	-31.006001,	-32.298000,	-30.912001,	-29.882000,	-26.392000,	-24.664000,	-27.233999,	-25.374001,	-25.417999,	-25.417999,	-25.417999},
		/* Load 2.480000 */{	1.364000,	-10.490000,	-16.705999,	-22.441999,	-28.101999,	-30.238001,	-32.363998,	-30.719999,	-30.896000,	-26.608000,	-24.664000,	-24.431999,	-24.500000,	-25.510000,	-25.510000,	-25.510000},
		/* Load 2.693333 */{	9.864000,	-10.416000,	-11.680000,	-19.150000,	-25.754000,	-27.936001,	-32.554001,	-30.656000,	-30.153999,	-27.184000,	-25.252001,	-22.812000,	-24.452000,	-25.219999,	-25.219999,	-25.219999},
		/* Load 2.906667 */{	9.866000,	5.452000,	2.854000,	-17.212000,	-17.552000,	-20.688000,	-25.660000,	-27.809999,	-27.691999,	-27.224001,	-25.882000,	-25.360001,	-26.100000,	-27.992001,	-27.992001,	-27.992001},
		/* Load 3.120000 */{	9.864000,	5.452000,	2.854000,	-0.342000,	-12.526000,	-16.218000,	-21.364000,	-27.590000,	-25.780001,	-24.170000,	-24.664000,	-25.584000,	-26.490000,	-31.968000,	-31.968000,	-31.968000},
		/* Load 3.333333 */{	9.864000,	5.516000,	2.854000,	-0.226000,	-2.738000,	-3.816000,	-11.924000,	-18.808001,	-21.038000,	-21.538000,	-21.209999,	-22.228001,	-25.046000,	-25.156000,	-25.156000,	-25.156000},
		/* Load 3.546667 */{	9.866000,	5.518000,	2.854000,	0.000000,	-3.022000,	-3.816000,	-6.428000,	-7.788000,	-19.426001,	-20.860001,	-19.966000,	-21.030001,	-21.396000,	-21.570000,	-21.570000,	-21.570000},
		/* Load 3.760000 */{	9.864000,	5.516000,	2.772000,	-0.226000,	-2.732000,	-3.500000,	-6.798000,	-8.102000,	-8.660000,	-9.500000,	-11.788000,	-20.132000,	-20.072001,	-20.510000,	-20.510000,	-20.510000},
		/* Load 3.973333 */{	9.864000,	5.518000,	2.854000,	0.000000,	-2.880000,	-3.816000,	-6.420000,	-8.320000,	-8.426000,	-8.532000,	-11.470000,	-11.442000,	-13.610000,	-12.022000,	-12.022000,	-12.022000},
		/* Load 4.186667 */{	9.750000,	5.518000,	2.604000,	0.000000,	-2.880000,	-3.654000,	-6.050000,	-6.888000,	-8.372000,	-9.364000,	-11.764000,	-11.732000,	-11.864000,	-12.376000,	-12.376000,	-12.376000},
		/* Load 4.400000 */{	0.350000,	5.590000,	0.502000,	0.910000,	0.864000,	0.954000,	1.324000,	-7.436000,	1.170000,	1.054000,	2.058000,	2.098000,	2.636000,	-12.352000,	-12.352000,	-12.352000}
		};

//float getBaseAdvance(int rpm, float key) {
//	// todo: use interpolation
//	int rpm_index = findIndex(ad_rpm_table, AD_RPM_COUNT, rpm);
//	rpm_index = max(rpm_index, 0);
//	int maf_index = findIndex(ad_maf_table, AD_LOAD_COUNT, key);
//	maf_index = max(maf_index, 0);
//
//	return ad_table[rpm_index][maf_index];
//}



#define newRpmSize 16
#define newKeySize 16

static float newRpmBin[newRpmSize];
static float newKeyBin[newKeySize];

//static float *fuel_ptrs[FUEL_LOAD_COUNT];

//EngineConfiguration *engineConfiguration;

extern bool needInterpolationLoggingValue;

void resizeMap(void) {
//	float keyMin = 1.2;
//	float keyMax = 4.4;
//
//	float rpmMin = 800;
//	float rpmMax = 7000;

//	for (int k = 0; k < FUEL_LOAD_COUNT; k++)
//		fuel_ptrs[k] = engineConfiguration->fuelTable[k];

//	for (int i = 0; i < FUEL_MAF_COUNT; i++)
//		engineConfiguration->fuelKeyBins[i] = default_fuel_maf_bins[i];
//	for (int i = 0; i < FUEL_RPM_COUNT; i++)
//		engineConfiguration->fuelRpmBins[i] = default_fuel_rpm_bins[i];
//	for (int k = 0; k < FUEL_MAF_COUNT; k++) {
//		for (int r = 0; r < FUEL_RPM_COUNT; r++) {
//			// todo: this is BAD, this needs to be fixed - TS table indexes are different from default indexes
//			engineConfiguration->fuelTable[k][r] = default_fuel_table[r][k];
//		}
//	}

//	ASSERT_EQ(15, interpolate3d(1.2, engineConfiguration->fuelKeyBins, FUEL_MAF_COUNT, 8000,
//			engineConfiguration->fuelRpmBins,
//			FUEL_RPM_COUNT, fuel_ptrs));

	needInterpolationLoggingValue = 0;

//	printf("static float ad_maf_table[AD_LOAD_COUNT] = {");
//	for (int i = 0; i < newKeySize; i++) {
//		newKeyBin[i] = interpolate(0, keyMin, newKeySize - 1, keyMax, i);
//		printf("/*%d*/ %f,\r\n", i, newKeyBin[i]);
//	}
//	printf("};\r\n");
//
//	printf("static float ad_rpm_table[AD_RPM_COUNT] = {");
//	for (int i = 0; i < newRpmSize; i++) {
//		newRpmBin[i] = interpolate(0, rpmMin, newRpmSize - 1, rpmMax, i);
//		printf("/*%d*/ %f,\r\n", i, newRpmBin[i]);
//	}
//	printf("};\r\n");

	printf("static float ad_table[AD_RPM_COUNT][AD_LOAD_COUNT] = {\r\n");

	printf("/* RPM\t\t");
	for (int r = 0; r < newRpmSize; r++) {
		float rpm = newRpmBin[r];
		printf("\t%f", rpm);
	}
	printf("*/\r\n");

	for (int k = 0; k < newKeySize; k++) {
		float load = newKeyBin[k];
		printf("/* Load %f */{", load);

		for (int r = 0; r < newRpmSize; r++) {
			float rpm = newRpmBin[r];

			float v = ad_table[k][r];

			printf("\t%f", v);
			if (r != newRpmSize - 1)
				printf(",");

		}
		printf("}");
		if (k != newKeySize - 1)
			printf(",");
		printf("\r\n");
	}
	printf("};\r\n");

}

#endif
