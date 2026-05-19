#pragma once
#include <stdint.h>
// fORMATO: static const uint8_t frame_1[] = {CENTENA, DECENA, UNIDAD};  // Frame
// Pagina para dibujar frmes y animarlos y obtener los .hex:
//  https://jasonacox.github.io/TM1637TinyDisplay/examples/7-segment-animator.html

static const uint8_t frame_1[] = {0x31, 0x00, 0x00};  // Frame 0
static const uint8_t frame_2[] = {0x38, 0x00, 0x00};  // Frame 1
static const uint8_t frame_3[] = {0x18, 0x08, 0x00};  // Frame 2
static const uint8_t frame_4[] = {0x08, 0x0c, 0x00};  // Frame 3
static const uint8_t frame_5[] = {0x00, 0x0e, 0x00};  // Frame 4
static const uint8_t frame_6[] = {0x00, 0x06, 0x01};  // Frame 5
static const uint8_t frame_7[] = {0x00, 0x02, 0x03};  // Frame 6
static const uint8_t frame_8[] = {0x00, 0x00, 0x07};  // Frame 7
static const uint8_t frame_9[] = {0x00, 0x00, 0x0e};  // Frame 8
static const uint8_t frame_10[] = {0x00, 0x08, 0x0c}; // Frame 9
static const uint8_t frame_11[] = {0x00, 0x18, 0x08}; // Frame 10
static const uint8_t frame_12[] = {0x00, 0x38, 0x00}; // Frame 11
static const uint8_t frame_13[] = {0x01, 0x30, 0x00}; // Frame 12
static const uint8_t frame_14[] = {0x21, 0x20, 0x00}; // Frame 13