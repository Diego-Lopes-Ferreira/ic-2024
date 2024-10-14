// dd_esp01.c
// Part 1: a circular buffer with 256 characters
// Part 2: esp01 abstraction?
#include "ring_buffer.h"

#include "string.h"

// Part 1: a circular buffer with 256 characters
RingBuffer_t RingBuffer() {
  uint16_t i;
  RingBuffer_t buf;
  buf.idx_write = 0;
  buf.idx_read = 0;
  for (i = 0; i < RING_BUFFER_SIZE; i++) { buf.ring_buffer[i] = 0; }
  return buf;
}

void RingBuffer_add(RingBuffer_t *buf, char c) {
  buf->ring_buffer[buf->idx_write] = c;

  buf->idx_write++;
  if (buf->idx_write >= RING_BUFFER_SIZE) { buf->idx_write = 0; }
}

uint8_t RingBuffer_available_char(RingBuffer_t *buf) {
  if (buf->idx_read == buf->idx_write) {
    return 0;
  } else {
    return 1;
  }
}

char RingBuffer_read_char(RingBuffer_t *buf) {
  // Assumes you checked if there is a character available
  char c = buf->ring_buffer[buf->idx_read];

  buf->idx_read++;
  if (buf->idx_read >= RING_BUFFER_SIZE) buf->idx_read = 0;

  return c;
}

// ===== ===== UTILITIES ===== =====

void RingBuffer_read_last_n(RingBuffer_t *buf, char output[], uint16_t size_text) {
  uint16_t idx_text, idx_buff;

  if (buf->idx_write > size_text) {
    idx_buff = buf->idx_write - size_text;
  } else {
    idx_buff = buf->idx_write + RING_BUFFER_SIZE - size_text;
  }

  for (idx_text = 0; idx_text < size_text; idx_text++) {
    output[idx_text] = buf->ring_buffer[idx_buff];

    // TODO: Understand the best way to deal with this
    if (output[idx_text] < 32) { output[idx_text] = '*'; }

    idx_buff++;
    if (idx_buff >= RING_BUFFER_SIZE) { idx_buff = 0; }
  }
}

void RingBuffer_read_from_start(RingBuffer_t *buf, char output[]) {
  uint16_t idx;

  if (buf->idx_write == 0) {
    output[0] = '*';
    return;
  }

  for (idx = 0; idx < buf->idx_write; idx++) {
    output[idx] = buf->ring_buffer[idx];

    // TODO: Understand the best way to deal with this
    //       For now it just replaces the "\r" or "\n" with "*"
    if (output[idx] < 32) { output[idx] = '*'; }
  }
}

int RingBuffer_end_with(RingBuffer_t *buf, char text[]) {
  uint16_t idx_text, idx_buff, size_text;
  char c_text, c_buff;

  size_text = strlen(text);

  if (buf->idx_write > size_text) {
    idx_buff = buf->idx_write - size_text;
  } else {
    idx_buff = buf->idx_write + RING_BUFFER_SIZE - size_text;
  }

  for (idx_text = 0; idx_text < size_text; idx_text++) {
    c_text = text[idx_text];
    c_buff = buf->ring_buffer[idx_buff];
    if (c_buff != c_text) { return 0; }

    idx_buff++;
    if (idx_buff >= RING_BUFFER_SIZE) { idx_buff = 0; }
  }
  return 1;
}

/*
USART1 write messages
> Need to escape 3 caracters: [\], ["] and [,]
> Command should be less then 256
> Command end at "CR-LF"

USART1 get messages (mirror every line: search for \r\n)
> Types of messages
>>> empty line
>>> OK, ERROR, SEND OK, SEND FAIL, SET OK
>>> +<command name>: some data here
>>> > (esp is waiting bytes . . .)
>>> other stuff

if (thing == '>' && ring_buffer.) {
  ~send data~ actually: end the line
} else if (thing == '\n') {
  interpret line
}
interpret line:
if (line is empty) {
  ignore
} else if (line is OK) {
  check for error
} else if (line is ERROR) {
  check for error
} else if (line is "SEND OK") {
  check for error
} else if (line is "SEND FAIL") {
  check for error
} else if (line is "SET OK") {
  check for error
} else if (line start with "+") {
  deal with data
} else {
  ignore?
}
lsi_esp01_send_cmd(char *cmd) {}

*/
