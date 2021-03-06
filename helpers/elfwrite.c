/*
 * Copyright (C) 2011, Stefan Hacker
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include "elfwrite.h"

#include <elf.h>
#include <memory.h>
#include <unistd.h>


/**
 * @brief Creates a padding for the given addr. that ensures that (addr. + padding - reference) % alignment == 0
 * @param addr Addr. to create padding for
 * @param reference Reference to align relative to
 * @param alignment Alignment (Must be bigger 0 and power of two)
 */
uint32_t padding_for(const uint32_t addr, const uint32_t reference, const uint32_t alignment)
{
    const uint32_t mask = alignment - 1;
    const uint32_t addr_end = addr & mask;
    const uint32_t ref_end = reference & mask;

    if (addr_end < ref_end)
        return ref_end - addr_end;
    else if (addr_end > ref_end)
        return alignment - addr_end + ref_end;

    return 0;
}


/**
 * @brief Writes size bytes of padding to the given file
 * @param size Number of 0 bytes to write
 * @file File to write to
 */
size_t write_padding(const size_t size, FILE *file)
{
    const char zero[0x100] = {0};
    size_t result = 0;
    uint32_t iterations = size / sizeof(zero);
    const size_t left = size % sizeof(zero);

    while (iterations)
    {
        result += fwrite(zero, sizeof(zero), 1, file);
        --iterations;
    }

    if (left)
    {
        result += fwrite(zero, left, 1, file);
    }

    return result;
}


