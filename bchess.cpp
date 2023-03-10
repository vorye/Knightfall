#include <iostream>
#include <stdio.h>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <time.h>
#include <windows.h>
#include <cstdint>

#define U64 unsigned long long  
using namespace std;

enum space {
a8, b8, c8, d8, e8, f8, g8, h8,
a7, b7, c7, d7, e7, f7, g7, h7,
a6, b6, c6, d6, e6, f6, g6, h6,
a5, b5, c5, d5, e5, f5, g5, h5,
a4, b4, c4, d4, e4, f4, g4, h4,
a3, b3, c3, d3, e3, f3, g3, h3,
a2, b2, c2, d2, e2, f2, g2, h2,
a1, b1, c1, d1, e1, f1, g1, h1,
};

enum colour {
  white = 0,
  black = 1
};

/*
 -- macros to make bitboard manipulation easier
*/

#define set_bit(bitboard, space) (bitboard |= (1ULL << space))    // sets the bit at the given space          
#define get_bit(bitboard, space) (bitboard & (1ULL << space))   // gets the bit at the given space
#define pop_bit(bitboard,space)  (get_bit(bitboard, space) ? bitboard ^= (1ULL << space) : 0) // pops the bit at the given space              

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
  -- ATTACKS
*/

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

// no A column

const U64 not_A_column = 18374403900871474942ULL; // bitboard to store the attacks

// no H column

const U64 not_H_column = 9187201950435737471ULL; 

// no HG column

const U64 not_HG_column = 4557430888798830399ULL; 

// no AB column

const U64 not_AB_column = 18229723555195321596ULL; 

// masked pawn attack

U64 masked_pawn_attack(int colour, int space) // returns the pawn attacks for the given space and colour
{
    U64 attacks = 0ULL; 
    U64 bitboard = 0ULL;  // bitboard to store the pawn

    set_bit(bitboard, space);  // set the bitboard to the given space

    if (colour == white)  // white pawn
    {
       if ((bitboard >> 7) & not_A_column) attacks |= (bitboard >> 7);   // set the attacks bitboard to the left diagonal
       if ((bitboard >> 9) & not_H_column) attacks |= (bitboard >> 9);   // set the attacks bitboard to the right diagonal
    }

    // black pawn
 else if (colour) 
  {
       if ((bitboard << 7) & not_H_column) attacks |= (bitboard << 7);   // set the attacks bitboard to the left diagonal
       if ((bitboard << 9) & not_A_column) attacks |= (bitboard << 9);   // set the attacks bitboard to the right diagonal; // set the attacks bitboard to the right diagonal
  }

    // returns the attacks bitboard
    return attacks;
}
// initialise leaping pieces potential attacks

void leaping_attacks()
{ 
    
    // loop through the bitboard spaces (64 of them)
    for (int space = 0; space < 64; space++)
    {
        // initialising pawn attacks
        pawn_attacks[white][space] = masked_pawn_attack(space, white); // white pawn attacks
        pawn_attacks[black][space] = masked_pawn_attack(space, black); // black pawn attacks
    }    

}


/*
  ==             ==
  == MAIN DRIVER ==
  ==             ==
*/ 

int main() { 
print_bitboard(masked_pawn_attack(black, a4)); // prints the pawn attacks for the given space and colou
 

    return 0;
}
