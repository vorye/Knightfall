/*

------

HEADERS

------

*/

#include <iostream>
#include <stdio.h>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <time.h>
#include <windows.h>
#include <cstdint>
#include <bitset>
#include <stdint.h>
#include <unordered_map>

// defined bitboard type
#define U64 unsigned long long  
using namespace std;

// FEN debug positions
#define empty_board "8/8/8/8/8/8/8/8 w - - "
#define start_position "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1 "
#define tricky_position "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1 "
#define killer_position "rnbqkb1r/pp1p1pPp/8/2p1pP2/1P1P4/3P3P/P1P1P3/RNBQKBNR w KQkq e6 0 1"
#define cmk_position "r2q1rk1/ppp2ppp/2n1bn2/2b1p3/3pP3/3P1NPP/PPP1NPB1/R1BQ1RK1 b - - 0 9 "

enum space {
a8, b8, c8, d8, e8, f8, g8, h8,
a7, b7, c7, d7, e7, f7, g7, h7,
a6, b6, c6, d6, e6, f6, g6, h6,
a5, b5, c5, d5, e5, f5, g5, h5,
a4, b4, c4, d4, e4, f4, g4, h4,
a3, b3, c3, d3, e3, f3, g3, h3,
a2, b2, c2, d2, e2, f2, g2, h2,
a1, b1, c1, d1, e1, f1, g1, h1, no_space,
};

enum { P, N, B, R, Q, K, p, n, b, r, q, k };

enum {
  white, black, both
};

// bishop and rook
enum {
  rook, bishop
};

enum {
    wk = 1, wq = 2, bk = 4, bq = 8
};

// encoding pieces



const char *space_to_coordinates[] = { 
    "a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8",
    "a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
    "a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
    "a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
    "a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
    "a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
    "a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
    "a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1", "-",
};


// ASCII pieces
char ascii_pieces[] = "PNBRQKpnbrqk";


// unicode pieces
char *unicode_pieces[12] = {"♙", "♘", "♗", "♖", "♕", "♔", "♟︎", "♞", "♝", "♜", "♛", "♚"};


// map for piece encoding
std::unordered_map<int, char> char_pieces = {
    {'P', static_cast<char>(P)},
    {'N', static_cast<char>(N)},
    {'B', static_cast<char>(B)},
    {'R', static_cast<char>(R)},
    {'Q', static_cast<char>(Q)},
    {'K', static_cast<char>(K)},
    {'p', static_cast<char>(p)},
    {'n', static_cast<char>(n)}, 
    {'b', static_cast<char>(b)},
    {'r', static_cast<char>(r)},
    {'q', static_cast<char>(q)},
    {'k', static_cast<char>(k)}
};

// map for promoted pieces
std::unordered_map<int, char> promoted_pieces = {
    {Q, static_cast<char>('Q')},
    {R, static_cast<char>('R')},
    {B, static_cast<char>('B')},
    {N, static_cast<char>('N')},
    {q, static_cast<char>('q')},
    {r, static_cast<char>('r')},
    {b, static_cast<char>('b')},
    {n, static_cast<char>('n')}
};

   

/*
------

MACROS FOR BITBOARD MANIPULATION
 
------

*/


#define set_bit(bitboard, space) (bitboard |= (1ULL << space))  // sets the bit at the given space
#define get_bit(bitboard, space) (bitboard & (1ULL << space))   // gets the bit at the given space
#define pop_bit(bitboard, space) ((bitboard) &= ~(1ULL << (space))) // pops the bit at the given space


void print_bitboard(uint64_t bitboard) // prints the bitboard in a 8x8 grid
{
    for (int rows = 0; rows < 8; rows++)    // 8 rows
    {
        for (int columns = 0; columns < 8; columns++)   // 8 columns
        {
            int space = rows * 8 + columns;     // 8 spaces per row

            if (!columns)       // if the column is 0
                cout << " " << 8 - rows << " ";     // print the row number

            cout << " " << (get_bit(bitboard, space) ? 1 : 0 ) << " ";   // print the bit
        }

        cout << endl;  // print a new line
    }

    printf("\n    a  b  c  d  e  f  g  h\n\n"); // print the column letters

    cout << "Bitboard: " << dec << bitboard << endl << endl;  // print the bitboard in decimal
};



/*

------

BIT COUNT + LEAST SIG BIT

------

*/

static inline int count_bits(U64 bitboard)
{
return __builtin_popcountll(bitboard);
}

static inline int get_leastsigbit(U64 bitboard)
{   
// get the least significant bit
if (bitboard)
{
    return count_bits((bitboard & -bitboard) - 1);
}
else
return -1;
}


/*  

! all bits EXCEPT the A column

 8   0  1  1  1  1  1  1  1 
 7   0  1  1  1  1  1  1  1
 6   0  1  1  1  1  1  1  1
 5   0  1  1  1  1  1  1  1
 4   0  1  1  1  1  1  1  1
 3   0  1  1  1  1  1  1  1
 2   0  1  1  1  1  1  1  1
 1   0  1  1  1  1  1  1  1

     a  b  c  d  e  f  g  h


! all bits EXCEPT the H column

 8   1  1  1  1  1  1  1  0
 7   1  1  1  1  1  1  1  0
 6   1  1  1  1  1  1  1  0
 5   1  1  1  1  1  1  1  0
 4   1  1  1  1  1  1  1  0
 3   1  1  1  1  1  1  1  0
 2   1  1  1  1  1  1  1  0
 1   1  1  1  1  1  1  1  0


! all bits EXCEPT the HG columns

     a  b  c  d  e  f  g  h

 8   1  1  1  1  1  1  0  0
 7   1  1  1  1  1  1  0  0
 6   1  1  1  1  1  1  0  0 
 5   1  1  1  1  1  1  0  0
 4   1  1  1  1  1  1  0  0
 3   1  1  1  1  1  1  0  0
 2   1  1  1  1  1  1  0  0
 1   1  1  1  1  1  1  0  0

     a  b  c  d  e  f  g  h



*/

