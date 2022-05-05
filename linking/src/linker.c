#include <stdio.h>
#include <string.h>

#include "linker.h"

SymbolBook book;

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        printf("missing argument <filename>\nusage: linker <filename_0> <filename_1> ...\n");
    }

    int elf_n = argc - 1;
    ELF **elfs = malloc(sizeof(ELF *) * elf_n);
    for (int i = 0; i < elf_n; i ++)
    {
        elfs[i] = malloc(sizeof(ELF));
        ParseELF(argv[i + 1], elfs[i]);
    }

    /* init symbol book */
    book.word_n = 0;
    book.indices = HashMapCreate(sizeof(int));

    /* link source ELFs into EOF */
    LinkELFs(elfs, elf_n);

    /* debug print */
    for (int i = 0; i < book.word_n; i ++)
    {
        STEntry *symbol = book.words[i].symbol;
        printf("%s,%d,%d,%s,%ld,%ld\n", symbol->st_name, symbol->bind, symbol->type, symbol->st_shndx, symbol->st_value, symbol->st_size);
    }

    ELF *eof = malloc(sizeof(ELF));
    SHEntry sections[4] = {{0}};
    int sections_n[4];
    for (int i = 0; i < book.word_n; i ++)
    {
        STEntry *symbol = book.words[i].symbol;
        if (strcmp(".text", symbol->st_shndx) == 0)
        {
            sections_n[0] ++;
            sections[0].sh_size += symbol->st_size;
        }
        
        if (strcmp(".rodata", symbol->st_shndx) == 0)
        {
            sections_n[1] ++;
            sections[1].sh_size += symbol->st_size;
        }

        if (strcmp(".data", symbol->st_shndx) == 0)
        {
            sections_n[2] ++;
            sections[2].sh_size += symbol->st_size;
        }

        sections_n[3] ++;
        sections[3].sh_size += 1;
    }

    int sh_count = (sections_n[0] > 0) + (sections_n[1] > 0) + (sections_n[2] > 0) + 1;
    int line_count = 1 + 1 + sh_count;
    int offset = line_count;;
    for (int i = 0; i < 4; i ++)
    {
        if (sections_n[i] > 0)
        {
            sections[i].sh_offset = offset;
            offset += sections[i].sh_size;
            line_count += sections[i].sh_size;
        }
    }

    /* open file and write */
    char *filename = "output.eof.protocol";
    FILE *fp = fopen(filename, "w");
    if (fp == NULL)
    {
        /* TODO: debug print */
        exit(1);
    }

    fprintf(fp, "//line cout\n%d\n\n", line_count);
    fprintf(fp, "//section header count\n%d\n\n", sh_count);
    fprintf(fp, "// section header\n");
    fprintf(fp, "%s,0x%d,%ld,%ld\n", ".text", 0, sections[0].sh_offset, sections[0].sh_size);
    fprintf(fp, "%s,0x%d,%ld,%ld\n", ".data", 0, sections[2].sh_offset, sections[2].sh_size);
    fprintf(fp, "%s,0x%d,%ld,%ld\n\n", ".symtab", 0, sections[3].sh_offset, sections[3].sh_size);

    fprintf(fp, "// .text\n");
    for (int i = 0; i < book.word_n; i ++)
    {
        ELF *elf = book.words[i].elf;
        STEntry *symbol = book.words[i].symbol;
        char *s;
        if (strcmp(".text", symbol->st_shndx) == 0)
        {
            for (int j = 0; j < elf->sh_count; j ++)
            {
                SHEntry *she = &elf->sh_ptr[j];
                if (strcmp(".text", she->sh_name) == 0)
                {
                    for (int k = 0; k < symbol->st_size; k++)
                    {
                        s = elf->buffer[she->sh_offset - 1 + k + symbol->st_value];
                        fprintf(fp, "%s\n", s);
                    }
                }
            }
        }
    }

    fprintf(fp, "\n// .data\n");
    for (int i = 0; i < book.word_n; i ++)
    {
        ELF *elf = book.words[i].elf;
        const STEntry *symbol = book.words[i].symbol;
        char *s;
        if (strcmp(".data", symbol->st_shndx) == 0)
        {
            for (int j = 0; j < elf->sh_count; j ++)
            {
                SHEntry *she = &elf->sh_ptr[j];
                if (strcmp(".data", she->sh_name) == 0)
                {
                    for (int k = 0; k < symbol->st_size; k++)
                    {
                        s = elf->buffer[she->sh_offset - 1 + k + symbol->st_value];
                        fprintf(fp, "%s\n", s);
                    }
                }
            }
        }
    }

    fprintf(fp, "\n// .symtab\n");
    for (int i = 0; i < book.word_n; i ++)
    {
        STEntry *symbol = book.words[i].symbol;
        fprintf(fp, "%s,%d,%d,%s,%ld,%ld\n", symbol->st_name, symbol->bind, symbol->type, symbol->st_shndx, symbol->st_value, symbol->st_size);
    }

    return EXIT_SUCCESS;
}