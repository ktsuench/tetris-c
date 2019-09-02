#include "tetrisPieces.h"

#define ROWS 20
#define COLS 12
#define NORMAL_SPEED 125000000
#define FAST_SPEED 20000000

#define bool int
#define TRUE 1
#define FALSE 0

typedef enum MotionDirection {
  DNONE, DUP, DDOWN, DLEFT, DRIGHT, DSPACE, DDROP, DHOLD
} CtrlAction;

typedef struct TetrisGrid {
  unsigned int cell;
} Grid;

/* Function declarations */
void startGame(Grid *grid, WINDOW *field);
void setupColors(void);
void copyBlock(Block *blockDest, Block *blockSrc);
void newBlock(Block *block);
void setBlockToOrigin(Block *block);
void addBlockToGrid(Grid *grid, Block *block, bool newBlock, bool normalizeBlock);
void clearBlockFromGrid(Grid *grid, Block block);
bool dropBlock(Grid *grid, Block *block);
void rotateBlock(Grid *grid, Block *block);
void slideBlock(Grid *grid, Block *block, CtrlAction dir);
bool canMoveInDir(Grid *grid, Block block, CtrlAction action);
void handleCtrls(Grid *grid, Block *block, int *spd, int ch);
bool clearLines(Grid *grid);
bool gameEnd(Grid *grid, bool creatingNewBlock);
void initGrid(Grid *grid);
void printGrid(WINDOW *w, Grid *grid);
