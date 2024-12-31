// Headers
#include "magic.h"
#include "utils.h"
#include <string.h>
#include <windows.h>
#include <stdio.h>
// define bitboard data type
#define U64 unsigned long long

// enumerate board squares
enum {
    a8,
    b8,
    c8,
    d8,
    e8,
    f8,
    g8,
    h8,
    a7,
    b7,
    c7,
    d7,
    e7,
    f7,
    g7,
    h7,
    a6,
    b6,
    c6,
    d6,
    e6,
    f6,
    g6,
    h6,
    a5,
    b5,
    c5,
    d5,
    e5,
    f5,
    g5,
    h5,
    a4,
    b4,
    c4,
    d4,
    e4,
    f4,
    g4,
    h4,
    a3,
    b3,
    c3,
    d3,
    e3,
    f3,
    g3,
    h3,
    a2,
    b2,
    c2,
    d2,
    e2,
    f2,
    g2,
    h2,
    a1,
    b1,
    c1,
    d1,
    e1,
    f1,
    g1,
    h1,
    no_sq // Invalid square
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

enum {
    P,
    N,
    B,
    R,
    Q,
    K,
    p,
    n,
    b,
    r,
    q,
    k
}; // Uppercase white pieces

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
    if ((pieceBitboard >> 17) & not_h_file)
        attacks |= (pieceBitboard >> 17);
    if ((pieceBitboard >> 15) & not_a_file)
        attacks |= (pieceBitboard >> 15);
    if ((pieceBitboard >> 10) & not_hg_file)
        attacks |= (pieceBitboard >> 10);
    if ((pieceBitboard >> 6) & not_ab_file)
        attacks |= (pieceBitboard >> 6);

    if ((pieceBitboard << 17) & not_a_file)
        attacks |= (pieceBitboard << 17);
    if ((pieceBitboard << 15) & not_h_file)
        attacks |= (pieceBitboard << 15);
    if ((pieceBitboard << 10) & not_ab_file)
        attacks |= (pieceBitboard << 10);
    if ((pieceBitboard << 6) & not_hg_file)
        attacks |= (pieceBitboard << 6);

    return attacks;
}

