#include <bit>
#include <iostream>
#include <vector>
#include <algorithm>
#include <chrono>
#include "immintrin.h"

#define ulong unsigned long long

#define popcount __popcnt64
#define x_to_bit(x) (1ULL << (x))

ulong first_bit(ulong b) {
	return b & (~b + 1);
}
int next_bit(ulong* b, ulong i) {
	*b ^= i;
	return first_bit(*b);
}

#define foreach_bit(i, b) for (i = first_bit(b); i; i = next_bit(&b, i))

const int PLAYER_B = 1;
const int PLAYER_W = -1;

using std::string, std::max, std::min, std::vector;

enum {
	A1, B1, C1, D1, E1, F1, G1, H1,
	A2, B2, C2, D2, E2, F2, G2, H2,
	A3, B3, C3, D3, E3, F3, G3, H3,
	A4, B4, C4, D4, E4, F4, G4, H4,
	A5, B5, C5, D5, E5, F5, G5, H5,
	A6, B6, C6, D6, E6, F6, G6, H6,
	A7, B7, C7, D7, E7, F7, G7, H7,
	A8, B8, C8, D8, E8, F8, G8, H8,
	PASS, NOMOVE
};

/** coordinate to feature conversion */
typedef struct CoordinateToFeature {
	int n_feature;
	struct {
		int i;
		int x;
	} feature[16];
} CoordinateToFeature;

/** feature to coordinates conversion */
typedef struct FeatureToCoordinate {
	int n_square;
	int x[16];
} FeatureToCoordinate;

/** array to convert features into coordinates */
static const FeatureToCoordinate EVAL_F2X[] = {
	{ 9, {A1, B1, A2, B2, C1, A3, C2, B3, C3}},
	{ 9, {H1, G1, H2, G2, F1, H3, F2, G3, F3}},
	{ 9, {A8, A7, B8, B7, A6, C8, B6, C7, C6}},
	{ 9, {H8, H7, G8, G7, H6, F8, G6, F7, F6}},

	{10, {A5, A4, A3, A2, A1, B2, B1, C1, D1, E1}},
	{10, {H5, H4, H3, H2, H1, G2, G1, F1, E1, D1}},
	{10, {A4, A5, A6, A7, A8, B7, B8, C8, D8, E8}},
	{10, {H4, H5, H6, H7, H8, G7, G8, F8, E8, D8}},

	{10, {B2, A1, B1, C1, D1, E1, F1, G1, H1, G2}},
	{10, {B7, A8, B8, C8, D8, E8, F8, G8, H8, G7}},
	{10, {B2, A1, A2, A3, A4, A5, A6, A7, A8, B7}},
	{10, {G2, H1, H2, H3, H4, H5, H6, H7, H8, G7}},

	{10, {A1, C1, D1, C2, D2, E2, F2, E1, F1, H1}},
	{10, {A8, C8, D8, C7, D7, E7, F7, E8, F8, H8}},
	{10, {A1, A3, A4, B3, B4, B5, B6, A5, A6, A8}},
	{10, {H1, H3, H4, G3, G4, G5, G6, H5, H6, H8}},

	{ 8, {A2, B2, C2, D2, E2, F2, G2, H2}},
	{ 8, {A7, B7, C7, D7, E7, F7, G7, H7}},
	{ 8, {B1, B2, B3, B4, B5, B6, B7, B8}},
	{ 8, {G1, G2, G3, G4, G5, G6, G7, G8}},

	{ 8, {A3, B3, C3, D3, E3, F3, G3, H3}},
	{ 8, {A6, B6, C6, D6, E6, F6, G6, H6}},
	{ 8, {C1, C2, C3, C4, C5, C6, C7, C8}},
	{ 8, {F1, F2, F3, F4, F5, F6, F7, F8}},

	{ 8, {A4, B4, C4, D4, E4, F4, G4, H4}},
	{ 8, {A5, B5, C5, D5, E5, F5, G5, H5}},
	{ 8, {D1, D2, D3, D4, D5, D6, D7, D8}},
	{ 8, {E1, E2, E3, E4, E5, E6, E7, E8}},

	{ 8, {A1, B2, C3, D4, E5, F6, G7, H8}},
	{ 8, {A8, B7, C6, D5, E4, F3, G2, H1}},

	{ 7, {B1, C2, D3, E4, F5, G6, H7}},
	{ 7, {H2, G3, F4, E5, D6, C7, B8}},
	{ 7, {A2, B3, C4, D5, E6, F7, G8}},
	{ 7, {G1, F2, E3, D4, C5, B6, A7}},

	{ 6, {C1, D2, E3, F4, G5, H6}},
	{ 6, {A3, B4, C5, D6, E7, F8}},
	{ 6, {F1, E2, D3, C4, B5, A6}},
	{ 6, {H3, G4, F5, E6, D7, C8}},

	{ 5, {D1, E2, F3, G4, H5}},
	{ 5, {A4, B5, C6, D7, E8}},
	{ 5, {E1, D2, C3, B4, A5}},
	{ 5, {H4, G5, F6, E7, D8}},

	{ 4, {D1, C2, B3, A4}},
	{ 4, {A5, B6, C7, D8}},
	{ 4, {E1, F2, G3, H4}},
	{ 4, {H5, G6, F7, E8}},

	{ 0, {NOMOVE}}
};