/*

------

DEFINING DIFFERENT BITBOARDS

------

*/

// different piece bitboards

U64 bitboards[12]; 

// occupancy bitboards
U64 occupancies[3];

// side to move
int side;

// enpassant space
int enpassant = no_space;

// castling rights
int castle;


// print board

void print_board()
{
    // print offset
    printf("\n");

    // loop over rows
    for (int row = 0; row <8; row++)
    { 
        for (int column = 0; column < 8; column++)
        {
            // initialise space
            int space = row * 8 + column;

            // print rows
            if (!column)
                printf("  %d ", 8 - row);

            // define piece varaible
            int piece = -1;

            // loop over all piece bitboards
            for (int bitboard_piece = P; bitboard_piece <= k; bitboard_piece++)
            {
                // if the board has a piece on a space,
                if (get_bit(bitboards[bitboard_piece], space))
                // retrieve the piece type
                piece = bitboard_piece;

            }

            // loop over pieces
            #ifdef WIN64
            printf(" %c", (piece == -1) ? '.' : ascii_pieces[piece]);
            #else
            printf(" %s", (piece == -1) ? "." : ascii_pieces[piece]);
            #endif
        }
        // print new line after every row
        printf("\n");
    }
    // print board columns
    printf("\n     a b c d e f g h\n\n");

    // print side to move
    printf("    Side:     %s\n", (!side ? "white" : "black \n"));

    // print enpassant space
    printf("    Enpassant: %s\n", (enpassant != no_space) ? space_to_coordinates[enpassant] : "no \n");

    // print castling rights
    printf("    Castling:  %c%c%c%c\n\n", (castle & wk) ? 'K' : '-', (castle & wq) ? 'Q' : '-', (castle & bk) ? 'k' : '-', (castle & bq) ? 'q' : '-');
}

// parsing the FEN strings


// parse FEN string
void parse_fen(char *fen)
{
    // reset board position (bitboards)
    memset(bitboards, 0ULL, sizeof(bitboards));
    
    // reset occupancies (bitboards)
    memset(occupancies, 0ULL, sizeof(occupancies));
    
    // reset game state variables
    side = 0;
    enpassant = no_space;
    castle = 0;
    
    // loop over board rows
    for (int row = 0; row < 8; row++)
    {
        // loop over board columns
        for (int column = 0; column < 8; column++)
        {
            // init current space
            int space = row * 8 + column;
            
            // match ascii pieces within FEN string
            if ((*fen >= 'a' && *fen <= 'z') || (*fen >= 'A' && *fen <= 'Z'))
            {
                // init piece type
                int piece = char_pieces[*fen];
                
                // set piece on corresponding bitboard
                set_bit(bitboards[piece], space);
                
                // increment pointer to FEN string
                fen++;
            }
            
            // match empty space numbers within FEN string
            if (*fen >= '0' && *fen <= '9')
            {
                // init offset (convert char 0 to int 0)
                int offset = *fen - '0';
                
                // define piece variable
                int piece = -1;
                
                // loop over all piece bitboards
                for (int bitboard_piece = P; bitboard_piece <= k; bitboard_piece++)
                {
                    // if there is a piece on current space
                    if (get_bit(bitboards[bitboard_piece], space))
                        // get piece code
                        piece = bitboard_piece;
                }
                
                // on empty current space
                if (piece == -1)
                    // decrement column
                    column--;
                
                // adjust column counter
                column += offset;
                
                // increment pointer to FEN string
                fen++;
            }
            
            // match row separator
            if (*fen == '/')
                // increment pointer to FEN string
                *fen++;
        }
    }
    
    // got to parsing side to move (increment pointer to FEN string)
    fen++;
    
    // parse side to move
    (*fen == 'w') ? (side = white) : (side = black);
    
    // go to parsing castling rights
    fen += 2;
    
    // parse castling rights
    while (*fen != ' ')
    {
        switch (*fen)
        {
            case 'K': castle |= wk; break;
            case 'Q': castle |= wq; break;
            case 'k': castle |= bk; break;
            case 'q': castle |= bq; break;
            case '-': break;
        }

        // increment pointer to FEN string
        fen++;
    }
    
    // got to parsing enpassant space (increment pointer to FEN string)
    fen++;
    
    // parse enpassant space
    if (*fen != '-')
    {
        // parse enpassant column & row
        int column = fen[0] - 'a';
        int row = 8 - (fen[1] - '0');
        
        // init enpassant space
        enpassant = row * 8 + column;
    }
    
    // no enpassant space
    else
        enpassant = no_space;
    
    // loop over white pieces bitboards
    for (int piece = P; piece <= K; piece++)
        // populate white occupancy bitboard
        occupancies[white] |= bitboards[piece];
    
    // loop over black pieces bitboards
    for (int piece = p; piece <= k; piece++)
        // populate white occupancy bitboard
        occupancies[black] |= bitboards[piece];
    
    // init all occupancies
    occupancies[both] |= occupancies[white];
    occupancies[both] |= occupancies[black];
}





// no A column

const U64 not_A_column = 18374403900871474942ULL; // bitboard to store the attacks

// no H column

const U64 not_H_column = 9187201950435737471ULL; 

// no HG column

const U64 not_HG_column = 4557430888798830399ULL; 

// no AB column

const U64 not_AB_column = 18229723555195321596ULL; 


/*

------

TABLES

------

*/

// table to store the pawn attacks for each space and side
U64 pawn_captures[2][64]; 


// table to store the king attacks

U64 king_attacks[64];

// table to store the knight attacks
U64 knight_attacks[64];

// table to store bishop masks
U64 bishop_masks[64];
U64 rook_masks[64];

// setting up attack tables
U64 rook_table[64][512];
U64 bishop_table[64][4096];



/*

------

ATTACKS

------

*/

