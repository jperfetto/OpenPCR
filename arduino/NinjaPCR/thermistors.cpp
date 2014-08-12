/*
 *  program.cpp - OpenPCR control software.
 *  Copyright (C) 2010-2012 Josh Perfetto. All Rights Reserved.
 *
 *  OpenPCR control software is free software: you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License as published
 *  by the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  OpenPCR control software is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along with
 *  the OpenPCR control software.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "pcr_includes.h"
#include "thermistors.h"

/* Use original ohm-to-celsius table */
//#define LID_THERMISTOR_ORIGINAL
/* Use ohm-to-celsius table for NXFT15XH103FA */
//#define LID_THERMISTOR_NXFT15XH103FA
#define LID_THERMISTOR_CONSTB_3980

// #define PLATE_THERMISTOR_OPENPCR
// #define PLATE_THERMISTOR_NO1
// #define PLATE_THERMISTOR_NO2
// #define PLATE_THERMISTOR_NO3
// #define PLATE_THERMISTOR_NO4
// #define PLATE_THERMISTOR_NO5
// #define PLATE_THERMISTOR_NO6
// #define PLATE_THERMISTOR_NO7
// #define PLATE_THERMISTOR_NO8
// #define PLATE_THERMISTOR_NO9
#define PLATE_THERMISTOR_NXFT15WF104FA2B100

#define LID_RESISTANCE_TABLE_OFFSET 0
#define PLATE_RESISTANCE_TABLE_OFFSET -10*4

// lid resistance table, in Ohms
#ifdef LID_THERMISTOR_ORIGINAL
PROGMEM const unsigned int LID_RESISTANCE_TABLE[] = {
	32919, 31270, 29715, 28246, 26858, 25547, 24307, 23135, 22026, 20977,	//0-9
	19987, 19044, 18154, 17310, 16510, 15752, 15034, 14352, 13705, 13090,	//10-19
	12507, 11953, 11427, 10927, 10452, 10000, 9570, 9161, 8771, 8401,		//20-29
	8048, 7712, 7391, 7086, 6795, 6518, 6254, 6001, 5761, 5531, 5311,		//30-39
	5102, 4902, 4710, 4528, 4353, 4186, 4026, 3874, 3728, 3588,				//40-49
	3454, 3326, 3203, 3085, 2973, 2865, 2761, 2662, 2567, 2476,				//50-59
	2388, 2304, 2223, 2146, 2072, 2000, 1932, 1866, 1803, 1742,				//60-69
	1684, 1627, 1573, 1521, 1471, 1423, 1377, 1332, 1289, 1248,				//70-79
	1208, 1170, 1133, 1097, 1063, 1030, 998, 968, 938, 909,					//80-89
	882, 855, 829, 805, 781, 758, 735, 714, 693, 673,						//90-99
	653, 635, 616, 599, 582, 565, 550, 534, 519, 505,						//100-109
	491, 478, 465, 452, 440, 428, 416, 405, 395, 384,						//110-119
	374, 364, 355, 345, 337
};
#endif

//  NXFT15XH103FA 10k Ohms
#ifdef LID_THERMISTOR_NXFT15XH103FA
PROGMEM const unsigned int LID_RESISTANCE_TABLE[] = {
	28854, 27555, 26322, 25153, 24044, 22991, 21991, 21041, 20139, 19282,
	18466, 17691, 16953, 16250, 15582, 14945, 14339, 13761, 13210, 12684,
	12183, 11705, 11249, 10813, 10397, 10000, 9620, 9258, 8911, 8579,
	8262, 7958, 7667, 7389, 7123, 6868, 6623, 6389, 6165, 5949, 5743,
	5545, 5355, 5173, 4998, 4830, 4668, 4513, 4364, 4221, 4083, 3951,
	3823, 3701, 3583, 3470, 3361, 3256, 3154, 3057, 2963, 2873, 2785,
	2701, 2620, 2542, 2467, 2394, 2324, 2256, 2191, 2128, 2067, 2008,
	1951, 1896, 1843, 1792, 1742, 1694, 1648, 1603, 1559, 1518, 1477,
	1438, 1399, 1363, 1327, 1292, 1259, 1226, 1195, 1165, 1135, 1106,
	1079, 1052, 1026, 1000, 976, 952, 929, 906, 885, 863,
	843, 823, 804, 785, 766, 749, 731, 715, 698, 682,
	667, 652, 637, 623, 610, 596, 583, 570, 558, 546
};
#endif

