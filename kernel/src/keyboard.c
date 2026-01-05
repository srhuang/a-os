#include "keyboard.h"
#include "interrupt.h"
#include "printk.h"
#include "io.h"

//=========================
// debugging
//=========================
//#define DEBUG

#ifdef DEBUG
    #define pr_debug(fmt, ...) printk(fmt, ##__VA_ARGS__)
#else
    #define pr_debug(fmt, ...) do { } while (0)
#endif

//=========================
// internal struct
//=========================
struct kb_key {
    char key;
    char uppercase;
    bool isalpha;
};

static struct kb_key keymap[] = {
    //key, uppercase, is alphabet
    {0, 0, false},          // 0x00
    {'\033','\033', false}, // 0x01
    {'1', '!', false},      // 0x02
    {'2', '@', false},      // 0x03
    {'3', '#', false},      // 0x04
    {'4', '$', false},      // 0x05
    {'5', '%', false},      // 0x06
    {'6', '^', false},      // 0x07
    {'7', '&', false},      // 0x08
    {'8', '*', false},      // 0x09
    {'9', '(', false},      // 0x0A
    {'0', ')', false},      // 0x0B
    {'-', '_', false},      // 0x0C
    {'=', '+', false},      // 0x0D
    {'\b', '\b', false},    // 0x0E
    {'\t', '\t', false},    // 0x0F
    {'q', 'Q', true},       // 0x10
    {'w', 'W', true},       // 0x11
    {'e', 'E', true},       // 0x12
    {'r', 'R', true},       // 0x13
    {'t', 'T', true},       // 0x14
    {'y', 'Y', true},       // 0x15
    {'u', 'U', true},       // 0x16
    {'i', 'I', true},       // 0x17
    {'o', 'O', true},       // 0x18
    {'p', 'P', true},       // 0x19
    {'[', '{', false},      // 0x1A
    {']', '}', false},      // 0x1B
    {'\r',  '\r', false},   // 0x1C
    {0, 0, false},          // 0x1D
    {'a', 'A', true},       // 0x1E
    {'s', 'S', true},       // 0x1F
    {'d', 'D', true},       // 0x20
    {'f', 'F', true},       // 0x21
    {'g', 'G', true},       // 0x22
    {'h', 'H', true},       // 0x23
    {'j', 'J', true},       // 0x24
    {'k', 'K', true},       // 0x25
    {'l', 'L', true},       // 0x26
    {';', ':', false},      // 0x27
    {'\'', '"', false},     // 0x28
    {'`', '~', false},      // 0x29
    {0, 0, false},          // 0x2A
    {'\\', '|', false},     // 0x2B
    {'z', 'Z', true},       // 0x2C
    {'x', 'X', true},       // 0x2D
    {'c', 'C', true},       // 0x2E
    {'v', 'V', true},       // 0x2F
    {'b', 'B', true},       // 0x30
    {'n', 'N', true},       // 0x31
    {'m', 'M', true},       // 0x32
    {',', '<', false},      // 0x33
    {'.', '>', false},      // 0x34
    {'/', '?', false},      // 0x35
    {0, 0, false},          // 0x36
    {'*', '*', false},      // 0x37
    {0, 0, false},          // 0x38
    {' ', ' ', false},      // 0x39
    {0, 0, false}           // 0x3A
};

//=========================
// global variable
//=========================
static bool ext_code;
static bool shift_btn;
static bool ctrl_btn;
static bool alt_btn;
static bool caps_btn = false;

struct ioqueue* kb_buf;

//=========================
// internal functions
//=========================
static void keyboard_handler(void)
{
    //pr_debug("%s:+++\n", __func__);

    uint8_t kb_data = inb(KB_BUF_PORT);
    //pr_debug("%s:keyboard data=0x%x\n", __func__, kb_data);

    // get scan code
    uint16_t scancode = 0;
    if (KB_EXT_CODE == kb_data) {
        ext_code = true;
        return;
    }
    if (ext_code) {
        scancode = (KB_EXT_CODE << 8) | kb_data;
        ext_code = false;
    } else {
        scancode = kb_data;
    }
    //pr_debug("%s:scan code=0x%x\n", __func__, scancode);

    // check break code
    if (scancode & KB_BREAK_CODE) {
        switch(scancode)
        {
            case KB_SHIFT_L_BREAK:
            case KB_SHIFT_R_BREAK:
                shift_btn = false;
                return;
            case KB_CTRL_L_BREAK:
            case KB_CTRL_R_BREAK:
                ctrl_btn = false;
                return;
            case KB_ALT_L_BREAK:
            case KB_ALT_R_BREAK:
                alt_btn = false;
                return;
            default:
                return;
        }
    }

    // deal with make code
    //pr_debug("%s:scan code=0x%x\n", __func__, scancode);
    switch(scancode)
    {
        case KB_SHIFT_L_MAKE:
        case KB_SHIFT_R_MAKE:
            shift_btn = true;
            return;
        case KB_CTRL_L_MAKE:
        case KB_CTRL_R_MAKE:
            ctrl_btn = true;
            return;
        case KB_ALT_L_MAKE:
        case KB_ALT_R_MAKE:
            alt_btn = true;
            return;
        case KB_CAPS_MAKE:
            caps_btn = !caps_btn;
            return;
        default:
    }

    // others make code
    //pr_debug("%s:scan code=0x%x\n", __func__, scancode);
    uint32_t idx = scancode & 0x00FF;
    if (idx >= sizeof(keymap) / sizeof(keymap[0])) {
        return;
    }

    // determine uppercase
    bool uppercase = shift_btn;
    if (true == keymap[idx].isalpha) {
        if (true == caps_btn) {
            uppercase = !uppercase;
        }
    }

    // get the character
    char buf;
    if (true == uppercase) {
        buf = keymap[idx].uppercase;
    } else {
        buf = keymap[idx].key;
    }

    // check hotkey
    if (true == ctrl_btn) {
        switch (buf)
        {
            case 'l':
                buf = KB_CTRL_L;
                break;
            case 'u':
                buf = KB_CTRL_U;
                break;
            default:
        }
    }
    //pr_debug("%s:char=%c\n", __func__, buf);
    ioq_put(kb_buf, buf);
}

//=========================
// external functions
//=========================
void keyboard_init()
{
    pr_debug("%s +++\n", __func__);
    kb_buf = ioq_init(KB_IOQ_SIZE);
    register_handler(0x21, keyboard_handler);
    pr_debug("%s ---\n", __func__);
}