void elf_write(FILE *file,
        uint32_t entry_point,
        uint32_t text_vaddr,
        const unsigned char *text, size_t text_size,
        uint32_t rodata_vaddr,
        const unsigned char *rodata, size_t rodata_size,
        uint32_t data_vaddr,
        const unsigned char *data, size_t data_size,
        uint32_t bss_vaddr,
        size_t bss_size)
{
    /*
     *  Memory layout
     *  .text | .rodata | .data | .bss
     *  ------ --------- --------
     *  PF_X/R    PF_R     PF_W
     */

    const char strtab[] = "\0"
                          ".shstrtab\0"
                          ".text\0"
                          ".rodata\0"
                          ".data\0"
                          ".bss";

    const uint32_t strtab_shstrtab_index = 1;
    const uint32_t strtab_text_index = 11;
    const uint32_t strtab_rodata_index = 17;
    const uint32_t strtab_data_index = 25;
    const uint32_t strtab_bss_index = 31;

    const uint32_t strtab_size = sizeof(strtab);

    /*
     * Layout in file
     */
    const uint32_t phdr_count = 4;
    const uint32_t phdr_offset              = sizeof(Elf32_Ehdr);
    const uint32_t phdr_offset_end          = phdr_offset + phdr_count * sizeof(Elf32_Phdr);

    const uint32_t content_offset           = phdr_offset_end;

    const uint32_t text_alignment           = 1<<12;
    const uint32_t text_padding_prefix      = padding_for(content_offset, text_vaddr, text_alignment);
    const uint32_t text_offset              = content_offset + text_padding_prefix;
    const uint32_t text_end                 = text_offset + text_size;

    const uint32_t rodata_alignment         = 1<<12;
    const uint32_t rodata_padding_prefix    = padding_for(text_end, rodata_vaddr, rodata_alignment);
    const uint32_t rodata_offset            = text_end + rodata_padding_prefix;
    const uint32_t rodata_end               = rodata_offset + rodata_size;

    const uint32_t data_alignment           = 1<<12;
    const uint32_t data_padding_prefix      = padding_for(rodata_end, data_vaddr, data_alignment);
    const uint32_t data_offset              = rodata_end + data_padding_prefix;
    const uint32_t data_end                 = data_offset + data_size;

    const uint32_t bss_alignment            = 1<<12;

    const uint32_t strtab_alignment         = 0;
    const uint32_t strtab_padding_prefix    = 0;
    const uint32_t strtab_offset            = data_end + strtab_padding_prefix;
    const uint32_t strtab_end               = strtab_offset + strtab_size;


    const uint32_t content_offset_end       = strtab_end;

    const uint32_t shdr_count               = 6;
    const uint32_t shdr_offset              = content_offset_end;

    Elf32_Ehdr ehdr;
    Elf32_Phdr phdr_text;
    Elf32_Phdr phdr_rodata;
    Elf32_Phdr phdr_data;
    Elf32_Phdr phdr_bss;

    Elf32_Shdr shdr_null;
    Elf32_Shdr shdr_strtab;
    Elf32_Shdr shdr_text;
    Elf32_Shdr shdr_rodata;
    Elf32_Shdr shdr_data;
    Elf32_Shdr shdr_bss;

    /*
     * Elf-header
     */
    memset(&ehdr, 0, sizeof(Elf32_Ehdr));

    ehdr.e_ident[EI_MAG0]       = ELFMAG0;         /* Magic bytes */
    ehdr.e_ident[EI_MAG1]       = ELFMAG1;
    ehdr.e_ident[EI_MAG2]       = ELFMAG2;
    ehdr.e_ident[EI_MAG3]       = ELFMAG3;
    ehdr.e_ident[EI_CLASS]      = ELFCLASS32;      /* 32bit application */
    ehdr.e_ident[EI_DATA]       = ELFDATA2LSB;     /* Two's complement, little-endian */
    ehdr.e_ident[EI_VERSION]    = EV_CURRENT;
    ehdr.e_ident[EI_OSABI]      = ELFOSABI_LINUX;  /* Linux target */

    ehdr.e_type                 = ET_EXEC;         /* Executable file */
    ehdr.e_machine              = EM_386;          /* i386 arch */
    ehdr.e_version              = EV_CURRENT;
    ehdr.e_entry                = entry_point;      /* Entry point (virtual addr.) */
    ehdr.e_phoff                = phdr_offset;     /* Program header table offset (file offset) */
    ehdr.e_shoff                = shdr_offset;     /* Section header table offset (file offset) */
    ehdr.e_ehsize               = sizeof(Elf32_Ehdr);
    ehdr.e_phentsize            = sizeof(Elf32_Phdr);
    ehdr.e_phnum                = phdr_count;
    ehdr.e_shentsize            = sizeof(Elf32_Shdr);
    ehdr.e_shnum                = shdr_count;
    ehdr.e_shstrndx             = 1;               /* Section header index of string table */

    /*
     * Elf program headers
     */

    /* .text segment program header */

    memset(&phdr_text, 0, sizeof(Elf32_Phdr));

    phdr_text.p_type            = PT_LOAD;
    phdr_text.p_offset          = text_offset; /* Offset to first byte of segment (file offset) */
    phdr_text.p_vaddr           = text_vaddr;  /* Virtual address */
    phdr_text.p_paddr           = 0;
    phdr_text.p_align           = text_alignment;
    phdr_text.p_filesz          = text_size; /* Size in file image */
    phdr_text.p_memsz           = text_size; /* Size in memory image */
    phdr_text.p_flags           = PF_X | PF_R; /* Executable & Readable */

    /* .rodata segment program header */

    memset(&phdr_rodata, 0, sizeof(Elf32_Phdr));

    phdr_rodata.p_type          = PT_LOAD;
    phdr_rodata.p_offset        = rodata_offset; /* Offset to first byte of segment (file offset) */
    phdr_rodata.p_vaddr         = rodata_vaddr;  /* Virtual address */
    phdr_rodata.p_paddr         = 0;
    phdr_rodata.p_align         = rodata_alignment;
    phdr_rodata.p_filesz        = rodata_size; /* Size in file image */
    phdr_rodata.p_memsz         = rodata_size; /* Size in memory image */
    phdr_rodata.p_flags         = PF_R; /* Read-only */

    /* .data segment program header */

    memset(&phdr_data, 0, sizeof(Elf32_Phdr));

    phdr_data.p_type            = PT_LOAD;
    phdr_data.p_offset          = data_offset; /* Offset to first byte of segment (file offset) */
    phdr_data.p_vaddr           = data_vaddr;  /* Virtual address */
    phdr_data.p_paddr           = 0;
    phdr_data.p_align           = data_alignment;
    phdr_data.p_filesz          = data_size; /* Size in file image */
    phdr_data.p_memsz           = data_size; /* Size in memory image */
    phdr_data.p_flags           = PF_R | PF_W; /* Read & Write */

    /* .bss segment program header */

    memset(&phdr_bss, 0, sizeof(Elf32_Phdr));

    phdr_bss.p_type             = PT_LOAD;
    phdr_bss.p_offset           = 0; /* Offset to first byte of segment (file offset) */
    phdr_bss.p_vaddr            = bss_vaddr;  /* Virtual address */
    phdr_bss.p_paddr            = 0;
    phdr_bss.p_align            = bss_alignment;
    phdr_bss.p_filesz           = 0; /* Size in file image */
    phdr_bss.p_memsz            = bss_size; /* Size in memory image */
    phdr_bss.p_flags            = PF_R | PF_W; /* Read & Write */

    /*
     * Elf segment headers
     */

    /* Empty section header */

    memset(&shdr_null, 0, sizeof(Elf32_Shdr));

    /* .strtab (String table) */

    memset(&shdr_strtab, 0, sizeof(Elf32_Shdr));

    shdr_strtab.sh_name         = strtab_shstrtab_index; /* Offset into naming table*/
    shdr_strtab.sh_type         = SHT_STRTAB;
    shdr_strtab.sh_flags        = 0;
    shdr_strtab.sh_addr         = 0; /* Not loaded into virtual space*/
    shdr_strtab.sh_addralign    = strtab_alignment;
    shdr_strtab.sh_offset       = strtab_offset;
    shdr_strtab.sh_size         = strtab_size;

    /* .text (Code segment)*/

    memset(&shdr_text, 0, sizeof(Elf32_Shdr));

    shdr_text.sh_name           = strtab_text_index; /* Offset into naming table*/
    shdr_text.sh_type           = SHT_PROGBITS;
    shdr_text.sh_flags          = SHF_ALLOC | SHF_EXECINSTR;
    shdr_text.sh_addr           = text_vaddr;
    shdr_text.sh_addralign      = text_alignment;
    shdr_text.sh_offset         = text_offset;
    shdr_text.sh_size           = text_size;

    /* .rodata (Read only data segment)*/

    memset(&shdr_rodata, 0, sizeof(Elf32_Shdr));

    shdr_rodata.sh_name         = strtab_rodata_index; /* Offset into naming table*/
    shdr_rodata.sh_type         = SHT_PROGBITS;
    shdr_rodata.sh_flags        = SHF_ALLOC;
    shdr_rodata.sh_addr         = rodata_vaddr;
    shdr_rodata.sh_addralign    = rodata_alignment;
    shdr_rodata.sh_offset       = rodata_offset;
    shdr_rodata.sh_size         = rodata_size;

    /* .data (Writable data segment) */

    memset(&shdr_data, 0, sizeof(Elf32_Shdr));

    shdr_data.sh_name           = strtab_data_index; /* Offset into naming table*/
    shdr_data.sh_type           = SHT_PROGBITS;
    shdr_data.sh_flags          = SHF_ALLOC | SHF_WRITE;
    shdr_data.sh_addr           = data_vaddr;
    shdr_data.sh_addralign      = data_alignment;
    shdr_data.sh_offset         = data_offset;
    shdr_data.sh_size           = data_size;

    /* .bss (Zero initialized variables) */

    memset(&shdr_bss, 0, sizeof(Elf32_Shdr));

    shdr_bss.sh_name            = strtab_bss_index; /* Offset into naming table*/
    shdr_bss.sh_type            = SHT_NOBITS;
    shdr_bss.sh_flags           = SHF_ALLOC | SHF_WRITE;
    shdr_bss.sh_addr            = bss_vaddr;
    shdr_bss.sh_addralign       = bss_alignment;
    shdr_bss.sh_offset          = 0;
    shdr_bss.sh_size            = bss_size;


    /*
     * Write to file
     */

    /*
     * File layout
     *
     * elfhdr | elfphdr... | text | rodata | data | strtab | shdr strings | shdr...
     */

    fwrite(&ehdr, sizeof(ehdr), 1, file);

    fwrite(&phdr_text, sizeof(phdr_text), 1, file);
    fwrite(&phdr_rodata, sizeof(phdr_rodata), 1, file);
    fwrite(&phdr_data, sizeof(phdr_data), 1, file);
    fwrite(&phdr_bss, sizeof(phdr_bss), 1, file);

    write_padding(text_padding_prefix, file);
    fwrite(text, text_size, 1, file);

    write_padding(rodata_padding_prefix, file);
    fwrite(rodata, rodata_size, 1, file);

    write_padding(data_padding_prefix, file);
    fwrite(data, data_size, 1, file);

    write_padding(strtab_padding_prefix, file);
    fwrite(strtab, strtab_size, 1, file);

    fwrite(&shdr_null, sizeof(shdr_null), 1, file);
    fwrite(&shdr_strtab, sizeof(shdr_strtab), 1, file);
    fwrite(&shdr_text, sizeof(shdr_text), 1, file);
    fwrite(&shdr_rodata, sizeof(shdr_rodata), 1, file);
    fwrite(&shdr_data, sizeof(shdr_data), 1, file);
    fwrite(&shdr_bss, sizeof(shdr_bss), 1, file);
}

void elf_optimize_alignment(
        uint32_t base_vaddr,
        uint32_t text_size,
        uint32_t rodata_size,
        uint32_t data_size,
        uint32_t *text_vaddr,
        uint32_t *rodata_vaddr,
        uint32_t *data_vaddr,
        uint32_t *bss_vaddr)
{
    const uint32_t text_file_offset = sizeof(Elf32_Ehdr) + 4 * sizeof(Elf32_Phdr);
    const uint32_t rodata_file_offset = text_file_offset + text_size;
    const uint32_t data_file_offset = rodata_file_offset + rodata_size;

    *text_vaddr = base_vaddr + padding_for(base_vaddr, text_file_offset, 0x1000);
    *rodata_vaddr = *text_vaddr + text_size + padding_for(*text_vaddr + text_size, rodata_file_offset, 0x1000);
    *data_vaddr = *rodata_vaddr + rodata_size + padding_for(*rodata_vaddr + rodata_size, data_file_offset, 0x1000);
    *bss_vaddr = *data_vaddr + data_size + padding_for(*data_vaddr + data_size, 0, 0x1000);
}