// ConstB=3,980.00
#ifdef LID_THERMISTOR_CONSTB_3980
PROGMEM const unsigned int LID_RESISTANCE_TABLE[] = {
	33932, 32175, 30522, 28964, 27496, 26113, 24808, 23577, 22415,
	21318, 20282, 19303, 18378, 17503, 16676, 15893, 15151, 14449,
	13784, 13154, 12557, 11990, 11453, 10943, 10459, 10000, 9564,
	9149, 8755, 8380, 8024, 7685, 7362, 7055, 6763, 6484, 6219,
	5966, 5725, 5495, 5276, 5067, 4867, 4677, 4495, 4321, 4155,
	3996, 3844, 3699, 3560, 3428, 3301, 3179, 3063, 3040, 2951,
	2844, 2742, 2644, 2550, 2460, 2442, 2374, 2291, 2211, 2135,
	2062, 1991, 1964, 1924, 1859, 1797, 1737, 1679, 1624, 1602,
	1571, 1520, 1470, 1423, 1377, 1333, 1316, 1291, 1251, 1211,
	1174, 1137, 1102, 1069, 1036, 1005, 974, 945, 917, 909, 890,
	863, 838, 814, 790, 767, 752, 745, 724, 703, 684, 664, 646,
	628, 610, 594, 577, 562, 546, 532, 517, 504, 490, 477, 465,
	453, 441, 429, 418, 408, 397, 387, 377, 368, 359, 350
};
#endif

// plate resistance table, in 0.1 Ohms
#if defined(PLATE_THERMISTOR_OPENPCR)
PROGMEM const unsigned long PLATE_RESISTANCE_TABLE[] = {
		3364790, 3149040, 2948480, 2761940, 2588380, 2426810, 2276320, 2136100, 2005390, 1883490,
		1769740, 1663560, 1564410, 1471770, 1385180, 1304210, 1228470, 1157590, 1091220, 1029060,
		970810, 916210, 865010, 816980, 771900, 729570, 689820, 652460, 617360, 584340,
		553290, 524070, 496560, 470660, 446260, 423270, 401590, 381150, 361870, 343680,
		326500, 310290, 294980, 280520, 266850, 253920, 241700, 230130, 219180, 208820,
		199010, 189710, 180900, 172550, 164630, 157120, 149990, 143230, 136810, 130720,
		124930, 119420, 114190, 109220, 104500, 100000, 95720, 91650, 87770, 84080,
		80570, 77220, 74020, 70980, 68080, 65310, 62670, 60150, 57750, 55450,
		53260, 51170, 49170, 47250, 45430, 43680, 42010, 40410, 38880, 37420,
		36020, 34680, 33400, 32170, 30990, 29860, 28780, 27740, 26750, 25790,
		24880, 24000, 23160, 22350, 21570, 20830, 20110, 19420, 18760, 18130,
		17520, 16930, 16370, 15820, 15300, 14800, 14320, 13850, 13400, 12970,
		12550, 12150, 11770, 11400, 11040, 10700, 10370, 10050, 9738, 9441,
		9155, 8878, 8612, 8354, 8106, 7866, 7635, 7412, 7196, 6987, 6786,
		6591, 6403, 6222, 6046, 5876
};
#elif defined(PLATE_THERMISTOR_NO1)
	//No.1
