#include "stm32g4xx.h"

#define MSG_BUFFER_SIZE 256
#define RING_BUFFER_SIZE 256

// dd_esp01.c
// Part 1: a normal buffer with 256 characters
// Part 2: a circular buffer with 256 characters
// Part 3: esp01 abstraction?
// Enjoy :)

/*
// Part 1: a normal buffer with 256 characters
typedef struct {
  char buffer[MSG_BUFFER_SIZE];  // The buffer
  uint16_t idx;                  // Current index
} MsgBuffer_t;
*/

// Part 2: a circular buffer with 256 characters
typedef struct {
  volatile char ring_buffer[RING_BUFFER_SIZE];  // The buffer
  volatile uint16_t idx_write;                  // Current index
  volatile uint16_t idx_read;                   // Current index
} RingBuffer_t;

RingBuffer_t RingBuffer();

void RingBuffer_add(RingBuffer_t *buf, char c);

void RingBuffer_read_last_n(RingBuffer_t *buf, char output[], uint16_t size_text);

char RingBuffer_read_char(RingBuffer_t *buf);

uint8_t RingBuffer_available_char(RingBuffer_t *buf);

void RingBuffer_read_from_start(RingBuffer_t *buf, char output[]);

int RingBuffer_end_with(RingBuffer_t *buf, char text[]);
