/*

------

HEADERS

------

*/

#include <iostream>
#include <stdio.h>
#include <string>
#include <windows.h>
#include <cstdint>
#include <bitset>
#include <stdint.h>
#include <unordered_map>
#include <chrono>
#include <stdlib.h>

// defined bitboard type
#define U64 unsigned long long  
using namespace std;

// FEN debug positions
#define empty_board "8/8/8/8/8/8/8/8 b - - "
#define start_position "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1 "
#define tricky_position "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1 "
#define killer_position "rnbqkb1r/pp1p1pPp/8/2p1pP2/1P1P4/3P3P/P1P1P3/RNBQKBNR w KQkq e6 0 1"
#define cmk_position "r2q1rk1/ppp2ppp/2n1bn2/2b1p3/3pP3/3P1NPP/PPP1NPB1/R1BQ1RK1 b - - 0 9 "

enum {
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
    unordered_map<int, char> char_pieces = {
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
    unordered_map<int, char> promoted_pieces = {
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

static  int count_bits(U64 bitboard)
{
return __builtin_popcountll(bitboard);
}

static  int get_leastsigbit(U64 bitboard)
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
                fen++;
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
U64 rook_table[64][4096];
U64 bishop_table[64][512];



/*

------

ATTACKS

------

*/

U64 masked_knight_attack(int space) {
    U64 attacks = 0ULL;
    U64 bitboard = 1ULL << space;
    set_bit(bitboard, space);
    
    if ((bitboard >> 17) & not_H_column) attacks |= (bitboard >> 17);
    if ((bitboard >> 15) & not_A_column) attacks |= (bitboard >> 15);
    if ((bitboard >> 10) & not_HG_column) attacks |= (bitboard >> 10);
    if ((bitboard >> 6) & not_AB_column) attacks |= (bitboard >> 6);
    if ((bitboard << 17) & not_A_column) attacks |= (bitboard << 17);
    if ((bitboard << 15) & not_H_column) attacks |= (bitboard << 15);
    if ((bitboard << 10) & not_AB_column) attacks |= (bitboard << 10);
    if ((bitboard << 6) & not_HG_column) attacks |= (bitboard << 6);
    
    return attacks;
}



U64 masked_king_attack(int space) {
    U64 attacks = 0ULL;
    U64 bitboard = 1ULL << space; 

    set_bit(bitboard, space);

    if (bitboard >> 8ULL) attacks |= (bitboard >> 8ULL);
    if ((bitboard >> 9ULL) & not_H_column) attacks |= (bitboard >> 9ULL);
    if ((bitboard >> 7ULL) & not_A_column) attacks |= (bitboard >> 7ULL);
    if ((bitboard >> 1ULL) & not_H_column) attacks |= (bitboard >> 1ULL);
    if (bitboard << 8ULL) attacks |= (bitboard <<8ULL);
    if ((bitboard << 9ULL) & not_A_column) attacks |= (bitboard << 9ULL);
    if ((bitboard << 7ULL) & not_H_column) attacks |= (bitboard << 7ULL);
    if ((bitboard << 1ULL) & not_A_column) attacks |= (bitboard << 1ULL);
    
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
static  U64 get_bishop_attacks(int space, U64 occupancy){

    occupancy &= bishop_masks[space];
    occupancy *= bishopmagics[space];
    occupancy >>= 64 - bishopspace[space];

    return bishop_table[space][occupancy];
}

// get rook attacks
static  U64 get_rook_attacks(int space, U64 occupancy){

    occupancy &= rook_masks[space];
    occupancy *= rookmagics[space];
    occupancy >>= 64 - rookspace[space];

    return rook_table[space][occupancy];
}

// get queen attacks

static  U64 get_queen_attacks(int space, U64 occupancy)
{
    // init result attacks bitboard
    U64 queen_attacks = 0ULL;
    
    // init bishop occupancies
    U64 bishop_occupancy = occupancy;
    
    // init rook occupancies
    U64 rook_occupancy = occupancy;
    
    // get bishop attacks assuming current board occupancy
    bishop_occupancy &= bishop_masks[space];
    bishop_occupancy *= bishopmagics[space];
    bishop_occupancy >>= 64 - bishopspace[space];
    
    // get bishop attacks
    queen_attacks = bishop_table[space][bishop_occupancy];
    
    // get rook attacks assuming current board occupancy
    rook_occupancy &= rook_masks[space];
    rook_occupancy *= rookmagics[space];
    rook_occupancy >>= 64 - rookspace[space];
    
    // get rook attacks
    queen_attacks |= rook_table[space][rook_occupancy];
    
    // return queen attacks
    return queen_attacks;
}



/*

------

SPACE ATTACKED

------

*/

static  int is_space_attacked(int space, int side)
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



/*

------

ENCODING

------

*/

// encode moves macro
#define encode_move(source, target, piece, promoted, capture, double_push, enpassant_m, castling) \
(source) |                   \
(target << 6) |             \
(piece << 12) |                 \
(promoted << 16) |          \
(capture << 20) |           \
(double_push << 21)  |            \
(enpassant_m << 22) |         \
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
static  void add_move(moves *move_list, int move)
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







#define board_copy()                                                                \
    U64 bitboards_copy[12], occupancies_copy[3];                                    \
    int side_copy, enpassant_copy, castle_copy;                                    \
    memcpy(bitboards_copy, bitboards, 96);                                    \
    memcpy(occupancies_copy, occupancies, 24);                                    \
    side_copy = side, enpassant_copy = enpassant, castle_copy = castle;                                    \



#define board_back()                                     \
    memcpy(bitboards, bitboards_copy, 96);                                     \
    memcpy(occupancies, occupancies_copy, 24);                                     \
    side = side_copy, enpassant = enpassant_copy, castle = castle_copy;                                     \


    // types of moves

enum { all_moves, only_capture };

/*
                           castling   move     in      in
                              right update     binary  decimal
 king & rooks didn't move:     1111 & 1111  =  1111    15
        white king  moved:     1111 & 1100  =  1100    12
  white king's rook moved:     1111 & 1110  =  1110    14
 white queen's rook moved:     1111 & 1101  =  1101    13
     
         black king moved:     1111 & 0011  =  1011    3
  black king's rook moved:     1111 & 1011  =  1011    11
 black queen's rook moved:     1111 & 0111  =  0111    7
*/

// castling rights update constants
const int castling_rights[64] = {
    // a8, b8, c8, d8, e8, f8, g8, h8
     7, 15, 15, 15,  3, 15, 15, 11,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    13, 15, 15, 15, 12, 15, 15, 14
    // a1, b1, c1, d1, e1, f1, g1, h1
};

/*

------

MOVE GENERATION + MOVE MAKING

------

*/

// make move on chess board
static int make_move(int move, int move_flag)
{
    // quite moves
    if (move_flag == all_moves)
    {
        // preserve board state
        board_copy();
        
        // parse move
        int source_space = get_move_source(move);
        int target_space = get_move_target(move);
        int piece = get_move_piece(move);
        int promoted_piece = get_move_promoted(move);
        int capture = get_move_capture(move);
        int double_push = get_move_double(move);
        int enpass = get_move_enpassant(move);
        int castling = get_move_castling(move);
        
        // move piece
        pop_bit(bitboards[piece], source_space);
        set_bit(bitboards[piece], target_space);
        
        // handling capture moves
        if (capture)
        {
            // pick up bitboard piece index ranges depending on side
            int start_piece, end_piece;
            
            // white to move
            if (side == white)
            {
                start_piece = p;
                end_piece = k;
            }
            
            // black to move
            else
            {
                start_piece = P;
                end_piece = K;
            }
            
            // loop over bitboards opposite to the current side to move
            for (int bitboard_piece = start_piece; bitboard_piece <= end_piece; bitboard_piece++)
            {
                // if there's a piece on the target space
                if (get_bit(bitboards[bitboard_piece], target_space))
                {
                    // remove it from corresponding bitboard
                    pop_bit(bitboards[bitboard_piece], target_space);
                    break;
                }
            }
        }
        
        // handle pawn promotions
        if (promoted_piece)
        {
            // erase the pawn from the target space
            pop_bit(bitboards[(side == white) ? P : p], target_space);
            
            // set up promoted piece on chess board
            set_bit(bitboards[promoted_piece], target_space);
        }
        
        // handle enpassant captures
        if (enpass)
        {
            // erase the pawn depending on side to move
            (side == white) ? pop_bit(bitboards[p], target_space + 8) :
                              pop_bit(bitboards[P], target_space - 8);
        }
        
        // reset enpassant space
        enpassant = no_space;
        
        // handle double pawn push
        if (double_push)
        {
            // set enpassant aquare depending on side to move
            (side == white) ? (enpassant = target_space + 8) :
                              (enpassant = target_space - 8);
        }
        
        // handle castling moves
        if (castling)
        {
            // switch target space
            switch (target_space)
            {
                // white castles king side
                case (g1):
                    // move H rook
                    pop_bit(bitboards[R], h1);
                    set_bit(bitboards[R], f1);
                    break;
                
                // white castles queen side
                case (c1):
                    // move A rook
                    pop_bit(bitboards[R], a1);
                    set_bit(bitboards[R], d1);
                    break;
                
                // black castles king side
                case (g8):
                    // move H rook
                    pop_bit(bitboards[r], h8);
                    set_bit(bitboards[r], f8);
                    break;
                
                // black castles queen side
                case (c8):
                    // move A rook
                    pop_bit(bitboards[r], a8);
                    set_bit(bitboards[r], d8);
                    break;
            }
        }
        
        // update castling rights
        castle &= castling_rights[source_space];
        castle &= castling_rights[target_space];

        // rsetting occupancies
        memset(occupancies, 0ULL, 24);

        for (int bitboard_pieces = P; bitboard_pieces <= K; bitboard_pieces++)
            occupancies[white] |= bitboards[bitboard_pieces];
        
        for (int bitboard_pieces = p; bitboard_pieces <= k; bitboard_pieces++)
            occupancies[black] |= bitboards[bitboard_pieces];

        // update both sides occupancies
         occupancies[both] |= occupancies[white];
         occupancies[both] |= occupancies[black];

         // change the side

         side ^= 1;
        
        // check if the move is legal
        if (is_space_attacked((side == white) ? get_leastsigbit(bitboards[k]) : get_leastsigbit(bitboards[K]), side))
        {
        // move is illegal, so it is undone
        board_back();

        return 0;
        }

        //
        else
        // legal move
        return 1;

    }
    
    // capture moves
    else
    {
        // make sure move is the capture
        if (get_move_capture(move))
            make_move(move, all_moves);
        
        // otherwise the move is not a capture
        else
            // don't make it
            return 0;
    }
    return 0;
}

// generate all moves
static  void generate_moves(moves *move_list)
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
                                // double pawn push
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
                            add_move(move_list, encode_move(source_space, target_enpassant, piece, 0, 1, 0, 1, 0));
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
                    
                    // generate quiet pawn moves
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
                            if ((source_space >= a7 && source_space <= h7) && !get_bit(occupancies[both], (target_space + 8))) {
                            if (!get_bit(occupancies[both], target_space)) { // check if the space in front of the pawn is also unoccupied
                            add_move(move_list, encode_move(source_space, target_space + 8, piece, 0, 0, 1, 0, 0));
                         }
                      }
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
                            add_move(move_list, encode_move(e8, g8, piece, 0, 0, 0, 0, 1));

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
                        add_move(move_list, encode_move(e8, c8, piece, 0, 0, 0, 0, 1));
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
                    
                    // quiet move
                    if (!get_bit(((side == white) ? occupancies[black] : occupancies[white]), target_space))
                        add_move(move_list, encode_move(source_space, target_space, piece, 0, 0, 0, 0, 0));
                    
                    else
                        // capture move
                        add_move(move_list, encode_move(source_space, target_space, piece, 0, 1, 0, 0, 0));
                    
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
                    
                    // quiet move
                    if (!get_bit(((side == white) ? occupancies[black] : occupancies[white]), target_space))
                        add_move(move_list, encode_move(source_space, target_space, piece, 0, 0, 0, 0, 0));
                    
                    else
                        // capture move
                        add_move(move_list, encode_move(source_space, target_space, piece, 0, 1, 0, 0, 0));
                    
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
                    
                    // quiet move
                    if (!get_bit(((side == white) ? occupancies[black] : occupancies[white]), target_space))
                        add_move(move_list, encode_move(source_space, target_space, piece, 0, 0, 0, 0, 0));
                    
                    else
                        // capture move
                    add_move(move_list, encode_move(source_space, target_space, piece, 0, 1, 0, 0, 0));
                    
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
                    
                    // quiet move
                    if (!get_bit(((side == white) ? occupancies[black] : occupancies[white]), target_space))
                       add_move(move_list, encode_move(source_space, target_space, piece, 0, 0, 0, 0, 0));
                    
                    else
                        // capture move
                        add_move(move_list, encode_move(source_space, target_space, piece, 0, 1, 0, 0, 0));
                    
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
                    
                    // quiet move
                    if (!get_bit(((side == white) ? occupancies[black] : occupancies[white]), target_space))
                        add_move(move_list, encode_move(source_space, target_space, piece, 0, 0, 0, 0, 0));
                    
                    else
                        // capture move
                        add_move(move_list, encode_move(source_space, target_space, piece, 0, 1, 0, 0, 0));
                    
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

PERFT DRIVERS + TEST FOR NODE AND DEPTH TESTING

------

*/

// leaf nodes (number of positions reached during the test of the move generator at a given depth)
long nodes;

// perft driver
static inline void perft_driver(int depth)
{
    // reccursion escape condition
    if (depth == 0)
    {
        // increment nodes count (count reached positions)
        nodes++;
        return;
    }
    
    // create move list instance
    moves move_list[1];
    
    // generate moves
    generate_moves(move_list);
    
        // loop over generated moves
    for (int move_count = 0; move_count < move_list->count; move_count++)
    {   
        // preserve board state
        board_copy();
        
        // make move
        if (!make_move(move_list->moves[move_count], all_moves))
            // skip to the next move
            continue;
        
        // call perft driver recursively
        perft_driver(depth - 1);
        
        // take back
        board_back();
    }
}

// perft test

void perft_test (int depth)
{
    printf("\n ---------------- \n");
    printf("\n Performance:   \n");

        // create move list instance
    moves move_list[1];
    
    // generate moves
    generate_moves(move_list);
    
    // start timer 
    auto start = chrono::high_resolution_clock::now();
        // loop over generated moves
    for (int move_count = 0; move_count < move_list->count; move_count++)
    {   
        // preserve board state
        board_copy();
        
        // make move
        if (!make_move(move_list->moves[move_count], all_moves))
            // skip to the next move
            continue;
        
        long cummulative_nodes = nodes;

        // call perft driver recursively
        perft_driver(depth - 1);

        long old_nodes = nodes - cummulative_nodes;
        
        // take back
        board_back();

        // print move
        printf("     move: %s%s%c  nodes: %ld\n", space_to_coordinates[get_move_source(move_list->moves[move_count])],
                                                 space_to_coordinates[get_move_target(move_list->moves[move_count])],
                                                 get_move_promoted(move_list->moves[move_count]) ? promoted_pieces[get_move_promoted(move_list->moves[move_count])] : ' ',
                                                 old_nodes);
    }
    // stop timer
    auto finish = chrono::high_resolution_clock::now();
    // print results
    printf("\n    Depth: %d\n", depth);
    printf("    Nodes: %ld\n", nodes);
    // print time in miliseconds
    chrono::duration<double> elapsed = finish - start;
    printf("    Time: %f ms\n", elapsed.count() * 1000);
}

/*

------

Evaluation

------

*/

int material_score[12] = {
    100,      // white pawn score
    325,      // white knight score
    350,      // white bishop score
    500,      // white rook score
    900,      // white queen score
  10000,      // white king score
   -100,      // black pawn score
   -325,      // black knight score
   -350,      // black bishop score
   -500,      // black rook score
  -900,      // black queen score
 -10000,      // black king score
};

const int pawn_score[64] = 
{
    90,  90,  90,  90,  90,  90,  90,  90,
    30,  30,  30,  40,  40,  30,  30,  30,
    20,  20,  20,  30,  30,  30,  20,  20,
    10,  10,  10,  20,  20,  10,  10,  10,
     5,   5,  25,  25,  20,   5,   5,   5,
     0,   0,  10,  20,   5,   0,   0,   0,
    -10, -10, 10,  10, -10,  -5, -10, -10,
     0,   0,   0,   0,   0,   0,   0,   0
};

// knight positional score
const int knight_score[64] = 
{
    -5,   0,   0,   0,   0,   0,   0,  -5,
    -5,   0,   0,  10,  10,   0,   0,  -5,
    -5,   5,  20,  20,  20,  20,   5,  -5,
    -5,  10,  20,  30,  30,  20,  10,  -5,
    -5,  10,  20,  30,  30,  20,  10,  -5,
    -5,   0, -10,  10,  10,  -10,   0,  -5,
    -5,   0,   0,   0,   0,   0,   0,  -5,
    -5, 0,   0,   0,   0,   0, 0,  -5
};

// bishop positional score
const int bishop_score[64] = 
{
     0,   0,   0,   0,   0,   0,   0,   0,
     0,   0,   0,   0,   0,   0,   0,   0,
     0,   0,   0,  10,  10,   0,   0,   0,
     0,   0,  10,  20,  20,  10,   0,   0,
     0,   0,  10,  20,  20,  10,   0,   0,
     0,  10,   0,   0,   0,   0,  10,   0,
     0,  30,   0,   0,   0,   0,  30,   0,
     0,   0, -10,   0,   0, -10,   0,   0

};

// rook positional score
const int rook_score[64] =
{
    50,  50,  50,  50,  50,  50,  50,  50,
    50,  50,  50,  50,  50,  50,  50,  50,
     0,   0,  10,  20,  20,  10,   0,   0,
     0,   0,  10,  20,  20,  10,   0,   0,
     0,   0,  10,  20,  20,  10,   0,   0,
     0,   0,  10,  20,  20,  10,   0,   0,
     0,   0,  10,  20,  20,  10,   0,   0,
     -15,   0,   0,  20,  20,   0,   0,   0

};

// king positional score
const int king_score[64] = 
{
     0,   0,   0,   0,   0,   0,   0,   0,
     0,   0,   5,   5,   5,   5,   0,   0,
     0,   5,   5,  10,  10,   5,   5,   0,
     0,   5,  10,  20,  20,  10,   5,   0,
     0,   5,  10,  20,  20,  10,   5,   0,
     0,   0,   5,  10,  10,   5,   0,   0,
     0,   5,   5,  10,  10,   0,   5,   0,
     0,   0,   5,   0, -15,   0,  10,   0
};

const int queen_score[64]
{
     0,   0,   0,   0,   0,   0,   0,   0,
     0,   0,   0,   0,   0,   0,   0,   0,
     0,   5,   5,  0,  0,   0,   0,   0,
     0,   5,  15,  20,  20,  15,   5,   0,
     0,   5,  15,  20,  20,  15,   5,   0,
     0,   0,   5,  15,  15,   5,   0,   0,
     0,   5,  -10,  -10,  -10,   0,   5,   0,
     0,   0,   5,   0, -15,   0,  10,   0

};

// mirror positional score tables for opposite side
const int mirror_score[128] =
{
	a1, b1, c1, d1, e1, f1, g1, h1,
	a2, b2, c2, d2, e2, f2, g2, h2,
	a3, b3, c3, d3, e3, f3, g3, h3,
	a4, b4, c4, d4, e4, f4, g4, h4,
	a5, b5, c5, d5, e5, f5, g5, h5,
	a6, b6, c6, d6, e6, f6, g6, h6,
	a7, b7, c7, d7, e7, f7, g7, h7,
	a8, b8, c8, d8, e8, f8, g8, h8
};


static inline int evaluate()
{
    // static evaluation score
    int score = 0;
    
    // current pieces bitboard copy
    U64 bitboard;
    
    // init piece & space
    int piece, space;
    
    // loop over piece bitboards
    for (int bitboard_pieces = P; bitboard_pieces <= k; bitboard_pieces++)
    {
        // init piece bitboard copy
        bitboard = bitboards[bitboard_pieces];
        
        // loop over pieces within a bitboard
        while (bitboard)
        {
            // init piece
            piece = bitboard_pieces;
            
            // init space
            space = get_leastsigbit(bitboard);
            
            // score material weights
            score += material_score[piece];
            
            // score positional piece scores
            switch (piece)
            {
                // evaluate white pieces
                case P: score += pawn_score[space]; break;
                case N: score += knight_score[space]; break;
                case B: score += bishop_score[space]; break;
                case R: score += rook_score[space]; break;
                case Q: score += queen_score[space]; break;
                case K: score += king_score[space]; break;

                // evaluate black pieces
                case p: score -= pawn_score[mirror_score[space]]; break;
                case n: score -= knight_score[mirror_score[space]]; break;
                case b: score -= bishop_score[mirror_score[space]]; break;
                case r: score -= rook_score[mirror_score[space]]; break;
                case q: score += queen_score[mirror_score[space]]; break;
                case k: score -= king_score[mirror_score[space]]; break;
            }
            
            
            // pop ls1b
            pop_bit(bitboard, space);
        }
    }
    
    // return final evaluation based on side
    return (side == white) ? score : -score;
}

/*

------

Search

------

*/

/**********************************\
 ==================================
 
               Search
 
 ==================================
\**********************************/

// most valuable victim & less valuable attacker

/*
                          
P - 6002 20225 20250 20400 20800 26900
N - 4775  6004 20025 20175 20575 26675
B - 4750  4975  6006 20150 20550 26650
R - 4600  4825  4850  6008 20400 26500
Q - 4200  4425  4450  4600  6010 26100
K - 3100  3325  3350  3500  3900 26000
*/

// MVV LVA [attacker][victim]
static int mvv_lva[12][12] = {
 	6002, 20225, 20250, 20400, 20800, 26900, 
	4775,  6004, 20025, 20175, 20575, 26675,  
	4750,  4975,  6006, 20150, 20550, 26650,  
	4600,  4825,  4850,  6008, 20400, 26500,  
	4200,  4425,  4450,  4600,  6010, 26100, 
	3100,  3325,  3350,  3500,  3900, 2600, 
    6002, 20225, 20250, 20400, 20800, 26900, 
	4775,  6004, 20025, 20175, 20575, 26675,  
	4750,  4975,  6006, 20150, 20550, 26650,  
	4600,  4825,  4850,  6008, 20400, 26500,  
	4200,  4425,  4450,  4600,  6010, 26100, 
	3100,  3325,  3350,  3500,  3900, 2600,
    6002, 20225, 20250, 20400, 20800, 26900, 
	4775,  6004, 20025, 20175, 20575, 26675,  
	4750,  4975,  6006, 20150, 20550, 26650,  
	4600,  4825,  4850,  6008, 20400, 26500,  
	4200,  4425,  4450,  4600,  6010, 26100, 
	3100,  3325,  3350,  3500,  3900, 2600, 
    6002, 20225, 20250, 20400, 20800, 26900, 
	4775,  6004, 20025, 20175, 20575, 26675,  
	4750,  4975,  6006, 20150, 20550, 26650,  
	4600,  4825,  4850,  6008, 20400, 26500,  
	4200,  4425,  4450,  4600,  6010, 26100, 
	3100,  3325,  3350,  3500,  3900, 2600, 

};

// killer moves [id] (of killer) [ply]

int killer_moves[2][64];

// history moves [piece][space]
int history_moves[12][64];

// half move counter
int ply;

// best move
int best_move;

static inline int score_move(int move)
{
    // score capture move
    if (get_move_capture(move))
    {
        // init target piece
        int target_piece = P;
        
        // pick up bitboard piece index ranges depending on side
        int start_piece, end_piece;
        
        // pick up side to move
        if (side == white) { start_piece = p; end_piece = k; }
        else { start_piece = P; end_piece = K; }
        
        // loop over bitboards opposite to the current side to move
        for (int bitboard_piece = start_piece; bitboard_piece <= end_piece; bitboard_piece++)
        {
            // if there's a piece on the target space
            if (get_bit(bitboards[bitboard_piece], get_move_target(move)))
            {
                // remove it from corresponding bitboard
                target_piece = bitboard_piece;
                break;
            }
        }
                
        // score move by MVV LVA lookup [source piece][target piece]
        return mvv_lva[get_move_piece(move)][target_piece] + 100;
    }
    
    // score quiet move
    else
    {
        // score 1st killer move
        if (killer_moves[0][ply] == move) return 90;

        // score 2nd killer move
        if (killer_moves[1][ply] == move) return 80;

        // score history move
        else return history_moves[get_move_piece(move)][get_move_target(move)];
    }
    
    return 0;
}

void merge(moves *move_list, int start, int mid, int end, int move_scores[]) {
    int i = start, j = mid+1, k = start;
    int temp_move_list[move_list->count];

    while (i <= mid && j <= end) {
        if (move_scores[i] < move_scores[j]) {
            temp_move_list[k] = move_list->moves[i];
            i++;
        } else {
            temp_move_list[k] = move_list->moves[j];
            j++;
        }
        k++;
    }

    while (i <= mid) {
        temp_move_list[k] = move_list->moves[i];
        i++;
        k++;
    }

    while (j <= end) {
        temp_move_list[k] = move_list->moves[j];
        j++;
        k++;
    }

    for (int x = start; x <= end; x++) {
        move_list->moves[x] = temp_move_list[x];
    }
}

void merge_sort(moves *move_list, int start, int end, int move_scores[]) {
    if (start < end) {
        int mid = (start + end) / 2;
        merge_sort(move_list, start, mid, move_scores);
        merge_sort(move_list, mid+1, end, move_scores);
        merge(move_list, start, mid, end, move_scores);
    }
}

static inline int sort_moves(moves *move_list)
{
    // move scores
    int move_scores[move_list->count];
    
    // score all the moves within a move list
    for (int count = 0; count < move_list->count; count++)
        // score move
        move_scores[count] = score_move(move_list->moves[count]);
    
    // loop over current move within a move list
    for (int current_move = 0; current_move < move_list->count; current_move++)
    {
        // loop over next move within a move list
        for (int next_move = current_move + 1; next_move < move_list->count; next_move++)
        {
            // compare current and next move scores
            if (move_scores[current_move] < move_scores[next_move])
            {
                // swap scores
                int temp_score = move_scores[current_move];
                move_scores[current_move] = move_scores[next_move];
                move_scores[next_move] = temp_score;
                
                // swap moves
                int temp_move = move_list->moves[current_move];
                move_list->moves[current_move] = move_list->moves[next_move];
                move_list->moves[next_move] = temp_move;
            }
        }
    }
    return 0;
}

// print move scores
void print_move_scores(moves *move_list)
{
    printf("     Move scores:\n\n");
        
    // loop over moves within a move list
    for (int count = 0; count < move_list->count; count++)
    {
        printf("     move: ");
        print_move(move_list->moves[count]);
        printf(" score: %d\n", score_move(move_list->moves[count]));
    }
}


// quiescence search
static inline int quiescence(int alpha, int beta)
{
    // increment nodes count
    nodes++;

    // evaluate position
    int evaluation = evaluate();
    
    // fail-hard beta cutoff
    if (evaluation >= beta)
    {
        // node (move) fails high
        return beta;
    }
    
    // found a better move
    if (evaluation > alpha)
    {
        // PV node (move)
        alpha = evaluation;
    }
    
    // create move list instance
    moves move_list[1];
    
    // generate moves
    generate_moves(move_list);
    
    sort_moves(move_list);

    // loop over moves within a movelist
    for (int count = 0; count < move_list->count; count++)
    {
        // preserve board state
        board_copy();
        
        // increment ply
        ply++;
        
        // make sure to make only legal moves
        if (make_move(move_list->moves[count], only_capture) == 0)
        {
            // decrement ply
            ply--;
            
            // skip to next move
            continue;
        }

        // score current move
        int score = -quiescence(-beta, -alpha);
        
        // decrement ply
        ply--;

        // take move back
        board_back();
        
        // fail-hard beta cutoff
        if (score >= beta)
        {


            // node (move) fails high
            return beta;
        }
        
        // found a better move
        if (score > alpha)
        {
            // PV node (move)
            alpha = score;
            
        }
    }
    
    // node (move) fails low
    return alpha;
}

// negamax alpha beta search
static inline int negamax(int alpha, int beta, int depth)
{
    // recursion escapre condition
    if (depth == 0)
        // run quiescence search
        return quiescence(alpha, beta);
    
    // increment nodes count
    nodes++;
    
    // is king in check
    int in_check = is_space_attacked((side == white) ? get_leastsigbit(bitboards[K]) : 
                                                        get_leastsigbit(bitboards[k]),
                                                        side ^ 1);
    
    // increase search depth if the king has been exposed into a check
    if (in_check) depth++;
    
    // legal moves counter
    int legal_moves = 0;
    
    // best move so far
    int best_sofar;
    
    // old value of alpha
    int old_alpha = alpha;
    
    // create move list instance
    moves move_list[1];
    
    // generate moves
    generate_moves(move_list);
    
    // sort moves
    
    sort_moves(move_list);

    // loop over moves within a movelist
    for (int count = 0; count < move_list->count; count++)
    {
        // preserve board state
        board_copy();
        
        // increment ply
        ply++;
        
        // make sure to make only legal moves
        if (make_move(move_list->moves[count], all_moves) == 0)
        {
            // decrement ply
            ply--;
            
            // skip to next move
            continue;
        }
        
        // increment legal moves
        legal_moves++;
        
        // score current move
        int score = -negamax(-beta, -alpha, depth - 1);
        
        // decrement ply
        ply--;

        // take move back
        board_back();
        
        // fail-hard beta cutoff
        if (score >= beta)
        {
                        // store killer moves
            killer_moves[1][ply] = killer_moves[0][ply];
            killer_moves[0][ply] = move_list->moves[count];
            // node (move) fails high
            return beta;
        }
        
        // found a better move
        if (score > alpha)
        {
            // store history 
            history_moves[get_move_piece(move_list->moves[count])][get_move_target(move_list->moves[count])] += depth;
            // PV node (move)
            alpha = score;
            
            
            // if root move
            if (ply == 0)
                // associate best move with the best score
                best_sofar = move_list->moves[count];
        }
    }
    
    // we don't have any legal moves to make in the current postion
    if (legal_moves == 0)
    {
        // king is in check
        if (in_check)
            // return mating score (assuming closest distance to mating position)
            return -49000 + ply;
        
        // king is not in check
        else
            // return stalemate score
            return 0;
    }
    
    // found better move
    if (old_alpha != alpha)
        // init best move
        best_move = best_sofar;
    
    // node (move) fails low
    return alpha;
}

// search position for the best move
void search_position(int depth)
{
    // find best move within a given position
    int score = negamax(-50000, 50000, depth);
    
    // best move placeholder
if (best_move)
{
    printf("info score cp %d depth %d nodes %ld\n", score, depth, nodes);

    printf("bestmove ");
    print_move(best_move);
    printf("\n");
}
}





/*

------

UCI

------

*/

// parse user/gui move string input ( example: "a7a8q")
int move_parse(char *move_string)
{
    // create move list instance
    moves move_list[1];
    
    // generate moves
    generate_moves(move_list);
    
    // parse source space
    int source_space = (move_string[0] - 'a') + (8 - (move_string[1] - '0')) * 8;
    
    // parse target space
    int target_space = (move_string[2] - 'a') + (8 - (move_string[3] - '0')) * 8;
    
    // loop over the moves within a move list
    for (int move_count = 0; move_count < move_list->count; move_count++)
    {
        // init move
        int move = move_list->moves[move_count];
        
        // make sure source & target spaces are available within the generated move
        if (source_space == get_move_source(move) && target_space == get_move_target(move))
        {
            // init promoted piece
            int promoted_piece = get_move_promoted(move);
            
            // promoted piece is available
            if (promoted_piece)
            {
                // promoted to queen
                if ((promoted_piece == Q || promoted_piece == q) && move_string[4] == 'q')
                    // return legal move
                    return move;
                
                // promoted to rook
                else if ((promoted_piece == R || promoted_piece == r) && move_string[4] == 'r')
                    // return legal move
                    return move;
                
                // promoted to bishop
                else if ((promoted_piece == B || promoted_piece == b) && move_string[4] == 'b')
                    // return legal move
                    return move;
                
                // promoted to knight
                else if ((promoted_piece == N || promoted_piece == n) && move_string[4] == 'n')
                    // return legal move
                    return move;
                
                // continue the loop on possible wrong promotions (e.g. "e7e8f")
                continue;
            }
            
            // return legal move
            return move;
        }
    }
    
    // return illegal move
    return 0;
}

// uci position input

// parse UCI "position" input
void uci_position(const char *command)
{
    // shift pointer to the right where next token begins
    command += 9;
    
    // init pointer to the current character in the command string
    const char *current_char = command;
    
    // parse UCI "startpos" command
    if (strncmp(command, "startpos", 8) == 0)
        // init chess board with start position
        parse_fen(start_position);
    
    // parse UCI "fen" command 
    else
    {
        // make sure "fen" command is available within command string
        current_char = strstr(command, "fen");
        
        // if no "fen" command is available within command string
        if (current_char == NULL)
            // init chess board with start position
            parse_fen(start_position);
            
        // found "fen" substring
        else
        {
            // shift pointer to the right where next token begins
            current_char += 4;
            
            // init chess board with position from FEN string
            parse_fen(const_cast<char*>(current_char));
        }
    }
    
    // parse moves after position
    current_char = strstr(command, "moves");
    
    // moves available
    if (current_char != NULL)
    {
        // shift pointer to the right where next token begins
        current_char += 6;
        
        // loop over moves within a move string
        while(*current_char)
        {
            // parse next move
            int move = move_parse(const_cast<char*>(current_char));
            
            // if no more moves
            if (move == 0)
                // break out of the loop
                break;
            
            // make move on the chess board
            make_move(move, all_moves);
            
            // move current character mointer to the end of current move
            while (*current_char && *current_char != ' ') current_char++;
            
            // go to the next move
            current_char++;
        }
        
        
    }
    print_board();
}
    

/*

------

GO command

------

*/

void uci_go(char *command)
{
    int depth = -1;

    char *current_depth = NULL;

    if ((current_depth = strstr(command, "depth")))

    depth = atoi(current_depth + 6);

    else
    
        depth = 6;
    
    search_position(depth);
    

}

void uci_loop()
{
    // reset in and out buffers
    setbuf(stdin, NULL);
    setbuf(stdout, NULL);

    // define the gui
    char input[2000];

    // print engine info
    printf("id name Knightfall\n");
    printf("id author Rayane\n");
    printf("uciok\n");

    // main loop
    while (1)
    {
        // reset input
        memset(input, 0, sizeof(input));

        // output input
        fflush(stdout);

        // get input
        if (!fgets(input, 2000, stdin))
            continue;

        // check if input is available
        if (input[0] == '\n')
            continue;

        // parse uci isready
        if (strncmp(input, "isready", 7) == 0)
        {
            printf("readyok\n");
            continue;
        }

       else if (strncmp(input, "position", 8) == 0)
        uci_position(input);

       else if (strncmp(input, "ucinewgame", 10) == 0)
        
            uci_position("position startpos");
        
        // uci go parse
        else if (strncmp(input, "go", 2) == 0)
            uci_go(input);

        // parse quit input
        else if (strncmp(input, "quit", 4) == 0)
            break;
        
        // prase uci uci input
        else if (strncmp(input, "uci", 3) == 0)
        {
            printf("id name Knightfall\n");
            printf("id author Rayane\n");
            printf("uciok\n");
            continue;
        }



    }

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

MAIN PROGRAM

------

*/



int main()
{
    // init all
    init_all();

    // debug mode variable
    int debug = 1;
    
    // if debugging
    if (debug)
    {
        // parse fen
        parse_fen("rn1q2nr/1b2k1bp/3pppp1/p1B5/P1B5/1P2P2Q/2P2PPP/RN2K1NR b KQ - 1 12");
        print_board();
        search_position(5);
        
        // create move list instance
        moves move_list[1];
        
        // generate moves
        generate_moves(move_list);
        
        // sort move
        sort_moves(move_list);
        
        // print move scores
        print_move_scores(move_list);
    }
    
    else
        // connect to the GUI
        uci_loop();

    return 0;
}