PROGMEM const unsigned long PLATE_RESISTANCE_TABLE[] = {
		560580, 544933, 529722, 514937, 500563, 486591, 473009, 459806, 446972, 434496,
		422368, 410579, 399118, 387978, 377149, 366621, 356388, 346440, 336770, 327370,
		318232, 309350, 300715, 292321, 284162, 276230, 268520, 261025, 253739, 246657,
		239772, 233079, 226573, 220249, 214101, 208125, 202316, 196669, 191179, 185843,
		180656, 175613, 170711, 165946, 161314, 156812, 152435, 148180, 144044, 140023,
		136115, 132315, 128622, 125032, 121542, 118149, 114852, 111646, 108529, 105500,
		102555, 99693, 96910, 94205, 91576, 89019, 86535, 84119, 81771, 79489,
		77270, 75113, 73017, 70979, 68997, 67072, 65199, 63380, 61610, 59891,
		58219, 56594, 55014, 53479, 51986, 50535, 49124, 47753, 46420, 45125,
		43865, 42641, 41450, 40293, 39169, 38075, 37013, 35980, 34975, 33999,
		33050, 32128, 31231, 30359, 29512, 28688, 27887, 27109, 26352, 25617,
		24901, 24206, 23531, 22874, 22235, 21615, 21012, 20425, 19855, 19301,
		18762, 18238, 17729, 17234, 16753, 16286, 15831, 15389, 14960, 14542,
		14136, 13742, 13358, 12985, 12623, 12270, 11928, 11595, 11271, 10957,
		10651, 10354, 10065, 9784, 9511, 9245
};
#elif defined(PLATE_THERMISTOR_NO2)
	//No.2
	//TODO
#elif defined(PLATE_THERMISTOR_NO3)
	//No.3
PROGMEM const unsigned long PLATE_RESISTANCE_TABLE[] = {
	455142, 443323, 431812, 420599, 409678, 399040, 388678, 378585, 368755, 359180,
	349853, 340769, 331920, 323301, 314906, 306729, 298765, 291007, 283450, 276090,
	268921, 261938, 255137, 248512, 242059, 235773, 229651, 223688, 217880, 212222,
	206711, 201344, 196116, 191023, 186063, 181232, 176526, 171942, 167477, 163128,
	158893, 154767, 150748, 146834, 143021, 139307, 135690, 132166, 128735, 125392,
	122136, 118964, 115875, 112866, 109936, 107081, 104301, 101592, 98954, 96385,
	93882, 91444, 89070, 86757, 84504, 82310, 80173, 78091, 76063, 74088,
	72164, 70290, 68465, 66687, 64956, 63269, 61626, 60026, 58467, 56949,
	55470, 54030, 52627, 51260, 49929, 48633, 47370, 46140, 44942, 43775,
	42638, 41531, 40453, 39402, 38379, 37383, 36412, 35466, 34546, 33648,
	32775, 31924, 31095, 30287, 29501, 28735, 27989, 27262, 26554, 25865,
	25193, 24539, 23902, 23281, 22676, 22088, 21514, 20955, 20411, 19881,
	19365, 18862, 18372, 17895, 17431, 16978, 16537, 16108, 15690, 15282,
	14885, 14499, 14122, 13756, 13398, 13050, 12712, 12382, 12060, 11747,
	11442, 11145, 10855, 10573, 10299, 10032
};
#elif defined(PLATE_THERMISTOR_NO4)
	//No.4
PROGMEM const unsigned long PLATE_RESISTANCE_TABLE[] = {
	468312, 455924, 443863, 432121, 420690, 409562, 398727, 388180, 377911, 367914,
	358181, 348706, 339482, 330501, 321759, 313247, 304961, 296893, 289040, 281394,
	273950, 266703, 259648, 252779, 246092, 239582, 233245, 227074, 221068, 215220,
	209526, 203984, 198588, 193334, 188220, 183241, 178394, 173675, 169080, 164608,
	160253, 156014, 151887, 147869, 143957, 140149, 136442, 132832, 129319, 125898,
	122567, 119325, 116168, 113095, 110104, 107191, 104355, 101595, 98907, 96291,
	93744, 91264, 88850, 86499, 84211, 81983, 79815, 77703, 75648, 73647,
	71698, 69802, 67955, 66158, 64408, 62704, 61045, 59430, 57858, 56328,
	54838, 53387, 51975, 50600, 49261, 47958, 46689, 45454, 44252, 43081,
	41942, 40832, 39752, 38700, 37677, 36680, 35710, 34765, 33845, 32950,
	32078, 31230, 30404, 29599, 28816, 28054, 27312, 26590, 25886, 25201,
	24535, 23886, 23254, 22639, 22040, 21457, 20889, 20337, 19799, 19275,
	18765, 18269, 17785, 17315, 16857, 16411, 15977, 15554, 15143, 14742,
	14352, 13972, 13603, 13243, 12893, 12552, 12220, 11896, 11582, 11275,
	10977, 10687, 10404, 10129, 9861, 9600
};
#elif defined(PLATE_THERMISTOR_NO5)
	//No.5
