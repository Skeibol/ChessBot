from enum import Enum


class PieceEnum(Enum):
    r = 0
    n = 1
    b = 2
    q = 3
    k = 4
    p = 5


class Piece:
    def __init__(self, position, type, color):
        self._position = position
        self.type = type
        self.color = color
        self.hasMoved = False
        self.canBePassanted = False
        self.legalMoves = []
        self.attackMoves = []
        

    @property
    def position(self):
        return self._position  # Return the stored position

    def setPosition(self, new_position, mark_as_moved=True):
        if mark_as_moved and not self.hasMoved:  # Only mark as moved if allowed
            self.hasMoved = True
        
        if mark_as_moved:
            if abs(new_position[0] - self._position[0]) == 2:
                self.canBePassanted = True
            else:
                self.canBePassanted = False
        self._position = new_position  # Set the new position

    def getLegalMoves(self, board):
        return []

    def getAttackMoves(self, board):
        return self.getLegalMoves(board)


class PiecePawn(Piece):
    def __init__(self, position, color):
        super().__init__(position, PieceEnum.p.value, color)
        self.canBePassanted = False

    def getLegalMoves(self, board):
        moves = board.straights[self.position][
            self.color
        ][  # Get the moves and diagonals relative to the color of the piece
            0 : 2 - self.hasMoved
        ]
        diag = moves[0]
        diags = [(diag[0], diag[1] + 1), (diag[0], diag[1] - 1)]

        legalMoves = []
        for move in diags:
            if move[1] < 0 or move[1] >= 8:
                continue
            if board.checkForPieceOnSquare(move, color=1 - self.color)[0] is not None:
                if not board.simulateMoveForCheck(self, move):
                    legalMoves.append(move)

        for move in moves:
            if board.checkForPieceOnSquare(move, anyColor=True)[0] is None:
                if not board.simulateMoveForCheck(self, move):
                    legalMoves.append(move)
                    
            else:
                break
            
        # En Passant
        leftPassant = board.straights[self.position][2]
        rightPassant = board.straights[self.position][3]
        
        if leftPassant:
            leftPassant = leftPassant[0]
            pieceName, pieceOnSquare = board.checkForPieceOnSquare(leftPassant, anyColor=True)
            if pieceName is not None and pieceOnSquare.type == 5 and pieceOnSquare.color != self.color and pieceOnSquare.canBePassanted:
                legalMoves.append(leftPassant)
                
        if rightPassant:
            rightPassant = rightPassant[0]
            pieceName, pieceOnSquare = board.checkForPieceOnSquare(rightPassant, anyColor=True)
            if pieceName is not None and pieceOnSquare.type == 5 and pieceOnSquare.color != self.color and pieceOnSquare.canBePassanted:
                legalMoves.append(rightPassant)
        
            

       
        

        self.legalMoves = legalMoves

    def getAttackMoves(self, board):
        attackMoves = []
        diag = board.straights[self.position][self.color][0]
        diags = [(diag[0], diag[1] + 1), (diag[0], diag[1] - 1)]
        for d in diags:
            if d[1] < 0 or d[1] >= 8:
                continue
            attackMoves.append(d)
        return attackMoves


class PieceRook(Piece):
    def __init__(self, position, color):
        super().__init__(position, PieceEnum.r.value, color)

    def getLegalMoves(self, board):
        legalMoves = []
        for dir in board.straights[self.position]:
            for move in dir:
                pieceName, pieceOnSquare = board.checkForPieceOnSquare(move, anyColor=True)
                if pieceOnSquare is None:
                    if not board.simulateMoveForCheck(self, move):
                        legalMoves.append(move)
                else:
                    if pieceOnSquare.color != self.color:
                        if not board.simulateMoveForCheck(self, move):
                            legalMoves.append(move)
                    break
        self.legalMoves = legalMoves
    def getAttackMoves(self, board):
        attackMoves = []
        for dir in board.straights[self.position]:
            for move in dir:
                pieceName, pieceOnSquare = board.checkForPieceOnSquare(move, anyColor=True)
                if pieceOnSquare is None:
                    attackMoves.append(move)
                else:
                    if pieceOnSquare.color != self.color:
                        attackMoves.append(move)
                    break
        return attackMoves


class PieceKnight(Piece):
    def __init__(self, position, color):
        super().__init__(position, PieceEnum.n.value, color)

    def getLegalMoves(self, board):
        legalMoves = []
        for move in board.horseyJumps[self.position]:
            pieceName, pieceOnSquare = board.checkForPieceOnSquare(move, anyColor=True)
            if pieceOnSquare is None:
                if not board.simulateMoveForCheck(self, move):
                    legalMoves.append(move)
            else:
                if pieceOnSquare.color != self.color:
                    if not board.simulateMoveForCheck(self, move):
                        legalMoves.append(move)
                continue
        self.legalMoves = legalMoves

    def getAttackMoves(self, board):
        attackMoves = []
        for move in board.horseyJumps[self.position]:
            pieceName, pieceOnSquare = board.checkForPieceOnSquare(move, anyColor=True)
            if pieceOnSquare is None:
                attackMoves.append(move)
            else:
                if pieceOnSquare.color != self.color:
                    attackMoves.append(move)
                continue
        return attackMoves


