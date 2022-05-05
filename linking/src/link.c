#include <string.h>

#include "linker.h"
#include "../../common/hashmap.h"

extern SymbolBook book;

/* get global symbol priority 
*       bind    type    shndx       priority
*       -----------------------------------------
*       global  notype  undef       0 - undefined
*       global  object  COMMON      1 - tentative
*       global  object  data/bss    2 - defined
*       global  func    text        2 - defined
*/
static int SymbolPriority(STEntry *ste)
{   
    /* TODO: sanity check - bind must be STB_GLOBAL */

    if (strcmp(ste->st_shndx, "SHN_UNDEF") == 0 && ste->type == STT_NOTYPE)
    {
        /* undefined: symbol referenced but not assigned a storage address */
        return 0;
    }

    if (strcmp(ste->st_shndx, "SHN_COMMON") == 0 && ste->type == STT_OBJECT)
    {
        /* tentative: section can be decided by symbol resolution */
        return 1;
    }

    if ((ste->type == STT_FUNC && strcmp(ste->st_shndx, ".text") == 0) ||
        (ste->type == STT_OBJECT && 
         (strcmp(ste->st_shndx, ".data") == 0 ||
          strcmp(ste->st_shndx, ".rodata") == 0 ||
          strcmp(ste->st_shndx, ".bss") == 0)))
    {
        /* defined */
        return 2;
    }

    /* TODO: debug print - cannot decide symbol priority (sym: ste->st_name) */
    exit(1);
}

/* the resolution of symbols depends on their priorities.
 * defined symbols -> tentative symbols -> undefined symbols */
static int SymbolResolute(STEntry *ste_a, STEntry *ste_b)
{
    int prio_a = SymbolPriority(ste_a);
    int prio_b = SymbolPriority(ste_b);

    if (prio_a == prio_b)
    {
        if (prio_a == 2)
        {
            /* DEBUG print: cannot resolute multiple strong symbols! */
            exit(1);
        }
        /* keep the old symbol if have same priority */
        return 0;
    }

    return prio_a > prio_b;
}

static void SymbolProcess(ELF *elf)
{
    for (int i = 0; i < elf->st_count; i ++)
    {
        STEntry *s = &elf->st_ptr[i];
        switch (s->bind)
        {
            case STB_LOCAL:
            case STB_WEAK:
                break;
            case STB_GLOBAL:
            {
                int *index_ptr = (int *)HashMapGet(book.indices, s->st_name);
                if (index_ptr == NULL)
                {
                    /* unseen symbol name - push the symbol into book */
                    SymbolWord *word = &book.words[book.word_n];
                    word->elf = elf;
                    word->symbol = s;

                    /* add index into map */
                    HashMapSet(book.indices, s->st_name, &book.word_n);
                    book.word_n ++;
                }
                else
                {
                    /* existed symbol name - check symbol priority */
                    SymbolWord *word = &book.words[*index_ptr];
                    /* update symbol book if new symbol has high priority */
                    if (SymbolResolute(s, word->symbol))
                    {
                        word->elf = elf;
                        word->symbol = s;
                    }
                }
                break;
            }
            default:
                /* DEBUG print: symbol bind ste->bind is not supported */
                exit(1);
        }
    }
}

static void SymbolRelocate(ELF *eof)
{
    for (int i = 0; i < book.word_n; i ++)
    {
        SymbolWord word = book.words[i];
        STEntry *s = word.symbol;
        ELF *elf = word.elf;

        /* EOF sections:
         * .text
         * .rodata
         * .data
         * .symtab
         * .rel.text
         * .rel.data
         */
        SHEntry *sh = malloc(6 * sizeof(SHEntry));
        
        if (strcmp(".text", s->st_shndx) == 0)
        {
           sh[0].sh_size ++;
        }
        
        if (strcmp(".rodata", s->st_shndx) == 0)
        {
            sh[1].sh_size ++;
        }

        if (strcmp(".data", s->st_shndx) == 0)
        {
            sh[2].sh_size ++;
        }
    }
}

static uint32_t SymbolGetRunTimeAddress(STEntry *s)
{
    
}

static void SymbolRefRelocate()
{

}

void LinkELFs(ELF **elfs, int elf_n)
{
    for (int i = 0; i < elf_n; i ++)
    {
        ELF *elf = elfs[i];
        SymbolProcess(elf);
    }

    /* sanity check: no undefined symbol should exist now */
    for (int i = 0; i < book.word_n; i ++)
    {
        SymbolWord word = book.words[i];

        switch (SymbolPriority(word.symbol))
        {
            case 0:
                /* DEBUG print: no undefined symbols is allowed: b->key */
                exit(1);
            case 1:
            {
                const char *bss = ".bss"; 
                strcpy(word.symbol->st_shndx, bss);
                word.symbol->st_value = 0;
                break;
            }
            case 2:
            default:
                break;
        }
    }
}