// Headers
#include <stdio.h>
#include <string.h>
#include "magic.h"
#include "utils.h"

// define bitboard data type
#define U64 unsigned long long


// enumerate board squares
enum {
    a8, b8, c8, d8, e8, f8, g8, h8,
    a7, b7, c7, d7, e7, f7, g7, h7,
    a6, b6, c6, d6, e6, f6, g6, h6,
    a5, b5, c5, d5, e5, f5, g5, h5,
    a4, b4, c4, d4, e4, f4, g4, h4,
    a3, b3, c3, d3, e3, f3, g3, h3,
    a2, b2, c2, d2, e2, f2, g2, h2,
    a1, b1, c1, d1, e1, f1, g1, h1, no_sq // Invalid square
};

// sides to move
enum {
    white,
    black,
    both
};

enum {
    rook,
    bishop
};

enum { P, N, B, R, Q, K, p, n, b, r, q, k }; // Uppercase white pieces

const char *square_to_coordinate[64] = {
    "a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8",
    "a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
    "a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
    "a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
    "a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
    "a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
    "a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
    "a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1"
};
// Ascii pieces
char ascii_pieces[12] = "PNBRQKpnbrqk";
char *unicode_pieces[12] = {"♟", "♞", "♝", "♜", "♛", "♚", "♙", "♘", "♗", "♖", "♕", "♔"};
// convert ASCII character pieces to encoded constants
int char_pieces[] = {
    ['P'] = P,
    ['N'] = N,
    ['B'] = B,
    ['R'] = R,
    ['Q'] = Q,
    ['K'] = K,
    ['p'] = p,
    ['n'] = n,
    ['b'] = b,
    ['r'] = r,
    ['q'] = q,
    ['k'] = k
};

/*
 *
0001 - 1 - white king can castle to the king side
0010 - 2 - white king can castle to the queens ide
0100 - 4 - black king can castle to the king side
1000 - 8 - black king can castle to the queen side

ex. 0101 - white king castle kingside, black king kingside
    1111 - everyone can castle everywhere

*/
enum {
    wk = 1,
    wq = 2,
    bk = 4,
    bq = 8
};


/********************************************
 *                 ATTACKS                  *
 * Description: Handles bitboard calculations
 * and logic for piece attacks on the board.
 ********************************************/

const U64 not_a_file = 18374403900871474942ULL; // not a file constant - all zeros on A file

const U64 not_h_file = 9187201950435737471ULL; // not h file constant - all zeros on H

const U64 not_hg_file = 4557430888798830399ULL; // not hg file constant - all zeros on HG file

const U64 not_ab_file = 18229723555195321596ULL; // not ab file constant - all zeros on AB file

//  Relevant occupancy bit count for every square on board
//  How many squares are attacked by a piece standing on the square
const int bishop_relevant_bits[64] = {
    6, 5, 5, 5, 5, 5, 5, 6,
    5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 7, 7, 7, 7, 5, 5,
    5, 5, 7, 9, 9, 7, 5, 5,
    5, 5, 7, 9, 9, 7, 5, 5,
    5, 5, 7, 7, 7, 7, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5,
    6, 5, 5, 5, 5, 5, 5, 6
};
const int rook_relevant_bits[64] = {
    12, 11, 11, 11, 11, 11, 11, 12,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    12, 11, 11, 11, 11, 11, 11, 12
};

//  Pawn attack table [side][square]
U64 pawnAttacks[2][64];

//  Knight attack table [side][square]
U64 knightAttacks[64];

//  Knight attack table [side][square]
U64 kingAttacks[64];

// Bishop attacks mask
U64 bishopAttackMasks[64];

// Rook attacks mask
U64 rookAttackMasks[64];

// Bishop attacks table [square][occupancies]
U64 bishopAttacks[64][512];

// Rook attacks table [square][occupancies]
U64 rookAttacks[64][4096];

U64 getPawnAttacks(int side, int square) {
    // Piece bitboard
    U64 pieceBitboard = 0ULL;

    // Result attack bitboard
    U64 attacks = 0ULL;

    // Set piece on board
    set_bit(pieceBitboard, square);

    // white pawns
    if (!side)
    {
        if ((pieceBitboard >> 7) & not_a_file)
        {
            attacks |= (pieceBitboard >> 7);
        }
        if ((pieceBitboard >> 9) & not_h_file)
        {
            attacks |= (pieceBitboard >> 9);
        }
    } else // black pawns
    {
        if ((pieceBitboard << 7) & not_h_file)
        {
            attacks |= (pieceBitboard << 7);
        }
        if ((pieceBitboard << 9) & not_a_file)
        {
            attacks |= (pieceBitboard << 9);
        }
    }
    return attacks;
}