PROGMEM const unsigned long PLATE_RESISTANCE_TABLE[] = {
	479492, 466481, 453823, 441509, 429529, 417874, 406535, 395504, 384772, 374331,
	364174, 354292, 344679, 335326, 326227, 317375, 308763, 300385, 292234, 284305,
	276590, 269085, 261783, 254680, 247769, 241046, 234506, 228142, 221952, 215929,
	210070, 204370, 198825, 193429, 188181, 183075, 178107, 173274, 168572, 163998,
	159548, 155219, 151007, 146910, 142923, 139045, 135272, 131602, 128031, 124557,
	121177, 117889, 114690, 111578, 108550, 105605, 102739, 99952, 97239, 94601,
	92034, 89537, 87107, 84743, 82444, 80207, 78030, 75913, 73853, 71849,
	69900, 68003, 66158, 64363, 62616, 60917, 59264, 57656, 56092, 54570,
	53089, 51648, 50247, 48883, 47557, 46267, 45011, 43790, 42602, 41446,
	40321, 39227, 38162, 37127, 36120, 35139, 34186, 33258, 32356, 31478,
	30624, 29793, 28984, 28198, 27433, 26688, 25964, 25260, 24574, 23907,
	23259, 22628, 22014, 21416, 20835, 20270, 19720, 19185, 18664, 18158,
	17665, 17186, 16719, 16266, 15824, 15395, 14977, 14571, 14175, 13791,
	13417, 13053, 12698, 12354, 12019, 11692, 11375, 11067, 10766, 10474,
	10190, 9913, 9644, 9383, 9128, 8880
};
#elif defined(PLATE_THERMISTOR_NO6)
	//No.6
PROGMEM const unsigned long PLATE_RESISTANCE_TABLE[] = {
	830023, 805243, 781203, 757880, 735254, 713303, 692007, 671348, 651305, 631860,
	612996, 594695, 576941, 559716, 543006, 526795, 511067, 495810, 481007, 466647,
	452715, 439200, 426087, 413367, 401026, 389053, 377438, 366170, 355238, 344632,
	334343, 324362, 314678, 305283, 296169, 287327, 278749, 270427, 262353, 254521,
	246922, 239550, 232399, 225461, 218729, 212199, 205864, 199718, 193756, 187971,
	182359, 176915, 171633, 166509, 161538, 156715, 152037, 147498, 143094, 138822,
	134678, 130657, 126756, 122972, 119301, 115739, 112283, 108931, 105679, 102524,
	99463, 96494, 93613, 90818, 88107, 85476, 82925, 80449, 78047, 75717,
	73457, 71263, 69136, 67072, 65069, 63127, 61242, 59414, 57640, 55919,
	54250, 52630, 51059, 49535, 48056, 46621, 45229, 43879, 42569, 41298,
	40065, 38869, 37708, 36583, 35491, 34431, 33403, 32406, 31438, 30500,
	29589, 28706, 27849, 27017, 26211, 25428, 24669, 23933, 23218, 22525,
	21852, 21200, 20567, 19953, 19357, 18780, 18219, 17675, 17147, 16635,
	16139, 15657, 15189, 14736, 14296, 13869, 13455, 13053, 12664, 12286,
	11919, 11563, 11218, 10883, 10558, 10243
};
#elif defined(PLATE_THERMISTOR_NO7)
	//No.7, No.10
