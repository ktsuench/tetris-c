#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ncurses.h>
#include "tetris.h"

#define DEBUG 1

void delay(int x) {
  for(;;)
    if (x-- == 0) break;
}

#if defined DEBUG
  WINDOW *debug;
  WINDOW *dgrid;
#endif

/* Game */
int main(void) {
  /* Playing field */
  Grid grid[ROWS * COLS];
  WINDOW *field;
  char cmd;
  bool killApp = FALSE;

  // start curses
  initscr();
  // change input mode to cbreak https://en.wikipedia.org/wiki/Terminal_mode
  cbreak();
  // disable software echo http://pubs.opengroup.org/onlinepubs/7908799/xcurses/noecho.html
#ifndef DEBUG
  noecho();
#endif
  // set up window
  field = newwin(ROWS + 2, COLS + 2, 0, 0);
  // set wait on input to 0
//#if defined DEBUG
//  wtimeout(field, 10000);
//#else
  wtimeout(field, 0);
//#endif
  // enable arrow keys
  keypad(field, TRUE);
  // hide terminal cursor
  curs_set(0);
  setupColors();

  while (killApp == FALSE) {
    startGame(grid, field);

    while ((cmd = wgetch(field)) != 'r' && cmd != 'e');

    if (cmd == 'e')
      killApp = TRUE;
  }

  clear(); // clear screen
  endwin(); // stop curses
}

/* Functions */
void startGame(Grid *grid, WINDOW *field) {
  Block currentBlock;
  bool gameRunning = FALSE, droppingBlock = FALSE, doNotCreateBlock;
  int speed = NORMAL_SPEED;
  char *goMsg = "GAME OVER";

  /* @todo have input to start game */
  gameRunning = TRUE;

#ifdef DEBUG
  debug = newwin(8, 30, 0, COLS * 2 + 8);
  box(debug, 0, 0);
  wrefresh(debug);

  dgrid = newwin(ROWS + 2, COLS + 2, 0, COLS + 4);
  box(dgrid, 0, 0);
  wrefresh(dgrid);
#endif

  initGrid(grid);

  while(gameRunning) {
    if (droppingBlock == FALSE && (doNotCreateBlock = gameEnd(grid, TRUE)) == FALSE) {
      newBlock(&currentBlock);
      addBlockToGrid(grid, &currentBlock, TRUE, TRUE);
      handleCtrls(grid, &currentBlock, &speed, wgetch(field));
      droppingBlock = TRUE;
    } else if (doNotCreateBlock == TRUE) {
      gameRunning = FALSE;
      break;
    } else {
      handleCtrls(grid, &currentBlock, &speed, wgetch(field));
      droppingBlock = dropBlock(grid, &currentBlock);
    }

    if (droppingBlock == FALSE) {
      clearLines(grid);
    }
    printGrid(field, grid);
    doupdate();

    usleep(speed / 500);
    if (doNotCreateBlock == TRUE && gameEnd(grid, FALSE) == TRUE) {
      gameRunning = FALSE;
    }
  }

  wmove(field, ROWS / 2 , COLS / 2 - strlen(goMsg) / 2);
  wprintw(field, goMsg);
}

