#ifndef _KILO_H_
#define _KILO_H_

#include "pico/stdlib.h"

#define KILO_VERSION "0.0.1"

typedef struct {
    int size;
    int rsize;  // rendered size
    char *chars;
    char *render;
} erow_t;


typedef struct  {
    int cx, cy;  // cursor position
    int rx; // cursor position on the render string
    int row_offset;
    int col_offset;
    int screen_rows;
    int screen_cols;
    int num_rows;
    int dirty;
    erow_t *row;
    char *filename;
    char statusmsg[80];
    absolute_time_t statusmsg_time;
} editor_config_t;

editor_config_t E;

int kilo_row_cx_to_rx(erow_t *row, int cx);
void kilo_scroll();

#endif
