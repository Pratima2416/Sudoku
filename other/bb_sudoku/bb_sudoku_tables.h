/****************************************************************\
**  BB_Sudoku  Bit Based Sudoku Solver                          **
\****************************************************************/

/****************************************************************\
**  (c) Copyright Brian Turner, 2008-2009.  This file may be    **
**      freely used, modified, and copied for personal,         **
**      educational, or non-commercial purposes provided this   **
**      notice remains attached.                                **
\****************************************************************/

/****************************************************************\
**  BB_Sudoku_Tables.h : This file contains the various tables  **
**    used by the solver:                                       **
**                                                              **
**  General purpose tables:                                     **
**    V2B     : Value to Binary provides a lookup from a value  **
**                to its bit position.                          **
**    B2V     : Binary to Value provides a lookup from a bit    **
**                to a value, but only if a single bit is set.  **
**                If multiple bits are set, it returns 0.       **
**    NSet    : Number of bits set for a given possibility      **
**                value (so 0x0F0 gives 4, 0x004 gives 1)       **
**                                                              **
**  Group Tables (for Naked and Hidden Singles)                 **
**    Group   : Lists the cells that belong to each group       **
**    C2Group : Cell to Group, provides a list of groups that   **
**                contain a given cell                          **
**                                                              **
**  Locked Candidate tables (for Lock Candidates)               **
**    LC_Segment : defines related segments for the LC search   **
**                                                              **
\****************************************************************/

#ifndef _bb_sudoku_header_
#define _bb_sudoku_header_

// Forward declarations for the solver and related routines
int  Solver (char num_search, unsigned int use_methods, char ret_puzzle, int initp, char* buffer);
int  Generate_Puzzle (char* buffer);
int  Generate_Sol (char* buffer);
void Minimize (char* buffer);
void Normalize (char* buffer);
void Randomize (char* buffer);
int  RatePuzzle (void);
int  CatPuzzle (char *buf);


// masks for methodologies to use
#define USE_GUESSES     0x01  // use backtracking and guesses
#define USE_LOCK_CAND   0x02  // use locked candidates
#define USE_SUBSETS     0x04  // use subset searching
#define USE_FISHIES     0x08  // use fishies (x-wing, swordfish, jellyfish)
#define USE_1_STEP      0x10  // use one step commonalitytest
#define USE_2_STEP      0x20  // use two step commonality test

// masks for variations
#define GT_LT_PUZZLE   0x100  // Greater Than and Less Than puzzle
#define KILLER_PUZZLE  0x200  // Killer Sudoku puzzle


// V2B and B2V stand for "Binary to Value" and "Value to
//   Binary", and provide a quick lookup between bit positions
//   and values.  This provides an easy method to convert
//   between values and bit positions for single values.
//
// NSet looks up the number of bits set (potential values)
//   for numbers 0 to 511 (9 bits).  NSetX provides a lookup
//   to the xth bit in the value

// Values to Bit positions lookup
unsigned int V2B[33] = {0x0000, 0x00000001, 0x00000002, 0x00000004, 0x00000008,
                                0x00000010, 0x00000020, 0x00000040, 0x00000080,
                                0x00000100, 0x00000200, 0x00000400, 0x00000800,
                                0x00001000, 0x00002000, 0x00004000, 0x00008000,
                                0x00010000, 0x00020000, 0x00040000, 0x00080000,
                                0x00100000, 0x00200000, 0x00400000, 0x00800000,
                                0x01000000, 0x02000000, 0x04000000, 0x08000000,
                                0x10000000, 0x20000000, 0x40000000, 0x80000000};

// Bits to Value lookup
int B2V[512];

// Number of bits set in the number, up to 9 bits, along with
//   a lookup for getting the xth bit of the value
int NSet[512], NSetX[512][10];


// Groups and P2Group define the groups for a standard
//   3x3 grid and a look from a position (0-80) to the
//   three groups it is a part of.