PROGMEM const unsigned long PLATE_RESISTANCE_TABLE[] = {
	503746, 489930, 476493, 463425, 450715, 438353, 426331, 414638, 403266, 392206,
	381449, 370987, 360812, 350917, 341292, 331932, 322828, 313974, 305363, 296988,
	288843, 280921, 273216, 265723, 258435, 251347, 244454, 237749, 231229, 224887,
	218719, 212720, 206886, 201212, 195693, 190326, 185106, 180030, 175092, 170290,
	165619, 161077, 156659, 152363, 148184, 144120, 140167, 136323, 132584, 128948,
	125411, 121972, 118626, 115373, 112209, 109131, 106138, 103227, 100396, 97642,
	94964, 92360, 89827, 87363, 84967, 82637, 80370, 78166, 76022, 73937,
	71909, 69937, 68019, 66154, 64339, 62575, 60858, 59189, 57566, 55987,
	54452, 52958, 51506, 50093, 48719, 47383, 46084, 44820, 43590, 42395,
	41232, 40101, 39001, 37932, 36891, 35880, 34896, 33939, 33008, 32102,
	31222, 30366, 29533, 28723, 27935, 27169, 26424, 25699, 24994, 24309,
	23642, 22994, 22363, 21750, 21153, 20573, 20009, 19460, 18926, 18407,
	17902, 17411, 16934, 16469, 16018, 15578, 15151, 14736, 14331, 13938,
	13556, 13184, 12823, 12471, 12129, 11796, 11473, 11158, 10852, 10554,
	10265, 9983, 9710, 9443, 9184, 8932
};
#elif defined(PLATE_THERMISTOR_NO8)
	//No.8
PROGMEM const unsigned long PLATE_RESISTANCE_TABLE[] = {
	473218, 460469, 448064, 435994, 424248, 412819, 401697, 390876, 380345, 370099,
	360128, 350427, 340986, 331800, 322861, 314163, 305700, 297464, 289451, 281653,
	274065, 266682, 259497, 252507, 245704, 239085, 232644, 226376, 220278, 214344,
	208569, 202950, 197483, 192163, 186986, 181948, 177047, 172277, 167636, 163120,
	158725, 154449, 150289, 146240, 142300, 138467, 134736, 131106, 127574, 124138,
	120793, 117539, 114373, 111291, 108293, 105376, 102537, 99775, 97087, 94471,
	91926, 89450, 87040, 84695, 82413, 80193, 78033, 75931, 73885, 71895,
	69958, 68073, 66239, 64455, 62718, 61029, 59385, 57785, 56228, 54713,
	53239, 51805, 50409, 49051, 47730, 46444, 45193, 43975, 42791, 41638,
	40516, 39425, 38363, 37329, 36323, 35345, 34393, 33466, 32565, 31687,
	30834, 30003, 29195, 28408, 27643, 26898, 26174, 25468, 24782, 24115,
	23465, 22833, 22218, 21619, 21037, 20470, 19919, 19382, 18860, 18352,
	17857, 17376, 16908, 16453, 16009, 15578, 15158, 14750, 14353, 13966,
	13590, 13224, 12867, 12521, 12184, 11855, 11536, 11225, 10923, 10628,
	10342, 10064, 9792, 9529, 9272, 9022
};
#elif defined(PLATE_THERMISTOR_NO9)
	//No.9