static const CoordinateToFeature EVAL_X2F[] = {
	{7, {{ 0,    6561}, { 4,     243}, { 8,    6561}, {10,    6561}, {12,   19683}, {14,   19683}, {28,    2187}}},  /* a1 */
	{5, {{ 0,    2187}, { 4,      27}, { 8,    2187}, {18,    2187}, {30,     729}}},                                /* b1 */
	{6, {{ 0,      81}, { 4,       9}, { 8,     729}, {12,    6561}, {22,    2187}, {34,     243}}},                 /* c1 */
	{7, {{ 4,       3}, { 5,       1}, { 8,     243}, {12,    2187}, {26,    2187}, {38,      81}, {42,      27}}},  /* d1 */
	{7, {{ 4,       1}, { 5,       3}, { 8,      81}, {12,       9}, {27,    2187}, {40,      81}, {44,      27}}},  /* e1 */
	{6, {{ 1,      81}, { 5,       9}, { 8,      27}, {12,       3}, {23,    2187}, {36,     243}}},                 /* f1 */
	{5, {{ 1,    2187}, { 5,      27}, { 8,       9}, {19,    2187}, {33,     729}}},                                /* g1 */
	{7, {{ 1,    6561}, { 5,     243}, { 8,       3}, {11,    6561}, {12,       1}, {15,   19683}, {29,       1}}},  /* h1 */
	{5, {{ 0,     729}, { 4,     729}, {10,    2187}, {16,    2187}, {32,     729}}},                                /* a2 */
	{7, {{ 0,     243}, { 4,      81}, { 8,   19683}, {10,   19683}, {16,     729}, {18,     729}, {28,     729}}},  /* b2 */
	{6, {{ 0,       9}, {12,     729}, {16,     243}, {22,     729}, {30,     243}, {42,       9}}},                 /* c2 */
	{5, {{12,     243}, {16,      81}, {26,     729}, {34,      81}, {40,      27}}},                                /* d2 */
	{5, {{12,      81}, {16,      27}, {27,     729}, {36,      81}, {38,      27}}},                                /* e2 */
	{6, {{ 1,       9}, {12,      27}, {16,       9}, {23,     729}, {33,     243}, {44,       9}}},                 /* f2 */
	{7, {{ 1,     243}, { 5,      81}, { 8,       1}, {11,   19683}, {16,       3}, {19,     729}, {29,       3}}},  /* g2 */
	{5, {{ 1,     729}, { 5,     729}, {11,    2187}, {16,       1}, {31,     729}}},                                /* h2 */
	{6, {{ 0,      27}, { 4,    2187}, {10,     729}, {14,    6561}, {20,    2187}, {35,     243}}},                 /* a3 */
	{6, {{ 0,       3}, {14,     729}, {18,     243}, {20,     729}, {32,     243}, {42,       3}}},                 /* b3 */
	{5, {{ 0,       1}, {20,     243}, {22,     243}, {28,     243}, {40,       9}}},                                /* c3 */
	{4, {{20,      81}, {26,     243}, {30,      81}, {36,      27}}},                                               /* d3 */
	{4, {{20,      27}, {27,     243}, {33,      81}, {34,      27}}},                                               /* e3 */
	{5, {{ 1,       1}, {20,       9}, {23,     243}, {29,       9}, {38,       9}}},                                /* f3 */
	{6, {{ 1,       3}, {15,     729}, {19,     243}, {20,       3}, {31,     243}, {44,       3}}},                 /* g3 */
	{6, {{ 1,      27}, { 5,    2187}, {11,     729}, {15,    6561}, {20,       1}, {37,     243}}},                 /* h3 */
	{7, {{ 4,    6561}, { 6,   19683}, {10,     243}, {14,    2187}, {24,    2187}, {39,      81}, {42,       1}}},  /* a4 */
	{5, {{14,     243}, {18,      81}, {24,     729}, {35,      81}, {40,       3}}},                                /* b4 */
	{4, {{22,      81}, {24,     243}, {32,      81}, {36,       9}}},                                               /* c4 */
	{4, {{24,      81}, {26,      81}, {28,      81}, {33,      27}}},                                               /* d4 */
	{4, {{24,      27}, {27,      81}, {29,      27}, {30,      27}}},                                               /* e4 */
	{4, {{23,      81}, {24,       9}, {31,      81}, {34,       9}}},                                               /* f4 */
	{5, {{15,     243}, {19,      81}, {24,       3}, {37,      81}, {38,       3}}},                                /* g4 */
	{7, {{ 5,    6561}, { 7,   19683}, {11,     243}, {15,    2187}, {24,       1}, {41,      81}, {44,       1}}},  /* h4 */
	{7, {{ 4,   19683}, { 6,    6561}, {10,      81}, {14,       9}, {25,    2187}, {40,       1}, {43,      27}}},  /* a5 */
	{5, {{14,      81}, {18,      27}, {25,     729}, {36,       3}, {39,      27}}},                                /* b5 */
	{4, {{22,      27}, {25,     243}, {33,       9}, {35,      27}}},                                               /* c5 */
	{4, {{25,      81}, {26,      27}, {29,      81}, {32,      27}}},                                               /* d5 */
	{4, {{25,      27}, {27,      27}, {28,      27}, {31,      27}}},                                               /* e5 */
	{4, {{23,      27}, {25,       9}, {30,       9}, {37,      27}}},                                               /* f5 */
	{5, {{15,      81}, {19,      27}, {25,       3}, {34,       3}, {41,      27}}},                                /* g5 */
	{7, {{ 5,   19683}, { 7,    6561}, {11,      81}, {15,       9}, {25,       1}, {38,       1}, {45,      27}}},  /* h5 */
	{6, {{ 2,      81}, { 6,    2187}, {10,      27}, {14,       3}, {21,    2187}, {36,       1}}},                 /* a6 */
	{6, {{ 2,       9}, {14,      27}, {18,       9}, {21,     729}, {33,       3}, {43,       9}}},                 /* b6 */
	{5, {{ 2,       1}, {21,     243}, {22,       9}, {29,     243}, {39,       9}}},                                /* c6 */
	{4, {{21,      81}, {26,       9}, {31,       9}, {35,       9}}},                                               /* d6 */
	{4, {{21,      27}, {27,       9}, {32,       9}, {37,       9}}},                                               /* e6 */
	{5, {{ 3,       1}, {21,       9}, {23,       9}, {28,       9}, {41,       9}}},                                /* f6 */
	{6, {{ 3,       9}, {15,      27}, {19,       9}, {21,       3}, {30,       3}, {45,       9}}},                 /* g6 */
	{6, {{ 3,      81}, { 7,    2187}, {11,      27}, {15,       3}, {21,       1}, {34,       1}}},                 /* h6 */
	{5, {{ 2,    2187}, { 6,     729}, {10,       9}, {17,    2187}, {33,       1}}},                                /* a7 */
	{7, {{ 2,     243}, { 6,      81}, { 9,   19683}, {10,       1}, {17,     729}, {18,       3}, {29,     729}}},  /* b7 */
	{6, {{ 2,       3}, {13,     729}, {17,     243}, {22,       3}, {31,       3}, {43,       3}}},                 /* c7 */
	{5, {{13,     243}, {17,      81}, {26,       3}, {37,       3}, {39,       3}}},                                /* d7 */
	{5, {{13,      81}, {17,      27}, {27,       3}, {35,       3}, {41,       3}}},                                /* e7 */
	{6, {{ 3,       3}, {13,      27}, {17,       9}, {23,       3}, {32,       3}, {45,       3}}},                 /* f7 */
	{7, {{ 3,     243}, { 7,      81}, { 9,       1}, {11,       1}, {17,       3}, {19,       3}, {28,       3}}},  /* g7 */
	{5, {{ 3,    2187}, { 7,     729}, {11,       9}, {17,       1}, {30,       1}}},                                /* h7 */
	{7, {{ 2,    6561}, { 6,     243}, { 9,    6561}, {10,       3}, {13,   19683}, {14,       1}, {29,    2187}}},  /* a8 */
	{5, {{ 2,     729}, { 6,      27}, { 9,    2187}, {18,       1}, {31,       1}}},                                /* b8 */
	{6, {{ 2,      27}, { 6,       9}, { 9,     729}, {13,    6561}, {22,       1}, {37,       1}}},                 /* c8 */
	{7, {{ 6,       3}, { 7,       1}, { 9,     243}, {13,    2187}, {26,       1}, {41,       1}, {43,       1}}},  /* d8 */
	{7, {{ 6,       1}, { 7,       3}, { 9,      81}, {13,       9}, {27,       1}, {39,       1}, {45,       1}}},  /* e8 */
	{6, {{ 3,      27}, { 7,       9}, { 9,      27}, {13,       3}, {23,       1}, {35,       1}}},                 /* f8 */
	{5, {{ 3,     729}, { 7,      27}, { 9,       9}, {19,       1}, {32,       1}}},                                /* g8 */
	{7, {{ 3,    6561}, { 7,     243}, { 9,       3}, {11,       3}, {13,       1}, {15,       1}, {28,       1}}},  /* h8 */
	{0, {{ 0, 0}}} // <- PASS
};