U64 masked_knight_attack(int space) {
    U64 attacks = 0ULL;
    U64 bitboard = 1ULL << space;
    
    if (space <= 18 && (not_HG_column & (bitboard << 6))) attacks |= (bitboard << 6) & not_HG_column;
    if (space <= 17 && (not_H_column & (bitboard << 15))) attacks |= (bitboard << 15) & not_H_column;
    if (space >= 16 && (not_H_column & (bitboard >> 17))) attacks |= (bitboard >> 17) & not_H_column;
    if (space >= 17 && (not_HG_column & (bitboard >> 10))) attacks |= (bitboard >> 10) & not_HG_column;
    if (space >= 10 && (not_A_column & (bitboard >> 6))) attacks |= (bitboard >> 6) & not_A_column;
    if (space >= 9 && (not_A_column & (bitboard << 10))) attacks |= (bitboard << 10) & not_A_column;
    if (space <= 56 && (not_H_column & (bitboard >> 15))) attacks |= (bitboard >> 15) & not_H_column;
    if (space <= 47 && (not_A_column & (bitboard << 17))) attacks |= (bitboard << 17) & not_A_column;
    
    return attacks;
}



U64 masked_king_attack(int space) {
    U64 attacks = 0ULL;
    U64 bitboard = 1ULL << space; 

    if (space % 8 != 0) { // not in A-column
        attacks |= (bitboard << 7) | (bitboard >> 1) | (bitboard >> 9);
    }
    if (space % 8 != 7) { // not in H-column
        attacks |= (bitboard << 9) | (bitboard << 1) | (bitboard >> 7);
    }
    attacks |= (bitboard << 8) | (bitboard >> 8);
    return attacks;
}


U64 masked_bishop_attack(int space){

// attack bitboard
U64 attacks = 0ULL;

// initialise rows and columns
int row, column;

// initialise rows and columns targetting
int Trow = space / 8;
int Tcolumn = space % 8;

// generate the attacks
for (row = Trow + 1, column = Tcolumn + 1; row <= 6 && column <= 6; row++, column++) attacks |= (1ULL << (row * 8 + column)); 
for (row = Trow - 1, column = Tcolumn + 1; row >= 1 && column <= 6; row--, column++) attacks |= (1ULL << (row * 8 + column)); 
for (row = Trow + 1, column = Tcolumn - 1; row <= 6 && column >= 1; row++, column--) attacks |= (1ULL << (row * 8 + column));
for (row = Trow - 1, column = Tcolumn - 1; row >= 1 && column >= 1; row--, column--) attacks |= (1ULL << (row * 8 + column));

return attacks;
}

U64 masked_rook_attack(int space) 
{
    U64 attacks = 0ULL;
    int row, column;

    int Trow = space / 8;
    int Tcolumn = space % 8;

    for (row = Trow + 1; row <= 6; row++) attacks |= (1ULL << (row * 8 + Tcolumn));
    for (row = Trow - 1; row >= 1; row--) attacks |= (1ULL << (row * 8 + Tcolumn));
    for (column = Tcolumn + 1; column <= 6; column++) attacks |= (1ULL << (Trow * 8 + column));
    for (column = Tcolumn - 1; column >= 1; column--) attacks |= (1ULL << (Trow * 8 + column));

    return attacks;
}

// generate bishop attacks

U64 bishopgenattacks(int space, U64 blockers){

U64 attacks = 0ULL;
int row, column;
int Trow = space / 8;
int Tcolumn = space % 8;

// generating the attacks

for (row = Trow +1, column = Tcolumn + 1; row <= 7 && column <= 7; row++, column++) {
    attacks |= (1ULL << (row * 8 + column));
    if (blockers & (1ULL << (row * 8 + column))) break;
}
for (row = Trow - 1, column = Tcolumn + 1; row >= 0 && column <= 7; row--, column++) {
    attacks |= (1ULL << (row * 8 + column));
    if (blockers & (1ULL << (row * 8 + column))) break;
}
for (row = Trow + 1, column = Tcolumn - 1; row <= 7 && column >= 0; row++, column--) {
    attacks |= (1ULL << (row * 8 + column));
    if (blockers & (1ULL << (row * 8 + column))) break;
}
for (row = Trow - 1, column = Tcolumn - 1; row >= 0 && column >= 0; row--, column--) {
    attacks |= (1ULL << (row * 8 + column));
    if (blockers & (1ULL << (row * 8 + column))) break;
}
return attacks;

};

// generate rook attacks

U64 rookgenattacks(int space, U64 blockers) {
    
    U64 attacks = 0ULL;
    int row, column;
    int Trow = space / 8;
    int Tcolumn = space % 8;

    for (row = Trow +1; row <= 7; row++)
    {
        attacks |= (1Ull << (row * 8 + Tcolumn));
        if (blockers & (1Ull << (row * 8 + Tcolumn))) break;
    }
    for (row = Trow - 1; row >= 0; row--){
        attacks |= (1Ull << (row * 8 + Tcolumn));
        if (blockers & (1Ull << (row * 8 + Tcolumn))) break;
    }
    for (column = Tcolumn + 1; column <= 7; column++){
        attacks |= (1Ull << (Trow * 8 + column));
        if (blockers & (1Ull << (Trow * 8 + column))) break;
    }
    for (column = Tcolumn - 1; column >= 0; column--){
        attacks |= (1Ull << (Trow * 8 + column));
        if (blockers & (1Ull << (Trow * 8 + column))) break;
    }
    return attacks;
}






U64 masked_pawn_attack(int side, int space)
{
    // result attacks bitboard
    U64 attacks = 0ULL;

    // piece bitboard
    U64 bitboard = 0ULL;
    
    // set piece on board
    set_bit(bitboard, space);
    
    // white pawns
    if (!side)
    {
        // generate pawn attacks
        if ((bitboard >> 7) & not_A_column) attacks |= (bitboard >> 7);
        if ((bitboard >> 9) & not_H_column) attacks |= (bitboard >> 9);
    }
    
    // black pawns
    else
    {
        // generate pawn attacks
        if ((bitboard << 7) & not_H_column) attacks |= (bitboard << 7);
        if ((bitboard << 9) & not_A_column) attacks |= (bitboard << 9);    
    }
    
    // return attack map
    return attacks;
}


