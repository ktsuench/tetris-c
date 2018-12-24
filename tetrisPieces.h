#include <stdlib.h>

#define NUM_BLOCKS 7

typedef enum TetrisType {
  TNONE, TT, TL, TJ, TI, TS, TZ, TO
} CellType;

typedef enum Orientation {
  OUP, ORIGHT, ODOWN, OLEFT
} Rotation;

typedef struct TetrisBlock {
  unsigned int coords[4][4];
  CellType type;
  Rotation orientation;
  unsigned int topRow;
  unsigned int bottomRow;
  struct TetrisBlock *nextOrientation;
  unsigned int row;
  unsigned int col;
} Block;

Block TTLEFT = {
  {
    {0,1,0,0},
    {0,1,1,0},
    {0,1,0,0},
    {0,0,0,0}
  }, TT, OLEFT, 1, 3, NULL
};
Block TTDOWN = {
  {
    {0,0,0,0},
    {0,0,1,0},
    {0,1,1,1},
    {0,0,0,0}
  }, TT, ODOWN, 2, 3,
  (struct TetrisBlock *) &TTLEFT
};
Block TTRIGHT = {
  {
    {0,0,0,1},
    {0,0,1,1},
    {0,0,0,1},
    {0,0,0,0}
  }, TT, ORIGHT, 1, 3,
  (struct TetrisBlock *) &TTDOWN
};
Block TTUP = {
  {
    {0,1,1,1},
    {0,0,1,0},
    {0,0,0,0},
    {0,0,0,0}
  }, TT, OUP, 1, 2,
  (struct TetrisBlock *) &TTRIGHT
};
Block TLLEFT = {
  {
    {0,1,0,0},
    {0,1,0,0},
    {0,1,1,0},
    {0,0,0,0}
  }, TL, OLEFT, 1, 3, NULL
};
Block TLDOWN = {
  {
    {0,0,0,0},
    {0,0,0,1},
    {0,1,1,1},
    {0,0,0,0}
  }, TL, ODOWN, 2, 3,
  (struct TetrisBlock *) &TLLEFT
};
Block TLRIGHT = {
  {
    {0,0,1,1},
    {0,0,0,1},
    {0,0,0,1},
    {0,0,0,0}
  }, TL, ORIGHT, 1, 3,
  (struct TetrisBlock *) &TLDOWN
};
Block TLUP = {
  {
    {0,1,1,1},
    {0,1,0,0},
    {0,0,0,0},
    {0,0,0,0}
  }, TL, OUP, 1, 2,
  (struct TetrisBlock *) &TLRIGHT
};
Block TJLEFT = {
  {
    {0,1,1,0},
    {0,1,0,0},
    {0,1,0,0},
    {0,0,0,0}
  }, TJ, OLEFT, 1, 3, NULL
};
Block TJDOWN = {
  {
    {0,0,0,0},
    {0,1,0,0},
    {0,1,1,1},
    {0,0,0,0}
  }, TJ, ODOWN, 2, 3,
  (struct TetrisBlock *) &TJLEFT
};
Block TJRIGHT = {
  {
    {0,0,0,1},
    {0,0,0,1},
    {0,0,1,1},
    {0,0,0,0}
  }, TJ, ORIGHT, 1, 3,
  (struct TetrisBlock *) &TJDOWN
};
Block TJUP = {
  {
    {0,1,1,1},
    {0,0,0,1},
    {0,0,0,0},
    {0,0,0,0}
  }, TJ, OUP, 1, 2,
  (struct TetrisBlock *) &TJRIGHT
};
Block TIRIGHT = {
  {
    {0,1,0,0},
    {0,1,0,0},
    {0,1,0,0},
    {0,1,0,0}
  }, TI, ORIGHT, 1, 4, NULL
};
Block TIUP = {
  {
    {1,1,1,1},
    {0,0,0,0},
    {0,0,0,0},
    {0,0,0,0}
  }, TI, OUP, 1, 1,
  (struct TetrisBlock *) &TIRIGHT
};
Block TSRIGHT = {
  {
    {0,0,1,0},
    {0,0,1,1},
    {0,0,0,1},
    {0,0,0,0}
  }, TS, ORIGHT, 1, 3, NULL
};
Block TSUP = {
  {
    {0,0,1,1},
    {0,1,1,0},
    {0,0,0,0},
    {0,0,0,0}
  }, TS, OUP, 1, 2,
  (struct TetrisBlock *) &TSRIGHT
};
Block TZRIGHT = {
  {
    {0,0,0,1},
    {0,0,1,1},
    {0,0,1,0},
    {0,0,0,0}
  }, TZ, ORIGHT, 1, 2, NULL
};
Block TZUP = {
  {
    {0,1,1,0},
    {0,0,1,1},
    {0,0,0,0},
    {0,0,0,0}
  }, TZ, OUP, 1, 2,
  (struct TetrisBlock *) &TZRIGHT
};
Block TOUP = {
  {
    {0,1,1,0},
    {0,1,1,0},
    {0,0,0,0},
    {0,0,0,0}
  }, TO, OUP, 1, 2, NULL
};

typedef struct TetrisBlockPtr {
  Block *ptr;
} BlockPtr;

/* Block coordinates */
BlockPtr Shapes[NUM_BLOCKS] = {
  &TTUP, &TLUP, &TJUP, &TIUP, &TSUP, &TZUP, &TOUP
};