/** feature size */
static const int EVAL_SIZE[] = { 19683, 59049, 59049, 59049, 6561, 6561, 6561, 6561, 2187,729, 243, 81, 1 };

/** packed feature size */
static const int EVAL_PACKED_SIZE[] = { 10206, 29889, 29646, 29646, 3321, 3321, 3321, 3321, 1134, 378, 135, 45, 1 };

/** feature offset */
static const int EVAL_OFFSET[] = {
		 0,      0,      0,      0,
	 19683,  19683,  19683,  19683,
	 78732,  78732,  78732,  78732,
	137781,	137781, 137781, 137781,
	196830,	196830, 196830, 196830,
	203391,	203391, 203391, 203391,
	209952,	209952, 209952, 209952,
	216513,	216513,
	223074,	223074,	223074,	223074,
	225261,	225261,	225261,	225261,
	225990,	225990, 225990,	225990,
	226233,	226233, 226233, 226233,
	226314,
};

/** feature symetry packing */
static int EVAL_C10[59049];
static int EVAL_S10[59049];
static int EVAL_C9[19683];
static int EVAL_S8[6561];
static int EVAL_S7[2187];
static int EVAL_S6[729];
static int EVAL_S5[243];
static int EVAL_S4[81];

/** number of (unpacked) weights */
static const int EVAL_N_WEIGHT = 226315;

