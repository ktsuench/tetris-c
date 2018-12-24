#include "tetrisPieces.h"

#define ROWS 20
#define COLS 15
#define NORMAL_SPEED 125000000
#define FAST_SPEED 20000000

typedef enum MotionDirection {
  DNONE, DUP, DDOWN, DLEFT, DRIGHT, DSPACE, DDROP, DHOLD
} CtrlAction;

typedef struct TetrisGrid {
  CellType cell;
} Grid;



/* Function declarations */
void setupColors();
void copyBlock();
void newBlock();
void addBlockToGrid(Grid *grid, Block *block, bool newBlock);
void clearBlockFromGrid();
bool dropBlock();
void rotateBlock();
bool canMoveInDir();
void handleCtrls();
void initGrid();
void printGrid();