U64 getKnightAttacks(int square) {
    // Piece bitboard
    U64 pieceBitboard = 0ULL;

    // Result attack bitboard
    U64 attacks = 0ULL;

    // Set piece on board
    set_bit(pieceBitboard, square);
    // 17,15  10,6 - Offsets for attacked squares, shift other way for other side
    if ((pieceBitboard >> 17) & not_h_file) attacks |= (pieceBitboard >> 17);
    if ((pieceBitboard >> 15) & not_a_file) attacks |= (pieceBitboard >> 15);
    if ((pieceBitboard >> 10) & not_hg_file) attacks |= (pieceBitboard >> 10);
    if ((pieceBitboard >> 6) & not_ab_file) attacks |= (pieceBitboard >> 6);

    if ((pieceBitboard << 17) & not_a_file) attacks |= (pieceBitboard << 17);
    if ((pieceBitboard << 15) & not_h_file) attacks |= (pieceBitboard << 15);
    if ((pieceBitboard << 10) & not_ab_file) attacks |= (pieceBitboard << 10);
    if ((pieceBitboard << 6) & not_hg_file) attacks |= (pieceBitboard << 6);


    return attacks;
}

U64 getKingAttacks(int square) {
    // Piece bitboard
    U64 pieceBitboard = 0ULL;

    // Result attack bitboard
    U64 attacks = 0ULL;

    // Set piece on board
    set_bit(pieceBitboard, square);

    if (pieceBitboard >> 8) attacks |= pieceBitboard >> 8;
    if ((pieceBitboard >> 9) & not_h_file) attacks |= pieceBitboard >> 9;
    if ((pieceBitboard >> 7) & not_a_file) attacks |= pieceBitboard >> 7;
    if ((pieceBitboard >> 1) & not_h_file) attacks |= pieceBitboard >> 1;

    if (pieceBitboard << 8) attacks |= pieceBitboard << 8;
    if ((pieceBitboard << 9) & not_a_file) attacks |= pieceBitboard << 9;
    if ((pieceBitboard << 7) & not_h_file) attacks |= pieceBitboard << 7;
    if ((pieceBitboard << 1) & not_a_file) attacks |= pieceBitboard << 1;

    return attacks;
}

U64 maskBishopAttacks(int square) {
    // Result attack bitboard
    U64 attacks = 0ULL;


    int rank, file;

    int targetRank = square / 8;
    int targetFile = square % 8;

    // mask relevant bishop occupancy squares - four diagonals
    for (rank = targetRank + 1, file = targetFile + 1; rank <= 6 && file <= 6; rank++, file++)
    {
        set_bit(attacks, rank * 8 + file);
    }
    for (rank = targetRank - 1, file = targetFile - 1; rank > 0 && file > 0; rank--, file--)
    {
        set_bit(attacks, rank * 8 + file);
    }
    for (rank = targetRank - 1, file = targetFile + 1; rank > 0 && file <= 6; rank--, file++)
    {
        set_bit(attacks, rank * 8 + file);
    }
    for (rank = targetRank + 1, file = targetFile - 1; rank <= 6 && file > 0; rank++, file--)
    {
        set_bit(attacks, rank * 8 + file);
    }


    return attacks;
}

U64 maskRookAttacks(int square) {
    // Result attack bitboard
    U64 attacks = 0ULL;


    int rank, file;

    int targetRank = square / 8; // Rank = row
    int targetFile = square % 8; // File = column

    // mask relevant bishop occupancy squares - four diagonals
    for (rank = targetRank + 1; rank <= 6; rank++)
    {
        set_bit(attacks, rank * 8 + targetFile);
    }
    for (rank = targetRank - 1; rank > 0; rank--)
    {
        set_bit(attacks, rank * 8 + targetFile);
    }
    for (file = targetFile + 1; file <= 6; file++)
    {
        set_bit(attacks, targetRank * 8 + file);
    }
    for (file = targetFile - 1; file > 0; file--)
    {
        set_bit(attacks, targetRank * 8 + file);
    }


    return attacks;
}

U64 getBishopAttacksOnTheFly(int square, U64 blockingBits) {
    // Result attack bitboard
    U64 attacks = 0ULL;


    int rank, file;

    int targetRank = square / 8;
    int targetFile = square % 8;

    // mask relevant bishop occupancy squares - four diagonals
    for (rank = targetRank + 1, file = targetFile + 1; rank <= 7 && file <= 7; rank++, file++)
    {
        set_bit(attacks, rank * 8 + file);
        if ((1ULL << (rank * 8 + file)) & blockingBits) break;
    }
    for (rank = targetRank - 1, file = targetFile - 1; rank >= 0 && file >= 0; rank--, file--)
    {
        set_bit(attacks, rank * 8 + file);
        if ((1ULL << (rank * 8 + file)) & blockingBits) break;
    }
    for (rank = targetRank - 1, file = targetFile + 1; rank >= 0 && file <= 7; rank--, file++)
    {
        set_bit(attacks, rank * 8 + file);
        if ((1ULL << (rank * 8 + file)) & blockingBits) break;
    }
    for (rank = targetRank + 1, file = targetFile - 1; rank <= 7 && file >= 0; rank++, file--)
    {
        set_bit(attacks, rank * 8 + file);
        if ((1ULL << (rank * 8 + file)) & blockingBits) break;
    }


    return attacks;
}