/** number of plies */
static const int EVAL_N_PLY = 2;

/** number of features */
static const int EVAL_N_FEATURE = 47;

/** eval weight load status */
static int EVAL_LOADED = 0;

/** eval weights */
short EVAL_WEIGHT[EVAL_N_PLY][EVAL_N_WEIGHT];

void eval_open(unsigned char* w)
{
	const int n_w = 114364;
	int* T;
	int ply, i, j, k, l, n;
	int offset;

	T = (int*)malloc(59049 * sizeof(*T));

	for (l = n = 0; l < 6561; l++) { /* 8 squares : 6561 -> 3321 */
		k = ((l / 2187) % 3) + ((l / 729) % 3) * 3 + ((l / 243) % 3) * 9 +
			((l / 81) % 3) * 27 + ((l / 27) % 3) * 81 + ((l / 9) % 3) * 243 +
			((l / 3) % 3) * 729 + (l % 3) * 2187;
		if (k < l) T[l] = T[k];
		else T[l] = n++;
		EVAL_S8[l] = T[l];
	}

	for (l = n = 0; l < 2187; l++) { /* 7 squares : 2187 -> 1134 */
		k = ((l / 729) % 3) + ((l / 243) % 3) * 3 + ((l / 81) % 3) * 9 +
			((l / 27) % 3) * 27 + ((l / 9) % 3) * 81 + ((l / 3) % 3) * 243 +
			(l % 3) * 729;
		if (k < l) T[l] = T[k];
		else T[l] = n++;
		EVAL_S7[l] = T[l];
	}
	for (l = n = 0; l < 729; l++) { /* 6 squares : 729 -> 378 */
		k = ((l / 243) % 3) + ((l / 81) % 3) * 3 + ((l / 27) % 3) * 9 +
			((l / 9) % 3) * 27 + ((l / 3) % 3) * 81 + (l % 3) * 243;
		if (k < l) T[l] = T[k];
		else T[l] = n++;
		EVAL_S6[l] = T[l];
	}
	for (l = n = 0; l < 243; l++) { /* 5 squares : 243 -> 135 */
		k = ((l / 81) % 3) + ((l / 27) % 3) * 3 + ((l / 9) % 3) * 9 +
			((l / 3) % 3) * 27 + (l % 3) * 81;
		if (k < l) T[l] = T[k];
		else T[l] = n++;
		EVAL_S5[l] = T[l];
	}
	for (l = n = 0; l < 81; l++) { /* 4 squares : 81 -> 45 */
		k = ((l / 27) % 3) + ((l / 9) % 3) * 3 + ((l / 3) % 3) * 9 + (l % 3) * 27;
		if (k < l) T[l] = T[k];
		else T[l] = n++;
		EVAL_S4[l] = T[l];
	}
	for (l = n = 0; l < 19683; l++) { /* 9 corner squares : 19683 -> 10206 */
		k = ((l / 6561) % 3) * 6561 + ((l / 729) % 3) * 2187 +
			((l / 2187) % 3) * 729 + ((l / 243) % 3) * 243 + ((l / 27) % 3) * 81 +
			((l / 81) % 3) * 27 + ((l / 3) % 3) * 9 + ((l / 9) % 3) * 3 + (l % 3);
		if (k < l) T[l] = T[k];
		else T[l] = n++;
		EVAL_C9[l] = T[l];
	}
	for (l = n = 0; l < 59049; l++) { /* 10 squares (edge +X ) : 59049 -> 29646 */
		k = ((l / 19683) % 3) + ((l / 6561) % 3) * 3 + ((l / 2187) % 3) * 9 +
			((l / 729) % 3) * 27 + ((l / 243) % 3) * 81 + ((l / 81) % 3) * 243 +
			((l / 27) % 3) * 729 + ((l / 9) % 3) * 2187 + ((l / 3) % 3) * 6561 +
			(l % 3) * 19683;
		if (k < l) T[l] = T[k];
		else T[l] = n++;
		EVAL_S10[l] = T[l];
	}
	for (l = n = 0; l < 59049; l++) { /* 10 squares (angle + X) : 59049 -> 29889 */
		k = ((l / 19683) % 3) + ((l / 6561) % 3) * 3 + ((l / 2187) % 3) * 9 +
			((l / 729) % 3) * 27 + ((l / 243) % 3) * 243 + ((l / 81) % 3) * 81 +
			((l / 27) % 3) * 729 + ((l / 9) % 3) * 2187 + ((l / 3) % 3) * 6561 +
			(l % 3) * 19683;
		if (k < l) T[l] = T[k];
		else T[l] = n++;
		EVAL_C10[l] = T[l];
	}
	free(T);

	short p[EVAL_N_PLY][12][8] = {
		{
			{ -307, -69, -53, -13, 16, 32, 61, 303 },
			{ -165, -36, -10, 0, 3, 17, 41, 169 },
			{ -154, -34, -10, 0, 1, 11, 36, 165 },
			{ -119, -22, -4, 0, 0, 9, 30, 139 },
			{ -40, -2, 0, 0, 6, 15, 32, 100 },
			{ -110, -22, -5, 0, 0, 5, 19, 73 },
			{ -173, -37, -11, -2, 0, 3, 20, 102 },
			{ -71, -11, 0, 0, 7, 20, 48, 173 },
			{ -27, -1, 0, 2, 9, 20, 46, 182 },
			{ 0, 0, 2, 7, 15, 29, 55, 186 },
			{ -7, 0, 4, 8, 13, 23, 47, 158 },
			{ 0, 1, 6, 9, 13, 20, 45, 116 },
		},
		{
			{ -391, -111, -48, -16, 24, 51, 105, 388 },
			{ -307, -101, -40, -8, 16, 48, 112, 329 },
			{ -300, -102, -41, -9, 10, 43, 110, 315 },
			{ -276, -84, -28, -1, 11, 43, 109, 310 },
			{ -161, -33, -1, 11, 35, 70, 143, 333 },
			{ -294, -95, -36, -8, 7, 37, 97, 262 },
			{ -337, -136, -57, -21, 0, 27, 95, 296 },
			{ -205, -55, -10, 10, 40, 87, 184, 418 },
			{ -82, -15, 4, 22, 47, 91, 177, 417 },
			{ -39, 9, 25, 45, 78, 124, 205, 438 },
			{ -67, 2, 17, 42, 70, 114, 169, 351 },
			{ -36, 9, 39, 62, 109, 146, 180, 308 },
		}
	};

	for (ply = 0; ply < EVAL_N_PLY; ply++) {
		offset = 114364 * ply;
		i = j = 0;
		for (k = 0; k < EVAL_SIZE[i]; k++, j++) {
			EVAL_WEIGHT[ply][j] = p[ply][0][w[EVAL_C9[k] + offset]];
		}
		offset += EVAL_PACKED_SIZE[i];
		i++;
		for (k = 0; k < EVAL_SIZE[i]; k++, j++) {
			EVAL_WEIGHT[ply][j] = p[ply][1][w[EVAL_C10[k] + offset]];
		}
		offset += EVAL_PACKED_SIZE[i];
		i++;
		for (k = 0; k < EVAL_SIZE[i]; k++, j++) {
			EVAL_WEIGHT[ply][j] = p[ply][2][w[EVAL_S10[k] + offset]];
		}
		offset += EVAL_PACKED_SIZE[i];
		i++;
		for (k = 0; k < EVAL_SIZE[i]; k++, j++) {
			EVAL_WEIGHT[ply][j] = p[ply][3][w[EVAL_S10[k] + offset]];
		}
		offset += EVAL_PACKED_SIZE[i];
		i++;
		for (k = 0; k < EVAL_SIZE[i]; k++, j++) {
			EVAL_WEIGHT[ply][j] = p[ply][4][w[EVAL_S8[k] + offset]];
		}
		offset += EVAL_PACKED_SIZE[i];
		i++;
		for (k = 0; k < EVAL_SIZE[i]; k++, j++) {
			EVAL_WEIGHT[ply][j] = p[ply][5][w[EVAL_S8[k] + offset]];
		}
		offset += EVAL_PACKED_SIZE[i];
		i++;
		for (k = 0; k < EVAL_SIZE[i]; k++, j++) {
			EVAL_WEIGHT[ply][j] = p[ply][6][w[EVAL_S8[k] + offset]];
		}
		offset += EVAL_PACKED_SIZE[i];
		i++;
		for (k = 0; k < EVAL_SIZE[i]; k++, j++) {
			EVAL_WEIGHT[ply][j] = p[ply][7][w[EVAL_S8[k] + offset]];
		}
		offset += EVAL_PACKED_SIZE[i];
		i++;
		for (k = 0; k < EVAL_SIZE[i]; k++, j++) {
			EVAL_WEIGHT[ply][j] = p[ply][8][w[EVAL_S7[k] + offset]];
		}
		offset += EVAL_PACKED_SIZE[i];
		i++;
		for (k = 0; k < EVAL_SIZE[i]; k++, j++) {
			EVAL_WEIGHT[ply][j] = p[ply][9][w[EVAL_S6[k] + offset]];
		}
		offset += EVAL_PACKED_SIZE[i];
		i++;
		for (k = 0; k < EVAL_SIZE[i]; k++, j++) {
			EVAL_WEIGHT[ply][j] = p[ply][10][w[EVAL_S5[k] + offset]];
		}
		offset += EVAL_PACKED_SIZE[i];
		i++;
		for (k = 0; k < EVAL_SIZE[i]; k++, j++) {
			EVAL_WEIGHT[ply][j] = p[ply][11][w[EVAL_S4[k] + offset]];
		}
		offset += EVAL_PACKED_SIZE[i];
		i++;
		EVAL_WEIGHT[ply][j] = w[offset];
	}
	free(w);
}

