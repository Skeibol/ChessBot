import numpy as np
from Pieces import *
from types import MappingProxyType


def getStraightMovesFromPosition(pos, limit):
    """
    Precompute directions for pieces that move straight for every square to avoid computing at game time.
    Return array of allMoves from that square (ignores other pieces)
    """

    allMoves = []

    upMoves = []
    newY = pos[0] - 1
    while newY >= 0:  # GET UP MOVES
        upMoves.append((newY, pos[1]))
        newY -= 1
    allMoves.append(upMoves)

    downMoves = []
    newY = pos[0] + 1
    while newY < limit:  # GET DOWN MOVES
        downMoves.append((newY, pos[1]))
        newY += 1
    allMoves.append(downMoves)

    leftMoves = []
    newX = pos[1] - 1
    while newX >= 0:  # GET LEFT MOVES
        leftMoves.append((pos[0], newX))
        newX -= 1
    allMoves.append(leftMoves)

    rightMoves = []
    newX = pos[1] + 1
    while newX < limit:  # GET RIGHT MOVES
        rightMoves.append((pos[0], newX))
        newX += 1
    allMoves.append(rightMoves)

    return allMoves


def getDiagonalMovesFromPosition(pos, limit):
    """
    Precompute directions for pieces that move diagonal for every square to avoid computing at game time.
    Return array of allMoves from that square (ignores other pieces)
    """

    allMoves = []

    tempMoves = []
    newY = pos[0] - 1
    newX = pos[1] - 1
    while newY >= 0 and newX >= 0:  # GET UP LEFT MOVES
        tempMoves.append((newY, newX))
        newY -= 1
        newX -= 1
    allMoves.append(tempMoves)

    tempMoves = []
    newY = pos[0] - 1
    newX = pos[1] + 1
    while newY >= 0 and newX < limit:  # GET UP RIGHT MOVES
        tempMoves.append((newY, newX))
        newY -= 1
        newX += 1
    allMoves.append(tempMoves)

    tempMoves = []
    newY = pos[0] + 1
    newX = pos[1] + 1
    while newY < limit and newX < limit:  # GET DOWN RIGHT MOVES
        tempMoves.append((newY, newX))
        newY += 1
        newX += 1
    allMoves.append(tempMoves)

    tempMoves = []
    newY = pos[0] + 1
    newX = pos[1] - 1
    while newY < limit and newX >= 0:  # GET DOWN LEFT MOVES
        tempMoves.append((newY, newX))
        newY += 1
        newX -= 1
    allMoves.append(tempMoves)

    return allMoves


def getHorseyJumpsFromPosition(pos, limit):
    allMoves = []
    y = pos[0]
    x = pos[1]
    jumpLocations = [
        (y - 2, x - 1),  # 2 squares up, 1 square left
        (y - 2, x + 1),  # 2 squares up, 1 square right
        (y + 2, x - 1),  # 2 squares down, 1 square left
        (y + 2, x + 1),  # 2 squares down, 1 square right
        (y - 1, x - 2),  # 1 square up, 2 squares left
        (y - 1, x + 2),  # 1 square up, 2 squares right
        (y + 1, x - 2),  # 1 square down, 2 squares left
        (y + 1, x + 2),  # 1 square down, 2 squares right
    ]

    for loc in jumpLocations:
        jumpY = loc[0]
        jumpX = loc[1]
        if jumpY >= 0 and jumpY < limit and jumpX >= 0 and jumpX < limit:
            allMoves.append(loc)
    return allMoves