/*

------

MAGIC NUMBERS + INDEX BITS GENERATED BY SHALLOW BLUE

https://github.com/GunshipPenguin/shallow-blue/blob/c6d7e9615514a86533a9e0ffddfc96e058fc9cfd/src/attacks.h#L120

------

*/

const U64 rookmagics[64] = {
    0xa8002c000108020ULL, 0x6c00049b0002001ULL, 0x100200010090040ULL, 0x2480041000800801ULL, 0x280028004000800ULL,
    0x900410008040022ULL, 0x280020001001080ULL, 0x2880002041000080ULL, 0xa000800080400034ULL, 0x4808020004000ULL,
    0x2290802004801000ULL, 0x411000d00100020ULL, 0x402800800040080ULL, 0xb000401004208ULL, 0x2409000100040200ULL,
    0x1002100004082ULL, 0x22878001e24000ULL, 0x1090810021004010ULL, 0x801030040200012ULL, 0x500808008001000ULL,
    0xa08018014000880ULL, 0x8000808004000200ULL, 0x201008080010200ULL, 0x801020000441091ULL, 0x800080204005ULL,
    0x1040200040100048ULL, 0x120200402082ULL, 0xd14880480100080ULL, 0x12040280080080ULL, 0x100040080020080ULL,
    0x9020010080800200ULL, 0x813241200148449ULL, 0x491604001800080ULL, 0x100401000402001ULL, 0x4820010021001040ULL,
    0x400402202000812ULL, 0x209009005000802ULL, 0x810800601800400ULL, 0x4301083214000150ULL, 0x204026458e001401ULL,
    0x40204000808000ULL, 0x8001008040010020ULL, 0x8410820820420010ULL, 0x1003001000090020ULL, 0x804040008008080ULL,
    0x12000810020004ULL, 0x1000100200040208ULL, 0x430000a044020001ULL, 0x280009023410300ULL, 0xe0100040002240ULL,
    0x200100401700ULL, 0x2244100408008080ULL, 0x8000400801980ULL, 0x2000810040200ULL, 0x8010100228810400ULL,
    0x2000009044210200ULL, 0x4080008040102101ULL, 0x40002080411d01ULL, 0x2005524060000901ULL, 0x502001008400422ULL,
    0x489a000810200402ULL, 0x1004400080a13ULL, 0x4000011008020084ULL, 0x26002114058042ULL
};

const U64 bishopmagics[64] = {
    0x89a1121896040240ULL, 0x2004844802002010ULL, 0x2068080051921000ULL, 0x62880a0220200808ULL, 0x4042004000000ULL,
    0x100822020200011ULL, 0xc00444222012000aULL, 0x28808801216001ULL, 0x400492088408100ULL, 0x201c401040c0084ULL,
    0x840800910a0010ULL, 0x82080240060ULL, 0x2000840504006000ULL, 0x30010c4108405004ULL, 0x1008005410080802ULL,
    0x8144042209100900ULL, 0x208081020014400ULL, 0x4800201208ca00ULL, 0xf18140408012008ULL, 0x1004002802102001ULL,
    0x841000820080811ULL, 0x40200200a42008ULL, 0x800054042000ULL, 0x88010400410c9000ULL, 0x520040470104290ULL,
    0x1004040051500081ULL, 0x2002081833080021ULL, 0x400c00c010142ULL, 0x941408200c002000ULL, 0x658810000806011ULL,
    0x188071040440a00ULL, 0x4800404002011c00ULL, 0x104442040404200ULL, 0x511080202091021ULL, 0x4022401120400ULL,
    0x80c0040400080120ULL, 0x8040010040820802ULL, 0x480810700020090ULL, 0x102008e00040242ULL, 0x809005202050100ULL,
    0x8002024220104080ULL, 0x431008804142000ULL, 0x19001802081400ULL, 0x200014208040080ULL, 0x3308082008200100ULL,
    0x41010500040c020ULL, 0x4012020c04210308ULL, 0x208220a202004080ULL, 0x111040120082000ULL, 0x6803040141280a00ULL,
    0x2101004202410000ULL, 0x8200000041108022ULL, 0x21082088000ULL, 0x2410204010040ULL, 0x40100400809000ULL,
    0x822088220820214ULL, 0x40808090012004ULL, 0x910224040218c9ULL, 0x402814422015008ULL, 0x90014004842410ULL,
    0x1000042304105ULL, 0x10008830412a00ULL, 0x2520081090008908ULL, 0x40102000a0a60140ULL,
};

// rook index bits
const int rookspace[64] = {
    12, 11, 11, 11, 11, 11, 11, 12,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    12, 11, 11, 11, 11, 11, 11, 12
};

const int bishopspace[64] = {
    6, 5, 5, 5, 5, 5, 5, 6,
    5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 7, 7, 7, 7, 5, 5,
    5, 5, 7, 9, 9, 7, 5, 5,
    5, 5, 7, 9, 9, 7, 5, 5,
    5, 5, 7, 7, 7, 7, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5,
    6, 5, 5, 5, 5, 5, 5, 6
};
 



/* 

------

LEAPING PIECES

------

*/

void leaping_attacks()
{ 
    // loop through the bitboard spaces (64 of them)
    for (int space = 0; space < 64; space++)
    {
        // initializing pawn attacks
        pawn_captures[white][space] = masked_pawn_attack(white, space); // white pawn attacks
        pawn_captures[black][space] = masked_pawn_attack(black, space); // black pawn attacks
        knight_attacks[space] = masked_knight_attack(space); // knight attacks
        king_attacks[space] = masked_king_attack(space); // king attacks
    }    
}




/* 

------

SETTING OCCUPANCIES

------

*/

U64 set_occupancy(int index, int bits_in_mask, U64 attack_mask)
{
  U64 occupancy = 0Ull;
  
  // loop through the number of bits in the attack mask
  for (int count = 0; count < bits_in_mask; count++)

  {
    // get the least significant bit of the attack mask
  int space = get_leastsigbit(attack_mask);

    // pop the least significant bit of the attack mask

  pop_bit(attack_mask, space); 
  
  // if the index bit is set in the index, set the occupancy bit
  if (index & (1 << count)){
    occupancy |= (1ULL << space);
  }}
    return occupancy;
}




