/*
 ============================================================================
 Name        : spasm.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */
#ifndef BOOL
#define BOOL unsigned char
#define TRUE 1
#define FALSE 0
#endif

#include <stdio.h>
#include <stdlib.h>
#include <elf.h>
#include <memory.h>
#include <unistd.h>
#include <assert.h>

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

size_t write_padding(const size_t size, FILE *file)
{
	const char zero[0x100] = {0};
	size_t result = 0;
	uint32_t iterations = size / sizeof(zero);
	while (iterations)
	{
		result += fwrite(zero, sizeof(zero), 1, file);
		--iterations;
	}

	const size_t left = size % sizeof(zero);
	if (left)
	{
		result += fwrite(zero, left, 1, file);
	}

	return result;
}

/**
 *  @brief Writes ELF executable with the given parameters to the given file.
 *  @param file File to write to
 *  @param text_vaddr Address to load .text segment to
 *  @param text Code to write
 *  @param text_size Size of the given code
 *  @param rodata_vaddr Address to load .rodata segment to
 *  @param rodata Read only data to write
 *  @param rodata_size Size of the given read only data
 *  @param data_vaddr Address to load .data segment to
 *  @param data Writable data to write (== initialized variables)
 *  @param data_size Size of the given writable data
 *  @param bss_vaddr Address to put the zero initialized writable segment at (== 0 initialized variables)
 *  @param bss_size Size to reserve for the zero initialized data
 */
