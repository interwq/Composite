include Makefile.src

CC := gcc -m32
LD := ld -m elf_i386
AS := as --32 -g

INCPATH := ../../kernel/include
CFLAGS := -g3 -Werror -ffreestanding -Wno-unused-function -Wno-unused-variable -Wno-unused-but-set-variable -nostdlib -mno-red-zone -I$(INCPATH)
LDFLAGS := -nostdlib -fno-builtin -nostartfiles -nostdinc -nodefaultlibs 

KERNEL := kernel.img

WARNINGS += -Wall
WARNINGS += -Wcast-align
WARNINGS += -Wformat=2
WARNINGS += -Winit-self
#WARNINGS += -Wmissing-declarations
#WARNINGS += -Wmissing-prototypes
WARNINGS += -Wnested-externs
WARNINGS += -Wno-system-headers
WARNINGS += -Wold-style-definition
WARNINGS += -Wredundant-decls
WARNINGS += -Wsign-compare
WARNINGS += -Wstrict-prototypes
WARNINGS += -Wundef
WARNINGS += -Wvolatile-register-var
WARNINGS += -Wwrite-strings

CFLAGS += $(WARNINGS)

OBJS += kernel.o
OBJS += gdt.o
OBJS += isr.o
OBJS += idt.o
OBJS += vm.o
OBJS += printk.o
OBJS += string.o
OBJS += vtxprintf.o
OBJS += tss.o
OBJS += user.o
OBJS += serial.o
OBJS += timer.o
OBJS += chal.o
OBJS += boot_comp.o
#OBJS += console.o

COS_OBJ += pgtbl.o 
COS_OBJ += retype_tbl.o
COS_OBJ += liveness_tbl.o
COS_OBJ += capinv.o
COS_OBJ += measurement.o
COS_OBJ += captbl.o
COS_OBJ += per_cpu.o

DEPS :=$(patsubst %.o, %.d, $(OBJS))

OBJS += $(COS_OBJ)

all: $(KERNEL)

$(KERNEL): linker.ld $(DEPS) $(OBJS) loader.o
	$(info |     [LD]   Linking $@)
	$(LD) -T linker.ld loader.o asm.o entry.o $(OBJS) -o $@

loader.o: loader.S interrupt.s entry.S
	$(info |     [AS]   Assembling $@)
	@$(AS) -c -o asm.o interrupt.s
	@$(CC) -c -I$(INCPATH) entry.S
	@$(CC) -c -I$(INCPATH) loader.S

%.d: %.c
	@$(CC) -M -MT $(patsubst %.d, %.o, $@) $(CFLAGS) $(LDFLAGS) $< -o $@

per_cpu.o: ../../kernel/per_cpu.c
	$(info |     [CC]   Compiling $@)
	@$(CC) $(CFLAGS) -c $< -o $@

pgtbl.o: ../../kernel/pgtbl.c
	$(info |     [CC]   Compiling $@)
	@$(CC) $(CFLAGS) -c $< -o $@

retype_tbl.o: ../../kernel/retype_tbl.c
	$(info |     [CC]   Compiling $@)
	@$(CC) $(CFLAGS) -c $< -o $@

liveness_tbl.o: ../../kernel/liveness_tbl.c
	$(info |     [CC]   Compiling $@)
	@$(CC) $(CFLAGS) -c $< -o $@

capinv.o: ../../kernel/capinv.c
	$(info |     [CC]   Compiling $@)
	@$(CC) $(CFLAGS) -c $< -o $@

measurement.o: ../../kernel/measurement.c
	$(info |     [CC]   Compiling $@)
	@$(CC) $(CFLAGS) -c $< -o $@

captbl.o: ../../kernel/captbl.c
	$(info |     [CC]   Compiling $@)
	@$(CC) $(CFLAGS) -c $< -o $@


%.o: %.c
	$(info |     [CC]   Compiling $@)
	@$(CC) $(CFLAGS) -c $< -o $@

clean:
	@rm -f *.d *.o $(KERNEL)

cp: $(KERNEL)
	$(info |     [CP]   Copying native booter to $(TRANS_DIR))
	@cp -f $(KERNEL) .gdbinit qemu.sh qemu2.sh qemu-g.sh ppos.sh $(TRANS_DIR)