PROGMEM const unsigned long PLATE_RESISTANCE_TABLE[] = {
	491368, 478131, 465250, 452716, 440520, 428652, 417104, 405868, 394934, 384294, //0-9 (-10)
	373941, 363867, 354065, 344526, 335245, 326213, 317425, 308874, 300553, 292456, //10-19 (0)
	284577, 276910, 269450, 262191, 255128, 248255, 241567, 235059, 228727, 222565, //20-29 (10)
	216569, 210735, 205057, 199533, 194158, 188927, 183837, 178885, 174066, 169376, //30-39 (20)
	164813, 160373, 156053, 151849, 147758, 143777, 139904, 136135, 132468, 128899, //40-49 (30)
	125426, 122047, 118759, 115560, 112447, 109418, 106470, 103602, 100811, 98095, //50-59 (40)
	95452, 92881, 90378, 87944, 85574, 83269, 81026, 78843, 76719, 74652, //60-69 (50)
	72641, 70684, 68780, 66927, 65124, 63369, 61662, 60001, 58385, 56812, //70-79 (60)
	55281, 53792, 52343, 50933, 49561, 48225, 46926, 45662, 44432, 43235, //80-89 (70)
	42070, 40937, 39834, 38761, 37717, 36701, 35712, 34750, 33814, 32903, //90-99 (80)
	32016, 31154, 30314, 29498, 28703, 27930, 27177, 26445, 25733, 25040, //100-109 (90)
	24365, 23709, 23070, 22448, 21844, 21255, 20683, 20125, 19583, 19056, //110-119 (100)
	18542, 18043, 17557, 17084, 16623, 16176, 15740, 15316, 14903, 14502, //120-129 (110)
	14111, 13731, 13361, 13001, 12651, 12310, 11978, 11656, 11342, 11036, //130-139 (120)
	10739, 10450, 10168, 9894, 9628, 9368 //140-146 (130)
};

#elif defined(PLATE_THERMISTOR_NXFT15WF104FA2B100)
PROGMEM const unsigned long PLATE_RESISTANCE_TABLE[] = {
	691288, 649503, 610530, 574163, 540210, 508497, 478860, 451151, 425232, 400977, // 0-9 (-10)
	378268, 356997, 337063, 318376, 300848, 284401, 268961, 254461, 240837, 228032, // 10-19 (0)
	215991, 204664, 194004, 183969, 174517, 165611, 157216, 149301, 141834, 134789, // 20-29 (10)
	128137, 121856, 115923, 110315, 105014, 100000, 95257, 90768, 86519, 82495, // 30-39 (20)
	78682, 75069, 71644, 68397, 65316, 62392, 59617, 56983, 54480, 52103, // 40-49 (30)
	49843, 47695, 45652, 43709, 41860, 40100, 38424, 36828, 35308, 33860, // 50-59 (40)
	32479, 31163, 29907, 28709, 27567, 26476, 25435, 24440, 23490, 22583, // 60-69 (50)
	21715, 20886, 20093, 19335, 18610, 17915, 17251, 16615, 16006, 15422, // 70-79 (60)
	14863, 14328, 13815, 13322, 12850, 12398, 11964, 11547, 11147, 10763, // 80-89 (70)
	10394, 10040, 9700, 9374, 9060, 8758, 8468, 8189, 7920, 7662, // 90-99 (80)
	7414, 7175, 6945, 6723, 6510, 6304, 6106, 5916, 5732, 5555, // 100-109 (90)
	5384, 5220, 5061, 4908, 4760, 4618, 4480, 4348, 4219, 4096, // 110-119 (100)
	3976, 3861, 3749, 3642, 3538, 3437, 3340, 3246, 3155, 3067 // 120-129 (110)
};
#else
	#error "Number of the plate thermistor is not specified!"
#endif

//spi
#define DATAOUT 11//MOSI
#define DATAIN  12//MISO
#define SPICLOCK  13//sck
#define SLAVESELECT 10//ss

//------------------------------------------------------------------------------
float TableLookup(const unsigned long lookupTable[], unsigned int tableSize, int startValue, unsigned long searchValue) {
  //simple linear search for now
  int i;
  for (i = 0; i < tableSize; i++) {
    if (searchValue >= pgm_read_dword_near(lookupTable + i))
      break;
  }

  if (i > 0) {
    unsigned long high_val = pgm_read_dword_near(lookupTable + i - 1);
    unsigned long low_val = pgm_read_dword_near(lookupTable + i);
    return i + startValue - (float)(searchValue - low_val) / (float)(high_val - low_val);
  } else {
    return startValue;
  }
}
//------------------------------------------------------------------------------
float TableLookup(const unsigned int lookupTable[], unsigned int tableSize, int startValue, unsigned long searchValue) {
  //simple linear search for now
  int i;
  for (i = 0; i < tableSize; i++) {
    if (searchValue >= pgm_read_word_near(lookupTable + i))
      break;
  }

  if (i > 0) {
    unsigned long high_val = pgm_read_word_near(lookupTable + i - 1);
    unsigned long low_val = pgm_read_word_near(lookupTable + i);
    return i + startValue - (float)(searchValue - low_val) / (float)(high_val - low_val);
  } else {
    return startValue;
  }
}

