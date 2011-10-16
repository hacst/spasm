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

void write_executable(FILE *file,
		const char *text, unsigned int text_size,
		const char *rodata, unsigned int rodata_size,
		const char *data, unsigned int data_size,
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
	const uint32_t phdr_count = 3;
	const uint32_t phdr_offset 			= sizeof(Elf32_Ehdr);
	const uint32_t phdr_offset_end 		= phdr_offset + phdr_count * sizeof(Elf32_Phdr);

	const uint32_t content_offset		= phdr_offset_end;
	const uint32_t text_offset 			= content_offset;
	const uint32_t rodata_offset 		= text_offset + text_size;
	const uint32_t data_offset 			= rodata_offset + rodata_size;
	const uint32_t strtab_offset		= data_offset + data_size;
	const uint32_t content_offset_end	= strtab_offset + strtab_size;

	const uint32_t shdr_count = 6;
	const uint32_t shdr_offset 			= content_offset_end;

	//
	// Layout in virtual memory
	//
	const uint32_t virtual_entry_addr = 0x400000;
	const uint32_t virtual_text_addr = virtual_entry_addr;
	const uint32_t virtual_rodata_addr = virtual_text_addr + text_size;
	const uint32_t virtual_data_addr = virtual_rodata_addr + rodata_size;
	const uint32_t virtual_bss_addr = virtual_data_addr + data_size;

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
	ehdr.e_entry				= virtual_entry_addr; // Entry point (virtual addr.)
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
	phdr_text.p_vaddr			= virtual_text_addr;  // Virtual address
	phdr_text.p_filesz			= text_size; // Size in file image
	phdr_text.p_memsz			= text_size; // Size in memory image
	phdr_text.p_flags			= PF_X | PF_R; // Executable & Readable
	phdr_text.p_align			= 0; // No alignemnt required

	// .rodata segment program header

	Elf32_Phdr phdr_rodata;
	memset(&phdr_rodata, 0, sizeof(Elf32_Phdr));

	phdr_rodata.p_type 			= PT_LOAD;
	phdr_rodata.p_offset		= rodata_offset; // Offset to first byte of segment (file offset)
	phdr_rodata.p_vaddr			= virtual_rodata_addr;  // Virtual address
	phdr_rodata.p_filesz		= rodata_size; // Size in file image
	phdr_rodata.p_memsz			= rodata_size; // Size in memory image
	phdr_rodata.p_flags			= PF_R; // Read-only
	phdr_rodata.p_align			= 0; // No alignemnt required

	// .data segment program header

	Elf32_Phdr phdr_data;
	memset(&phdr_data, 0, sizeof(Elf32_Phdr));

	phdr_data.p_type 			= PT_LOAD;
	phdr_data.p_offset			= data_offset; // Offset to first byte of segment (file offset)
	phdr_data.p_vaddr			= virtual_data_addr;  // Virtual address
	phdr_data.p_filesz			= data_size; // Size in file image
	phdr_data.p_memsz			= data_size; // Size in memory image
	phdr_data.p_flags			= PF_R | PF_W; // Read & Write
	phdr_data.p_align			= 0; // No alignemnt required

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
	shdr_strtab.sh_offset		= strtab_offset;
	shdr_strtab.sh_size			= strtab_size;

	// .text (Code segment)

	Elf32_Shdr shdr_text;
	memset(&shdr_text, 0, sizeof(Elf32_Shdr));

	shdr_text.sh_name 			= strtab_text_index; // Offset into naming table
	shdr_text.sh_type 			= SHT_PROGBITS;
	shdr_text.sh_flags			= SHF_ALLOC | SHF_EXECINSTR;
	shdr_text.sh_addr			= virtual_text_addr;
	shdr_text.sh_offset			= text_offset;
	shdr_text.sh_size			= text_size;

	// .rodata (Read only data segment)

	Elf32_Shdr shdr_rodata;
	memset(&shdr_rodata, 0, sizeof(Elf32_Shdr));

	shdr_rodata.sh_name 		= strtab_rodata_index; // Offset into naming table
	shdr_rodata.sh_type 		= SHT_PROGBITS;
	shdr_rodata.sh_flags		= SHF_ALLOC;
	shdr_rodata.sh_addr			= virtual_rodata_addr;
	shdr_rodata.sh_offset		= rodata_offset;
	shdr_rodata.sh_size			= rodata_size;

	// .data (Writable data segment)

	Elf32_Shdr shdr_data;
	memset(&shdr_data, 0, sizeof(Elf32_Shdr));

	shdr_data.sh_name 			= strtab_data_index; // Offset into naming table
	shdr_data.sh_type 			= SHT_PROGBITS;
	shdr_data.sh_flags			= SHF_ALLOC | SHF_WRITE;
	shdr_data.sh_addr			= virtual_data_addr;
	shdr_data.sh_offset			= data_offset;
	shdr_data.sh_size			= data_size;

	// .bss (Zero initialized variables)

	Elf32_Shdr shdr_bss;
	memset(&shdr_bss, 0, sizeof(Elf32_Shdr));

	shdr_bss.sh_name 			= strtab_bss_index; // Offset into naming table
	shdr_bss.sh_type 			= SHT_NOBITS;
	shdr_bss.sh_flags			= SHF_ALLOC | SHF_WRITE;
	shdr_bss.sh_addr			= virtual_bss_addr;
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

	fwrite(text, text_size, 1, file);
	fwrite(rodata, rodata_size, 1, file);
	fwrite(data, data_size, 1, file);
	fwrite(strtab, strtab_size, 1, file);

	fwrite(&shdr_null, sizeof(shdr_null), 1, file);
	fwrite(&shdr_strtab, sizeof(shdr_strtab), 1, file);
	fwrite(&shdr_text, sizeof(shdr_text), 1, file);
	fwrite(&shdr_rodata, sizeof(shdr_rodata), 1, file);
	fwrite(&shdr_data, sizeof(shdr_data), 1, file);
	fwrite(&shdr_bss, sizeof(shdr_bss), 1, file);
}

int main(void) {
	const char text[] = "Code here.";
	const char rodata[] = "Read only goes here.";
	const char data[] = "Data goes here.";
	FILE *file = fopen("test.bin", "wb");
	write_executable(file,
			text, sizeof(text),
			rodata, sizeof(rodata),
			data, sizeof(data),
			0);
	fclose(file);

	return EXIT_SUCCESS;
}