// In_Groups defines all the cells included in all the groups associated with
//   with the index cell.  It includes 8 for the 8, 8 for the column, and 4 for
//   the remaining cells in the box that are not in the row or column.
const unsigned char In_Groups[81][20] =
  {{  1,  2,  3,  4,  5,  6,  7,  8,  9, 18, 27, 36, 45, 54, 63, 72, 10, 11, 19, 20 },
   {  0,  2,  3,  4,  5,  6,  7,  8, 10, 19, 28, 37, 46, 55, 64, 73,  9, 11, 18, 20 },
   {  0,  1,  3,  4,  5,  6,  7,  8, 11, 20, 29, 38, 47, 56, 65, 74,  9, 10, 18, 19 },
   {  0,  1,  2,  4,  5,  6,  7,  8, 12, 21, 30, 39, 48, 57, 66, 75, 13, 14, 22, 23 },
   {  0,  1,  2,  3,  5,  6,  7,  8, 13, 22, 31, 40, 49, 58, 67, 76, 12, 14, 21, 23 },
   {  0,  1,  2,  3,  4,  6,  7,  8, 14, 23, 32, 41, 50, 59, 68, 77, 12, 13, 21, 22 },
   {  0,  1,  2,  3,  4,  5,  7,  8, 15, 24, 33, 42, 51, 60, 69, 78, 16, 17, 25, 26 },
   {  0,  1,  2,  3,  4,  5,  6,  8, 16, 25, 34, 43, 52, 61, 70, 79, 15, 17, 24, 26 },
   {  0,  1,  2,  3,  4,  5,  6,  7, 17, 26, 35, 44, 53, 62, 71, 80, 15, 16, 24, 25 },
   { 10, 11, 12, 13, 14, 15, 16, 17,  0, 18, 27, 36, 45, 54, 63, 72,  1,  2, 19, 20 },
   {  9, 11, 12, 13, 14, 15, 16, 17,  1, 19, 28, 37, 46, 55, 64, 73,  0,  2, 18, 20 },
   {  9, 10, 12, 13, 14, 15, 16, 17,  2, 20, 29, 38, 47, 56, 65, 74,  0,  1, 18, 19 },
   {  9, 10, 11, 13, 14, 15, 16, 17,  3, 21, 30, 39, 48, 57, 66, 75,  4,  5, 22, 23 },
   {  9, 10, 11, 12, 14, 15, 16, 17,  4, 22, 31, 40, 49, 58, 67, 76,  3,  5, 21, 23 },
   {  9, 10, 11, 12, 13, 15, 16, 17,  5, 23, 32, 41, 50, 59, 68, 77,  3,  4, 21, 22 },
   {  9, 10, 11, 12, 13, 14, 16, 17,  6, 24, 33, 42, 51, 60, 69, 78,  7,  8, 25, 26 },
   {  9, 10, 11, 12, 13, 14, 15, 17,  7, 25, 34, 43, 52, 61, 70, 79,  6,  8, 24, 26 },
   {  9, 10, 11, 12, 13, 14, 15, 16,  8, 26, 35, 44, 53, 62, 71, 80,  6,  7, 24, 25 },
   { 19, 20, 21, 22, 23, 24, 25, 26,  0,  9, 27, 36, 45, 54, 63, 72,  1,  2, 10, 11 },
   { 18, 20, 21, 22, 23, 24, 25, 26,  1, 10, 28, 37, 46, 55, 64, 73,  0,  2,  9, 11 },
   { 18, 19, 21, 22, 23, 24, 25, 26,  2, 11, 29, 38, 47, 56, 65, 74,  0,  1,  9, 10 },
   { 18, 19, 20, 22, 23, 24, 25, 26,  3, 12, 30, 39, 48, 57, 66, 75,  4,  5, 13, 14 },
   { 18, 19, 20, 21, 23, 24, 25, 26,  4, 13, 31, 40, 49, 58, 67, 76,  3,  5, 12, 14 },
   { 18, 19, 20, 21, 22, 24, 25, 26,  5, 14, 32, 41, 50, 59, 68, 77,  3,  4, 12, 13 },
   { 18, 19, 20, 21, 22, 23, 25, 26,  6, 15, 33, 42, 51, 60, 69, 78,  7,  8, 16, 17 },
   { 18, 19, 20, 21, 22, 23, 24, 26,  7, 16, 34, 43, 52, 61, 70, 79,  6,  8, 15, 17 },
   { 18, 19, 20, 21, 22, 23, 24, 25,  8, 17, 35, 44, 53, 62, 71, 80,  6,  7, 15, 16 },
   { 28, 29, 30, 31, 32, 33, 34, 35,  0,  9, 18, 36, 45, 54, 63, 72, 37, 38, 46, 47 },
   { 27, 29, 30, 31, 32, 33, 34, 35,  1, 10, 19, 37, 46, 55, 64, 73, 36, 38, 45, 47 },
   { 27, 28, 30, 31, 32, 33, 34, 35,  2, 11, 20, 38, 47, 56, 65, 74, 36, 37, 45, 46 },
   { 27, 28, 29, 31, 32, 33, 34, 35,  3, 12, 21, 39, 48, 57, 66, 75, 40, 41, 49, 50 },
   { 27, 28, 29, 30, 32, 33, 34, 35,  4, 13, 22, 40, 49, 58, 67, 76, 39, 41, 48, 50 },
   { 27, 28, 29, 30, 31, 33, 34, 35,  5, 14, 23, 41, 50, 59, 68, 77, 39, 40, 48, 49 },
   { 27, 28, 29, 30, 31, 32, 34, 35,  6, 15, 24, 42, 51, 60, 69, 78, 43, 44, 52, 53 },
   { 27, 28, 29, 30, 31, 32, 33, 35,  7, 16, 25, 43, 52, 61, 70, 79, 42, 44, 51, 53 },
   { 27, 28, 29, 30, 31, 32, 33, 34,  8, 17, 26, 44, 53, 62, 71, 80, 42, 43, 51, 52 },
   { 37, 38, 39, 40, 41, 42, 43, 44,  0,  9, 18, 27, 45, 54, 63, 72, 28, 29, 46, 47 },
   { 36, 38, 39, 40, 41, 42, 43, 44,  1, 10, 19, 28, 46, 55, 64, 73, 27, 29, 45, 47 },
   { 36, 37, 39, 40, 41, 42, 43, 44,  2, 11, 20, 29, 47, 56, 65, 74, 27, 28, 45, 46 },
   { 36, 37, 38, 40, 41, 42, 43, 44,  3, 12, 21, 30, 48, 57, 66, 75, 31, 32, 49, 50 },
   { 36, 37, 38, 39, 41, 42, 43, 44,  4, 13, 22, 31, 49, 58, 67, 76, 30, 32, 48, 50 },
   { 36, 37, 38, 39, 40, 42, 43, 44,  5, 14, 23, 32, 50, 59, 68, 77, 30, 31, 48, 49 },
   { 36, 37, 38, 39, 40, 41, 43, 44,  6, 15, 24, 33, 51, 60, 69, 78, 34, 35, 52, 53 },
   { 36, 37, 38, 39, 40, 41, 42, 44,  7, 16, 25, 34, 52, 61, 70, 79, 33, 35, 51, 53 },
   { 36, 37, 38, 39, 40, 41, 42, 43,  8, 17, 26, 35, 53, 62, 71, 80, 33, 34, 51, 52 },
   { 46, 47, 48, 49, 50, 51, 52, 53,  0,  9, 18, 27, 36, 54, 63, 72, 28, 29, 37, 38 },
   { 45, 47, 48, 49, 50, 51, 52, 53,  1, 10, 19, 28, 37, 55, 64, 73, 27, 29, 36, 38 },
   { 45, 46, 48, 49, 50, 51, 52, 53,  2, 11, 20, 29, 38, 56, 65, 74, 27, 28, 36, 37 },
   { 45, 46, 47, 49, 50, 51, 52, 53,  3, 12, 21, 30, 39, 57, 66, 75, 31, 32, 40, 41 },
   { 45, 46, 47, 48, 50, 51, 52, 53,  4, 13, 22, 31, 40, 58, 67, 76, 30, 32, 39, 41 },
   { 45, 46, 47, 48, 49, 51, 52, 53,  5, 14, 23, 32, 41, 59, 68, 77, 30, 31, 39, 40 },
   { 45, 46, 47, 48, 49, 50, 52, 53,  6, 15, 24, 33, 42, 60, 69, 78, 34, 35, 43, 44 },
   { 45, 46, 47, 48, 49, 50, 51, 53,  7, 16, 25, 34, 43, 61, 70, 79, 33, 35, 42, 44 },
   { 45, 46, 47, 48, 49, 50, 51, 52,  8, 17, 26, 35, 44, 62, 71, 80, 33, 34, 42, 43 },
   { 55, 56, 57, 58, 59, 60, 61, 62,  0,  9, 18, 27, 36, 45, 63, 72, 64, 65, 73, 74 },
   { 54, 56, 57, 58, 59, 60, 61, 62,  1, 10, 19, 28, 37, 46, 64, 73, 63, 65, 72, 74 },
   { 54, 55, 57, 58, 59, 60, 61, 62,  2, 11, 20, 29, 38, 47, 65, 74, 63, 64, 72, 73 },
   { 54, 55, 56, 58, 59, 60, 61, 62,  3, 12, 21, 30, 39, 48, 66, 75, 67, 68, 76, 77 },
   { 54, 55, 56, 57, 59, 60, 61, 62,  4, 13, 22, 31, 40, 49, 67, 76, 66, 68, 75, 77 },
   { 54, 55, 56, 57, 58, 60, 61, 62,  5, 14, 23, 32, 41, 50, 68, 77, 66, 67, 75, 76 },
   { 54, 55, 56, 57, 58, 59, 61, 62,  6, 15, 24, 33, 42, 51, 69, 78, 70, 71, 79, 80 },
   { 54, 55, 56, 57, 58, 59, 60, 62,  7, 16, 25, 34, 43, 52, 70, 79, 69, 71, 78, 80 },
   { 54, 55, 56, 57, 58, 59, 60, 61,  8, 17, 26, 35, 44, 53, 71, 80, 69, 70, 78, 79 },
   { 64, 65, 66, 67, 68, 69, 70, 71,  0,  9, 18, 27, 36, 45, 54, 72, 55, 56, 73, 74 },
   { 63, 65, 66, 67, 68, 69, 70, 71,  1, 10, 19, 28, 37, 46, 55, 73, 54, 56, 72, 74 },
   { 63, 64, 66, 67, 68, 69, 70, 71,  2, 11, 20, 29, 38, 47, 56, 74, 54, 55, 72, 73 },
   { 63, 64, 65, 67, 68, 69, 70, 71,  3, 12, 21, 30, 39, 48, 57, 75, 58, 59, 76, 77 },
   { 63, 64, 65, 66, 68, 69, 70, 71,  4, 13, 22, 31, 40, 49, 58, 76, 57, 59, 75, 77 },
   { 63, 64, 65, 66, 67, 69, 70, 71,  5, 14, 23, 32, 41, 50, 59, 77, 57, 58, 75, 76 },
   { 63, 64, 65, 66, 67, 68, 70, 71,  6, 15, 24, 33, 42, 51, 60, 78, 61, 62, 79, 80 },
   { 63, 64, 65, 66, 67, 68, 69, 71,  7, 16, 25, 34, 43, 52, 61, 79, 60, 62, 78, 80 },
   { 63, 64, 65, 66, 67, 68, 69, 70,  8, 17, 26, 35, 44, 53, 62, 80, 60, 61, 78, 79 },
   { 73, 74, 75, 76, 77, 78, 79, 80,  0,  9, 18, 27, 36, 45, 54, 63, 55, 56, 64, 65 },
   { 72, 74, 75, 76, 77, 78, 79, 80,  1, 10, 19, 28, 37, 46, 55, 64, 54, 56, 63, 65 },
   { 72, 73, 75, 76, 77, 78, 79, 80,  2, 11, 20, 29, 38, 47, 56, 65, 54, 55, 63, 64 },
   { 72, 73, 74, 76, 77, 78, 79, 80,  3, 12, 21, 30, 39, 48, 57, 66, 58, 59, 67, 68 },
   { 72, 73, 74, 75, 77, 78, 79, 80,  4, 13, 22, 31, 40, 49, 58, 67, 57, 59, 66, 68 },
   { 72, 73, 74, 75, 76, 78, 79, 80,  5, 14, 23, 32, 41, 50, 59, 68, 57, 58, 66, 67 },
   { 72, 73, 74, 75, 76, 77, 79, 80,  6, 15, 24, 33, 42, 51, 60, 69, 61, 62, 70, 71 },
   { 72, 73, 74, 75, 76, 77, 78, 80,  7, 16, 25, 34, 43, 52, 61, 70, 60, 62, 69, 71 },
   { 72, 73, 74, 75, 76, 77, 78, 79,  8, 17, 26, 35, 44, 53, 62, 71, 60, 61, 69, 70 }};