void setupColors(void) {
  start_color();
  // id, foreground, background
  init_pair(TNONE,  COLOR_BLACK,  COLOR_WHITE);
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
  blockDest->startCol = blockSrc->startCol - 1;
  blockDest->endCol = blockSrc->endCol - 1;
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

void setBlockToOrigin(Block *block) {
  int i, j;
  Block *swap = malloc(sizeof(Block));

  copyBlock(swap, block);

  for (i = 0; i < 4; i++) {
    for (j = 0; j < 4; j++) {
      if (block->topRow + i < 4 && block->startCol + j < 4) {
        block->coords[i][j] = swap->coords[block->topRow + i][block->startCol + j];
      } else {
        block->coords[i][j] = TNONE;
      }
    }
  }

  free(swap);

  block->endCol = block->endCol - block->startCol;
  block->startCol = 0;
  block->bottomRow = block->bottomRow - block->topRow;
  block->topRow = 0;
}

void addBlockToGrid(Grid *grid, Block *block, bool newBlock, bool normalizeBlock) {
  int i, j, loc;

  if (newBlock == TRUE) {
    block->row = 0;
    block->col = (int) COLS/2 - 2;
  }

  for (i = block->topRow; i < block->bottomRow + 1; i++) {
    for (j = block->startCol; j < block->endCol + 1; j++) {
      if (block->coords[i][j] != TNONE) {
        loc = (block->row - block->topRow + i) * COLS + block->col - block->startCol + j;
        grid[loc].cell = block->type;
      }
    }
  }

  if (normalizeBlock == TRUE) {
    setBlockToOrigin(block);
  }
}

void clearBlockFromGrid(Grid *grid, Block block) {
  int i, j, loc;

  for (i = block.topRow; i < block.bottomRow + 1; i++) {
    for (j = block.startCol; j < block.endCol + 1; j++) {
      if (block.coords[i][j] == 1) {
        loc = (block.row - block.topRow + i) * COLS + block.col - block.startCol + j;
        grid[loc].cell = TNONE;
      }
    }
  }
}

bool dropBlock(Grid *grid, Block *block) {
  clearBlockFromGrid(grid, *block);

  if (canMoveInDir(grid, *block, DDROP)) {
    block->row = block->row + 1;
    addBlockToGrid(grid, block, FALSE, FALSE);

    return TRUE;
  }

  addBlockToGrid(grid, block, FALSE, FALSE);
  return FALSE;
}

void rotateBlock(Grid *grid, Block *block) {
  bool canRotate;

  clearBlockFromGrid(grid, *block);
  canRotate = canMoveInDir(grid, *block, DUP);

  if (canRotate == TRUE) {
    copyBlock(block, block->nextOrientation);
    // block->col = block->col + block->startCol;
  }

  addBlockToGrid(grid, block, FALSE, canRotate);
}

void slideBlock(Grid *grid, Block *block, CtrlAction dir) {
  clearBlockFromGrid(grid, *block);

  if (canMoveInDir(grid, *block, dir)) {
    block->col = block->col + (dir == DLEFT ? -1 : 1);
  }

  addBlockToGrid(grid, block, FALSE, FALSE);
}

bool canMoveInDir(Grid *grid, Block block, CtrlAction action) {
  bool notAtEnd = TRUE, notAtSide = TRUE, nothingInWay = TRUE;
  int i, j, loc;
  Block *blockPtr;

#ifdef DEBUG
  char *cc;

  sprintf(cc, "Current row: %d ", block.row);
  wmove(debug, 1, 1);
  wprintw(debug, cc);
  sprintf(cc, "Current col: %d ", block.col);
  wmove(debug, 2, 1);
  wprintw(debug, cc);
  sprintf(cc, "Top row: %d ", block.topRow);
  wmove(debug, 3, 1);
  wprintw(debug, cc);
  sprintf(cc, "Bottom row: %d ", block.bottomRow);
  wmove(debug, 4, 1);
  wprintw(debug, cc);
  sprintf(cc, "Start col: %d ", block.startCol);
  wmove(debug, 5, 1);
  wprintw(debug, cc);
  sprintf(cc, "End col: %d ", block.endCol);
  wmove(debug, 6, 1);
  wprintw(debug, cc);
  wnoutrefresh(debug);

  for (i = 0; i < ROWS; i++) {
    for (j = 0; j < COLS; j++) {
      sprintf(cc, "%d", grid[i * COLS + j].cell);
      wmove(dgrid, 1 + i, 1 + j);
      waddch(dgrid, cc[0]);
    }
  }
  wnoutrefresh(dgrid);
#endif

  switch (action) {
    case DDROP:
      // drop check
      notAtEnd = block.row - block.topRow + block.bottomRow + 1 < ROWS;

      if (notAtEnd == TRUE) {
        for (i = block.bottomRow; i > block.topRow - 1; i--) {
          for (j = block.startCol; j < block.endCol + 1; j++) {
            loc = (block.row + i + 1) * COLS + block.col + j;

            if (loc < ROWS * COLS && grid[loc].cell != TNONE && block.coords[i][j] != TNONE) {
              nothingInWay = false;
              break;
            }
          }
        }
      }

      return notAtEnd && nothingInWay;
    case DHOLD:
      // check if condition is valid
    case DUP:
      // rotation check
      blockPtr = block.nextOrientation;

      for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
          loc = (block.row + i) * COLS + block.col + j;

          if (loc < ROWS * COLS && grid[loc].cell != TNONE && blockPtr->coords[i][j] != TNONE) {
            nothingInWay = FALSE;
            break;
          } else if (loc >= ROWS * COLS) {
            nothingInWay = FALSE;
            break;
          }
        }

        if (nothingInWay == FALSE)
          break;
      }

      return nothingInWay;
    case DDOWN:
      // not used
      break;
    case DLEFT:
      // horizontal move check
      notAtSide = block.col - 1 > -1;

      if (notAtSide == TRUE) {
        for (j = block.startCol; j < block.endCol + 1; j++) {
          for (i = block.topRow; i < block.bottomRow + 1; i++) {
            loc = (block.row + i) * COLS + block.col - block.startCol + j - 1;

            if (loc < ROWS * COLS && grid[loc].cell != TNONE && block.coords[i][j] != TNONE) {
              nothingInWay = false;
              break;
            }
          }
        }
      }

      return notAtSide && nothingInWay;
    case DRIGHT:
      // horizontal move check
      notAtSide = block.col - block.startCol + block.endCol + 1 < COLS;

      if (notAtSide == TRUE) {
        for (j = block.endCol; j > block.startCol - 1; j--) {
          for (i = block.topRow; i < block.bottomRow + 1; i++) {
            loc = (block.row + i) * COLS + block.col - block.startCol + j + 1;

            if (loc < ROWS * COLS && grid[loc].cell != TNONE && block.coords[i][j] != TNONE) {
              nothingInWay = false;
              break;
            }
          }
        }
      }

      return notAtSide && nothingInWay;
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
    case KEY_UP:
    case 'i': // up
      rotateBlock(grid, block);
      break;
    case KEY_RIGHT:
    case 'l': // right
      slideBlock(grid, block, DRIGHT);
      break;
    case KEY_LEFT:
    case 'j': // left
      slideBlock(grid, block, DLEFT);
      break;
    case KEY_DOWN:
    case 'k': // down
      *spd = FAST_SPEED;
      speedChanged = TRUE;
      break;
  }

  if (speedChanged == FALSE)
    *spd = NORMAL_SPEED;
}