U64 getRookAttacksOnTheFly(int square, U64 blockingBits) {
    // Result attack bitboard
    U64 attacks = 0ULL;


    int rank, file;

    int targetRank = square / 8; // Rank = row
    int targetFile = square % 8; // File = column

    // mask relevant bishop occupancy squares - four diagonals
    for (rank = targetRank + 1; rank <= 7; rank++)
    {
        set_bit(attacks, rank * 8 + targetFile);
        if ((1ULL << (rank * 8 + targetFile)) & blockingBits) break;
    }
    for (rank = targetRank - 1; rank >= 0; rank--)
    {
        set_bit(attacks, rank * 8 + targetFile);
        if ((1ULL << (rank * 8 + targetFile)) & blockingBits) break;
    }
    for (file = targetFile + 1; file <= 7; file++)
    {
        set_bit(attacks, targetRank * 8 + file);
        if ((1ULL << (targetRank * 8 + file)) & blockingBits) break;
    }
    for (file = targetFile - 1; file >= 0; file--)
    {
        set_bit(attacks, targetRank * 8 + file);
        if ((1ULL << (targetRank * 8 + file)) & blockingBits) break;
    }


    return attacks;
}


// set occupancies
U64 setOccupancy(int index, int bitsInMask, U64 attackMask) {
    // occupancy map
    U64 occupancy = 0ULL;

    // loop over the range of bits within attack mask
    for (int count = 0; count < bitsInMask; count++)
    {
        // get LS1B index of attacks mask
        int square = getLSBIndex(attackMask);

        // pop LS1B in attack map
        pop_bit(attackMask, square);

        // make sure occupancy is on board
        if (index & (1 << count))
            // populate occupancy map
            occupancy |= (1ULL << square);
    }

    // return occupancy map
    return occupancy;
}

// my
void initSliderAttacks(int bishop) {
    // Init bishop and rook masks
    for (int square = 0; square < 64; square++)
    {
        bishopAttackMasks[square] = maskBishopAttacks(square);
        rookAttackMasks[square] = maskRookAttacks(square);

        // Init current mask
        U64 attackMask = bishop ? bishopAttackMasks[square] : rookAttackMasks[square];

        // Init relevant occupancy bit count
        int relevantBitCount = countBits(attackMask);

        // Init occupancy indices
        int occupancyIndex = (1 << relevantBitCount);

        for (int index = 0; index < occupancyIndex; index++)
        {
            if (bishop)
            {
                // occupancy variation
                U64 occupancy = setOccupancy(index, relevantBitCount, attackMask);
                int magicIndex = (occupancy * bishop_magic_numbers[square]) >> (64 - bishop_relevant_bits[square]);

                // init bishop attacks
                bishopAttacks[square][magicIndex] = getBishopAttacksOnTheFly(square, occupancy);
            } else
            {
                // occupancy variation
                U64 occupancy = setOccupancy(index, relevantBitCount, attackMask);
                int magicIndex = (occupancy * rook_magic_numbers[square]) >> (64 - rook_relevant_bits[square]);

                // init bishop attacks
                rookAttacks[square][magicIndex] = getRookAttacksOnTheFly(square, occupancy);
            }
        }
    }
}


void initLeaperAttacks() {
    for (int square = 0; square < 64; square++)
    {
        pawnAttacks[white][square] = getPawnAttacks(white, square);
        pawnAttacks[black][square] = getPawnAttacks(black, square);

        knightAttacks[square] = getKnightAttacks(square);
        kingAttacks[square] = getKingAttacks(square);
    }
}

// get bishop attacks
static inline U64 getBishopAttacks(int square, U64 occupancy) {
    // get bishop attacks assuming current board occupancy
    occupancy &= bishopAttackMasks[square];
    occupancy *= bishop_magic_numbers[square];
    occupancy >>= 64 - bishop_relevant_bits[square];

    // return bishop attacks
    return bishopAttacks[square][occupancy];
}

// get rook attacks
static inline U64 getRookAttacks(int square, U64 occupancy) {
    // get bishop attacks assuming current board occupancy
    occupancy &= rookAttackMasks[square];
    occupancy *= rook_magic_numbers[square];
    occupancy >>= 64 - rook_relevant_bits[square];

    // return rook attacks
    return rookAttacks[square][occupancy];
}