U64 getKingAttacks(int square) {
    // Piece bitboard
    U64 pieceBitboard = 0ULL;

    // Result attack bitboard
    U64 attacks = 0ULL;

    // Set piece on board
    set_bit(pieceBitboard, square);

    if (pieceBitboard >> 8)
        attacks |= pieceBitboard >> 8;
    if ((pieceBitboard >> 9) & not_h_file)
        attacks |= pieceBitboard >> 9;
    if ((pieceBitboard >> 7) & not_a_file)
        attacks |= pieceBitboard >> 7;
    if ((pieceBitboard >> 1) & not_h_file)
        attacks |= pieceBitboard >> 1;

    if (pieceBitboard << 8)
        attacks |= pieceBitboard << 8;
    if ((pieceBitboard << 9) & not_a_file)
        attacks |= pieceBitboard << 9;
    if ((pieceBitboard << 7) & not_h_file)
        attacks |= pieceBitboard << 7;
    if ((pieceBitboard << 1) & not_a_file)
        attacks |= pieceBitboard << 1;

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
        if ((1ULL << (rank * 8 + file)) & blockingBits)
            break;
    }
    for (rank = targetRank - 1, file = targetFile - 1; rank >= 0 && file >= 0; rank--, file--)
    {
        set_bit(attacks, rank * 8 + file);
        if ((1ULL << (rank * 8 + file)) & blockingBits)
            break;
    }
    for (rank = targetRank - 1, file = targetFile + 1; rank >= 0 && file <= 7; rank--, file++)
    {
        set_bit(attacks, rank * 8 + file);
        if ((1ULL << (rank * 8 + file)) & blockingBits)
            break;
    }
    for (rank = targetRank + 1, file = targetFile - 1; rank <= 7 && file >= 0; rank++, file--)
    {
        set_bit(attacks, rank * 8 + file);
        if ((1ULL << (rank * 8 + file)) & blockingBits)
            break;
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
        if ((1ULL << (rank * 8 + targetFile)) & blockingBits)
            break;
    }
    for (rank = targetRank - 1; rank >= 0; rank--)
    {
        set_bit(attacks, rank * 8 + targetFile);
        if ((1ULL << (rank * 8 + targetFile)) & blockingBits)
            break;
    }
    for (file = targetFile + 1; file <= 7; file++)
    {
        set_bit(attacks, targetRank * 8 + file);
        if ((1ULL << (targetRank * 8 + file)) & blockingBits)
            break;
    }
    for (file = targetFile - 1; file >= 0; file--)
    {
        set_bit(attacks, targetRank * 8 + file);
        if ((1ULL << (targetRank * 8 + file)) & blockingBits)
            break;
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
        if (countBits((attackMask * magicNumber) & 0xFF00000000000000) < 6)
            continue;
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

/**********************************\
              MOVES
\**********************************/
/*
 * Moves encoding binary
 *
 * - 0x3f     ==== 0000 0000 0000 0000 0011 1111 Source square - move = (move | square)
 * - 0xfc0    ==== 0000 0000 0000 1111 1100 0000 Target square -  move = (move | square) << 6, getting the move back - (move & 0xfc0) >> 6
 * - 0xf000   ==== 0000 0000 1111 0000 0000 0000 Piece - 12 pieces - move = (move | piece) << 12
 * - 0xf0000  ==== 0000 1111 0000 0000 0000 0000 Promoted piece
 * - 0x100000 ==== 0001 0000 0000 0000 0000 0000 Capture flag
 * - 0x200000 ==== 0010 0000 0000 0000 0000 0000 Double push flag
 * - 0x400000 ==== 0100 0000 0000 0000 0000 0000 Enpassant capture
 * - 0x800000 ==== 1000 0000 0000 0000 0000 0000 Castling flag
 *
 *
 */
// define encode move macros
#define move_encode(source, target, piece, promoted, capture, doublePush, enpassant, castle) \
    (source) | (target << 6) | (piece) << 12 | (promoted) << 16 | (capture) << 20 | (doublePush) << 21 | (enpassant) << 22 | (castle) << 23
#define move_get_source(move) (move & 0x3f)
#define move_get_target(move) ((move & 0xfc0) >> 6)
#define move_get_piece(move) ((move & 0xf000) >> 12)
#define move_get_promoted(move) ((move & 0xf0000) >> 16)
#define move_get_capture(move) (move & 0x100000)
#define move_get_doublepush(move) (move & 0x200000)
#define move_get_enpassant(move) (move & 0x400000)
#define move_get_castling(move) (move & 0x800000)

typedef struct {
    int moves[256];
    int count;
} moves;

static inline void addMoveToMoveList(moves *moveList, int move) {
    // store move
    moveList->moves[moveList->count] = move;
    moveList->count++;
}

char promoted_pieces_c[] = {
    [Q] = 'q',
    [R] = 'r',
    [B] = 'b',
    [N] = 'n',
    [q] = 'q',
    [r] = 'r',
    [b] = 'b',
    [n] = 'n'
};

void printMove(int move) {
    // For UCI purposes
    if (move_get_promoted(move))
        printf("%s%s%c", square_to_coordinate[move_get_source(move)],
               square_to_coordinate[move_get_target(move)],
               promoted_pieces_c[move_get_promoted(move)]);
    else
        printf("%s%s", square_to_coordinate[move_get_source(move)],
               square_to_coordinate[move_get_target(move)]);
}

void printMoveList(moves *moveList) {
    if (moveList->count == 0)
    {
        printf("No moves in the move list");
        return;
    }
    printf("\n    move   piece   capture   doublePush   enpassant  castle\n");
    for (int moveIndex = 0; moveIndex < moveList->count; moveIndex++)
    {
        // initialize move
        int move = moveList->moves[moveIndex];
        printf("    %s%s%c  %c       %d         %d            %d          %d\n",
               square_to_coordinate[move_get_source(move)],
               square_to_coordinate[move_get_target(move)],
               move_get_promoted(move) ? promoted_pieces_c[move_get_promoted(move)] : ' ',
               ascii_pieces[move_get_piece(move)],
               move_get_capture(move) ? 1 : 0,
               move_get_doublepush(move) ? 1 : 0,
               move_get_enpassant(move) ? 1 : 0,
               move_get_castling(move) ? 1 : 0);
    }
    // total number of moves
    printf("Total number of moves : %d", moveList->count);
}

/**********************************
                BOARD
    Definitions for bitboards
    needed to represent game state
 **********************************/
// castling rights binary encoding

/*

    bin  dec

   0001    1  white king can castle to the king side
   0010    2  white king can castle to the queen side
   0100    4  black king can castle to the king side
   1000    8  black king can castle to the queen side

   examples

   1111       both sides an castle both directions
   1001       black king => queen side
              white king => king side

*/
// castling rights update constants
const int castling_rights[64] = {
    7, 15, 15, 15, 3, 15, 15, 11,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    13, 15, 15, 15, 12, 15, 15, 14
};

#define copy_board()                           \
    U64 bitboardsCopy[12], occupanciesCopy[3]; \
    int sideCopy, enpassantCopy, castleCopy;   \
    memcpy(bitboardsCopy, bitboards, 96);      \
    memcpy(occupanciesCopy, occupancies, 24);  \
    sideCopy = side;                           \
    enpassantCopy = enpassant;                 \
    castleCopy = castle;
#define restore_board()                       \
    memcpy(bitboards, bitboardsCopy, 96);     \
    memcpy(occupancies, occupanciesCopy, 24); \
    side = sideCopy;                          \
    enpassant = enpassantCopy;                \
    castle = castleCopy;

U64 bitboards[12];

U64 occupancies[3];

int side = -1;

int enpassant = no_sq;

int castle;

enum {
    allMoves,
    onlyCaptures
};

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

static inline int makeMove(int move, int moveFlag) {
    // quiet moves
    if (moveFlag == allMoves)
    {
        copy_board();

        int source = move_get_source(move);
        int target = move_get_target(move);
        int piece = move_get_piece(move);
        int promotedPiece = move_get_promoted(move);
        int capture = move_get_capture(move);
        int doublePush = move_get_doublepush(move);
        int enpass = move_get_enpassant(move);
        int castling = move_get_castling(move);

        // move piece
        pop_bit(bitboards[piece], source);
        set_bit(bitboards[piece], target);

        if (capture) // Handle captures
        {
            for (int enemyPieceIdx = P + ((1 - side) * 6); enemyPieceIdx <= K + ((1 - side) * 6); enemyPieceIdx++)
            {
                if (get_bit(bitboards[enemyPieceIdx], target))
                {
                    pop_bit(bitboards[enemyPieceIdx], target);
                    break;
                }
            }
        }
        if (promotedPiece) // Handle promotions
        {
            // First, remove pawn and add the piece its promoting to
            pop_bit(bitboards[P + (6 * side)], source);
            set_bit(bitboards[promotedPiece], target);
        }
        if (enpass)
        {
            (side == white) ? pop_bit(bitboards[p], target + 8) : pop_bit(bitboards[P], target - 8);

            set_bit(bitboards[piece], target);
        }
        enpassant = no_sq;
        if (doublePush)
        {
            enpassant = (side == white) ? target + 8 : target - 8;
        }

        if (castling)
        {
            switch (target)
            {
                // white castle kingside
                case g1:
                    pop_bit(bitboards[R], h1);
                    set_bit(bitboards[R], f1);
                    break;
                // white castle queenside
                case c1:
                    pop_bit(bitboards[R], a1);
                    set_bit(bitboards[R], d1);
                    break;
                // black castle kingside
                case g8:
                    pop_bit(bitboards[r], h8);
                    set_bit(bitboards[r], f8);
                    break;
                // black castle queenside
                case c8:
                    pop_bit(bitboards[r], a8);
                    set_bit(bitboards[r], d8);
                    break;
            }
        }
        // Update castling rights
        // Check square where piece is moving or targeting, if its king or rook square, update the rights
        castle &= castling_rights[source];
        castle &= castling_rights[target];

        // Reset occupancies
        memset(occupancies, 0ULL, 24);

        for (int bbPiece = P; bbPiece <= K; bbPiece++)
        {
            occupancies[white] |= bitboards[bbPiece];
            occupancies[both] |= bitboards[bbPiece];
        }
        for (int bbPiece = p; bbPiece <= k; bbPiece++)
        {
            occupancies[black] |= bitboards[bbPiece];
            occupancies[both] |= bitboards[bbPiece];
        }
        // change side
        side ^= 1;

        // make sure king is not in check
        if (isSquareAttacked((side == white) ? getLSBIndex(bitboards[k]) : getLSBIndex(bitboards[K]), side))
        {
            // move is illegal
            restore_board();

            return 0;
        } else
        {
            // move is legal
            return 1;
        }
    }
    // capture moves
    else
    {
        // make sure move is the capture
        if (move_get_capture(move))
            makeMove(move, allMoves);

            // otherwise the move is not a capture
        else
            // don't make it
            return 0;
    }
}

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

static inline void generateMoves(moves *moveList) {
    // Initialize moves
    moveList->count = 0;
    int sourceSquare, targetSquare;

    // current piece bitboard copy
    U64 bitboard, attacks;
    for (int piece = P + (6 * side); piece <= k - (6 * (1 - side)); piece++)
    // If side is black, start at index 6, else 0 and end at 12 - 6
    {
        // printf("Moves for - %c\n", ascii_pieces[piece]);
        //  init piece bitboard copy
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
                        // Add white pawn promotion to list
                        addMoveToMoveList(moveList, move_encode(sourceSquare, targetSquare, piece, Q, 0, 0, 0, 0));
                        addMoveToMoveList(moveList, move_encode(sourceSquare, targetSquare, piece, R, 0, 0, 0, 0));
                        addMoveToMoveList(moveList, move_encode(sourceSquare, targetSquare, piece, N, 0, 0, 0, 0));
                        addMoveToMoveList(moveList, move_encode(sourceSquare, targetSquare, piece, B, 0, 0, 0, 0));
                    } else
                    {
                        // double pawn push
                        if ((sourceSquare >= a2 && sourceSquare <= h2) && (!
                                get_bit(occupancies[both], targetSquare - 8)))
                        {
                            // Add white double pawn push
                            addMoveToMoveList(
                                moveList, move_encode(sourceSquare, targetSquare - 8, piece, 0, 0, 1, 0, 0));
                        }
                        // Add white single pawn move
                        addMoveToMoveList(moveList, move_encode(sourceSquare, targetSquare, piece, 0, 0, 0, 0, 0));
                    }
                }
                // init pawn attack bitboard
                attacks = pawnAttacks[side][sourceSquare] & occupancies[black];
                while (attacks)
                {
                    targetSquare = getLSBIndex(attacks);
                    // pawn promotion while attacking
                    if (sourceSquare >= a7 && sourceSquare <= h7)
                    {
                        // Add white pawn promotion while attacking
                        addMoveToMoveList(moveList, move_encode(sourceSquare, targetSquare, piece, Q, 1, 0, 0, 0));
                        addMoveToMoveList(moveList, move_encode(sourceSquare, targetSquare, piece, R, 1, 0, 0, 0));
                        addMoveToMoveList(moveList, move_encode(sourceSquare, targetSquare, piece, B, 1, 0, 0, 0));
                        addMoveToMoveList(moveList, move_encode(sourceSquare, targetSquare, piece, N, 1, 0, 0, 0));
                    } else
                    {
                        // Add white pawn normal attack
                        addMoveToMoveList(moveList, move_encode(sourceSquare, targetSquare, piece, 0, 1, 0, 0, 0));
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
                        int targetEnpassant = getLSBIndex(enPassantAttacks); // Not the enemy pawn, the one behind him
                        addMoveToMoveList(moveList, move_encode(sourceSquare, targetEnpassant, piece, 0, 1, 0, 1, 0));
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
                        // Add white king side castle to moves
                        addMoveToMoveList(moveList, move_encode(e1, g1, piece, 0, 0, 0, 0, 1));
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
                        // Add white queen side castle to moves
                        addMoveToMoveList(moveList, move_encode(e1, c1, piece, 0, 0, 0, 0, 1));
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
                        // Add black pawn promotion to move list
                        addMoveToMoveList(moveList, move_encode(sourceSquare, targetSquare, piece, q, 0, 0, 0, 0));
                        addMoveToMoveList(moveList, move_encode(sourceSquare, targetSquare, piece, r, 0, 0, 0, 0));
                        addMoveToMoveList(moveList, move_encode(sourceSquare, targetSquare, piece, n, 0, 0, 0, 0));
                        addMoveToMoveList(moveList, move_encode(sourceSquare, targetSquare, piece, b, 0, 0, 0, 0));
                    } else
                    {
                        if ((sourceSquare >= a7 && sourceSquare <= h7) && (!
                                get_bit(occupancies[both], targetSquare + 8)))
                        {
                            // Add black double push pawn
                            addMoveToMoveList(
                                moveList, move_encode(sourceSquare, targetSquare + 8, piece, 0, 0, 1, 0, 0));
                        }
                        // Single black pawn move
                        addMoveToMoveList(moveList, move_encode(sourceSquare, targetSquare, piece, 0, 0, 0, 0, 0));
                    }
                }
                // init pawn attack bitboard
                attacks = pawnAttacks[side][sourceSquare] & occupancies[white];
                while (attacks)
                {
                    targetSquare = getLSBIndex(attacks);
                    if (sourceSquare >= a2 && sourceSquare <= h2)
                    {
                        // Add black attack promotion to move list
                        addMoveToMoveList(moveList, move_encode(sourceSquare, targetSquare, piece, q, 1, 0, 0, 0));
                        addMoveToMoveList(moveList, move_encode(sourceSquare, targetSquare, piece, r, 1, 0, 0, 0));
                        addMoveToMoveList(moveList, move_encode(sourceSquare, targetSquare, piece, b, 1, 0, 0, 0));
                        addMoveToMoveList(moveList, move_encode(sourceSquare, targetSquare, piece, n, 1, 0, 0, 0));
                    } else
                    {
                        // Add black normal attack to move list
                        addMoveToMoveList(moveList, move_encode(sourceSquare, targetSquare, piece, 0, 1, 0, 0, 0));
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
                        addMoveToMoveList(moveList, move_encode(sourceSquare, targetSquare, piece, 0, 1, 0, 1, 0));
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
                        // Add black kingside castle
                        addMoveToMoveList(moveList, move_encode(e8, g8, piece, 0, 0, 0, 0, 1));
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
                        // Add black queenside castle
                        addMoveToMoveList(moveList, move_encode(e8, c8, piece, 0, 0, 0, 0, 1));
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
                        // Add horsey capture moves
                        addMoveToMoveList(moveList, move_encode(sourceSquare, targetSquare, piece, 0, 1, 0, 0, 0));
                    } else
                    {
                        // Add horsey normal noves
                        addMoveToMoveList(moveList, move_encode(sourceSquare, targetSquare, piece, 0, 0, 0, 0, 0));
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

                attacks = getBishopAttacks(sourceSquare, occupancies[both]) & ~occupancies[side];

                // loop over target squares
                while (attacks)
                {
                    targetSquare = getLSBIndex(attacks);
                    if (get_bit(occupancies[1 - side], targetSquare))
                    {
                        // Add bishop capture moves
                        addMoveToMoveList(moveList, move_encode(sourceSquare, targetSquare, piece, 0, 1, 0, 0, 0));
                    } else
                    {
                        // Add bishop normal moves
                        addMoveToMoveList(moveList, move_encode(sourceSquare, targetSquare, piece, 0, 0, 0, 0, 0));
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

                attacks = getRookAttacks(sourceSquare, occupancies[both]) & ~occupancies[side];

                // loop over target squares
                while (attacks)
                {
                    targetSquare = getLSBIndex(attacks);
                    if (get_bit(occupancies[1 - side], targetSquare))
                    {
                        // Add rook capture moves
                        addMoveToMoveList(moveList, move_encode(sourceSquare, targetSquare, piece, 0, 1, 0, 0, 0));
                    } else
                    {
                        // Add rook normal moves
                        addMoveToMoveList(moveList, move_encode(sourceSquare, targetSquare, piece, 0, 0, 0, 0, 0));
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

                attacks = getQueenAttacks(sourceSquare, occupancies[both]) & ~occupancies[side];

                // loop over target squares
                while (attacks)
                {
                    targetSquare = getLSBIndex(attacks);
                    if (get_bit(occupancies[1 - side], targetSquare))
                    {
                        // Add queen capture moves
                        addMoveToMoveList(moveList, move_encode(sourceSquare, targetSquare, piece, 0, 1, 0, 0, 0));
                    } else
                    {
                        // Add queen normal moves
                        addMoveToMoveList(moveList, move_encode(sourceSquare, targetSquare, piece, 0, 0, 0, 0, 0));
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
                        // Add king capture moves
                        addMoveToMoveList(moveList, move_encode(sourceSquare, targetSquare, piece, 0, 1, 0, 0, 0));
                    } else
                    {
                        // Add king normal moves
                        addMoveToMoveList(moveList, move_encode(sourceSquare, targetSquare, piece, 0, 0, 0, 0, 0));
                    }
                    pop_bit(attacks, targetSquare);
                }

                pop_bit(bitboard, sourceSquare);
            }
        }
    }
}

/**********************************\
              Perft stuff
\**********************************/
// leaf nodes (number of positions reached during testing)
long nodes;
// perft driver

static inline void perftDriver(int depth) {
    if (depth == 0)
    {
        // increment nodes count
        nodes++;
        return;
    }
    moves moveList[1];
    generateMoves(moveList);

    for (int moveCount = 0; moveCount < moveList->count; moveCount++)
    {
        copy_board();
        if (!makeMove(moveList->moves[moveCount], allMoves))
        {
            continue;
        }
        perftDriver(depth - 1);

        restore_board();
    }
}

// pertf test
void perftTest(int depth) {
    printf("\nPerformance test\n");
    int start = GetTickCount();
    moves moveList[1];
    generateMoves(moveList);
    int move;

    for (int moveCount = 0; moveCount < moveList->count; moveCount++)
    {
        move = moveList->moves[moveCount];
        copy_board();
        if (!makeMove(moveList->moves[moveCount], allMoves))
        {
            continue;
        }

        long cumulativeNodes = nodes;

        // call driver recursively
        perftDriver(depth - 1);

        long oldNodes = nodes - cumulativeNodes;

        restore_board();
        printMove(move);
        printf("    move: %s%s%c   nodes: %ld\n",
               square_to_coordinate[move_get_source(move_get_source(move))],
               square_to_coordinate[move_get_target(move)],
               promoted_pieces_c[move_get_promoted(move)],
               oldNodes);
    }

    printf("\n    Depth : %d\n", depth);
    printf("    Nodes : %ld\n", nodes);
    printf("    Time  : %ldms", GetTickCount() - start);
}

void parseFENString(char *FEN) {
    // Reset board position and occupancies (set to 0ULL)
    memset(bitboards, 0ULL, sizeof(bitboards));
    memset(occupancies, 0ULL, sizeof(occupancies));
    // Reset gameState
    side = 0;
    enpassant = no_sq;
    castle = 0;

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
            case 'K':
                castle |= wk;
                break;
            case 'Q':
                castle |= wq;
                break;
            case 'k':
                castle |= bk;
                break;
            case 'q':
                castle |= bq;
                break;
            default:
                break;
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
}

/**********************************\
              Init all
\**********************************/

void init_all() {
    // findMagicNumber();
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

int parseMove(char *moveString) {
    // "d4e6n"
    moves moveList[1];

    generateMoves(moveList);

    int sourceSquare = (moveString[0] - 'a') + (8 - (moveString[1] - '0')) * 8;
    int targetSquare = (moveString[2] - 'a') + (8 - (moveString[3] - '0')) * 8;

    for (int moveCount = 0; moveCount < moveList->count; moveCount++)
    {
        int move = moveList->moves[moveCount];
        if (sourceSquare == move_get_source(move) && targetSquare == move_get_target(move))
        {
            int promotedPiece = move_get_promoted(move);
            if (promotedPiece)
            {
                if ((promotedPiece == Q || promotedPiece == q) && moveString[4] == 'q')
                {
                    return move;
                } else if ((promotedPiece == R || promotedPiece == r) && moveString[4] == 'r')
                {
                    return move;
                } else if ((promotedPiece == N || promotedPiece == n) && moveString[4] == 'n')
                {
                    return move;
                } else if ((promotedPiece == B || promotedPiece == b) && moveString[4] == 'b')
                {
                    return move;
                }
                continue;
            }

            return move;
        }
    }
}

/*
    Example UCI commands to init position on chess board

    // init start position
    position startpos

    // init start position and make the moves on chess board
    position startpos moves e2e4 e7e5

    // init position from FEN string
    position fen r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1

    // init position from fen string and make moves on chess board
    position fen r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1 moves e2a6 e8g8
*/
// EVALUATION
int material_score[12] = {
    100, // white pawn score
    300, // white knight scrore
    350, // white bishop score
    500, // white rook score
    1000, // white queen score
    10000, // white king score
    -100, // black pawn score
    -300, // black knight scrore
    -350, // black bishop score
    -500, // black rook score
    -1000, // black queen score
    -10000, // black king score
};
// pawn positional score
const int pawn_score[64] =
{
    90, 90, 90, 90, 90, 90, 90, 90,
    30, 30, 30, 40, 40, 30, 30, 30,
    20, 20, 20, 30, 30, 30, 20, 20,
    10, 10, 10, 20, 20, 10, 10, 10,
    5, 5, 10, 20, 20, 5, 5, 5,
    0, 0, 0, 5, 5, 0, 0, 0,
    0, 0, 0, -10, -10, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0
};

// knight positional score
const int knight_score[64] =
{
    -5, 0, 0, 0, 0, 0, 0, -5,
    -5, 0, 0, 10, 10, 0, 0, -5,
    -5, 5, 20, 20, 20, 20, 5, -5,
    -5, 10, 20, 30, 30, 20, 10, -5,
    -5, 10, 20, 30, 30, 20, 10, -5,
    -5, 5, 20, 10, 10, 20, 5, -5,
    -5, 0, 0, 0, 0, 0, 0, -5,
    -5, -10, 0, 0, 0, 0, -10, -5
};

// bishop positional score
const int bishop_score[64] =
{
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 10, 10, 0, 0, 0,
    0, 0, 10, 20, 20, 10, 0, 0,
    0, 0, 10, 20, 20, 10, 0, 0,
    0, 10, 0, 0, 0, 0, 10, 0,
    0, 30, 0, 0, 0, 0, 30, 0,
    0, 0, -10, 0, 0, -10, 0, 0

};

// rook positional score
const int rook_score[64] =
{
    50, 50, 50, 50, 50, 50, 50, 50,
    50, 50, 50, 50, 50, 50, 50, 50,
    0, 0, 10, 20, 20, 10, 0, 0,
    0, 0, 10, 20, 20, 10, 0, 0,
    0, 0, 10, 20, 20, 10, 0, 0,
    0, 0, 10, 20, 20, 10, 0, 0,
    0, 0, 10, 20, 20, 10, 0, 0,
    0, 0, 0, 20, 20, 0, 0, 0

};

// king positional score
const int king_score[64] =
{
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 5, 5, 5, 5, 0, 0,
    0, 5, 5, 10, 10, 5, 5, 0,
    0, 5, 10, 20, 20, 10, 5, 0,
    0, 5, 10, 20, 20, 10, 5, 0,
    0, 0, 5, 10, 10, 5, 0, 0,
    0, 5, 5, -5, -5, 0, 5, 0,
    0, 0, 5, 0, -15, 0, 10, 0
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

static inline int evaluate() {
    int score = 0;

    U64 bitboard; // current pieces bitboard copy

    // init piece and square
    int piece, square;

    for (int bbPiece = P; bbPiece <= k; bbPiece++)
    {
        // init piece bitboard copy
        bitboard = bitboards[bbPiece];
        while (bitboard)
        {
            piece = bbPiece;
            square = getLSBIndex(bitboard);
            score += material_score[piece]; // material score
            switch (piece)
            {
                case P:
                    score += pawn_score[square];
                    break;
                case N:
                    score += knight_score[square];
                    break;
                case B:
                    score += bishop_score[square];
                    break;
                case R:
                    score += rook_score[square];
                    break;
                case K:
                    score += king_score[square];
                    break;
                case p: // black pieces have mirrored boards
                    score -= pawn_score[mirror_score[square]];
                    break;
                case n:
                    score -= knight_score[mirror_score[square]];
                    break;
                case b:
                    score -= bishop_score[mirror_score[square]];
                    break;
                case r:
                    score -= rook_score[mirror_score[square]];
                    break;
                case k:
                    score -= king_score[mirror_score[square]];
                    break;
                default:
                    break;
            }
            pop_bit(bitboard, square);
        }
    }

    return (side == white) ? score : -score; // for minimax
}

// SEARCH

// MVV LVA [attacker][victim]
static int mvv_lva[12][12] = {
    105, 205, 305, 405, 505, 605, 105, 205, 305, 405, 505, 605,
    104, 204, 304, 404, 504, 604, 104, 204, 304, 404, 504, 604,
    103, 203, 303, 403, 503, 603, 103, 203, 303, 403, 503, 603,
    102, 202, 302, 402, 502, 602, 102, 202, 302, 402, 502, 602,
    101, 201, 301, 401, 501, 601, 101, 201, 301, 401, 501, 601,
    100, 200, 300, 400, 500, 600, 100, 200, 300, 400, 500, 600,

    105, 205, 305, 405, 505, 605, 105, 205, 305, 405, 505, 605,
    104, 204, 304, 404, 504, 604, 104, 204, 304, 404, 504, 604,
    103, 203, 303, 403, 503, 603, 103, 203, 303, 403, 503, 603,
    102, 202, 302, 402, 502, 602, 102, 202, 302, 402, 502, 602,
    101, 201, 301, 401, 501, 601, 101, 201, 301, 401, 501, 601,
    100, 200, 300, 400, 500, 600, 100, 200, 300, 400, 500, 600
};

// half move counter
int ply;

// best move
int bestMove;

static inline int scoreMove(int move) {
    if (move_get_capture(move))
    {
        // init target piece
        int targetPiece = P;

        for (int enemyPieceIdx = P + ((1 - side) * 6); enemyPieceIdx <= K + ((1 - side) * 6); enemyPieceIdx++)
        {
            if (get_bit(bitboards[enemyPieceIdx], move_get_target(move)))
            {
                targetPiece = enemyPieceIdx;
            }
        }

        // score by MVV LVA lookup
        return mvv_lva[move_get_piece(move)][targetPiece];
    } else
    {
    }

    return 0;
}

void print_move_scores(moves *move_list) {
    printf("     Move scores:\n\n");

    // loop over moves within a move list
    for (int count = 0; count < move_list->count; count++)
    {
        printf("     move: ");
        printMove(move_list->moves[count]);
        printf(" score: %d\n", scoreMove(move_list->moves[count]));
    }
}

static inline int sortMoves(moves *moveList) {
    int moveScores[moveList->count];

    for (int count = 0; count < moveList->count; count++)
    {
        moveScores[count] = scoreMove(moveList->moves[count]);
    }

    for (int current = 0; current<moveList->count; current++)
    {
        for (int next = current + 1; next<moveList->count; next++)
        {
            if (moveScores[current] < moveScores[next])
            {
                int tempScore = moveScores[current];
                moveScores[current] = moveScores[next];
                moveScores[next] = tempScore;

                int tempMove = moveList->moves[current];
                moveList->moves[current] = moveList->moves[next];
                moveList->moves[next] = tempMove;
            }
        }
    }
}

static inline int quiescenceSearch(int alpha, int beta) {
    // evaluate position
    int evaluation = evaluate();
    nodes++;

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
    moves moveList[1];

    // generate moves
    generateMoves(moveList);
    sortMoves(moveList); // JFC the speed

    // loop over moves within a movelist
    for (int count = 0; count < moveList->count; count++)
    {
        // preserve board state
        copy_board();

        // increment ply
        ply++;

        // make sure to make only legal moves
        if (makeMove(moveList->moves[count], onlyCaptures) == 0)
        {
            // decrement ply
            ply--;

            // skip to next move
            continue;
        }

        // score current move
        int score = -quiescenceSearch(-beta, -alpha);

        // decrement ply
        ply--;

        // take move back
        restore_board();

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

static inline int negamax(int alpha, int beta, int depth) {
    if (depth == 0)
    {
        return quiescenceSearch(alpha, beta);
    }

    nodes++;
    // is king in check, legal moves
    int inCheck = isSquareAttacked((side == white) ? getLSBIndex(bitboards[K]) : getLSBIndex(bitboards[k]), side ^ 1);

    // allow deeper search when in check
    if (inCheck) depth++;


    int legalMoves = 0;
    int bestMoveSoFar;
    int oldAlpha = alpha;

    // create moves list instance
    moves moveList[1];

    generateMoves(moveList);
    sortMoves(moveList); // JFC the speed
    // loop over generated moves
    for (int count = 0; count < moveList->count; count++)
    {
        // preserve board state
        copy_board();

        // increment ply
        ply++;

        // make sure to make only legal moves
        if (makeMove(moveList->moves[count], allMoves) == 0)
        {
            ply--;
            continue;
        }
        legalMoves++;
        // score current move
        int score = -negamax(-beta, -alpha, depth - 1);
        ply--;

        // take move back
        restore_board();

        // fail-hard beta cutoff
        if (score >= beta)
        {
            // node(move) fails high
            return beta;
        }

        if (score > alpha)
        {
            // PV node(move)
            alpha = score;

            // if root move
            if (ply == 0)
            {
                // associate best move with the best score
                bestMoveSoFar = moveList->moves[count];
            }
        }
    }
    if (legalMoves == 0)
    {
        // king is in check - checkmate
        if (inCheck)
        {
            // return mating score
            return -49000 + ply; // + ply to get the mate in least moves possible
        } else
        {
            // king is not in check - stalemate
            // return stalemate score
            return 0;
        }
    }
    if (oldAlpha != alpha)
    {
        bestMove = bestMoveSoFar;
    }
    // node (move) fails low
    return alpha;
}

// search position for the best move
void searchPosition(int depth) {
    // find best move within a given position
    int score = negamax(-50000, 50000, depth);

    if (bestMove)
    {
        printf("info score cp %d depth %d nodes %ld\n", score, depth, nodes);

        // best move placeholder
        printf("bestmove ");
        printMove(bestMove);
        printf("\n");
    }
}

// FEN debug positions
#define empty_board "8/8/8/8/8/8/8/8 w - - "
#define start_position "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1 "
#define tricky_position "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1 "
#define killer_position "rnbqkb1r/pp1p1pPp/8/2p1pP2/1P1P4/3P3P/P1P1P3/RNBQKBNR w KQkq e6 0 1"
#define cmk_position "r2q1rk1/ppp2ppp/2n1bn2/2b1p3/3pP3/3P1NPP/PPP1NPB1/R1BQ1RK1 b - - 0 9 "
// parse UCI position
void parseUCIPosition(char *command) {
    command += 9; // parse "position keyword"

    char *currentCharacter = command;
    if (strncmp(command, "startpos", 8) == 0) // parse "startpos"
    {
        printf("%c\n", *command);
        parseFENString(start_position);
    } else // parse "fen"
    {
        currentCharacter = strstr(command, "fen");
        if (currentCharacter == NULL)
        {
            parseFENString(start_position);
        } else
        {
            currentCharacter += 4;
            parseFENString(currentCharacter);
            printf("%s\n", currentCharacter);
        }
    }
    // parse moves after position (moves)
    currentCharacter = strstr(command, "moves");
    if (currentCharacter != NULL)
    {
        // shift right to after moves
        currentCharacter += 6;

        // loop over moves within a move stringf
        while (*currentCharacter)
        {
            // parse next move
            int move = parseMove(currentCharacter);
            if (move == 0)
            {
                printf("Illegal move in parseUCIposition");
                break;
            }

            makeMove(move, allMoves);
            while (*currentCharacter && *currentCharacter != ' ')
            {
                currentCharacter++;
            }
            currentCharacter++;
        }
    }
    printBoard();
}

// UCI Fixed depth search
// go depth 6
void parseUCCGoDepth(char *command) {
    int depth = -1;
    char *currentDepth = NULL;

    // handle fixed depth search
    if (currentDepth = strstr(command, "depth"))
    {
        depth = atoi(currentDepth + 6);
    } else
    {
        depth = 6;
    }

    // search position
    searchPosition(depth);
}

/*
 *  GUI -> isready
 *  Engine -> readyok
 *  GUI -> ucinewgame
 */
// main UCI loop
void uciLoop() {
    // reset STDIN and STDOUT buffers - console bullshit
    setbuf(stdin, NULL);
    setbuf(stdout, NULL);

    // define user / GUI input buffer
    char input[2000]; // can be pretty long
    // print engine info
    printf("id name Skeibot\n");
    printf("id author Skeibol\n");
    printf("uciok\n");

    // main game loop (UCI input loop)

    while (1)
    {
        // reset user/GUI input
        memset(input, 0, sizeof(input));

        // make sure output reaches the GUI
        fflush(stdout);

        // get user / GUI input
        if (!fgets(input, 2000, stdin))
        {
            // continue...
            continue;
        }
        // make sure input is available
        else if (input[0] == '\n')
        {
            continue;
        }
        // parse UCI 'isready' command
        else if (strncmp(input, "isready", 7) == 0) // parse "startpos"
        {
            printf("readyok\n");
            continue;
        }

        // parse UCI "position" command
        else if (strncmp(input, "position", 8) == 0) // parse "startpos"
        {
            parseUCIPosition(input);
        }

        // parse UCI "newgame" command
        else if (strncmp(input, "ucinewgame", 10) == 0) // parse "startpos"
        {
            parseUCIPosition("position startpos");
        }

        // parse UCI "newgame" command
        else if (strncmp(input, "go", 2) == 0) // parse "startpos"
        {
            parseUCCGoDepth(input);
        }

        // parse UCI "quit" command
        else if (strncmp(input, "quit", 4) == 0) // parse "startpos"
        {
            break;
        }

        // parse UCI "uci" command
        else if (strncmp(input, "uci", 3) == 0) // parse "startpos"
        {
            // print engine info - UCI specific commands
            printf("id name Skeibot\n");
            printf("id author Skeibol\n");
            printf("uciok\n");
        }
    }
}

/********************************************
 *                 MAIN DRIVER              *
 ********************************************/

int main(void) {
    init_all();
    int debug = 1;
    if (debug)
    {
        parseFENString(tricky_position);
        printBoard();
        searchPosition(1);
    } else
    {
        uciLoop();
    }

    return 0;
}
