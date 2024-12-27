import numpy as np
import pygame
import sys
from Board import Board
import os

pygame.init()
pygame.display.set_caption("ChessBot")
font = pygame.font.Font(None, 72)
WIDTH, HEIGHT = 512, 512
DIMENSION = 8
STEP = WIDTH // DIMENSION
BLUE = (0, 0, 255)
WHITE = (200, 200, 200)
BLACK = (50, 50, 50)
BOARD = Board()
PIECE_COLORS = ["White","Black"]

def loadPieceImages():
    piece_images = {}
    piece_names = ['r', 'n', 'b', 'q', 'k', 'p']
    colors = ['w', 'b']  # 'w' for white, 'b' for black
    
    for color in colors:
        for name in piece_names:
            path = os.path.join('pieces', f'{color}{name}.png')
            piece_images[f'{color}{name}'] = pygame.image.load(path)

    return piece_images

def drawLetter(screen,letter, x, y, w, h,color):
    text_surface = font.render(letter, True, color)

    box_rect = pygame.Rect(x, y, w, h)
    
  
    text_rect = text_surface.get_rect(center=box_rect.center)
    
    pygame.draw.rect(screen, BLACK, box_rect, 3)  # Box outline
    
    screen.blit(text_surface, text_rect)

def drawChessBoard(screen):
    '''
    Draw rows and columns of the chessboard
    '''
    for col in range(0, HEIGHT, STEP):
        for row in range(0, WIDTH, STEP):
            if ((col + row) // STEP) % 2 == 0:
                pygame.draw.rect(screen, WHITE, (col, row, STEP, STEP))
            else:
                pygame.draw.rect(screen, BLACK, (col, row, STEP, STEP))

def drawPieces(screen, piece_images):
    '''
    Draw pieces using icons
    '''
    piece_map = ['r', 'n', 'b', 'q', 'k', 'p']
    
    for piece in BOARD.pieces.values():
        piece_key = f"{'w' if piece.color == 0 else 'b'}{piece_map[piece.type]}"
        
        # Get the correct piece image
        icon = piece_images[piece_key]

        # Resize if needed (optional)
        icon = pygame.transform.scale(icon, (STEP, STEP))

        # Draw piece on the board
        screen.blit(icon, (piece.position[1] * STEP, piece.position[0] * STEP))

def drawMoves(screen,moves):
    '''
    Draw the possible moves
    '''
 
    for move in moves:
        assert move[1] >= 0 and move[1] < DIMENSION and move[0] >= 0 and move[0] < DIMENSION, "Out of bounds"
            
        rect_surface = pygame.Surface((STEP,STEP), pygame.SRCALPHA)  # SRCALPHA means surface supports transparency

        # Draw a rectangle on this transparent surface
        pygame.draw.rect(rect_surface, (125, 125, 0), (0, 0, STEP, STEP)) 

        # Set the transparency of the entire surface 
        rect_surface.set_alpha(128)

        screen.blit(rect_surface, (move[1] * STEP, move[0] * STEP))  # Blit the surface at coordinates (100, 150)


def nextTurn(turn):
    return (turn + 1 ) % 2
def main():
    screen = pygame.display.set_mode((WIDTH, HEIGHT))
    running = True
    screen.fill((255, 255, 255))
    selectedPiece = None
    pieceOnSquare = None
    mousePosNormalized = (0,0)
    legalMoves = []
    turn = 0
    possibleMoves = BOARD.getAllLegalMovesStart(turn)
    pieceImages = loadPieceImages()
    automatic = False
    while running:
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                running = False
            if event.type == pygame.MOUSEBUTTONDOWN:
                mousePos = event.pos 
                mousePosNormalized = (mousePos[1] // STEP ,  mousePos[0] // STEP ) 
                pieceOnSquareName, pieceOnSquare = BOARD.checkForPieceOnSquare(mousePosNormalized,anyColor=True) 

                
                
                if selectedPiece is not None: 
                    if mousePosNormalized in legalMoves: # If piece is selected, and valid move is clicked, do the move
                        BOARD.movePiece(selectedPiece,mousePosNormalized)
                   
                            
                        selectedPiece = None
                        legalMoves = []
                        turn = nextTurn(turn)
                        possibleMoves = BOARD.getAllLegalMovesStart(turn)
                        if len(possibleMoves) == 0:
                            print(f"{PIECE_COLORS[turn]} in checkmate")
                        
                        
                    elif pieceOnSquare is not None and pieceOnSquare.color == turn: # If piece is selected and another friendly piece is clicked, select that one
                        selectedPiece = pieceOnSquare
                        legalMoves = BOARD.getLegalMovesForPiece(pieceOnSquareName)
                        print(selectedPiece.canBePassanted)
                    else:
                        selectedPiece = None
                        legalMoves = []
                    
                else:
                    if pieceOnSquare is not None and pieceOnSquare.color == turn: # If nothing is selected, select the friendly piece that is clicked
                        selectedPiece = pieceOnSquare
                        legalMoves = BOARD.getLegalMovesForPiece(pieceOnSquareName)
                        print(selectedPiece.canBePassanted)
        import random
        if turn == 1 and automatic:
            rndPiece = random.choice(possibleMoves)
            rndMove = random.choice(rndPiece[1])
            rndPiece = rndPiece[0]
            BOARD.movePiece(rndPiece,rndMove)
                   
            selectedPiece = None
            legalMoves = []
            turn = nextTurn(turn)
            possibleMoves = BOARD.getAllLegalMovesStart(turn)
            if len(possibleMoves) == 0:
                print(f"{PIECE_COLORS[turn]} in checkmate")
                 
        drawChessBoard(screen) 
        if selectedPiece is not None:
            drawMoves(screen,legalMoves) 
        drawPieces(screen,pieceImages) 

        pygame.display.flip()

    pygame.quit()
    sys.exit()


if __name__ == "__main__":
    main()