void write_executable(FILE *file,
		unsigned int text_vaddr,
		const char *text, unsigned int text_size,
		unsigned int rodata_vaddr,
		const char *rodata, unsigned int rodata_size,
		unsigned int data_vaddr,
		const char *data, unsigned int data_size,
		unsigned int bss_vaddr,
		unsigned int bss_size)
{
	// Memory layout
	//
	//  .text | .rodata | .data | .bss
	//  ------ --------- --------
	//  PF_X/R    PF_R     PF_W

	const char strtab[] = "\0"
						  ".shstrtab\0"
						  ".text\0"
						  ".rodata\0"
						  ".data\0"
						  ".bss";

	const unsigned int strtab_shstrtab_index = 1;
	const unsigned int strtab_text_index = 11;
	const unsigned int strtab_rodata_index = 17;
	const unsigned int strtab_data_index = 25;
	const unsigned int strtab_bss_index = 31;

	const unsigned int strtab_size = sizeof(strtab);

	//
	// Layout in file
	//
	const uint32_t phdr_count = 4;
	const uint32_t phdr_offset 			= sizeof(Elf32_Ehdr);
	const uint32_t phdr_offset_end 		= phdr_offset + phdr_count * sizeof(Elf32_Phdr);

	const uint32_t content_offset		= phdr_offset_end;

	const uint32_t text_alignment		= 1<<12;
	const uint32_t text_padding_prefix	= padding_for(content_offset, text_vaddr, text_alignment);
	const uint32_t text_offset 			= content_offset + text_padding_prefix;
	const uint32_t text_end				= text_offset + text_size;

	const uint32_t rodata_alignment		= 1<<12;
	const uint32_t rodata_padding_prefix= padding_for(text_end, rodata_vaddr, rodata_alignment);
	const uint32_t rodata_offset 		= text_end + rodata_padding_prefix;
	const uint32_t rodata_end			= rodata_offset + rodata_size;

	const uint32_t data_alignment		= 1<<12;
	const uint32_t data_padding_prefix	= padding_for(rodata_end, data_vaddr, data_alignment);
	const uint32_t data_offset 			= rodata_end + data_padding_prefix;
	const uint32_t data_end				= data_offset + data_size;

	const uint32_t bss_alignment		= 1<<12;

	const uint32_t strtab_alignment		= 0;
	const uint32_t strtab_padding_prefix= 0;
	const uint32_t strtab_offset		= data_end + strtab_padding_prefix;
	const uint32_t strtab_end			= strtab_offset + strtab_size;


	const uint32_t content_offset_end	= strtab_end;

	const uint32_t shdr_count = 6;
	const uint32_t shdr_offset 			= content_offset_end;


	//
	// Elf-header
	//

	Elf32_Ehdr ehdr;
	memset(&ehdr, 0, sizeof(Elf32_Ehdr));

	ehdr.e_ident[EI_MAG0] 		= ELFMAG0;		// Magic bytes
	ehdr.e_ident[EI_MAG1] 		= ELFMAG1;
	ehdr.e_ident[EI_MAG2] 		= ELFMAG2;
	ehdr.e_ident[EI_MAG3] 		= ELFMAG3;
	ehdr.e_ident[EI_CLASS] 		= ELFCLASS32;	// 32bit application
	ehdr.e_ident[EI_DATA] 		= ELFDATA2LSB;	// Two's complement, little-endian
	ehdr.e_ident[EI_VERSION] 	= EV_CURRENT;
	ehdr.e_ident[EI_OSABI]		= ELFOSABI_LINUX; // Linux target

	ehdr.e_type 				= ET_EXEC; 		// Executable file
	ehdr.e_machine				= EM_386;		// i386 arch
	ehdr.e_version				= EV_CURRENT;
	ehdr.e_entry				= text_vaddr; // Entry point (virtual addr.)
	ehdr.e_phoff				= phdr_offset; // Program header table offset (file offset)
	ehdr.e_shoff				= shdr_offset; // Section header table offset (file offset)
	ehdr.e_ehsize 				= sizeof(Elf32_Ehdr);
	ehdr.e_phentsize 			= sizeof(Elf32_Phdr);
	ehdr.e_phnum 				= phdr_count;
	ehdr.e_shentsize 			= sizeof(Elf32_Shdr);
	ehdr.e_shnum 				= shdr_count;
	ehdr.e_shstrndx 			= 1; // Section header index of string table

	//
	// Elf program headers
	//

	// .text segment program header

	Elf32_Phdr phdr_text;
	memset(&phdr_text, 0, sizeof(Elf32_Phdr));

	phdr_text.p_type 			= PT_LOAD;
	phdr_text.p_offset			= text_offset; // Offset to first byte of segment (file offset)
	phdr_text.p_vaddr			= text_vaddr;  // Virtual address
	phdr_text.p_paddr			= 0;
	phdr_text.p_align			= text_alignment;
	phdr_text.p_filesz			= text_size; // Size in file image
	phdr_text.p_memsz			= text_size; // Size in memory image
	phdr_text.p_flags			= PF_X | PF_R; // Executable & Readable

	// .rodata segment program header

	Elf32_Phdr phdr_rodata;
	memset(&phdr_rodata, 0, sizeof(Elf32_Phdr));

	phdr_rodata.p_type 			= PT_LOAD;
	phdr_rodata.p_offset		= rodata_offset; // Offset to first byte of segment (file offset)
	phdr_rodata.p_vaddr			= rodata_vaddr;  // Virtual address
	phdr_rodata.p_paddr			= 0;
	phdr_rodata.p_align			= rodata_alignment;
	phdr_rodata.p_filesz		= rodata_size; // Size in file image
	phdr_rodata.p_memsz			= rodata_size; // Size in memory image
	phdr_rodata.p_flags			= PF_R; // Read-only

	// .data segment program header

	Elf32_Phdr phdr_data;
	memset(&phdr_data, 0, sizeof(Elf32_Phdr));

	phdr_data.p_type 			= PT_LOAD;
	phdr_data.p_offset			= data_offset; // Offset to first byte of segment (file offset)
	phdr_data.p_vaddr			= data_vaddr;  // Virtual address
	phdr_data.p_paddr			= 0;
	phdr_data.p_align			= data_alignment;
	phdr_data.p_filesz			= data_size; // Size in file image
	phdr_data.p_memsz			= data_size; // Size in memory image
	phdr_data.p_flags			= PF_R | PF_W; // Read & Write

	// .bss segment program header

	Elf32_Phdr phdr_bss;
	memset(&phdr_bss, 0, sizeof(Elf32_Phdr));

	phdr_bss.p_type 			= PT_LOAD;
	phdr_bss.p_offset			= 0; // Offset to first byte of segment (file offset)
	phdr_bss.p_vaddr			= bss_vaddr;  // Virtual address
	phdr_bss.p_paddr			= 0;
	phdr_bss.p_align			= bss_alignment;
	phdr_bss.p_filesz			= 0; // Size in file image
	phdr_bss.p_memsz			= bss_size; // Size in memory image
	phdr_bss.p_flags			= PF_R | PF_W; // Read & Write

	//
	// Elf segment headers
	//

	// Empty section header

	Elf32_Shdr shdr_null;
	memset(&shdr_null, 0, sizeof(Elf32_Shdr));

	// .strtab (String table)

	Elf32_Shdr shdr_strtab;
	memset(&shdr_strtab, 0, sizeof(Elf32_Shdr));

	shdr_strtab.sh_name 		= strtab_shstrtab_index; // Offset into naming table
	shdr_strtab.sh_type 		= SHT_STRTAB;
	shdr_strtab.sh_flags		= 0;
	shdr_strtab.sh_addr			= 0; // Not loaded into virtual space
	shdr_strtab.sh_addralign	= strtab_alignment;
	shdr_strtab.sh_offset		= strtab_offset;
	shdr_strtab.sh_size			= strtab_size;

	// .text (Code segment)

	Elf32_Shdr shdr_text;
	memset(&shdr_text, 0, sizeof(Elf32_Shdr));

	shdr_text.sh_name 			= strtab_text_index; // Offset into naming table
	shdr_text.sh_type 			= SHT_PROGBITS;
	shdr_text.sh_flags			= SHF_ALLOC | SHF_EXECINSTR;
	shdr_text.sh_addr			= text_vaddr;
	shdr_text.sh_addralign		= text_alignment;
	shdr_text.sh_offset			= text_offset;
	shdr_text.sh_size			= text_size;

	// .rodata (Read only data segment)

	Elf32_Shdr shdr_rodata;
	memset(&shdr_rodata, 0, sizeof(Elf32_Shdr));

	shdr_rodata.sh_name 		= strtab_rodata_index; // Offset into naming table
	shdr_rodata.sh_type 		= SHT_PROGBITS;
	shdr_rodata.sh_flags		= SHF_ALLOC;
	shdr_rodata.sh_addr			= rodata_vaddr;
	shdr_rodata.sh_addralign	= rodata_alignment;
	shdr_rodata.sh_offset		= rodata_offset;
	shdr_rodata.sh_size			= rodata_size;

	// .data (Writable data segment)

	Elf32_Shdr shdr_data;
	memset(&shdr_data, 0, sizeof(Elf32_Shdr));

	shdr_data.sh_name 			= strtab_data_index; // Offset into naming table
	shdr_data.sh_type 			= SHT_PROGBITS;
	shdr_data.sh_flags			= SHF_ALLOC | SHF_WRITE;
	shdr_data.sh_addr			= data_vaddr;
	shdr_data.sh_addralign		= data_alignment;
	shdr_data.sh_offset			= data_offset;
	shdr_data.sh_size			= data_size;

	// .bss (Zero initialized variables)

	Elf32_Shdr shdr_bss;
	memset(&shdr_bss, 0, sizeof(Elf32_Shdr));

	shdr_bss.sh_name 			= strtab_bss_index; // Offset into naming table
	shdr_bss.sh_type 			= SHT_NOBITS;
	shdr_bss.sh_flags			= SHF_ALLOC | SHF_WRITE;
	shdr_bss.sh_addr			= bss_vaddr;
	shdr_bss.sh_addralign		= bss_alignment;
	shdr_bss.sh_offset			= 0;
	shdr_bss.sh_size			= bss_size;


	//
	// Write to file
	//

	// File layout
	// elfhdr | elfphdr... | text | rodata | data | strtab | shdr strings | shdr...
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

int main(void) {
	const char rodata[] = "Hello World.\n";

	const char text[] = {	0xba, 0x0e, 0x00, 0x00, 0x00,	// mov    edx,0xc
							0xb9, 0xD6, 0x00, 0x09, 0x10,   // mov    ecx,0x10090000
							0xbb, 0x01, 0x00, 0x00, 0x00,   // mov    ebx,0x1
							0xb8, 0x04, 0x00, 0x00, 0x00,   // mov    eax,0x4
							0xcd, 0x80,		               	// int    0x80
							0xbb, 0x00, 0x00, 0x00, 0x00,   // mov    ebx,0x0
							0xb8, 0x01, 0x00, 0x00, 0x00,   // mov    eax,0x1
							0xcd, 0x80 };	               	// int    0x80


	const char data[] = "jajajaj";
	FILE *file = fopen("test.bin", "wb");
	write_executable(file,
			0x080480B4,
			text, sizeof(text),
			0x100900D6,
			rodata, sizeof(rodata),
			0x180D80E4,
			data, sizeof(data),
			0x20120000,
			1);
	fclose(file);

	return EXIT_SUCCESS;
}