class PieceBishop(Piece):
    def __init__(self, position, color):
        super().__init__(position, PieceEnum.b.value, color)

    def getLegalMoves(self, board):
        legalMoves = []
        for dir in board.diagonals[self.position]:
            for move in dir:
                pieceName, pieceOnSquare = board.checkForPieceOnSquare(move, anyColor=True)
                if pieceOnSquare is None:
                    if not board.simulateMoveForCheck(self, move):
                        legalMoves.append(move)
                else:
                    if pieceOnSquare.color != self.color:
                        if not board.simulateMoveForCheck(self, move):
                            legalMoves.append(move)
                    break

        self.legalMoves = legalMoves

    def getAttackMoves(self, board):
        attackMoves = []
        for dir in board.diagonals[self.position]:
            for move in dir:
                pieceName, pieceOnSquare = board.checkForPieceOnSquare(move, anyColor=True)
                if pieceOnSquare is None:
                    attackMoves.append(move)
                else:
                    if pieceOnSquare.color != self.color:
                        attackMoves.append(move)
                    break

        return attackMoves


class PieceQueen(Piece):
    def __init__(self, position, color):
        super().__init__(position, PieceEnum.q.value, color)

    def getLegalMoves(self, board):
        legalMoves = []
        for dir in board.straights[self.position]:
            for move in dir:
                pieceName, pieceOnSquare = board.checkForPieceOnSquare(move, anyColor=True)
                if pieceOnSquare is None:
                    if not board.simulateMoveForCheck(self, move):
                        legalMoves.append(move)
                else:
                    if pieceOnSquare.color != self.color:
                        if not board.simulateMoveForCheck(self, move):
                            legalMoves.append(move)
                    break

        for dir in board.diagonals[self.position]:
            for move in dir:
                pieceName, pieceOnSquare = board.checkForPieceOnSquare(move, anyColor=True)
                if pieceOnSquare is None:
                    if not board.simulateMoveForCheck(self, move):
                        legalMoves.append(move)
                else:
                    if pieceOnSquare.color != self.color:
                        if not board.simulateMoveForCheck(self, move):
                            legalMoves.append(move)
                    break

        self.legalMoves = legalMoves

    def getAttackMoves(self, board):
        attackMoves = []
        for dir in board.straights[self.position]:
            for move in dir:
                pieceName, pieceOnSquare = board.checkForPieceOnSquare(move, anyColor=True)
                if pieceOnSquare is None:
                    attackMoves.append(move)
                else:
                    if pieceOnSquare.color != self.color:
                        attackMoves.append(move)
                    break

        for dir in board.diagonals[self.position]:
            for move in dir:
                pieceName, pieceOnSquare = board.checkForPieceOnSquare(move, anyColor=True)
                if pieceOnSquare is None:
                    attackMoves.append(move)
                else:
                    if pieceOnSquare.color != self.color:
                        attackMoves.append(move)
                    break

        return attackMoves


class PieceKing(Piece):
    def __init__(self, position, color):
        super().__init__(position, PieceEnum.k.value, color)
        self.isInCheck = False

    def getLegalMoves(self, board):
        legalMoves = []

        for dir in board.diagonals[self.position]:
            for move in dir[:1]:
                pieceName, pieceOnSquare = board.checkForPieceOnSquare(move, anyColor=True)
                if pieceOnSquare is None:
                    if not board.simulateMoveForCheck(self, move):
                        legalMoves.append(move)
                else:
                    if pieceOnSquare.color != self.color:
                        if not board.simulateMoveForCheck(self, move):
                            legalMoves.append(move)
                    break

        for dir in board.straights[self.position]:
            for move in dir[:1]:
                pieceName, pieceOnSquare = board.checkForPieceOnSquare(move, anyColor=True)
                if pieceOnSquare is None:
                    if not board.simulateMoveForCheck(self, move):
                        legalMoves.append(move)
                else:
                    if pieceOnSquare.color != self.color:
                        if not board.simulateMoveForCheck(self, move):
                            legalMoves.append(move)
                    break
        
        if not self.hasMoved:
            for leftX in range(1,5):
                nextPos = (self.position[0],self.position[1] - leftX)
                pieceName, pieceOnSquare = board.checkForPieceOnSquare(nextPos, anyColor=True)
                if pieceOnSquare is not None:
                    if pieceOnSquare.type == 0 and pieceOnSquare.color == self.color and not pieceOnSquare.hasMoved and leftX == 4:
                        if not board.simulateCastleForCheck(self,pieceOnSquare,long=True):
                            legalMoves.append(nextPos)
                    else:
                        break
            for rightX in range(1,4):
                nextPos = (self.position[0],self.position[1] + rightX)
                pieceName, pieceOnSquare = board.checkForPieceOnSquare(nextPos, anyColor=True)
                if pieceOnSquare is not None:
                    if pieceOnSquare.type == 0 and pieceOnSquare.color == self.color and not pieceOnSquare.hasMoved and rightX == 3:
                        if not board.simulateCastleForCheck(self,pieceOnSquare,long=False):
                            legalMoves.append(nextPos)
                    else:
                        break
        self.legalMoves = legalMoves

    def getAttackMoves(self, board):
        attackMoves = []

        for dir in board.diagonals[self.position]:
            for move in dir[:1]:
                pieceName, pieceOnSquare = board.checkForPieceOnSquare(move, anyColor=True)
                if pieceOnSquare is None:
                    attackMoves.append(move)
                else:
                    if pieceOnSquare.color != self.color:
                        attackMoves.append(move)
                    break

        for dir in board.straights[self.position]:
            for move in dir[:1]:
                pieceName, pieceOnSquare = board.checkForPieceOnSquare(move, anyColor=True)
                if pieceOnSquare is None:
                    attackMoves.append(move)
                else:
                    if pieceOnSquare.color != self.color:
                        attackMoves.append(move)
                    break

        return attackMoves