const int SCORE_MIN = -64;
const int SCORE_MAX = 64;

float eval()
{
	int stage = 0;

	const short* w = EVAL_WEIGHT[stage];
	int f[46];

	int score = w[f[0]] + w[f[1]] + w[f[2]] + w[f[3]]
		+ w[f[4]] + w[f[5]] + w[f[6]] + w[f[7]]
		+ w[f[8]] + w[f[9]] + w[f[10]] + w[f[11]]
		+ w[f[12]] + w[f[13]] + w[f[14]] + w[f[15]]
		+ w[f[16]] + w[f[17]] + w[f[18]] + w[f[19]]
		+ w[f[20]] + w[f[21]] + w[f[22]] + w[f[23]]
		+ w[f[24]] + w[f[25]]
		+ w[f[26]] + w[f[27]] + w[f[28]] + w[f[29]]
		+ w[f[30]] + w[f[31]] + w[f[32]] + w[f[33]]
		+ w[f[34]] + w[f[35]] + w[f[36]] + w[f[37]]
		+ w[f[38]] + w[f[39]] + w[f[40]] + w[f[41]]
		+ w[f[42]] + w[f[43]] + w[f[44]] + w[f[45]]
		+ w[226314];

	if (score > 0) score += 64; else score -= 64;
	score /= 128;

	if (score <= SCORE_MIN) score = SCORE_MIN + 1;
	else if (score >= SCORE_MAX) score = SCORE_MAX - 1;

	return score;
}

