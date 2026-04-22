/************************ (C) COPYLEFT 2018 Merafour *************************
* File Name          : OHID_Layout.c
* Author             : 冷月追风@merafour.blog.163.com
* Version            : V1.0.0
* Last Modified Date : 2024.04.13
* Description        : OHID_Layout.
* Description        : 如无必要,勿增实体.
********************************************************************************
* https://merafour.blog.163.com
* merafour@163.com
* https://github.com/merafour
******************************************************************************/
#ifndef _OHID_LAYOUT_H_
#define _OHID_LAYOUT_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define  TKB_ROWS      (6)
#define  TKB_COLS      (21)

#define  OHID_ROW(pos)      ((pos>>5)&0x07)
#define  OHID_COL(pos)      (pos&0x1F)
#define  OHID_POS(row,col)  (((row<<5)&0xE0) | (col&0x1F)) //(((row&0x07)<<5) | (col&0x1F))

struct OHID_Key_t {
    uint16_t key;
    char name[8];
};

//struct tkb_Layout_matrix_t {  // Half Word, 2B
//    const uint16_t KeyBoard[TKB_ROWS][TKB_COLS];
//    const char* const Names[TKB_ROWS][TKB_COLS];
//};

struct tkb_Byte_matrix_t {  // Byte, 1B
    uint8_t Matrix[TKB_ROWS][TKB_COLS];
};
struct tkb_Half_matrix_t {  // Half Word, 2B
    uint16_t Matrix[TKB_ROWS][TKB_COLS];
};
struct tkb_udef_light_map_t {  // Half Word, 2B
    uint16_t Matrix[(TKB_ROWS*TKB_COLS)>>1];
};
struct tkb_Word_matrix_t {  // Word, 4B
    uint32_t Matrix[TKB_ROWS][TKB_COLS];
};


extern uint8_t OHID_layout_pos(const struct tkb_Half_matrix_t* const Layout, const uint16_t key_value);
//extern const struct tkb_Half_matrix_t* OHID_layout_Get(const uint16_t board_type);
extern const struct tkb_Half_matrix_t* OHID_board_layout_Get(const uint32_t board_id);
extern void OHID_layout(struct tkb_Half_matrix_t* const Layout, const uint16_t board_type);

extern const struct OHID_Key_t  OHID_Keys[];
extern const uint16_t OHID_keys_Size;
extern const char *OHID_key_name(const uint16_t key_value);
uint16_t OHID_board_valid_keys_count(const uint32_t board_id);

#ifdef __cplusplus
}
#endif

#endif // _OHID_LAYOUT_H_

/************************** (C) COPYRIGHT Merafour **************************/