static inline U64 getQueenAttacks(int square, U64 occupancy) {
    // results for combining
    U64 queenAttacks = 0ULL;

    // bishop occupancies
    U64 bishopOccupancy = occupancy;
    U64 rookOccupancy = occupancy;

    // get bishop attacks assuming current board occupancy
    bishopOccupancy &= bishopAttackMasks[square];
    bishopOccupancy *= bishop_magic_numbers[square];
    bishopOccupancy >>= 64 - bishop_relevant_bits[square];

    queenAttacks = bishopAttacks[square][bishopOccupancy];

    // get rook attacks assuming current board occupancy
    rookOccupancy &= rookAttackMasks[square];
    rookOccupancy *= rook_magic_numbers[square];
    rookOccupancy >>= 64 - rook_relevant_bits[square];

    queenAttacks |= rookAttacks[square][rookOccupancy];
    // return queen attacks
    return queenAttacks;
}

/********************************************
 *                 MAGIC NUMBERS            *
 ********************************************/

static inline U64 findMagicNumber(int square, int relevantBits, int bishop) {
    // bishop = true/false
    // Initialize occupancies
    U64 occupancies[4096];

    // Initialize attack tables
    U64 attacks[4096];

    // Initialize used attacks
    U64 usedAttacks[4096];

    // Initialize attack mask for current piece
    U64 attackMask = bishop ? maskBishopAttacks(square) : maskRookAttacks(square);

    // Initialize occupancy indices
    int occupancy_indices = 1 << relevantBits;

    // Loop over occupancy indices
    for (int index = 0; index < occupancy_indices; ++index)
    {
        // Init occupancies
        occupancies[index] = setOccupancy(index, relevantBits, attackMask);
        attacks[index] = bishop
                             ? getBishopAttacksOnTheFly(square, occupancies[index])
                             : getRookAttacksOnTheFly(square, occupancies[index]);
    }

    // Test magic numbers loop
    for (int randomCount = 0; randomCount < 1000000000; randomCount++)
    {
        // Generate magic number candidate
        U64 magicNumber = generateMagicNumberCandidate();

        // skip inappropriate magic numbers
        if (countBits((attackMask * magicNumber) & 0xFF00000000000000) < 6) continue;
        memset(usedAttacks, 0ULL, sizeof(usedAttacks));

        // Initialize index and fail flag
        int index, fail;

        // Test magic index loop
        for (index = 0, fail = 0; !fail && index < occupancy_indices; index++)
        {
            // init magic index
            int magicIndex = (int) ((occupancies[index] * magicNumber) >> (64 - relevantBits));

            // if magic index works
            if (usedAttacks[magicIndex] == 0ULL)
            {
                // Initialize used attacks
                usedAttacks[magicIndex] = attacks[index];
            } else if (usedAttacks[magicIndex] != attacks[index])
            {
                // magic index doesnt work

                fail = 1;
                break;
            }
        }
        if (!fail)
        {
            return magicNumber;
        }
    }
    return 0ULL;
}

/**********************************
                BOARD
    Definitions for bitboards
    needed to represent game state
 **********************************/


U64 bitboards[12];

U64 occupancies[3];

int side = -1;

int enpassant = no_sq;

int castle;

void printBoard() {
    printf("\n");
    for (int rank = 0; rank < 8; rank++)
    {
        printf(" %d  ", 8 - rank);
        for (int file = 0; file < 8; file++)
        {
            int square = rank * 8 + file;
            int piece = -1;
            for (int bitboardPiece = P; bitboardPiece <= k; bitboardPiece++)
            {
                if (get_bit(bitboards[bitboardPiece], square))
                {
                    piece = bitboardPiece;
                }
            }
            printf(" %c", (piece == -1) ? '.' : ascii_pieces[piece]);
        }
        printf("\n");
    }
    printf("\n");
    printf("     a b c d e f g h\n");
    printf("     Side = %s\n", !side ? "white" : "black");
    printf("     EnPas square = %s\n", enpassant != no_sq ? square_to_coordinate[enpassant] : "none");
    printf("     Castling = %c%c%c%c\n",
           (castle & wk) ? 'K' : '-',
           (castle & wq) ? 'Q' : '-',
           (castle & bk) ? 'k' : '-',
           (castle & bq) ? 'q' : '-');

    // for(int piece = P; piece <= k; piece++)
    // {
    //     printf("\nBitboard for : %c",ascii_pieces[piece]);
    //     printBitBoard(bitboards[piece],-1);
    // }
}

