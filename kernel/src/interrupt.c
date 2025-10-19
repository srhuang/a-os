#include "interrupt.h"
#include "kernel.h"
#include "stddef.h"
#include "print.h"
#include "io.h"

//=========================
// debugging
//=========================
#define DEBUG (1)
#define TRACE_STR(x) do {if(DEBUG) put_str(x);} while(0)
#define TRACE_INT(x) do {if(DEBUG) put_int(x);} while(0)

//=========================
// internal struct
//=========================
struct idt_desc {
    uint16_t    offset_1;
    uint16_t    selector;
    uint8_t     zero;
    uint8_t     attribute;
    uint16_t    offset_2;
};

static struct idt_desc idt_table[IDT_VEC_MAX+1];

//=========================
// global variable
//=========================
void* intr_func[IDT_VEC_MAX+1];
char* intr_name[IDT_VEC_MAX+1];

//=========================
// internal functions
//=========================
static void intr_general_handler(uint8_t vec)
{
    // spurious interrupt
    if (vec == 0x27 || vec == 0x2F) {
        return;
    }

    // print message
    put_str("\n!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
    put_str("    vector number: 0x");
    put_int(vec);
    put_str("\n");
    put_str("    ");
    put_str(intr_name[vec]);
    put_str("\n");

    // for page fault
    if (vec == 0xE) {
        int page_fault_vaddr = 0;
        asm ("movl %%cr2, %0" : "=r" (page_fault_vaddr));
        put_str("    page fault addr is ");
        put_int(page_fault_vaddr);
        put_str("\n");
    }
    put_str("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");

    //halt the system
    while(1);
}

static void make_idt_table(struct idt_desc* p_desc,\
    uint32_t attr, void* func)
{
    p_desc->offset_1    = (uint32_t)func & 0x0000FFFF;
    p_desc->selector    = SELECTOR_K_CODE;
    p_desc->zero        = 0;
    p_desc->attribute   = (attr >> 8) & 0xE0 | 0x0E;
    p_desc->offset_2    = ((uint32_t)func & 0xFFFF0000) >> 16;
}

static void idt_init(void)
{
    int idx;
    uint32_t attr;
    int intr_entry_size = sizeof(intr_entry) / 4;
    TRACE_STR("intr_entry_size: ");
    TRACE_INT(intr_entry_size);
    TRACE_STR("\n");
    for (idx=0; idx<=IDT_VEC_MAX; idx++)
    {
        intr_name[idx] = "unknown";
        intr_func[idx] = NULL;
        if (idx<intr_entry_size) {
            attr = GDT_P_1 + GDT_DPL_0;
            make_idt_table(&idt_table[idx], attr, intr_entry[idx]);
            intr_func[idx] = intr_general_handler;
        }
    }

    // for system call 0x80
    attr = GDT_P_1 + GDT_DPL_3;
    make_idt_table(&idt_table[0x80], attr, syscall_entry);

    // assign name of interrupt
    intr_name[0x00] = "#DE Divide Error";
    intr_name[0x01] = "#DB Debug Exception";
    intr_name[0x02] = "NMI Interrupt";
    intr_name[0x03] = "#BP Breakpoint Exception";
    intr_name[0x04] = "#OF Overflow Exception";
    intr_name[0x05] = "#BR BOUND Range Exceeded Exception";
    intr_name[0x06] = "#UD Invalid Opcode Exception";
    intr_name[0x07] = "#NM Device Not Available Exception";
    intr_name[0x08] = "#DF Double Fault Exception";
    intr_name[0x09] = "Coprocessor Segment Overrun";
    intr_name[0x0A] = "#TS Invalid TSS Exception";
    intr_name[0x0B] = "#NP Segment Not Present";
    intr_name[0x0C] = "#SS Stack Fault Exception";
    intr_name[0x0D] = "#GP General Protection Exception";
    intr_name[0x0E] = "#PF Page-Fault Exception";
    // intr_name[0x0F] reserved
    intr_name[0x10] = "#MF x87 FPU Floating-Point Error";
    intr_name[0x11] = "#AC Alignment Check Exception";
    intr_name[0x12] = "#MC Machine-Check Exception";
    intr_name[0x13] = "#XF SIMD Floating-Point Exception";
    intr_name[0x14] = "#VE Virtualization Exception";
    intr_name[0x15] = "#CP Control Protection Exception";
    // intr_name[0x16-0x1B] reserved
    intr_name[0x1C] = "#HV Hypervisor Injection Exception";
    intr_name[0x1D] = "#VC VMM Communication Exception";
    intr_name[0x1E] = "#SX Security Exception";
    // intr_name[0x1F] reserved

    // load IDT
    uint64_t idt_operand = ((sizeof(idt_table) - 1)\
        | ((uint64_t)(uint32_t)idt_table << 16));
    asm volatile("lidt %0" : : "m" (idt_operand));
}


static void pic_init(void)
{
    // master PIC
    outb (PIC_M_CTRL, 0x11);   // ICW1: edge trigger mode, ICW4 needed
    outb (PIC_M_DATA, 0x20);   // ICW2: vector address start from 0x20
    outb (PIC_M_DATA, 0x04);   // ICW3: IRQ2 for slave
    outb (PIC_M_DATA, 0x01);   // ICW4: 8086 mode, normal EOI

    //slave PIC
    outb (PIC_S_CTRL, 0x11);    // ICW1: edge trigger mode, ICW4 needed
    outb (PIC_S_DATA, 0x28);    // ICW2: vector address start from 0x28
    outb (PIC_S_DATA, 0x02);    // ICW3: IRQ2 for slave
    outb (PIC_S_DATA, 0x01);    // ICW4: 8086 mode, normal EOI

    // interrupt mask
    outb (PIC_M_DATA, 0xFF);    // master PIC
    outb (PIC_S_DATA, 0xFF);    // slave PIC
}

//=========================
// external functions
//=========================
bool intr_get_status(void)
{
    uint32_t eflags = 0;
    asm volatile("pushfl; popl %0" : "=g" (eflags));
    return (eflags & 0x200) ? true : false;
}

void intr_set_status (bool intr_status)
{
    if (true == intr_status) {
        asm volatile("sti");
    } else {
        asm volatile("cli");
    }
}

void register_handler(uint8_t vec, void* func)
{
    intr_func[vec] = func;
}

void intr_init()
{
    TRACE_STR("intr_init()\n");
    idt_init();
    pic_init();
}