/* 

------

MAGIC NUMBER IMPLEMENTATION FOR ROOK AND BISHOP   

------

*/

void initialise_sliders(int bishop)
{
    // initialise bishop & rook masks
    for (int space = 0; space < 64; space++){
        bishop_masks[space] = masked_bishop_attack(space);
        rook_masks[space] = masked_rook_attack(space);

// initialise current mask
        U64 attack_mask = bishop ? bishop_masks[space] : rook_masks[space];
        
    // initialise relevant occupancy bit count
        int relevant_bits_count = count_bits(attack_mask);

        // initialise the occupancy indices
        int occupancy_indices = (1 << relevant_bits_count);

        // loop over the occupancy indices
        for (int index = 0; index < occupancy_indices; index++)
        {
            // the bishop
            if (bishop)
            {
                // occupancy variation
                U64 occupancy = set_occupancy(index, relevant_bits_count, attack_mask);

                // magic index
                int magic_index = occupancy * bishopmagics[space] >> (64 - bishopspace[space]);

                // initialise the bishop attacks
                bishop_table[space][magic_index] = bishopgenattacks(space, occupancy);
            }
            // rook
            else{
                // occupancy variation
                U64 occupancy = set_occupancy(index, relevant_bits_count, attack_mask);

                // magic index
                int magic_index = occupancy * rookmagics[space] >> (64 - rookspace[space]);

                // initialise the bishop attacks
                rook_table[space][magic_index] = rookgenattacks(space, occupancy);
            }
        }
    }
}

// get bishop attacks 
static inline U64 get_bishop_attacks(int space, U64 occupancy){

    occupancy &= bishop_masks[space];
    occupancy *= bishopmagics[space];
    occupancy >>= 64 - bishopspace[space];

    return bishop_table[space][occupancy];
}

// get rook attacks
static inline U64 get_rook_attacks(int space, U64 occupancy){

    occupancy &= rook_masks[space];
    occupancy *= rookmagics[space];
    occupancy >>= 64 - rookspace[space];

    return rook_table[space][occupancy];
}

// get queen attacks

static inline U64 get_queen_attacks(int space, U64 occupancy){

    // initialise the results
    U64 queen_attacks = 0Ull;

    // initialise all of the bishop occupancies
    U64 bishop_occupancy = occupancy & bishop_masks[space];
    // initialise all of the rook occupancies
    U64 rook_occupancy = occupancy & rook_masks[space];

    // get the bishop attacks based on the occupancy
    bishop_occupancy &= rook_masks[space];
    bishop_occupancy *= rookmagics[space];
    bishop_occupancy >>= 64 - rookspace[space];
    
    // get bishop attacks based on the occupancy
    rook_occupancy &= bishop_masks[space];
    rook_occupancy *= bishopmagics[space];
    rook_occupancy >>= 64 - bishopspace[space];

    // return the queen attacks
    queen_attacks |= bishop_table[space][bishop_occupancy] | rook_table[space][rook_occupancy];

    return queen_attacks;

}


/*

------

INITIALISE ALL 

------

*/

void init_all()
{
    leaping_attacks();
    initialise_sliders(0);
    initialise_sliders(1);
}





/*

------

MOVE GENERATION

------

*/

static inline int is_space_attacked(int space, int side)
{
    // attacked by white pawns
    if ((side == white) && (pawn_captures[black][space] & bitboards[P])) return 1;
    
    // attacked by black pawns
    if ((side == black) && (pawn_captures[white][space] & bitboards[p])) return 1;
    
    // attacked by knights
    if (knight_attacks[space] & ((side == white) ? bitboards[N] : bitboards[n])) return 1;
    
    // attacked by bishops
    if (get_bishop_attacks(space, occupancies[both]) & ((side == white) ? bitboards[B] : bitboards[b])) return 1;

    // attacked by rooks
    if (get_rook_attacks(space, occupancies[both]) & ((side == white) ? bitboards[R] : bitboards[r])) return 1;    

    // attacked by bishops
    if (get_queen_attacks(space, occupancies[both]) & ((side == white) ? bitboards[Q] : bitboards[q])) return 1;
    
    // attacked by kings
    if (king_attacks[space] & ((side == white) ? bitboards[K] : bitboards[k])) return 1;

    // by default return false
    return 0;
}

// print attacked spaces
void print_attacked_spaces(int side)
{
    printf("\n");
    
    // loop over board rows
    for (int row = 0; row < 8; row++)
    {
        // loop over board columns
        for (int column = 0; column < 8; column++)
        {
            // init space
            int space = row * 8 + column;
            
            // print rows
            if (!column)
                printf("  %d ", 8 - row);
            
            // check whether current space is attacked or not
            printf(" %d", is_space_attacked(space, side) ? 1 : 0);
        }
        
        // print new line every row
        printf("\n");
    }
    
    // print columns
    printf("\n     a b c d e f g h\n\n");
}

/*

------

    move bits representation in binary                  hexidecimal constants

    0000 0000 0000 0000 0011 1111    SOURCE SPACE       0x3f
    0000 0000 0000 1111 1100 0000    TARGET SPACE       0xfc0
    0000 0000 1111 0000 0000 0000    PIECE               0xf000
    0000 1111 0000 0000 0000 0000    PROMOTED PIECE      0xf0000
    0001 0000 0000 0000 0000 0000    CAPTURE FLAG       0x100000
    0010 0000 0000 0000 0000 0000    DOUBLE PUSH FLAG    0x200000
    0100 0000 0000 0000 0000 0000    ENPASSANT FLAG      0x400000
    1000 0000 0000 0000 0000 0000    CASTLING FLAG       0x800000

------

*/

// encode moves macro
#define encode_move(source, target, piece, promoted, capture, double_push, enpassant, castling) \
(source) |                   \
(target << 6) |             \
(piece << 12) |                 \
(promoted << 16) |          \
(capture << 20) |           \
(double_push << 21)  |            \
(enpassant << 22) |         \
(castling << 23)  