// is square attacked by given side
static inline int isSquareAttacked(int square, int side) {
    if ((side == white) && (pawnAttacks[black][square] & bitboards[P]))
    {
        return 1;
    }
    if ((side == black) && (pawnAttacks[white][square] & bitboards[p]))
    {
        return 1;
    }

    if (knightAttacks[square] & ((side == white) ? bitboards[N] : bitboards[n]))
    {
        return 1;
    }
    if (kingAttacks[square] & ((side == white) ? bitboards[K] : bitboards[k]))
    {
        return 1;
    }

    if (getBishopAttacks(square, occupancies[both]) & ((side == white) ? bitboards[B] : bitboards[b]))
    {
        return 1;
    }

    if (getRookAttacks(square, occupancies[both]) & ((side == white) ? bitboards[R] : bitboards[r]))
    {
        return 1;
    }

    if (getQueenAttacks(square, occupancies[both]) & ((side == white) ? bitboards[Q] : bitboards[q]))
    {
        return 1;
    }
    return 0;
}

static inline void generateMoves() {
    int sourceSquare, targetSquare;

    // current piece bitboard copy
    U64 bitboard, attacks;
    for (int piece = P + (6 * side); piece <= k - (6 * (1 - side)); piece++)
    // If side is black, start at index 6, else 0 and end at 12 - 6
    {
        //printf("Moves for - %c\n", ascii_pieces[piece]);
        // init piece bitboard copy
        bitboard = bitboards[piece];


        if (piece == P) // White pawns
        {
            while (bitboard)
            {
                sourceSquare = getLSBIndex(bitboard);
                targetSquare = sourceSquare - 8;

                if (!(targetSquare < a8) && !get_bit(occupancies[both], targetSquare))
                {
                    // pawn promotion
                    if (sourceSquare >= a7 && sourceSquare <= h7)
                    {
                        // Add promotion to move list
                        printf("%s%sq pawn promotion\n", square_to_coordinate[sourceSquare],
                               square_to_coordinate[targetSquare]);
                        printf("%s%sr pawn promotion\n", square_to_coordinate[sourceSquare],
                               square_to_coordinate[targetSquare]);
                        printf("%s%sb pawn promotion\n", square_to_coordinate[sourceSquare],
                               square_to_coordinate[targetSquare]);
                        printf("%s%sn pawn promotion\n", square_to_coordinate[sourceSquare],
                               square_to_coordinate[targetSquare]);
                    }
                    // double pawn push
                    if ((sourceSquare >= a2 && sourceSquare <= h2) && (!
                            get_bit(occupancies[both], targetSquare - 8)))
                    {
                        // add double pawn push
                        printf("%s%s pawn double push\n", square_to_coordinate[sourceSquare],
                               square_to_coordinate[targetSquare - 8]);
                    }
                    printf("%s%s pawn single\n", square_to_coordinate[sourceSquare],
                           square_to_coordinate[targetSquare]);
                    // single pawn move
                }
                // init pawn attack bitboard
                attacks = pawnAttacks[side][sourceSquare] & occupancies[black];
                while (attacks)
                {
                    targetSquare = getLSBIndex(attacks);
                    // pawn promotion while attacking
                    if (sourceSquare >= a7 && sourceSquare <= h7)
                    {
                        // Add promotion to move list
                        printf("%s%sq+ pawn Capture promotion\n", square_to_coordinate[sourceSquare],
                               square_to_coordinate[targetSquare]);
                        printf("%s%sr+ pawn Capture promotion\n", square_to_coordinate[sourceSquare],
                               square_to_coordinate[targetSquare]);
                        printf("%s%sb+ pawn Capture promotion\n", square_to_coordinate[sourceSquare],
                               square_to_coordinate[targetSquare]);
                        printf("%s%sn+ pawn Capture promotion\n", square_to_coordinate[sourceSquare],
                               square_to_coordinate[targetSquare]);
                    } else
                    {
                        printf("%s%s+ pawn capture\n", square_to_coordinate[sourceSquare],
                               square_to_coordinate[targetSquare]);
                    }
                    pop_bit(attacks, targetSquare);
                }
                if (enpassant != no_sq)
                {
                    // handle en passant google it
                    U64 enPassantAttacks = pawnAttacks[side][sourceSquare] & (1ULL << enpassant);
                    if (enPassantAttacks)
                    {
                        // init enpassant capture target
                        targetSquare = getLSBIndex(enPassantAttacks);
                        printf("en passant: %s%s\n", square_to_coordinate[sourceSquare],
                               square_to_coordinate[targetSquare]);
                    }
                }
                // Final pop from piece bitboard copy
                pop_bit(bitboard, sourceSquare);
            }
        }

        if (piece == K) // White king
        {
            // kingside castling is available
            if (castle & wk)
            {
                // make sure path is clear
                if (!get_bit(occupancies[both], g1) && !get_bit(occupancies[both], f1))
                {
                    // make sure it doesnt put king in check
                    if (!isSquareAttacked(e1, black) && !isSquareAttacked(f1, black))
                    {
                        printf("e1g1 castle\n");
                    }
                }
            }
            // queenside castle
            if (castle & wq)
            {
                // make sure path is clear queenside
                if (!get_bit(occupancies[both], d1) && !get_bit(occupancies[both], c1) && !get_bit(
                        occupancies[both], b1))
                {
                    // make sure it doesnt put king in check
                    if (!isSquareAttacked(e1, black) && !isSquareAttacked(d1, black))
                    {
                        printf("e1c1 castle\n");
                    }
                }
            }
        }

        if (piece == p) // Black pawns
        {
            while (bitboard)
            {
                sourceSquare = getLSBIndex(bitboard);
                targetSquare = sourceSquare + 8;

                if (!(targetSquare > h1) && !get_bit(occupancies[both], targetSquare))
                {
                    // pawn promotion
                    if (sourceSquare >= a2 && sourceSquare <= h2)
                    {
                        // Add promotion to move list
                        printf("%s%sq pawn promotion\n", square_to_coordinate[sourceSquare],
                               square_to_coordinate[targetSquare]);
                        printf("%s%sr pawn promotion\n", square_to_coordinate[sourceSquare],
                               square_to_coordinate[targetSquare]);
                        printf("%s%sb pawn promotion\n", square_to_coordinate[sourceSquare],
                               square_to_coordinate[targetSquare]);
                        printf("%s%sn pawn promotion\n", square_to_coordinate[sourceSquare],
                               square_to_coordinate[targetSquare]);
                    } else
                    {
                        // double pawn push
                        if ((sourceSquare >= a7 && sourceSquare <= h7) && (!
                                get_bit(occupancies[both], targetSquare + 8)))
                        {
                            // add double pawn push
                            printf("%s%s  pawn double push\n", square_to_coordinate[sourceSquare],
                                   square_to_coordinate[targetSquare + 8]);
                        }
                        printf("%s%s  pawn move\n", square_to_coordinate[sourceSquare],
                               square_to_coordinate[targetSquare]);
                        // single pawn move
                    }
                }
                // init pawn attack bitboard
                attacks = pawnAttacks[side][sourceSquare] & occupancies[white];
                while (attacks)
                {
                    targetSquare = getLSBIndex(attacks);
                    if (sourceSquare >= a2 && sourceSquare <= h2)
                    {
                        // Add promotion to move list
                        printf("%s%sq pawn Capture promotion\n", square_to_coordinate[sourceSquare],
                               square_to_coordinate[targetSquare]);
                        printf("%s%sq pawn Capture promotion\n", square_to_coordinate[sourceSquare],
                               square_to_coordinate[targetSquare]);
                        printf("%s%sq pawn Capture promotion\n", square_to_coordinate[sourceSquare],
                               square_to_coordinate[targetSquare]);
                        printf("%s%sq pawn Capture promotion\n", square_to_coordinate[sourceSquare],
                               square_to_coordinate[targetSquare]);
                    } else
                    {
                        printf("%s%s+ pawn capture\n", square_to_coordinate[sourceSquare],
                               square_to_coordinate[targetSquare]);
                    }
                    pop_bit(attacks, targetSquare);
                }
                if (enpassant != no_sq)
                {
                    // handle en passant google it
                    U64 enPassantAttacks = pawnAttacks[side][sourceSquare] & (1ULL << enpassant);
                    if (enPassantAttacks)
                    {
                        // init enpassant capture target
                        targetSquare = getLSBIndex(enPassantAttacks);
                        printf("en passant: %s%s\n", square_to_coordinate[sourceSquare],
                               square_to_coordinate[targetSquare]);
                    }
                }
                // final pop
                pop_bit(bitboard, sourceSquare);
            }
        }
        if (piece == k) // Black king
        {
            // kingside castling is available
            if (castle & bk)
            {
                // make sure path is clear
                if (!get_bit(occupancies[both], g8) && !get_bit(occupancies[both], f8))
                {
                    // make sure it doesnt put king in check
                    if (!isSquareAttacked(e8, white) && !isSquareAttacked(f8, white))
                    {
                        printf("e8g8c castle\n");
                    }
                }
            }
            // queenside castle
            if (castle & bq)
            {
                // make sure path is clear queenside
                if (!get_bit(occupancies[both], d8) && !get_bit(occupancies[both], c8) && !get_bit(
                        occupancies[both], b8))
                {
                    // make sure it doesnt put king in check
                    if (!isSquareAttacked(e8, white) && !isSquareAttacked(d8, white))
                    {
                        printf("e1c1c castle\n");
                    }
                }
            }
        }
        if (piece == N || piece == n)
        {
            while (bitboard)
            {
                // init source square
                sourceSquare = getLSBIndex(bitboard);

                attacks = knightAttacks[sourceSquare] & ~occupancies[side];

                // loop over target squares
                while (attacks)
                {
                    targetSquare = getLSBIndex(attacks);
                    if (get_bit(occupancies[1 - side], targetSquare))
                    {
                        printf("N%s%s+ horse capture\n", square_to_coordinate[sourceSquare],
                               square_to_coordinate[targetSquare]);
                    } else
                    {
                        printf("N%s%s  horse move \n", square_to_coordinate[sourceSquare],
                               square_to_coordinate[targetSquare]);
                    }
                    pop_bit(attacks, targetSquare);
                }


                pop_bit(bitboard, sourceSquare);
            }
        }

        if (piece == B || piece == b)
        {
            while (bitboard)
            {
                // init source square
                sourceSquare = getLSBIndex(bitboard);

                attacks = getBishopAttacks(sourceSquare,occupancies[both]) & ~occupancies[side];

                // loop over target squares
                while (attacks)
                {
                    targetSquare = getLSBIndex(attacks);
                    if (get_bit(occupancies[1 - side], targetSquare))
                    {
                        printf("B%s%s+ bishop capture\n", square_to_coordinate[sourceSquare],
                               square_to_coordinate[targetSquare]);
                    } else
                    {
                        printf("B%s%s  bishop move \n", square_to_coordinate[sourceSquare],
                               square_to_coordinate[targetSquare]);
                    }
                    pop_bit(attacks, targetSquare);
                }


                pop_bit(bitboard, sourceSquare);
            }
        }
        if (piece == R || piece == r)
        {
            while (bitboard)
            {
                // init source square
                sourceSquare = getLSBIndex(bitboard);

                attacks = getRookAttacks(sourceSquare,occupancies[both]) & ~occupancies[side];

                // loop over target squares
                while (attacks)
                {
                    targetSquare = getLSBIndex(attacks);
                    if (get_bit(occupancies[1 - side], targetSquare))
                    {
                        printf("R%s%s+ rook capture\n", square_to_coordinate[sourceSquare],
                               square_to_coordinate[targetSquare]);
                    } else
                    {
                        printf("R%s%s  rook move \n", square_to_coordinate[sourceSquare],
                               square_to_coordinate[targetSquare]);
                    }
                    pop_bit(attacks, targetSquare);
                }


                pop_bit(bitboard, sourceSquare);
            }
        }
        if (piece == Q || piece == q)
        {
            while (bitboard)
            {
                // init source square
                sourceSquare = getLSBIndex(bitboard);

                attacks = getQueenAttacks(sourceSquare,occupancies[both]) & ~occupancies[side];

                // loop over target squares
                while (attacks)
                {
                    targetSquare = getLSBIndex(attacks);
                    if (get_bit(occupancies[1 - side], targetSquare))
                    {
                        printf("Q%s%s+ queen capture\n", square_to_coordinate[sourceSquare],
                               square_to_coordinate[targetSquare]);
                    } else
                    {
                        printf("Q%s%s  queen move \n", square_to_coordinate[sourceSquare],
                               square_to_coordinate[targetSquare]);
                    }
                    pop_bit(attacks, targetSquare);
                }


                pop_bit(bitboard, sourceSquare);
            }
        }
        if (piece == K || piece == k)
        {
            while (bitboard)
            {
                // init source square
                sourceSquare = getLSBIndex(bitboard);

                attacks = kingAttacks[sourceSquare] & ~occupancies[side];

                // loop over target squares
                while (attacks)
                {
                    targetSquare = getLSBIndex(attacks);
                    if (get_bit(occupancies[1 - side], targetSquare))
                    {
                        printf("K%s%s+ king capture\n", square_to_coordinate[sourceSquare],
                               square_to_coordinate[targetSquare]);
                    } else
                    {
                        printf("K%s%s  king move \n", square_to_coordinate[sourceSquare],
                               square_to_coordinate[targetSquare]);
                    }
                    pop_bit(attacks, targetSquare);
                }


                pop_bit(bitboard, sourceSquare);
            }
        }
    }
}