// Defines the groups used in a standard 3x3 grid
const unsigned char Group[27][9] =
  {{  0,  1,  2,  3,  4,  5,  6,  7,  8 }, {  9, 10, 11, 12, 13, 14, 15, 16, 17 }, { 18, 19, 20, 21, 22, 23, 24, 25, 26 },
   { 27, 28, 29, 30, 31, 32, 33, 34, 35 }, { 36, 37, 38, 39, 40, 41, 42, 43, 44 }, { 45, 46, 47, 48, 49, 50, 51, 52, 53 },
   { 54, 55, 56, 57, 58, 59, 60, 61, 62 }, { 63, 64, 65, 66, 67, 68, 69, 70, 71 }, { 72, 73, 74, 75, 76, 77, 78, 79, 80 },

   {  0,  9, 18, 27, 36, 45, 54, 63, 72 }, {  1, 10, 19, 28, 37, 46, 55, 64, 73 }, {  2, 11, 20, 29, 38, 47, 56, 65, 74 },
   {  3, 12, 21, 30, 39, 48, 57, 66, 75 }, {  4, 13, 22, 31, 40, 49, 58, 67, 76 }, {  5, 14, 23, 32, 41, 50, 59, 68, 77 },
   {  6, 15, 24, 33, 42, 51, 60, 69, 78 }, {  7, 16, 25, 34, 43, 52, 61, 70, 79 }, {  8, 17, 26, 35, 44, 53, 62, 71, 80 },

   {  0,  1,  2,  9, 10, 11, 18, 19, 20 }, {  3,  4,  5, 12, 13, 14, 21, 22, 23 }, {  6,  7,  8, 15, 16, 17, 24, 25, 26 },
   { 27, 28, 29, 36, 37, 38, 45, 46, 47 }, { 30, 31, 32, 39, 40, 41, 48, 49, 50 }, { 33, 34, 35, 42, 43, 44, 51, 52, 53 },
   { 54, 55, 56, 63, 64, 65, 72, 73, 74 }, { 57, 58, 59, 66, 67, 68, 75, 76, 77 }, { 60, 61, 62, 69, 70, 71, 78, 79, 80 }};