bool clearLines(Grid *grid) {
  struct nextRow {
    int row;
    struct nextRow *prev;
  } *rll = NULL, *p = NULL;

  int i, j, loc, pr, cr;
  bool removeRow, linesRemoved = FALSE;

  for (i = 0; i < ROWS; i++) {
    removeRow = TRUE;

    for (j = 0; j < COLS; j++) {
      loc = i * COLS + j;
      if (grid[loc].cell == TNONE) {
        removeRow = FALSE;
        break;
      }
    }

    if (removeRow) {
      for (j = 0; j < COLS; j++) {
        loc = i * COLS + j;
        grid[loc].cell = TNONE;
      }

      pr = i;
      linesRemoved = TRUE;
    } else {
      if (rll == NULL) {
        rll = malloc(sizeof(struct nextRow));
        rll->row = i;
        rll->prev = NULL;
      } else {
        p = rll;
        rll = malloc(sizeof(struct nextRow));
        rll->row = i;
        rll->prev = p;
      }
    }
  }

  p = rll;

  if (linesRemoved == TRUE) {
    while(p != NULL) {
      cr = p->row;

      if (pr >= cr) {
        if (pr != cr) {
          for (j = 0; j < COLS; j++) {
            loc = pr * COLS + j;
            grid[loc].cell = grid[cr * COLS + j].cell;
          }

          pr = pr - 1;
        } else {
          pr = cr;
        }
      }

      p = p->prev;
      free(rll);
      rll = p;
    }
  }

  return linesRemoved;
}

bool gameEnd(Grid *grid, bool creatingNewBlock) {
  int i, j;
  bool gameOver = FALSE;

  if (creatingNewBlock == TRUE) {
    for (i = 0; i < 2; i++) {
      for (j = COLS/2 - 2; j < COLS/2 + 3; j++) {
        if (grid[i * COLS + j].cell != TNONE) {
          gameOver = TRUE;
        }
      }
    }
  } else {
    for (j = 0; j < COLS; j++) {
      if (grid[j].cell != TNONE) {
        gameOver = TRUE;
      }
    }
  }

  return gameOver;
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

  wnoutrefresh(w);
}