void parseFENString(char *FEN) {
    // Reset board position and occupancies (set to 0ULL)
    memset(bitboards, 0ULL, sizeof(bitboards));
    memset(occupancies, 0ULL, sizeof(occupancies));
    // Reset gameState
    side = 0;
    enpassant = no_sq;
    castle = 0;
    /*
    start_position "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1 "
    tricky_position "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1 "
    */
    // Loop over board ranks and files
    for (int rank = 0; rank < 8; rank++)
    {
        for (int file = 0; file < 8; file++)
        {
            int square = rank * 8 + file;
            int offset = 1;

            // match ascii characters
            if ((*FEN >= 'a' && *FEN <= 'z') || (*FEN >= 'A' && *FEN <= 'Z'))
            {
                int piece = char_pieces[*FEN];

                set_bit(bitboards[piece], square);
                *FEN++;
            }
            if (*FEN >= '0' && *FEN <= '9')
            {
                // offset fen
                offset = *FEN - '0';

                file += offset;
                if (*(FEN - 1) == '/')
                {
                    file--;
                }
                *FEN++;
            }

            if (*FEN == '/')
            {
                *FEN++;
            }
        }
    }
    // go to parse game state information
    *FEN++;

    // parse side to move
    (*FEN == 'w') ? (side = white) : (side = black);
    *FEN++;
    *FEN++;

    // parse castling info
    while (*FEN != ' ')
    {
        switch (*FEN)
        {
            case 'K': castle |= wk;
                break;
            case 'Q': castle |= wq;
                break;
            case 'k': castle |= bk;
                break;
            case 'q': castle |= bq;
                break;
            default: break;
        }
        *FEN++;
    }
    *FEN++;

    // parse enpassant square
    if (*FEN != '-')
    {
        int file = FEN[0] - 'a';
        int rank = 8 - (FEN[1] - '0');
        enpassant = rank * 8 + file;
        *FEN++;
    } else
    {
        enpassant = no_sq;
    }
    *FEN++;
    *FEN++;

    // init white occupancy
    for (int piece = P; piece <= K; piece++)
    {
        occupancies[white] |= bitboards[piece];
    }
    // init black occupancy
    for (int piece = p; piece <= k; piece++)
    {
        occupancies[black] |= bitboards[piece];
    }
    occupancies[both] = occupancies[white] | occupancies[black];
    printf("\n%s", FEN);
}

