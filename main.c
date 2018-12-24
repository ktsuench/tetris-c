#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <ncurses.h>
#include "tetris.h"

void delay(int x) {
  for(;;)
    if (x-- == 0) break;
}

/* Game */
int main(void) {
  /* Playing field */
  Grid grid[ROWS * COLS];

  /* Other variables */
  Block currentBlock;
  bool gameRunning = FALSE, droppingBlock = FALSE;
  WINDOW *field;
  int speed = NORMAL_SPEED;
    
  int x = 500;
  
  // start curses
  initscr();
  // change input mode to cbreak https://en.wikipedia.org/wiki/Terminal_mode
  cbreak();
  // disable software echo http://pubs.opengroup.org/onlinepubs/7908799/xcurses/noecho.html
  noecho();
  // set wait on input to 0
  timeout(0);
  // hide terminal cursor
  curs_set(0);
  setupColors();

  /* @todo have input to start game */
  gameRunning = TRUE;

  field = newwin(ROWS + 2, COLS + 2, 0, 0);

  initGrid(grid);

  while(gameRunning) {
    if (droppingBlock == FALSE) {
      newBlock(&currentBlock);
      addBlockToGrid(grid, &currentBlock, TRUE);
      droppingBlock = TRUE;
    } else {
      droppingBlock = dropBlock(&grid, &currentBlock);
    }

    handleCtrls(grid, &currentBlock, &speed, getch());

    // int i, j;
    // for (i = 0; i < ROWS; i++) {
    //   for (j = 0; j < COLS; j++) {
    //     printf("%u", (unsigned int) grid[i * COLS + j].cell);
    //   }
    //   printf("\n");
    // }
    printGrid(field, grid);

    delay(speed);
    if (x-- == 0) gameRunning = FALSE;
  }

  clear(); // clear screen
  endwin(); // stop curses
  return 0;
}

/* Functions */
void setupColors() {
  start_color();
  // id, foreground, background
  init_pair(TNONE,  COLOR_BLACK,  COLOR_BLACK);
  init_pair(TT,     COLOR_BLACK,  COLOR_MAGENTA);
  init_pair(TL,     COLOR_BLACK,  COLOR_BLUE);
  init_pair(TJ,     COLOR_BLACK,  COLOR_WHITE);
  init_pair(TI,     COLOR_BLACK,  COLOR_CYAN);
  init_pair(TS,     COLOR_BLACK,  COLOR_RED);
  init_pair(TZ,     COLOR_BLACK,  COLOR_GREEN);
  init_pair(TO,     COLOR_BLACK,  COLOR_YELLOW);
}

void copyBlock(Block *blockDest, Block *blockSrc) {
  int i, j;

  for (i = 0; i < 4; i++) {
    for(j = 0; j < 4; j++) {
      blockDest->coords[i][j] = blockSrc->coords[i][j];
    }
  }

  blockDest->type = blockSrc->type;
  blockDest->orientation = blockSrc->orientation;
  blockDest->topRow = blockSrc->topRow - 1;
  blockDest->bottomRow = blockSrc->bottomRow - 1;
  blockDest->nextOrientation = blockSrc->nextOrientation != NULL
    ? blockSrc->nextOrientation
    : Shapes[blockSrc->type - 1].ptr;
}

void newBlock(Block *block) {
  int stype;
  time_t tref;

  // init rand
  srand((unsigned) time(&tref));

  // get shape
  stype = rand() % NUM_BLOCKS;

  copyBlock(block, Shapes[stype].ptr);
}

void addBlockToGrid(Grid *grid, Block *block, bool newBlock) {
  int i, j, loc;

  if (newBlock == TRUE) {
    block->row = 0;
    block->col = (int) COLS/2 - 2;
  }

  for (i = 0; i < block->bottomRow + 1; i++) {
    for (j = 0; j < 4; j++) {
      if (block->coords[i][j] != TNONE) {
        loc = (block->row + i) * COLS + block->col + j;
        grid[loc].cell = block->type;
      }
    }
  }
}