////////////////////////////////////////////////////////////////////
// Class CLidThermistor
CLidThermistor::CLidThermistor():
  iTemp(0.0) {
}

//------------------------------------------------------------------------------
void CLidThermistor::ReadTemp() {
	unsigned long voltage_mv = (unsigned long) analogRead(1) * 5000 / 1024;
  resistance = voltage_mv * 2200 / (5000 - voltage_mv);
  iTemp = TableLookup(LID_RESISTANCE_TABLE, sizeof(LID_RESISTANCE_TABLE) / sizeof(LID_RESISTANCE_TABLE[0]), 0, resistance);
}

////////////////////////////////////////////////////////////////////
// Class CPlateThermistor
CPlateThermistor::CPlateThermistor():
  iTemp(0.0) {

  //spi setup
  pinMode(DATAOUT, OUTPUT);
  pinMode(DATAIN, INPUT);
  pinMode(SPICLOCK,OUTPUT);
  pinMode(SLAVESELECT,OUTPUT);
  digitalWrite(SLAVESELECT,HIGH); //disable device
}
//------------------------------------------------------------------------------
#define RESISTOR_SINGLE 16000*10
#define RESISTOR_DOUBLE 6153*10
#define RESISTOR_MODE_THRESHOLD_TEMPERATURE 81.35

void CPlateThermistor::ReadTemp() {
	digitalWrite(SLAVESELECT, LOW);

	//read data
	while (digitalRead(DATAIN)) {}

	uint8_t spiBuf[4];
	memset(spiBuf, 0, sizeof(spiBuf));

	digitalWrite(SLAVESELECT, LOW);
	for (int i = 0; i < 4; i++)
		spiBuf[i] = SPITransfer(0xFF);

	unsigned long conv = (((unsigned long) spiBuf[3] >> 7) & 0x01)
			+ ((unsigned long) spiBuf[2] << 1)
			+ ((unsigned long) spiBuf[1] << 9)
			+ (((unsigned long) spiBuf[0] & 0x1F) << 17); //((spiBuf[0] & 0x1F) << 16) + (spiBuf[1] << 8) + spiBuf[2];

	unsigned long adcDivisor = 0x1FFFFF;
	float voltage = (float) conv * 5.0 / adcDivisor;

	unsigned int convHigh = (conv >> 16);

	digitalWrite(SLAVESELECT, HIGH);

	unsigned long voltage_mv = voltage * 1000;
	unsigned int resistor = (resistorMode==TEMP_LOW)? RESISTOR_SINGLE:RESISTOR_DOUBLE;
	resistance = voltage_mv * resistor / (5000 - voltage_mv); // in hecto ohms
	iTemp = TableLookup(PLATE_RESISTANCE_TABLE,
			sizeof(PLATE_RESISTANCE_TABLE) / sizeof(PLATE_RESISTANCE_TABLE[0]),
			PLATE_RESISTANCE_TABLE_OFFSET, resistance);

	if (resistorMode==TEMP_LOW  && iTemp>RESISTOR_MODE_THRESHOLD_TEMPERATURE) {
		// LOW -> HIGH
		resistorMode = TEMP_HIGH;
		digitalWrite(PIN_LID_RESISTOR_SWITCH, HIGH);
	}
	else if (resistorMode==TEMP_HIGH  && iTemp<RESISTOR_MODE_THRESHOLD_TEMPERATURE) {
		// HIGH -> LOW
		resistorMode = TEMP_HIGH;
		digitalWrite(PIN_LID_RESISTOR_SWITCH, LOW);
	}
}
//------------------------------------------------------------------------------
char CPlateThermistor::SPITransfer(volatile char data) {
  SPDR = data;                    // Start the transmission
  while (!(SPSR & (1<<SPIF)))     // Wait the end of the transmission
  {};
  return SPDR;                    // return the received byte
}