class Board:
    def __init__(self):
        self.dimension = 8
        self.capturedPieces = {}
        self.attackedSquares = []
        self.pieces = {
            "wR1": PieceRook((7, 0), 0),  # White Rook 1
            "wR2": PieceRook((7, 7), 0),  # White Rook 2
            "wN1": PieceKnight((7, 1), 0),  # White Knight 1
            "wN2": PieceKnight((7, 6), 0),  # White Knight 2
            "wB1": PieceBishop((7, 2), 0),  # White Bishop 1
            "wB2": PieceBishop((7, 5), 0),  # White Bishop 2
            "wQ": PieceQueen((7, 3), 0),  # White Queen
            "wK": PieceKing((7, 4), 0),  # White King
        }

        # Add White Pawns
        self.pieces.update({f"wP{i+1}": PiecePawn((6, i), 0) for i in range(8)})

        # Add Black Pieces
        self.pieces.update(
            {
                "bR1": PieceRook((0, 0), 1),  # Black Rook 1
                "bR2": PieceRook((0, 7), 1),  # Black Rook 2
                "bN1": PieceKnight((0, 1), 1),  # Black Knight 1
                "bN2": PieceKnight((0, 6), 1),  # Black Knight 2
                "bB1": PieceBishop((0, 2), 1),  # Black Bishop 1
                "bB2": PieceBishop((0, 5), 1),  # Black Bishop 2
                "bQ": PieceQueen((0, 3), 1),  # Black Queen
                "bK": PieceKing((0, 4), 1),  # Black King
            }
        )

        # Add Black Pawns
        self.pieces.update({f"bP{i+1}": PiecePawn((1, i), 1) for i in range(8)})

        self.board = np.zeros((self.dimension, self.dimension))
        self.diagonals = {}
        self.straights = {}
        self.horseyJumps = {}

        for col in range(self.dimension):
            for row in range(self.dimension):
                self.straights[(col, row)] = getStraightMovesFromPosition(
                    (col, row), self.dimension
                )
        for col in range(self.dimension):
            for row in range(self.dimension):
                self.diagonals[(col, row)] = getDiagonalMovesFromPosition(
                    (col, row), self.dimension
                )
        for col in range(self.dimension):
            for row in range(self.dimension):
                self.horseyJumps[(col, row)] = getHorseyJumpsFromPosition(
                    (col, row), self.dimension
                )

        self.diagonals = MappingProxyType(self.diagonals)
        self.straights = MappingProxyType(self.straights)
        self.horseyJumps = MappingProxyType(self.horseyJumps)
        self.moveLog = {}
        self.turn = 0

    def getLegalMovesForPiece(self, pieceName):
        return self.pieces[pieceName].legalMoves
    
    def getAllLegalMovesStart(self,color):
     
        movesToPickFrom = []
        for piece in self.pieces.copy().values():
            if piece.color == color:
                piece.getLegalMoves(self)
                if piece.canBePassanted:
                    piece.canBePassanted = False
                if piece.legalMoves == []:
                    continue
                movesToPickFrom.append((piece,piece.legalMoves))
            
        return movesToPickFrom
    def checkKingUnderAttack(self, color):
        self.getAttackedSquares(1 - color)
        if color == 0:
            king = self.pieces["wK"]
        else:
            king = self.pieces["bK"]
        for attacked in self.attackedSquares:
            if attacked == king.position:
                return True

        return False

    def movePiece(self, piece, nextPosition):
        destinationPieceName, destinationPieceObj = self.checkForPieceOnSquare(nextPosition,anyColor=True)
        if destinationPieceObj is not None:
                
            if destinationPieceObj.type == 0 and destinationPieceObj.color == piece.color:
                if destinationPieceObj.position < piece.position:
                    self.castle(piece,destinationPieceObj,long=True)
                else:
                    self.castle(piece,destinationPieceObj,long=False)
                return
            if piece.type == 5 and destinationPieceObj.type == 5 and destinationPieceObj.position[0] == piece.position[0]:
                self.enPassant(piece,destinationPieceObj,destinationPieceName)
                return
        
        self.moveLog[self.turn] = {piece: [piece.position, nextPosition]}
        
        piece.setPosition(nextPosition) 
        self.checkCapture(piece)
        self.turn += 1
        
    def castle(self,king,rook,long):
        if long:
            nextKingPosition = (king.position[0], king.position[1] - 2)
            nextRookPosition = (rook.position[0], rook.position[1] + 3)
        else:
            nextKingPosition = (king.position[0], king.position[1] + 2)
            nextRookPosition = (rook.position[0], rook.position[1] - 2)
        self.moveLog[self.turn] = {king: [long]}
        king.setPosition(nextKingPosition,True) 
        rook.setPosition(nextRookPosition,True) 
    
    def enPassant(self,pawn,enemyPawn,name):
        if pawn.color == 0:
            nextPawnPosition = (enemyPawn.position[0] - 1,enemyPawn.position[1])
        else:
            nextPawnPosition = (enemyPawn.position[0] + 1,enemyPawn.position[1])
        
        pawn.setPosition(nextPawnPosition,True)
        enemyPawn.captured = True
        self.capturedPieces[name] = enemyPawn
        del self.pieces[name]
       
        
    
    def undoMove(self):
        pass

        

    def simulateMoveForCheck(self, piece, nextPosition):
        startPos = piece.position
        piece.setPosition(nextPosition,False) 
        capturedName, capturedPiece = self.checkCapture(piece)
        inCheck = self.checkKingUnderAttack(piece.color)
        if capturedName is not None:
            self.pieces[capturedName] = capturedPiece
            del self.capturedPieces[capturedName]
        piece.setPosition(startPos,False) 
        
        if inCheck:
            return True
        else:
            return False
    def simulateCastleForCheck(self, king, rook, long):
        startKingPosition = king.position
        startRookPosition = rook.position
        if long:
            nextKingPosition = (king.position[0], king.position[1] - 2)
            nextRookPosition = (rook.position[0], rook.position[1] + 3)
        else:
            nextKingPosition = (king.position[0], king.position[1] + 2)
            nextRookPosition = (rook.position[0], rook.position[1] - 2)
        
        king.setPosition(nextKingPosition,False) 
        rook.setPosition(nextRookPosition,False) 
      
        inCheck = self.checkKingUnderAttack(king.color)
        
        king.setPosition(startKingPosition,False) 
        rook.setPosition(startRookPosition,False) 
    
        
        if inCheck:
            return True
        else:
            return False
        
    def checkCapture(self, capturingPiece):
        for name, piece in self.pieces.items():
            if (
                capturingPiece.position == piece.position
                and capturingPiece.color != piece.color
            ):
                piece.captured = True
                self.capturedPieces[name] = piece
                del self.pieces[name]

                return name, piece
        return None, None

    def getAttackedSquares(self, color):
        self.attackedSquares = []
        for piece in self.pieces.values():
            if piece.color == color:
                self.attackedSquares.extend(piece.getAttackMoves(self))

    def checkForPieceOnSquare(self, square, color=0, anyColor=False):
        for pieceName,pieceObj in self.pieces.items():
            if (pieceObj.color == color or anyColor) and pieceObj.position == square:
                return pieceName, pieceObj

        return None, None