void setGameStart() {
    // white to move
    side = white;
    enpassant = no_sq;
    castle = wk | wq | bk | bq;
    // white
    set_bit(bitboards[P], a2);
    set_bit(bitboards[P], b2);
    set_bit(bitboards[P], c2);
    set_bit(bitboards[P], d2);
    set_bit(bitboards[P], e2);
    set_bit(bitboards[P], f2);
    set_bit(bitboards[P], g2);
    set_bit(bitboards[P], h2);
    set_bit(bitboards[R], a1);
    set_bit(bitboards[R], h1);
    set_bit(bitboards[N], b1);
    set_bit(bitboards[N], g1);
    set_bit(bitboards[B], c1);
    set_bit(bitboards[B], f1);
    set_bit(bitboards[Q], d1);
    set_bit(bitboards[K], e1);
    // black
    set_bit(bitboards[p], a7);
    set_bit(bitboards[p], b7);
    set_bit(bitboards[p], c7);
    set_bit(bitboards[p], d7);
    set_bit(bitboards[p], e7);
    set_bit(bitboards[p], f7);
    set_bit(bitboards[p], g7);
    set_bit(bitboards[p], h7);
    set_bit(bitboards[r], a8);
    set_bit(bitboards[r], h8);
    set_bit(bitboards[n], b8);
    set_bit(bitboards[n], g8);
    set_bit(bitboards[b], c8);
    set_bit(bitboards[b], f8);
    set_bit(bitboards[q], d8);
    set_bit(bitboards[k], e8);
}