#define mirror_h _byteswap_uint64

ulong transpose(ulong b)
{
	__m256i	v = _mm256_sllv_epi64(_mm256_broadcastq_epi64(_mm_cvtsi64_si128(b)),
		_mm256_set_epi64x(0, 1, 2, 3));
	return ((ulong) _mm256_movemask_epi8(v) << 32)
		| (unsigned int)_mm256_movemask_epi8(_mm256_slli_epi64(v, 4));
}

ulong rotate90(ulong b) {
	return transpose(mirror_h(b));
}

ulong get_moves(const ulong P, const ulong O)
{
	__m256i	PP, mOO, MM, flip_l, flip_r, pre_l, pre_r, shift2;
	__m128i	M;
	const __m256i shift1897 = _mm256_set_epi64x(7, 9, 8, 1);
	const __m256i mflipH = _mm256_set_epi64x(0x7e7e7e7e7e7e7e7e, 0x7e7e7e7e7e7e7e7e, -1, 0x7e7e7e7e7e7e7e7e);

	PP = _mm256_broadcastq_epi64(_mm_cvtsi64_si128(P));
	mOO = _mm256_and_si256(_mm256_broadcastq_epi64(_mm_cvtsi64_si128(O)), mflipH);

	flip_l = _mm256_and_si256(mOO, _mm256_sllv_epi64(PP, shift1897));
	flip_r = _mm256_and_si256(mOO, _mm256_srlv_epi64(PP, shift1897));
	flip_l = _mm256_or_si256(flip_l, _mm256_and_si256(mOO, _mm256_sllv_epi64(flip_l, shift1897)));
	flip_r = _mm256_or_si256(flip_r, _mm256_and_si256(mOO, _mm256_srlv_epi64(flip_r, shift1897)));
	pre_l = _mm256_and_si256(mOO, _mm256_sllv_epi64(mOO, shift1897));
	pre_r = _mm256_srlv_epi64(pre_l, shift1897);
	shift2 = _mm256_add_epi64(shift1897, shift1897);
	flip_l = _mm256_or_si256(flip_l, _mm256_and_si256(pre_l, _mm256_sllv_epi64(flip_l, shift2)));
	flip_r = _mm256_or_si256(flip_r, _mm256_and_si256(pre_r, _mm256_srlv_epi64(flip_r, shift2)));
	flip_l = _mm256_or_si256(flip_l, _mm256_and_si256(pre_l, _mm256_sllv_epi64(flip_l, shift2)));
	flip_r = _mm256_or_si256(flip_r, _mm256_and_si256(pre_r, _mm256_srlv_epi64(flip_r, shift2)));
	MM = _mm256_sllv_epi64(flip_l, shift1897);
	MM = _mm256_or_si256(MM, _mm256_srlv_epi64(flip_r, shift1897));

	M = _mm_or_si128(_mm256_castsi256_si128(MM), _mm256_extracti128_si256(MM, 1));
	M = _mm_or_si128(M, _mm_unpackhi_epi64(M, M));
	return _mm_cvtsi128_si64(M) & ~(P | O);	// mask with empties
}


ulong GetReversedL(ulong move, ulong player, int offset, ulong mask)
{
	ulong r = (move << offset) & mask;
	r |= (r << offset) & mask;
	r |= (r << offset) & mask;
	r |= (r << offset) & mask;
	r |= (r << offset) & mask;
	r |= (r << offset) & mask;

	if (((r << offset) & player) != 0)
		return r;
	return 0;
}

ulong GetReversedR(ulong move, ulong player, int offset, ulong mask)
{
	ulong r = (move >> offset) & mask;
	r |= (r >> offset) & mask;
	r |= (r >> offset) & mask;
	r |= (r >> offset) & mask;
	r |= (r >> offset) & mask;
	r |= (r >> offset) & mask;

	if (((r >> offset) & player) != 0)
		return r;
	return 0;
}

ulong Reverse(ulong player, ulong opponent, ulong move)
{
	ulong verticalMask = opponent & 0x7e7e7e7e7e7e7e7eUL;

	ulong reversed = 0;
	reversed |= GetReversedL(move, player, 8, opponent);
	reversed |= GetReversedR(move, player, 8, opponent);
	reversed |= GetReversedL(move, player, 1, verticalMask);
	reversed |= GetReversedR(move, player, 1, verticalMask);
	reversed |= GetReversedL(move, player, 9, verticalMask);
	reversed |= GetReversedR(move, player, 9, verticalMask);
	reversed |= GetReversedL(move, player, 7, verticalMask);
	reversed |= GetReversedR(move, player, 7, verticalMask);

	return reversed;
}