// Defines the 3 groups each position is part of
const unsigned char C2Grp[81][3] =
  {{ 0,  9, 18}, { 0, 10, 18}, { 0, 11, 18}, { 0, 12, 19}, { 0, 13, 19}, { 0, 14, 19}, { 0, 15, 20}, { 0, 16, 20}, { 0, 17, 20},
   { 1,  9, 18}, { 1, 10, 18}, { 1, 11, 18}, { 1, 12, 19}, { 1, 13, 19}, { 1, 14, 19}, { 1, 15, 20}, { 1, 16, 20}, { 1, 17, 20},
   { 2,  9, 18}, { 2, 10, 18}, { 2, 11, 18}, { 2, 12, 19}, { 2, 13, 19}, { 2, 14, 19}, { 2, 15, 20}, { 2, 16, 20}, { 2, 17, 20},
   { 3,  9, 21}, { 3, 10, 21}, { 3, 11, 21}, { 3, 12, 22}, { 3, 13, 22}, { 3, 14, 22}, { 3, 15, 23}, { 3, 16, 23}, { 3, 17, 23},
   { 4,  9, 21}, { 4, 10, 21}, { 4, 11, 21}, { 4, 12, 22}, { 4, 13, 22}, { 4, 14, 22}, { 4, 15, 23}, { 4, 16, 23}, { 4, 17, 23},
   { 5,  9, 21}, { 5, 10, 21}, { 5, 11, 21}, { 5, 12, 22}, { 5, 13, 22}, { 5, 14, 22}, { 5, 15, 23}, { 5, 16, 23}, { 5, 17, 23},
   { 6,  9, 24}, { 6, 10, 24}, { 6, 11, 24}, { 6, 12, 25}, { 6, 13, 25}, { 6, 14, 25}, { 6, 15, 26}, { 6, 16, 26}, { 6, 17, 26},
   { 7,  9, 24}, { 7, 10, 24}, { 7, 11, 24}, { 7, 12, 25}, { 7, 13, 25}, { 7, 14, 25}, { 7, 15, 26}, { 7, 16, 26}, { 7, 17, 26},
   { 8,  9, 24}, { 8, 10, 24}, { 8, 11, 24}, { 8, 12, 25}, { 8, 13, 25}, { 8, 14, 25}, { 8, 15, 26}, { 8, 16, 26}, { 8, 17, 26}};


// Defines the interaction of segments used for Lock Candidate searches
const unsigned char LC_Segment[9][4] =
  {{1, 2, 3, 6}, {0, 2, 4, 7}, {0, 1, 5, 8}, {4, 5, 0, 6}, {3, 5, 1, 7}, {3, 4, 2, 8}, {7, 8, 0, 3}, {6, 8, 1, 4}, {6, 7, 2, 5}};

#endif