/**********************************\
              Init all
\**********************************/

void init_all() {
    //findMagicNumber();

    initLeaperAttacks();
    initSliderAttacks(bishop);
    initSliderAttacks(rook);
}


// print attacked squares given side
void printAttackedSquares(int side) {
    printf("\n");
    for (int rank = 0; rank < 8; rank++)
    {
        printf(" %d  ", 8 - rank);
        for (int file = 0; file < 8; file++)
        {
            int square = rank * 8 + file;

            printf(" %s", isSquareAttacked(square, side) ? "x" : ".");
        }
        printf("\n");
    }
    printf("\n");
    printf("     a b c d e f g h\n");
}

/********************************************
 *                 MAIN DRIVER              *
 ********************************************/
// FEN debug positions
#define empty_board "8/8/8/8/8/8/8/8 w - - "
#define start_position "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1 "
#define tricky_position "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1 "
#define killer_position "rnbqkb1r/pp1p1pPp/8/2p1pP2/1P1P4/3P3P/P1P1P3/RNBQKBNR w KQkq e6 0 1"
#define cmk_position "r2q1rk1/ppp2ppp/2n1bn2/2b1p3/3pP3/3P1NPP/PPP1NPB1/R1BQ1RK1 b - - 0 9 "

int main(void) {
    init_all();
    parseFENString("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1 ");


    printBoard();
    generateMoves();
    return 0;
}
