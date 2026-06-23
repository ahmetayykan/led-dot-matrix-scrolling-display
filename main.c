/*
 * Multi-Dot Matrix Scrolling Message Display
 * An 8051 (AT89C51) microcontroller project that displays
 * a scrolling text message across four 8x8 LED dot-matrix displays.
 */

#include <reg51.h>  /* include 8051 registers */

sbit SH_CP = P1^0;  /* shift register clock pin */
sbit ST_CP = P1^1;  /* shift register latch pin */
sbit DS    = P1^2;  /* shift register data pin */
sbit BTN1  = P3^2;  /* button to make it faster */
sbit BTN2  = P3^3;  /* button to make it slower */

unsigned char scroll_speed;    /* how fast the text moves */
unsigned int  scroll_pos;      /* current position of scrolling */
unsigned char display_buf[32]; /* buffer for 4 matrices, 8 columns each */
unsigned char msg_len;         /* length of the message */
unsigned int total_cols;       /* total number of pixel columns */

/* font table, each character is 6 bytes (5 columns + 1 empty column) */
code unsigned char font[] = {
    0x7E,0x11,0x11,0x11,0x7E,0x00, /* A */
    0x7F,0x49,0x49,0x49,0x36,0x00, /* B */
    0x3E,0x41,0x41,0x41,0x22,0x00, /* C */
    0x7F,0x41,0x41,0x41,0x3E,0x00, /* D */
    0x7F,0x49,0x49,0x49,0x41,0x00, /* E */
    0x7F,0x09,0x09,0x09,0x01,0x00, /* F */
    0x3E,0x41,0x49,0x49,0x7A,0x00, /* G */
    0x7F,0x08,0x08,0x08,0x7F,0x00, /* H */
    0x00,0x41,0x7F,0x41,0x00,0x00, /* I */
    0x20,0x40,0x41,0x3F,0x01,0x00, /* J */
    0x7F,0x08,0x14,0x22,0x41,0x00, /* K */
    0x7F,0x40,0x40,0x40,0x40,0x00, /* L */
    0x7F,0x02,0x0C,0x02,0x7F,0x00, /* M */
    0x7F,0x04,0x08,0x10,0x7F,0x00, /* N */
    0x3E,0x41,0x41,0x41,0x3E,0x00, /* O */
    0x7F,0x09,0x09,0x09,0x06,0x00, /* P */
    0x3E,0x41,0x51,0x21,0x5E,0x00, /* Q */
    0x7F,0x09,0x19,0x29,0x46,0x00, /* R */
    0x26,0x49,0x49,0x49,0x32,0x00, /* S */
    0x01,0x01,0x7F,0x01,0x01,0x00, /* T */
    0x3F,0x40,0x40,0x40,0x3F,0x00, /* U */
    0x1F,0x20,0x40,0x20,0x1F,0x00, /* V */
    0x3F,0x40,0x30,0x40,0x3F,0x00, /* W */
    0x63,0x14,0x08,0x14,0x63,0x00, /* X */
    0x03,0x04,0x78,0x04,0x03,0x00, /* Y */
    0x61,0x51,0x49,0x45,0x43,0x00, /* Z */
    0x20,0x54,0x54,0x54,0x78,0x00, /* a */
    0x7F,0x48,0x44,0x44,0x38,0x00, /* b */
    0x38,0x44,0x44,0x44,0x20,0x00, /* c */
    0x38,0x44,0x44,0x48,0x7F,0x00, /* d */
    0x38,0x54,0x54,0x54,0x18,0x00, /* e */
    0x08,0x7E,0x09,0x01,0x02,0x00, /* f */
    0x18,0xA4,0xA4,0xA4,0x7C,0x00, /* g */
    0x7F,0x08,0x04,0x04,0x78,0x00, /* h */
    0x00,0x44,0x7D,0x40,0x00,0x00, /* i */
    0x40,0x80,0x84,0x7D,0x00,0x00, /* j */
    0x7F,0x10,0x28,0x44,0x00,0x00, /* k */
    0x00,0x41,0x7F,0x40,0x00,0x00, /* l */
    0x7C,0x04,0x18,0x04,0x78,0x00, /* m */
    0x7C,0x08,0x04,0x04,0x78,0x00, /* n */
    0x38,0x44,0x44,0x44,0x38,0x00, /* o */
    0xFC,0x24,0x24,0x24,0x18,0x00, /* p */
    0x18,0x24,0x24,0x24,0xFC,0x00, /* q */
    0x7C,0x08,0x04,0x04,0x08,0x00, /* r */
    0x48,0x54,0x54,0x54,0x24,0x00, /* s */
    0x04,0x3F,0x44,0x40,0x20,0x00, /* t */
    0x3C,0x40,0x40,0x20,0x7C,0x00, /* u */
    0x1C,0x20,0x40,0x20,0x1C,0x00, /* v */
    0x3C,0x40,0x30,0x40,0x3C,0x00, /* w */
    0x44,0x28,0x10,0x28,0x44,0x00, /* x */
    0x1C,0xA0,0xA0,0xA0,0x7C,0x00, /* y */
    0x44,0x64,0x54,0x4C,0x44,0x00, /* z */
    0x3E,0x51,0x49,0x45,0x3E,0x00, /* 0 */
    0x00,0x42,0x7F,0x40,0x00,0x00, /* 1 */
    0x42,0x61,0x51,0x49,0x46,0x00, /* 2 */
    0x21,0x41,0x45,0x4B,0x31,0x00, /* 3 */
    0x18,0x14,0x12,0x7F,0x10,0x00, /* 4 */
    0x27,0x45,0x45,0x45,0x39,0x00, /* 5 */
    0x3C,0x4A,0x49,0x49,0x30,0x00, /* 6 */
    0x01,0x71,0x09,0x05,0x03,0x00, /* 7 */
    0x36,0x49,0x49,0x49,0x36,0x00, /* 8 */
    0x06,0x49,0x49,0x29,0x1E,0x00, /* 9 */
    0x00,0x00,0x00,0x00,0x00,0x00  /* space */
};