struct Board {
	ulong bit_b, bit_w;     /**< bitboard representation */

	Board(ulong b, ulong w) {
		this->bit_b = b;
		this->bit_w = w;
	}
};

ulong flip(const Board* b, ulong move) {
	return Reverse(b->bit_b, b->bit_w, move);
}

Board* apply_flip(const Board* b, const ulong move, const ulong f) {
	return new Board(b->bit_w ^ f, b->bit_b ^ (move | f));
}

Board* flipped(const Board* b, const ulong move) {
	ulong f = flip(b, move);
	return apply_flip(b, move, f);
}

ulong get_moves(const Board *b) {
	return get_moves(b->bit_b, b->bit_w);
}

ulong get_moves_opponent(const Board* b) {
	return get_moves(b->bit_w, b->bit_b);
}

void swap_color(Board* b) {
	const ulong tmp = b->bit_b;
	b->bit_b = b->bit_w;
	b->bit_w = tmp;
}

Board* swaped_color(Board* b) {
	return new Board(b->bit_w, b->bit_b);
}

int n_discs(Board *b) {
	return popcount(b->bit_b) + popcount(b->bit_w);
}

int n_discs_gap(Board *b) {
	return popcount(b->bit_b) - popcount(b->bit_w);
}

string bit_to_string(ulong b) {
	const string line = "+---+---+---+---+---+---+---+---+";
	string s = line + "\r\n";

	for (int i = 0; i < 64; i++) {
		if (i % 8 == 0 && i != 0) {
			s += "|\r\n" + line + "\r\n";
		}

		if (b & x_to_bit(i)) {
			s += "| X ";
		}
		else {
			s += "|   ";
		}
	}
	s += "|\r\n" + line + "\r\n";

	return s;
}

string b_to_string(Board* b) {
	const string line = "+---+---+---+---+---+---+---+---+";
	string s = line + "\r\n";

	for (int i = 0; i < 64; i++) {
		if (i % 8 == 0 && i != 0) {
			s += "|\r\n" + line + "\r\n";
		}

		if (b->bit_b & x_to_bit(i)) {
			s += "| X ";
		}
		else if (b->bit_w & x_to_bit(i)) {
			s += "| O ";
		}
		else {
			s += "|   ";
		}
	}
	s += "|\r\n" + line + "\r\n";

	return s;
}

class Search {
public:
	Board* b;

	void flip(ulong move, ulong flip) {
		ulong tmp = b->bit_b ^ (move | flip);
		b->bit_b = b->bit_w ^ move;
		b->bit_w = tmp;
	}

	void undo_flip(ulong move, ulong flip) {
		ulong tmp = b->bit_w ^ (move | flip);
		b->bit_w = b->bit_b ^ move;
		b->bit_b = tmp;
	}

	void flip_color() {
		ulong tmp = b->bit_b;
		b->bit_b = b->bit_w;
		b->bit_w = tmp;
	}
};

Search* search = new Search;

struct Move {
	ulong move;
	ulong reversed_b;
	ulong moves;

	Move(Board *b, ulong move)
	{
		this->move = move;
		this->reversed_b = flip(b, move);

		search->flip(move, this->reversed_b);
		this->moves = get_moves(search->b);
		search->undo_flip(move, this->reversed_b);
	}

	Move(ulong move, ulong reversed_b, ulong moves)
	{
		this->move = move;
		this->reversed_b = reversed_b;
		this->moves = moves;
	}

	Move() {
		this->move = 0;
		this->reversed_b = 0;
		this->moves = 0;
	}

	bool operator<(const Move& right) const {
		return popcount(moves) < popcount(right.moves);
	}
};

vector<Move> create_moves(Board* b, ulong moves) {
	vector<Move> v(popcount(moves));

	ulong m;
	int i = 0;
	while ((m = first_bit(moves)) != 0)
	{
		moves = moves ^ m;
		v[i++] = { b, m };
	}
	return v;
}

struct SP {
	int depth;
	float alpha, beta;
};

SP p_swap(SP p) {
	return { p.depth, -p.beta, -p.alpha };
}

SP p_deepen(SP p) {
	return { p.depth - 1, -p.beta, -p.alpha };
}

SP p_null_window(SP p) {
	return  { p.depth - 1, -p.alpha - 1, -p.alpha };
}

bool use_transposition_cut = true;
const int transposition = 1;
const int ordering_depth = 57;

float solve(Move* move, SP p);
float solve_(Move* move, SP p);

float null_window_search(Move* m, SP p) {
	return -solve(m, p_null_window(p));
}

float negascout(Board* b, ulong moves, SP p)
{
	ulong move = first_bit(moves);
	moves ^= move;
	float max_e = -solve(new Move(b, move), p_deepen(p));

	if (p.beta <= max_e)
		return max_e;

	p.alpha = max(p.alpha, max_e);

	while ((move = first_bit(moves)) != 0)
	{
		moves = moves ^ move;
		Move *m = new Move(b, move);

		float eval = null_window_search(m, p);

		if (p.beta <= eval)
		{
			return eval;
		}

		if (p.alpha < eval)
		{
			p.alpha = eval;
			eval = -solve(m, p_deepen(p));

			if (p.beta <= eval)
				return eval;

			p.alpha = max(p.alpha, eval);
		}
		max_e = max(max_e, eval);
	}
	return max_e;
}

