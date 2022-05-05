#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

#include "linker.h"

static int ReadELF(const char *filename, uint64_t buf_addr)
{
    /* open file and read */
    FILE *fp;
    fp = fopen(filename, "r");
    if (fp == NULL)
    {
        /* TODO: debug print */
        exit(1);
    }

    /* read elf file lines */
    char line[MAX_ELF_FILE_WIDTH];
    int line_counter = 0;

    while (fgets(line, MAX_ELF_FILE_WIDTH, fp) != NULL)
    {
        int len = strlen(line);
        if (line[0] == '\n' || line[0] == '\0' || line[0] == '\r')
            continue;
        if (len > 1 && (line[0] == '/' && line[1] == '/'))
            continue;

        /* to this line, this line is not white and contains information */
        if (line_counter < MAX_ELF_FILE_LENGTH)
        {
            /* store this line to buffer[line_counter] */
            uint64_t addr = buf_addr +
                            line_counter * MAX_ELF_FILE_WIDTH * sizeof(char);
            char *linebuf = (char *)addr;

            int i = 0;
            while (i < len && i < MAX_ELF_FILE_WIDTH)
            {
                if ((line[i] == '\n') ||
                    (line[i] == '\r') ||
                    ((i + 1 < len) && (i + 1 < MAX_ELF_FILE_WIDTH) &&
                     line[i] == '/' && line[i + 1] == '/'))
                {
                    break;
                }
                linebuf[i] = line[i];
                i++;
            }
            linebuf[i] = '\0';
            line_counter++;
        }
        else
        {
            /* TODO: debug print - file exceeds the limit <MAX_ELF_FILE_LENGTH> lines */
            fclose(fp);
            exit(1);
        }
    }
    fclose(fp);

    return line_counter;
}

static void ReadLine(char *line_ptr, char buckets[][MAX_ELF_BUCKET_WIDTH])
{
    const int len = strlen(line_ptr);
    int bucket_i = 0;  /* bucket index */
    int bucket_j = 0;

    for (int i = 0; i <= len; i++)
    {
        if (line_ptr[i] == ',')
        {
            buckets[bucket_i][bucket_j] = '\0';
            bucket_i++;
            bucket_j = 0;
        }
        else
        {
            buckets[bucket_i][bucket_j] = line_ptr[i];
            bucket_j++;
        }
    }
}

static void FillSHEntry(SHEntry *sh_entry, char buckets[][MAX_ELF_BUCKET_WIDTH])
{
    strcpy(sh_entry->sh_name, buckets[0]);
    sh_entry->sh_addr = strtoull(buckets[1], NULL, 10);
    sh_entry->sh_offset = strtoull(buckets[2], NULL, 10);
    sh_entry->sh_size = strtoull(buckets[3], NULL, 10);
}

static SHEntry *GetSHEntry(SHEntry *sh_ptr, int sh_count, char *sh_name)
{
    for (int i = 0; i < sh_count; i ++)
    {
        if (strcmp(sh_name, sh_ptr[i].sh_name) == 0)
        {
            return &sh_ptr[i];
        }
    }
    return NULL;
}

static inline STBind GetSTBind(const char *bind)
{
    if (strcmp(bind, "STB_GLOBAL") == 0)
    {
        return STB_GLOBAL;
    }
    else if (strcmp(bind, "STB_LOCAL") == 0)
    {
        return STB_LOCAL;
    }
    else if (strcmp(bind, "STB_WEAK") == 0)
    {
        return STB_WEAK;
    }
    else
    {
        printf("invalid symbol table bind: %s. bind must be STB_GLOBAL, "
               "STB_LOCAL or STB_WEAK!\n", bind);
        exit(EXIT_FAILURE);
    }
}

static inline STType GetSTType(const char *type)
{
    if (strcmp(type, "STT_NOTYPE") == 0)
    {
        return STT_NOTYPE;
    }
    else if (strcmp(type, "STT_FUNC") == 0)
    {
        return STT_FUNC;
    }
    else if (strcmp(type, "STT_OBJECT") == 0)
    {
        return STT_OBJECT;
    }
    else
    {
        printf("invalid symbol table type: %s. bind must be STT_NOTYPE, "
               "STT_FUNC or STT_OBJECT!\n", type);
        exit(EXIT_FAILURE);
    }
}

static void FillSTEntry(STEntry *st_entry, char buckets[][MAX_ELF_BUCKET_WIDTH])
{
    strcpy(st_entry->st_name, buckets[0]);
    st_entry->bind = GetSTBind(buckets[1]);
    st_entry->type = GetSTType(buckets[2]);
    strcpy(st_entry->st_shndx, buckets[3]);
    st_entry->st_value = strtoull(buckets[4], NULL, 10);
    st_entry->st_size = strtoull(buckets[5], NULL, 10);
}

void ParseELF(const char *filename, ELF *elf)
{
    elf->line_count = ReadELF(filename, (uint64_t)elf->buffer);
    int e_lines = strtol(elf->buffer[0], NULL, 10);
    /* TODO: sanity checks on line count */
    assert(e_lines == elf->line_count);

    for (int i = 0; i < elf->line_count; i++)
    {
        printf("[%d]\t%s\n", i, elf->buffer[i]);
    }

    /* parse ELF's section header table and fill sh_entries */
    int she_count = strtoul(elf->buffer[1], NULL, 10);
    SHEntry *she = malloc(she_count * sizeof(SHEntry));
    for (int i = 0; i < she_count; i++)
    {
        char *line_ptr = (char *)((uint64_t)elf->buffer[2] + i * MAX_ELF_FILE_WIDTH);
        char buckets[MAX_ELF_BUCKET_NUM][MAX_ELF_BUCKET_WIDTH];

        ReadLine(line_ptr, buckets);
        FillSHEntry(&she[i], buckets);
    }
    elf->sh_ptr = she;
    elf->sh_count = she_count;

    for (int i = 0; i < she_count; i++)
    {
        printf("sh_name: %s, sh_addr: %ld, sh_offset: %ld, sh_size: %ld\n", she[i].sh_name, she[i].sh_addr, she[i].sh_offset, she[i].sh_size);
    }

    SHEntry *symtab = GetSHEntry(elf->sh_ptr, she_count, ".symtab");
    int st_offset = symtab->sh_offset - 1;
    int st_count= symtab->sh_size;
    STEntry *st = malloc(st_count * sizeof(STEntry));
    for (int i = 0; i < st_count; i++)
    {
        char *line_ptr = (char *)((uint64_t)elf->buffer[st_offset] + i * MAX_ELF_FILE_WIDTH);
        char buckets[MAX_ELF_BUCKET_NUM][MAX_ELF_BUCKET_WIDTH];

        ReadLine(line_ptr, buckets);
        FillSTEntry(&st[i], buckets);
    }
    elf->st_ptr = st;
    elf->st_count = st_count;
    
    for (int i = 0; i < st_count; i++)
    {
        printf("st_name: %s, st_bind: %d, st_type: %d, st_shndx: %s, st_value: %ld, st_size: %ld\n", st[i].st_name, st[i].bind, st[i].type, st[i].st_shndx, st[i].st_value, st[i].st_size);
    }
}