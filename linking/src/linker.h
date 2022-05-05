#ifndef LINKER_GUARD
#define LINKER_GUARD

#include <stdint.h>
#include <stdlib.h>

#include "../../common/hashmap.h"

#define MAX_CHAR_SECTION_NAME (32)

/* section header entry */
typedef struct {
    char sh_name[MAX_CHAR_SECTION_NAME];
    uint64_t sh_addr;  /* section run-time address */
    uint64_t sh_offset;
    uint64_t sh_size;
} SHEntry;

typedef enum {
    STB_LOCAL,
    STB_GLOBAL,
    STB_WEAK,
} STBind;

typedef enum {
    STT_NOTYPE,
    STT_OBJECT,
    STT_FUNC,
} STType;

#define MAX_CHAR_SYMBOL_NAME (64)

/* symbol table entry */
typedef struct {
    char st_name[MAX_CHAR_SYMBOL_NAME];
    STBind bind;
    STType type;
    char st_shndx[MAX_CHAR_SECTION_NAME];
    uint64_t st_value;  /* in-section offset */
    uint64_t st_size;   /* count of lines of symbol */
} STEntry;

/* relocation types */
typedef struct {
    RELO_ABS;   /* use a 32-bit absolute address to relocate a reference */
    RELO_RELA;  /* use a 32-bit PC-relative address to relocate a reference */
    RELO_RELA_PLT;
} ReloType;

/* relocation entry */
typedef struct {
    unsigned int offset_r;
    unsigned int offset_l;
    ReloType type;
    unsigned int sym_index;
    int addon;
} ReloEntry;

#define MAX_ELF_FILE_LENGTH (64)
#define MAX_ELF_FILE_WIDTH (128)

typedef struct {
    char buffer[MAX_ELF_FILE_LENGTH][MAX_ELF_FILE_WIDTH];
    uint64_t line_count;

    SHEntry *sh_ptr;  /* dynamically allocated */
    uint64_t sh_count;

    STEntry *st_ptr;  /* dynamically allocated */
    uint64_t st_count;
} ELF;

#define MAX_ELF_BUCKET_NUM (8)
#define MAX_ELF_BUCKET_WIDTH (32)

#define MAX_SYMBOL_NUM (1024)

/* SymbolWord does not take ownership of the memory pointed by [elf, symbol]. */
typedef struct {
    ELF *elf;
    STEntry *symbol;
} SymbolWord;

typedef struct {
    SymbolWord words[MAX_SYMBOL_NUM];
    int word_n;

    HashMap *indices;
} SymbolBook;

void ParseELF(const char *filename, ELF *elf);
void LinkELFs(ELF **elfs, int elf_n);

#endif 