void clearBlockFromGrid(Grid *grid, Block block) {
  int i, j, loc;

  for (i = 0; i < block.bottomRow + 1; i++) {
    for (j = 0; j < 4; j++) {
      loc = (block.row + i) * COLS + block.col + j;
      grid[loc].cell = TNONE;
    }
  }
}

bool dropBlock(Grid *grid, Block *block) {
  int i, j, loc;

  if (canMoveInDir(grid, *block, DDROP)) {
    clearBlockFromGrid(grid, *block);

    block->row = block->row + 1;
    addBlockToGrid(grid, block, FALSE);

    return TRUE;
  }
  
  return FALSE;
}

void rotateBlock(Grid *grid, Block *block) {
  int i, j, loc;

  clearBlockFromGrid(grid, *block);

  if (canMoveInDir(grid, *block, DUP)) {
    copyBlock(block, block->nextOrientation);
  }

  addBlockToGrid(grid, block, FALSE);
}

bool canMoveInDir(Grid *grid, Block block, CtrlAction action) {
  bool notAtEnd = TRUE, notAtSide = TRUE, nothingInWay = TRUE;
  int i, j, loc;
  Block *blockPtr;

  switch (action) {
    case DDROP:
      // drop check
      notAtEnd = block.row + block.bottomRow < ROWS;

      if (notAtEnd) {
        i = 0;

        do {
          loc = (block.row + block.bottomRow + 1) * COLS + block.col + i;

          if (grid[loc].cell != TNONE && block.coords[block.bottomRow][i] != TNONE)
            nothingInWay = false;
        } while (nothingInWay && i++ < 4);
      }

      return notAtEnd && nothingInWay;
    case DHOLD:
      // check if possible
    case DUP:
      // rotation check
      blockPtr = block.nextOrientation;

      for (i = blockPtr->topRow; i < blockPtr->bottomRow + 1; i++) {
        for (j = 0; j < 4; j++) {
          loc = (block.row + i) * COLS + block.col + j;

          if (grid[loc].cell != TNONE && blockPtr->coords[i][j] != TNONE) {
            nothingInWay = FALSE;
            break;
          }
        }

        if (nothingInWay == FALSE)
          break;
      }

      return nothingInWay;
    case DDOWN:
      // quick drop check
    case DLEFT:
      // horizontal move check
    case DRIGHT:
      // horizontal move check
    case DSPACE:
      // instant drop
    case DNONE:
    default:
      // do nothing
      break;
  }

  return FALSE;
}

void handleCtrls(Grid *grid, Block *block, int *spd, int ch) {
  bool speedChanged = FALSE;

  switch (ch) {
    case 'i': // up
      rotateBlock(grid, block);
      break;
    case 'l': // right
      break;
    case 'j': // left
      break;
    case 'k': // down
      *spd = FAST_SPEED;
      speedChanged = TRUE;
      break;
  }

  if (speedChanged == FALSE)
    *spd = NORMAL_SPEED;
}

void initGrid(Grid *grid) {
  int i, j, loc;

  for (i = 0; i < ROWS; i++) {
    for (j = 0; j < COLS; j++) {
      loc = i * COLS + j;
      grid[loc].cell = TNONE;
    }
  }
}

void printGrid(WINDOW *w, Grid *grid) {
  int i, j, loc;
  wchar_t color;

  // border
  box(w, 0, 0);

  for (i = 0; i < ROWS; i++) {
    for(j = 0; j < COLS; j++) {
      loc = i * COLS + j;
      color = ' '|COLOR_PAIR(grid[loc].cell);

      // move 1 + i row down and 1 + j column over
      wmove(w, 1 + i, 1 + j);
      waddch(w, color);
    }
  }

  wrefresh(w);
}