#define CHAR_W 6  /* each character is 6 pixels wide */

/* this is the message that scrolls on the display */
code char message[] = "WELCOME TO ANTALYA BILIM UNIVERSITY SPRING 2026 ";

/* this function finds the index of a character in the font table */
unsigned char get_font_index(char c)
{
    if (c >= 'A' && c <= 'Z') return c - 'A';       /* big letters: 0 to 25 */
    if (c >= 'a' && c <= 'z') return c - 'a' + 26;  /* small letters: 26 to 51 */
    if (c >= '0' && c <= '9') return c - '0' + 52;   /* numbers: 52 to 61 */
    return 62;                                        /* space: 62 */
}

/* this function gets the pixel data for one column */
unsigned char get_column(unsigned int col)
{
    unsigned int ci, idx;    /* ci = which character */
    unsigned char cc;        /* cc = which column inside the character */
    ci = col / CHAR_W;      /* find which character we are in */
    cc = col % CHAR_W;      /* find which column of that character */
    if (ci >= msg_len) return 0x00;  /* if outside message, return empty */
    idx = (unsigned int)get_font_index(message[ci]) * CHAR_W + cc;  /* find position in font table */
    return font[idx];        /* return the pixel data */
}

/* this function sends 1 byte to shift register, bit by bit */
void shift_byte(unsigned char dat)
{
    unsigned char i;
    for (i = 0; i < 8; i++) {          /* send 8 bits one by one */
        DS = (dat & 0x80) ? 1 : 0;     /* put the highest bit on data pin */
        dat <<= 1;                      /* move to next bit */
        SH_CP = 0;                      /* clock low */
        SH_CP = 1;                      /* clock high, data shifts in */
    }
}

/* this function updates the shift register outputs */
void latch(void)
{
    ST_CP = 0;  /* latch low */
    ST_CP = 1;  /* latch high, outputs update now */
}

/* this function fills the display buffer with current scroll position */
void fill_buffer(void)
{
    unsigned char c;
    unsigned int src;
    for (c = 0; c < 32; c++) {                      /* for all 32 columns */
        src = (scroll_pos + c) % total_cols;         /* calculate which column from message */
        display_buf[c] = get_column(src);            /* put the column data in buffer */
    }
}

/* this function shows the buffer on 4 matrices using multiplexing */
void refresh_display(void)
{
    unsigned char row, col;
    unsigned char m1, m2, m3, m4;  /* column data for each matrix */

    for (row = 0; row < 8; row++) {        /* scan all 8 rows one by one */
        m1 = 0; m2 = 0; m3 = 0; m4 = 0;   /* clear column data */
        for (col = 0; col < 8; col++) {    /* check 8 columns for each matrix */
            if (display_buf[col] & (1 << row))        /* matrix 1: is this LED on? */
                m1 |= (1 << col);
            if (display_buf[8 + col] & (1 << row))    /* matrix 2: is this LED on? */
                m2 |= (1 << col);
            if (display_buf[16 + col] & (1 << row))   /* matrix 3: is this LED on? */
                m3 |= (1 << col);
            if (display_buf[24 + col] & (1 << row))   /* matrix 4: is this LED on? */
                m4 |= (1 << col);
        }
        shift_byte(m4);   /* send matrix 4 data (last in chain) */
        shift_byte(m3);   /* send matrix 3 data */
        shift_byte(m2);   /* send matrix 2 data */
        shift_byte(m1);   /* send matrix 1 data (first in chain) */
        latch();          /* update shift register outputs */
        P0 = ~(1 << row); /* turn on this row (active low) */
        { unsigned char d; for(d=0;d<200;d++); }  /* small delay so LEDs are visible */
        P0 = 0xFF;        /* turn off all rows */
    }
}

/* main program starts here */
void main(void)
{
    unsigned int refresh_count;  /* counter for refresh cycles */

    scroll_speed = 50;   /* starting scroll speed */
    scroll_pos = 0;      /* start from beginning */

    msg_len = 0;
    while (message[msg_len] != '\0') msg_len++;  /* count how many characters in message */
    total_cols = (unsigned int)msg_len * CHAR_W;  /* calculate total pixel columns */

    fill_buffer();  /* fill the buffer for the first time */

    while (1) {  /* infinite loop */
        refresh_count = 0;
        while (refresh_count < scroll_speed) {   /* refresh the display many times before scrolling */
            refresh_display();                    /* show the LEDs */
            refresh_count++;

            if (BTN1 == 0) {                          /* if speed up button is pressed */
                if (scroll_speed > 10) scroll_speed -= 10;  /* make it faster */
                while (BTN1 == 0);                    /* wait until button is released */
            }
            if (BTN2 == 0) {                          /* if slow down button is pressed */
                if (scroll_speed < 200) scroll_speed += 10;  /* make it slower */
                while (BTN2 == 0);                    /* wait until button is released */
            }
        }
        scroll_pos++;                              /* move text one pixel to the left */
        if (scroll_pos >= total_cols) scroll_pos = 0;  /* if end of message, go back to start */
        fill_buffer();                             /* update the buffer with new position */
    }
}