// extract source space
#define get_move_source(move) (move & 0x3f)

// extract target space
#define get_move_target(move) ((move & 0xfc0) >> 6)

// extract piece
#define get_move_piece(move) ((move & 0xf000) >> 12)

// extract promoted piece
#define get_move_promoted(move) ((move & 0xf0000) >> 16)

// extract capture flag
#define get_move_capture(move) (move & 0x100000)

// extract double pawn push flag
#define get_move_double(move) (move & 0x200000)

// extract enpassant flag
#define get_move_enpassant(move) (move & 0x400000)

// extract castling flag
#define get_move_castling(move) (move & 0x800000)

// move list struct
typedef struct
{
    // move list array
    int moves[256];
    
    // move count
    int count;
} moves;

// add a move to the move list
static inline void add_move(moves *move_list, int move)
{
    // add move to move list
    move_list->moves[move_list->count] = move;
    // increment it
    move_list->count++;
}



// print the move
void print_move(int move)
{
    printf("%s%s%c\n", space_to_coordinates[get_move_source(move)],
                     space_to_coordinates[get_move_target(move)],
                     promoted_pieces[get_move_promoted(move)]);
}

// print move list
void print_move_list(moves *move_list)
{
    // do nothing if empty
    if (!move_list->count)
    {
        printf("\n                No moves.\n");
    }

    printf("\n      move  piece  capture  double_push  enpassant  castling\n\n");

    // loop over the moves
   for (int move_count = 0; move_count < move_list->count; move_count++)
    {
        // init move
        int move = move_list->moves[move_count];
        
        #ifdef WIN64
            // print move
            printf("      %s%s%c   %c         %d         %d         %d         %d\n", space_to_coordinates[get_move_source(move)],
                                                                                  space_to_coordinates[get_move_target(move)],
                                                                                  get_move_promoted(move) ? promoted_pieces[get_move_promoted(move)] : ' ',
                                                                                  ascii_pieces[get_move_piece(move)],
                                                                                  get_move_capture(move) ? 1 : 0,
                                                                                  get_move_double(move) ? 1 : 0,
                                                                                  get_move_enpassant(move) ? 1 : 0,
                                                                                  get_move_castling(move) ? 1 : 0);
        #else
            // print move
            printf("     %s%s%c   %s         %d         %d         %d         %d\n", space_to_coordinates[get_move_source(move)],
                                                                                  space_to_coordinates[get_move_target(move)],
                                                                                  get_move_promoted(move) ? promoted_pieces[get_move_promoted(move)] : ' ',
                                                                                  unicode_pieces[get_move_piece(move)],
                                                                                  get_move_capture(move) ? 1 : 0,
                                                                                  get_move_double(move) ? 1 : 0,
                                                                                  get_move_enpassant(move) ? 1 : 0,
                                                                                  get_move_castling(move) ? 1 : 0);
        #endif
        
    }
    // print the total moves
    printf("\n                Total moves: %d\n", move_list->count);
}



