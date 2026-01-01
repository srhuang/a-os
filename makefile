#####################################
#		Environment					#
#####################################
BUILD_DIR		= ./build
OUT_DIR			= ./out

#####################################
#		Build Tools					#
#####################################
AS 				= nasm
CC 				= x86_64-linux-gnu-gcc
LD 				= x86_64-linux-gnu-ld

#####################################
#		Build Flags					#
#####################################
KERNEL_ENTRY	= 0xc0001000
USR_ENTRY		= 0x08048000
ASFLAGS			= -f elf
CFLAGS			= -c -m32 -fno-stack-protector -fno-builtin -nostdinc
LDFLAGS			= -m elf_i386 -Ttext $(KERNEL_ENTRY) -e main -z noexecstack 
LDFLAGS_USR		= -m elf_i386 -Ttext $(USR_ENTRY) -e init -z noexecstack 
LDFLAGS_PROG	= -m elf_i386 -z noexecstack

#####################################
#		Include Files				#
#####################################
LIB_INC			= ./lib/inc
KERNEL_INC		= ./kernel/inc
USR_INC			= ./usr/inc
USR_LIB_INC		= ./usr/lib/inc

#####################################
#		Source Files				#
#####################################
LIB_SRC			= ./lib/src
KERNEL_SRC		= ./kernel/src
USR_SRC			= ./usr/src
USR_LIB_SRC		= ./usr/lib/src
PROG_SRC		= ./usr/bin

#####################################
#		Output Files				#
#####################################
lib_obj			:=
kernel_obj		:=
usr_obj			:=
usr_lib_obj		:=
prog_obj		:=

#####################################
#		MBR: mbr.bin				#
#####################################
$(OUT_DIR)/mbr.bin: boot/mbr.S
	$(AS) -I boot/ $< -o $@

#####################################
#		Bootloader: loader.bin		#
#####################################
$(OUT_DIR)/loader.bin: boot/loader.S
	$(AS) -I boot/ $< -o $@

#####################################
#		Object Files: C				#
#####################################
$(BUILD_DIR)/%.o: $(LIB_SRC)/%.c
	$(CC) $(CFLAGS) -I $(LIB_INC) $< -o $@

$(BUILD_DIR)/%.o: $(KERNEL_SRC)/%.c
	$(CC) $(CFLAGS) -I $(LIB_INC) -I $(KERNEL_INC) $< -o $@

$(BUILD_DIR)/%.o: $(USR_SRC)/%.c
	$(CC) $(CFLAGS) -I $(LIB_INC) -I $(USR_LIB_INC) -I $(USR_INC) $< -o $@

$(BUILD_DIR)/%.o: $(USR_LIB_SRC)/%.c
	$(CC) $(CFLAGS) -I $(LIB_INC) -I $(USR_LIB_INC) $< -o $@

$(BUILD_DIR)/%.o: $(PROG_SRC)/%.c
	$(CC) $(CFLAGS) -I $(LIB_INC) -I $(USR_LIB_INC) $< -o $@

#####################################
#      	Object Files: Assembly		#
#####################################
$(BUILD_DIR)/%.o: $(LIB_SRC)/%.s
	$(AS) $(ASFLAGS) $< -o $@

$(BUILD_DIR)/%.o: $(KERNEL_SRC)/%.s
	$(AS) $(ASFLAGS) $< -o $@

$(BUILD_DIR)/%.o: $(USR_SRC)/%.s
	$(AS) $(ASFLAGS) $< -o $@

$(BUILD_DIR)/%.o: $(PROG_SRC)/%.s
	$(AS) $(ASFLAGS) $< -o $@

#####################################
#		Lib Object Files			#
#####################################
lib_obj		+= $(BUILD_DIR)/string.o 
lib_obj		+= $(BUILD_DIR)/stdio.o 

#####################################
#		Kernel Object Files			#
#####################################
kernel_obj 	+= $(BUILD_DIR)/main.o
kernel_obj	+= $(BUILD_DIR)/test.o
kernel_obj	+= $(BUILD_DIR)/print.o
kernel_obj	+= $(BUILD_DIR)/bitmap.o
kernel_obj	+= $(BUILD_DIR)/list.o
kernel_obj	+= $(BUILD_DIR)/init.o
kernel_obj	+= $(BUILD_DIR)/memory.o
kernel_obj	+= $(BUILD_DIR)/kernel.o
kernel_obj	+= $(BUILD_DIR)/interrupt.o
kernel_obj	+= $(BUILD_DIR)/syscall_sys.o
kernel_obj	+= $(BUILD_DIR)/timer.o
kernel_obj	+= $(BUILD_DIR)/thread.o
kernel_obj	+= $(BUILD_DIR)/sched.o
kernel_obj	+= $(BUILD_DIR)/switch.o
kernel_obj	+= $(BUILD_DIR)/lock.o
kernel_obj	+= $(BUILD_DIR)/printk.o
kernel_obj	+= $(BUILD_DIR)/assert.o
kernel_obj	+= $(BUILD_DIR)/ide.o
kernel_obj	+= $(BUILD_DIR)/fs.o
kernel_obj	+= $(BUILD_DIR)/inode.o
kernel_obj	+= $(BUILD_DIR)/dir.o
kernel_obj	+= $(BUILD_DIR)/file.o
kernel_obj	+= $(BUILD_DIR)/process.o
kernel_obj	+= $(BUILD_DIR)/ioqueue.o
kernel_obj	+= $(lib_obj)

#####################################
#		User Lib Object Files		#
#####################################
usr_lib_obj	+= $(BUILD_DIR)/syscall_usr.o
usr_lib_obj	+= $(BUILD_DIR)/printf.o

#####################################
#		User Object Files			#
#####################################
usr_obj		+= $(BUILD_DIR)/usr_init.o
usr_obj		+= $(usr_lib_obj)
usr_obj		+= $(lib_obj)

#####################################
#		Kernel: kernel.bin			#
#####################################
$(OUT_DIR)/kernel.bin: $(kernel_obj)
	$(LD) $(LDFLAGS) $^ -o $@

#####################################
#		User: user.bin				#
#####################################
$(OUT_DIR)/usr.bin: $(usr_obj)
	$(LD) $(LDFLAGS_USR) $^ -o $@

#####################################
#		Programs					#
#####################################
prog_obj	:= $(BUILD_DIR)/start.o
prog_obj	+= $(BUILD_DIR)/prog.o
prog_obj    += $(usr_lib_obj)
prog_obj	+= $(lib_obj)
$(OUT_DIR)/prog: $(prog_obj)
	$(LD) $(LDFLAGS_PROG) $^ -o $@

#####################################
#		Command						#
#####################################
.PHONY : dir clean all

dir:
	@echo ">>>Start $@..."
	@echo "mkdir $(BUILD_DIR)"
	@if [ ! -d $(BUILD_DIR) ];then mkdir $(BUILD_DIR);fi
	@echo "mkdir $(OUT_DIR)"
	@if [ ! -d $(OUT_DIR) ];then mkdir $(OUT_DIR);fi
	@echo ">>>make $@ done."

clean:
	@echo ">>>Start $@..."
	rm -f $(BUILD_DIR)/*
	rm -f $(OUT_DIR)/*
	@echo ">>>make $@ done."

all: dir $(OUT_DIR)/mbr.bin $(OUT_DIR)/loader.bin $(OUT_DIR)/kernel.bin \
		$(OUT_DIR)/usr.bin \
		$(OUT_DIR)/prog
	@echo ">>>make $@ done."