float negascout(vector<Move> moves, SP p)
{
	float max_e = -solve(&moves[0], p_deepen(p));

	if (p.beta <= max_e)
		return max_e;

	p.alpha = max(p.alpha, max_e);

	for (int i = 1; i < moves.size(); ++i)
	{
		Move *move = &moves[i];
		float eval = null_window_search(move, p);

		if (p.beta <= eval)
			return eval;

		if (p.alpha < eval)
		{
			p.alpha = eval;
			eval = -solve(move, p_deepen(p));

			if (p.beta <= eval)
				return eval;

			p.alpha = max(p.alpha, eval);
		}
		max_e = max(max_e, eval);
	}
	return max_e;
}

float negamax(Board* b, ulong moves, SP p)
{
	float max_e = -1000000;

	/*string indent = "";
	for (int i = 0; i < 64 - p.depth; i++)
		indent += "    ";

	std::cout << indent << "num moves: " << popcount(moves) <<  ", num discs: " << n_discs(b) << "\r\n";*/

	ulong move;
	while((move = first_bit(moves)) != 0)
	{
		moves = moves ^ move;
		/*std::cout << indent << "depth: " << p.depth << ", move: " << move
			<< ", ab: [" << p.alpha << ", " << p.beta << "]" << "\r\n";*/

		Move* m = new Move(b, move);
		float e = -solve(m, p_deepen(p));

		max_e = max(max_e, e);
		p.alpha = max(p.alpha, e);

		/*std::cout << indent << "depth: " << p.depth << ", move: " << move
			<< ", ab: [" << p.alpha  << ", " << p.beta << "]" << ", e: " << e << "\r\n";*/

		if (p.alpha >= p.beta)
			return max_e;
	}
	return max_e;
}

float solve(Move *move, SP p) {
	std::cout << "depth " << p.depth << "\r\n";
	std::cout << "\r\n";

	std::cout << "move\r\n";
	std::cout << bit_to_string(move->move);
	std::cout << "\r\n";

	std::cout << "board\r\n";
	std::cout << b_to_string(search->b);
	std::cout << "\r\n";

	if (move->move != 0)
		search->flip(move->move, move->reversed_b);
	else
		search->flip_color();

	float e = solve_(move, p);

	if (move->move != 0)
		search->undo_flip(move->move, move->reversed_b);
	else
		search->flip_color();

	return e;
}

int count = 0;

float solve_(Move *move, SP p) {
	if (p.depth <= 0)
		// return n_discs_gap(search->b);
		return eval();

	//std::cout << "moves\r\n";
	//std::cout << bit_to_string(moves) << "\r\n";
	//std::cout << p.depth << "\r\n";

	if (move->moves == 0)
	{
		ulong opponent_moves = get_moves_opponent(search->b);
		if (opponent_moves == 0)
		{
			count++;
			return n_discs_gap(search->b);
		}
		else
		{
			Move *next = new Move(0, 0, opponent_moves);
			return -solve(next, p_swap(p));
		}
	}

	//if (n_discs(move->reversed) == 63 && popcount(move->moves) == 1)
	//	return -n_discs_gap(flip_b(move->reversed, move->moves));

	float lower = -1000000;
	float upper = 1000000;

	float value;

	if (p.depth >= 3 && n_discs(search->b) < 60)
	{
		//if (popcount(move->moves) > 3)
		//{
		//	vector<Move> moves = create_moves(search->b, move->move);

		//	/*if (p.depth >= 4 && table_prev != null)
		//		Array.Sort(moves, comparer);
		//	else*/
		//		std::sort(moves.begin(), moves.end());

		//	value = negascout(moves, p);
		//}
		//else
			// value = negascout(search->b, move->moves, p);
			value = negamax(search->b, move->moves, p);
	}
	else
	{
		value = negamax(search->b, move->moves, p);
	}
	return value;
}

float solve_root(SP p) {
	vector<Move> moves = create_moves(search->b, get_moves(search->b));

	float max_e = -solve(&moves[0], p_deepen(p));

	if (p.beta <= max_e)
		return max_e;

	p.alpha = max(p.alpha, max_e);

	for (int i = 1; i < moves.size(); ++i)
	{
		Move* move = &moves[i];
		float eval = null_window_search(move, p);

		if (p.beta <= eval)
			return eval;

		if (p.alpha < eval)
		{
			p.alpha = eval;
			eval = -solve(move, p_deepen(p));

			if (p.beta <= eval)
				return eval;

			p.alpha = max(p.alpha, eval);
		}
		max_e = max(max_e, eval);
	}
	return max_e;
}

int main()
{
	//search->b = new Board(4049267728759866752, 364259906054797424);
	search->b = new Board(36170362438582400, 4635812603068025);

	auto start = std::chrono::system_clock::now();

	float e = solve_root({64, -10000000, 10000000});

	auto end = std::chrono::system_clock::now();
	double time = static_cast<double>(std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() / 1000.0);

	std::cout << "Result : " << e << "\r\n";
	std::cout << "Time : " << time << "\r\n";
	std::cout << "Nodes : " << count << "\r\n";
	std::cout << "Time/Node : " << (1000000 * time / count) << "\r\n";
}