// generate all moves
static inline void generate_moves(moves *move_list)
{
    // initialising move count
    move_list->count = 0;

    // define source & target spaces
    int source_space, target_space;
    
    // define current piece's bitboard copy & it's attacks
    U64 bitboard, attacks;
    
    // loop over all the bitboards
    for (int piece = P; piece <= k; piece++)
    {
        // init piece bitboard copy
        bitboard = bitboards[piece];
        
        // generate white pawns & white king castling moves
        if (side == white)
        {
            // pick up white pawn bitboards index
            if (piece == P)
            {
                // loop over white pawns within white pawn bitboard
                while (bitboard)
                {
                    // init source space
                    source_space = get_leastsigbit(bitboard);
                    
                    // init target space
                    target_space = source_space - 8;
                    
                    // generate quiet pawn moves
                    if (!(target_space < a8) && !get_bit(occupancies[both], target_space))
                    {
                        // pawn promotion
                        if (source_space >= a7 && source_space <= h7) 

                        {
                            add_move(move_list, encode_move(source_space, target_space, piece, Q, 0, 0, 0, 0));
                            add_move(move_list, encode_move(source_space, target_space, piece, R, 0, 0, 0, 0));
                            add_move(move_list, encode_move(source_space, target_space, piece, B, 0, 0, 0, 0));
                            add_move(move_list, encode_move(source_space, target_space, piece, N, 0, 0, 0, 0));
                        }
                        
                        else
                        {
                            // quiet pawn move, unless there is a piece on the target space
                                add_move(move_list, encode_move(source_space, target_space, piece, 0, 0, 0, 0, 0));
                            
                            // two spaces ahead pawn move
                            if ((source_space >= a2 && source_space <= h2) && !get_bit(occupancies[both], target_space - 8))
                                add_move(move_list, encode_move(source_space, target_space - 8, piece, 0, 0, 1, 0, 0));

                        }
                    }
                    
                    // init pawn attacks bitboard
                    attacks = pawn_captures[side][source_space] & occupancies[black]; 
                    
                    // generate pawn captures
                    while (attacks)
                    {
                        // init target space
                        target_space = get_leastsigbit(attacks);
                        
                        // pawn promotion
                        if (source_space >= a7 && source_space <= h7)
                        {
                            add_move(move_list, encode_move(source_space, target_space, piece, Q, 1, 0, 0, 0));
                            add_move(move_list, encode_move(source_space, target_space, piece, R, 1, 0, 0, 0));
                            add_move(move_list, encode_move(source_space, target_space, piece, B, 1, 0, 0, 0));
                            add_move(move_list, encode_move(source_space, target_space, piece, N, 1, 0, 0, 0));
                        }
                        
                        else
                            // pawn capture
                            add_move(move_list, encode_move(source_space, target_space, piece, 0, 1, 0, 0, 0));
                        
                        // pop ls1b of the pawn attacks
                        pop_bit(attacks, target_space);
                    }
                    
                    // generate enpassant captures
                    if (enpassant != no_space)
                    {
                        // lookup pawn attacks and bitwise AND with enpassant space (bit)
                        U64 enpassant_attacks = pawn_captures[side][source_space] & (1ULL << enpassant);
                        
                        // make sure enpassant capture available
                        if (enpassant_attacks)
                        {
                            // init enpassant capture target space
                            int target_enpassant = get_leastsigbit(enpassant_attacks);
                            add_move(move_list, encode_move(source_space, target_enpassant, piece, 0, 0, 0, 1, 0));
                        }
                    }
                    
                    // pop ls1b from piece bitboard copy
                    pop_bit(bitboard, source_space);
                }
            }
            
            // castling moves
            if (piece == K)
            {
                // king side castling is available
                if (castle & wk)
                {
                    // make sure space between king and king's rook are empty
                    if (!get_bit(occupancies[both], f1) && !get_bit(occupancies[both], g1))
                    {
                        // make sure king and the f1 spaces are not under attacks
                        if (!is_space_attacked(e1, black) && !is_space_attacked(f1, black))
                            printf("e1g1  castling move\n");
                            add_move(move_list, encode_move(e1, g1, piece, 0, 0, 0, 0, 1));
                    }
                }
                
                // queen side castling is available
                if (castle & wq)
                {
                    // make sure space between king and queen's rook are empty
                    if (!get_bit(occupancies[both], d1) && !get_bit(occupancies[both], c1) && !get_bit(occupancies[both], b1))
                    {
                        // make sure king and the d1 spaces are not under attacks
                        if (!is_space_attacked(e1, black) && !is_space_attacked(d1, black))
                            printf("e1c1  castling move\n");
                        add_move(move_list, encode_move(e1, c1, piece, 0, 0, 0, 0, 1));
                    }
                }
            }
        }
        
        // generate black pawns & black king castling moves
        else
        {
            // pick up black pawn bitboards index
            if (piece == p)
            {
                // loop over black pawns within black pawn bitboard
                while (bitboard)
                {
                    // init source space
                    source_space = get_leastsigbit(bitboard);
                    
                    // init target space
                    target_space = source_space + 8;
                    
                    // generate quite pawn moves
                    if (!(target_space > h1) && !get_bit(occupancies[both], target_space))
                    {
                        // pawn promotion
                        if (source_space >= a2 && source_space <= h2)
                        {
                            add_move(move_list, encode_move(source_space, target_space, piece, q, 0, 0, 0, 0));
                            add_move(move_list, encode_move(source_space, target_space, piece, r, 0, 0, 0, 0));
                            add_move(move_list, encode_move(source_space, target_space, piece, b, 0, 0, 0, 0));
                            add_move(move_list, encode_move(source_space, target_space, piece, n, 0, 0, 0, 0));
                        }
                        
                        else
                        {
                            // one space ahead pawn move
                            add_move(move_list, encode_move(source_space, target_space, piece, 0, 0, 0, 0, 0));
                            
                            // two spaces ahead pawn move
                            if ((source_space >= a7 && source_space <= h7) && !get_bit(occupancies[both], target_space + 8));
                                add_move(move_list, encode_move(source_space, target_space + 8, piece, 0, 0, 1, 0, 0));
                        }
                    }
                    
                    // init pawn attacks bitboard
                    attacks = pawn_captures[side][source_space] & occupancies[white];
                    
                    // generate pawn captures
                    while (attacks)
                    {
                        // init target space
                        target_space = get_leastsigbit(attacks);
                        
                        // pawn promotion
                        if (source_space >= a2 && source_space <= h2)
                        {
                            add_move(move_list, encode_move(source_space, target_space, piece, q, 1, 0, 0, 0));
                            add_move(move_list, encode_move(source_space, target_space, piece, r, 1, 0, 0, 0));
                            add_move(move_list, encode_move(source_space, target_space, piece, b, 1, 0, 0, 0));
                            add_move(move_list, encode_move(source_space, target_space, piece, n, 1, 0, 0, 0));
                        }
                        
                        else
                            // one space ahead pawn move
                            add_move(move_list, encode_move(source_space, target_space, piece, 0, 1, 0, 0, 0));
                        
                        // pop ls1b of the pawn attacks
                        pop_bit(attacks, target_space);
                    }
                    
                    // generate enpassant captures
                    if (enpassant != no_space)
                    {
                        // lookup pawn attacks, bitwise AND with enpassant space (bit)
                        U64 enpassant_attacks = pawn_captures[side][source_space] & (1ULL << enpassant);
                        
                        // make sure enpassant capture available
                        if (enpassant_attacks)
                        {
                            // init enpassant capture target space
                            int target_enpassant = get_leastsigbit(enpassant_attacks);
                            printf("%s%s  pawn enpassant capture\n", space_to_coordinates[source_space], space_to_coordinates[target_enpassant]);
                            add_move(move_list, encode_move(source_space, target_enpassant, piece, 0, 1, 0, 1, 0));
                        }
                    }
                    
                    // pop ls1b from piece bitboard copy
                    pop_bit(bitboard, source_space);
                }
            }
            
            // castling moves
            if (piece == k)
            {
                // king side castling is available
                if (castle & bk)
                {
                    // make sure space between king and king's rook are empty   
                    if (!get_bit(occupancies[both], f8) && !get_bit(occupancies[both], g8))
                    {
                        // make sure king and the f8 spaces are not under attacks
                        if (!is_space_attacked(e8, white) && !is_space_attacked(f8, white))
                            printf("e8g8  castling move\n");
                    }
                }
                
                // queen side castling is available
                if (castle & bq)
                {
                    // make sure space between king and queen's rook are empty
                    if (!get_bit(occupancies[both], d8) && !get_bit(occupancies[both], c8) && !get_bit(occupancies[both], b8))
                    {
                        // make sure king and the d8 spaces are not under attacks
                        if (!is_space_attacked(e8, white) && !is_space_attacked(d8, white))
                            printf("e8c8  castling move: \n");
                    }
                }
            }
        }
        
        // genarate knight moves
        if ((side == white) ? piece == N : piece == n)
        {
            // loop over source spaces of piece bitboard copy
            while (bitboard)
            {
                // init source space
                source_space = get_leastsigbit(bitboard);
                
                // init piece attacks in order to get set of target spaces
                attacks = knight_attacks[source_space] & ((side == white) ? ~occupancies[white] : ~occupancies[black]);
                
                // loop over target spaces available from generated attacks
                while (attacks)
                {
                    // init target space
                    target_space = get_leastsigbit(attacks);    
                    
                    // quite move
                    if (!get_bit(((side == white) ? occupancies[black] : occupancies[white]), target_space))
                        printf("%s%s  piece quiet move\n", space_to_coordinates[source_space], space_to_coordinates[target_space]);
                    
                    else
                        // capture move
                        printf("%s%s  piece capture\n", space_to_coordinates[source_space], space_to_coordinates[target_space]);
                    
                    // pop ls1b in current attacks set
                    pop_bit(attacks, target_space);
                }
                
                
                // pop ls1b of the current piece bitboard copy
                pop_bit(bitboard, source_space);
            }
        }
        
        // generate bishop moves
        if ((side == white) ? piece == B : piece == b)
        {
            // loop over source spaces of piece bitboard copy
            while (bitboard)
            {
                // init source space
                source_space = get_leastsigbit(bitboard);
                
                // init piece attacks in order to get set of target spaces
                attacks = get_bishop_attacks(source_space, occupancies[both]) & ((side == white) ? ~occupancies[white] : ~occupancies[black]);
                
                // loop over target spaces available from generated attacks
                while (attacks)
                {
                    // init target space
                    target_space = get_leastsigbit(attacks);    
                    
                    // quite move
                    if (!get_bit(((side == white) ? occupancies[black] : occupancies[white]), target_space))
                        printf("%s%s  piece quiet move\n", space_to_coordinates[source_space], space_to_coordinates[target_space]);
                    
                    else
                        // capture move
                        printf("%s%s  piece capture\n", space_to_coordinates[source_space], space_to_coordinates[target_space]);
                    
                    // pop ls1b in current attacks set
                    pop_bit(attacks, target_space);
                }
                
                
                // pop ls1b of the current piece bitboard copy
                pop_bit(bitboard, source_space);
            }
        }
        
        // generate rook moves
        if ((side == white) ? piece == R : piece == r)
        {
            // loop over source spaces of piece bitboard copy
            while (bitboard)
            {
                // init source space
                source_space = get_leastsigbit(bitboard);
                
                // init piece attacks in order to get set of target spaces
                attacks = get_rook_attacks(source_space, occupancies[both]) & ((side == white) ? ~occupancies[white] : ~occupancies[black]);
                
                // loop over target spaces available from generated attacks
                while (attacks)
                {
                    // init target space
                    target_space = get_leastsigbit(attacks);    
                    
                    // quite move
                    if (!get_bit(((side == white) ? occupancies[black] : occupancies[white]), target_space))
                        printf("%s%s  piece quiet move\n", space_to_coordinates[source_space], space_to_coordinates[target_space]);
                    
                    else
                        // capture move
                        printf("%s%s  piece capture\n", space_to_coordinates[source_space], space_to_coordinates[target_space]);
                    
                    // pop ls1b in current attacks set
                    pop_bit(attacks, target_space);
                }
                
                
                // pop ls1b of the current piece bitboard copy
                pop_bit(bitboard, source_space);
            }
        }
        
        // generate queen moves
        if ((side == white) ? piece == Q : piece == q)
        {
            // loop over source spaces of piece bitboard copy
            while (bitboard)
            {
                // init source space
                source_space = get_leastsigbit(bitboard);
                
                // init piece attacks in order to get set of target spaces
                attacks = get_queen_attacks(source_space, occupancies[both]) & ((side == white) ? ~occupancies[white] : ~occupancies[black]);
                
                // loop over target spaces available from generated attacks
                while (attacks)
                {
                    // init target space
                    target_space = get_leastsigbit(attacks);    
                    
                    // quite move
                    if (!get_bit(((side == white) ? occupancies[black] : occupancies[white]), target_space))
                        printf("%s%s  piece quiet move\n", space_to_coordinates[source_space], space_to_coordinates[target_space]);
                    
                    else
                        // capture move
                        printf("%s%s  piece capture\n", space_to_coordinates[source_space], space_to_coordinates[target_space]);
                    
                    // pop ls1b in current attacks set
                    pop_bit(attacks, target_space);
                }
                
                
                // pop ls1b of the current piece bitboard copy
                pop_bit(bitboard, source_space);
            }
        }

        // generate king moves
        if ((side == white) ? piece == K : piece == k)
        {
            // loop over source spaces of piece bitboard copy
            while (bitboard)
            {
                // init source space
                source_space = get_leastsigbit(bitboard);
                
                // init piece attacks in order to get set of target spaces
                attacks = king_attacks[source_space] & ((side == white) ? ~occupancies[white] : ~occupancies[black]);
                
                // loop over target spaces available from generated attacks
                while (attacks)
                {
                    // init target space
                    target_space = get_leastsigbit(attacks);    
                    
                    // quite move
                    if (!get_bit(((side == white) ? occupancies[black] : occupancies[white]), target_space))
                        printf("%s%s  piece quiet move\n", space_to_coordinates[source_space], space_to_coordinates[target_space]);
                    
                    else
                        // capture move
                        printf("%s%s  piece capture\n", space_to_coordinates[source_space], space_to_coordinates[target_space]);
                    
                    // pop ls1b in current attacks set
                    pop_bit(attacks, target_space);
                }

                // pop ls1b of the current piece bitboard copy
                pop_bit(bitboard, source_space);
            }
        }
    }
}

/*

------

MAIN PROGRAM

------

*/

int main()
{
    // init all
    init_all();
    // parse fen
    parse_fen("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPpP/R3K2R b KQkq a3 0 1 ");

    // create move list
    moves move_list[1];

generate_moves(move_list);

print_move_list(move_list);
    
    // return successx

// print a pawn_captures at a4
    print_board();
return 0;
}

