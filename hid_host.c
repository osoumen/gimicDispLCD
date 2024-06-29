#include "setup.h"

#include "class/hid/hid.h"
#include <sys/_intsup.h>
#include <sys/_stdint.h>
#include "hid_host.h"
#include "tusb.h"
#include "api/Common.h"

// PS4コントローラーなどを接続する場合は
// Arduino15/packages/rp2040/hardware/rp2040/xx.xx.xx/libraries/Adafruit_TinyUSB_Arduino/src/arduino/ports/rp2040/tusb_config_rp2040.h
// を修正してください.
static_assert( CFG_TUH_ENUMERATION_BUFSIZE > 499 );

#define MAX_REPORT  4
#define MAX_USAGE   32
#define MAX_INPUT_KEY 32
#define MAX_INPUT_KEY_MASK (MAX_INPUT_KEY - 1)

static uint8_t const keycode2ascii[128][2] =  {
    {0     , 0      }, /* 0x00 */
    {0     , 0      }, /* 0x01 */
    {0     , 0      }, /* 0x02 */
    {0     , 0      }, /* 0x03 */
    {'a'   , 'A'    }, /* 0x04 */
    {'b'   , 'B'    }, /* 0x05 */
    {'c'   , 'C'    }, /* 0x06 */
    {'d'   , 'D'    }, /* 0x07 */
    {'e'   , 'E'    }, /* 0x08 */
    {'f'   , 'F'    }, /* 0x09 */
    {'g'   , 'G'    }, /* 0x0a */
    {'h'   , 'H'    }, /* 0x0b */
    {'i'   , 'I'    }, /* 0x0c */
    {'j'   , 'J'    }, /* 0x0d */
    {'k'   , 'K'    }, /* 0x0e */
    {'l'   , 'L'    }, /* 0x0f */
    {'m'   , 'M'    }, /* 0x10 */
    {'n'   , 'N'    }, /* 0x11 */
    {'o'   , 'O'    }, /* 0x12 */
    {'p'   , 'P'    }, /* 0x13 */
    {'q'   , 'Q'    }, /* 0x14 */
    {'r'   , 'R'    }, /* 0x15 */
    {'s'   , 'S'    }, /* 0x16 */
    {'t'   , 'T'    }, /* 0x17 */
    {'u'   , 'U'    }, /* 0x18 */
    {'v'   , 'V'    }, /* 0x19 */
    {'w'   , 'W'    }, /* 0x1a */
    {'x'   , 'X'    }, /* 0x1b */
    {'y'   , 'Y'    }, /* 0x1c */
    {'z'   , 'Z'    }, /* 0x1d */
    {'1'   , '!'    }, /* 0x1e */
    {'2'   , '@'    }, /* 0x1f */
    {'3'   , '#'    }, /* 0x20 */
    {'4'   , '$'    }, /* 0x21 */
    {'5'   , '%'    }, /* 0x22 */
    {'6'   , '&'    }, /* 0x23 */
    {'7'   , '\''   }, /* 0x24 */
    {'8'   , '('    }, /* 0x25 */
    {'9'   , ')'    }, /* 0x26 */
    {'0'   , '0'    }, /* 0x27 */
    {'\r'  , '\r'   }, /* 0x28 */
    {'\x1b', '\x1b' }, /* 0x29 */
    {KEY_BS, KEY_BS }, /* 0x2a */
    {'\t'  , '\t'   }, /* 0x2b */
    {' '   , ' '    }, /* 0x2c */
    {'-'   , '='    }, /* 0x2d */
    {'^'   , '~'    }, /* 0x2e */
    {'@'   , '`'    }, /* 0x2f */
    {'['   , '{'    }, /* 0x30 */
    {'\\'  , '|'    }, /* 0x31 */
    {']'   , '}'    }, /* 0x32 */
    {';'   , '+'    }, /* 0x33 */
    {':'   , '*'    }, /* 0x34 */
    {'`'   , '~'    }, /* 0x35 */
    {','   , '<'    }, /* 0x36 */
    {'.'   , '>'    }, /* 0x37 */
    {'/'   , '?'    }, /* 0x38 */
                                
    {0     , 0      }, /* 0x39 */
    {KEY_FUNC_F1 , KEY_FUNC_F1}, /* 0x3a */
    {KEY_FUNC_F2 , KEY_FUNC_F2}, /* 0x3b */
    {KEY_FUNC_F3 , KEY_FUNC_F3}, /* 0x3c */
    {KEY_FUNC_F4 , KEY_FUNC_F4}, /* 0x3d */
    {KEY_FUNC_F5 , KEY_FUNC_F5}, /* 0x3e */
    {KEY_FUNC_F6 , KEY_FUNC_F6}, /* 0x3f */
    {KEY_FUNC_F7 , KEY_FUNC_F7}, /* 0x40 */
    {KEY_FUNC_F8 , KEY_FUNC_F8}, /* 0x41 */
    {KEY_FUNC_F9 , KEY_FUNC_F9}, /* 0x42 */
    {KEY_FUNC_F10, KEY_FUNC_F10}, /* 0x43 */
    {KEY_FUNC_F11, KEY_FUNC_F11}, /* 0x44 */
    {KEY_FUNC_F12, KEY_FUNC_F12}, /* 0x45 */
    {0     , 0      }, /* 0x46 */
    {0     , 0      }, /* 0x47 */
    {0     , 0      }, /* 0x48 */
    {0     , 0      }, /* 0x49 */
    {KEY_HOME, KEY_HOME}, /* 0x4a */
    {KEY_PAGE_UP, KEY_PAGE_UP}, /* 0x4b */
    {0     , 0      }, /* 0x4c */
    {KEY_END, KEY_END}, /* 0x4d */
    {KEY_PAGE_DOWN,KEY_PAGE_DOWN}, /* 0x4e */
    {KEY_CURSOR_RIGHT,KEY_CURSOR_RIGHT}, /* 0x4f */
    {KEY_CURSOR_LEFT,KEY_CURSOR_LEFT}, /* 0x50 */
    {KEY_CURSOR_DOWN,KEY_CURSOR_DOWN}, /* 0x51 */
    {KEY_CURSOR_UP,KEY_CURSOR_UP}, /* 0x52 */
    {0     , 0      }, /* 0x53 */
                                
    {'/'   , '/'    }, /* 0x54 */
    {'*'   , '*'    }, /* 0x55 */
    {'-'   , '-'    }, /* 0x56 */
    {'+'   , '+'    }, /* 0x57 */
    {'\r'  , '\r'   }, /* 0x58 */
    {'1'   , 0      }, /* 0x59 */
    {'2'   , 0      }, /* 0x5a */
    {'3'   , 0      }, /* 0x5b */
    {'4'   , 0      }, /* 0x5c */
    {'5'   , '5'    }, /* 0x5d */
    {'6'   , 0      }, /* 0x5e */
    {'7'   , 0      }, /* 0x5f */
    {'8'   , 0      }, /* 0x60 */
    {'9'   , 0      }, /* 0x61 */
    {'0'   , 0      }, /* 0x62 */
    {'.'   , 0      }, /* 0x63 */
    {0     , 0      }, /* 0x64 */
    {0     , 0      }, /* 0x65 */
    {0     , 0      }, /* 0x66 */
    {'='   , '='    }, /* 0x67 */
};

typedef struct {
  uint16_t  usage_page;
  uint16_t  usage_num;
  uint32_t  usage[MAX_USAGE];
  uint32_t  usage_min;
  uint32_t  usage_max;
  int32_t   logical_minimum;
  int32_t   logical_maximum;
  int32_t   physical_minimum;
  int32_t   physical_maximum;
  int32_t   unit_exponent;
  int32_t   unit;
  uint32_t  report_size;
  uint32_t  report_id;
  uint32_t  report_count;
} HIDInputReport;

typedef struct {
  uint32_t  report_id;
  uint32_t  x_axis_stbit;
  uint32_t  x_axis_bits;
  uint32_t  y_axis_stbit;
  uint32_t  y_axis_bits;
  int32_t   x_off_min;
  int32_t   x_off_max;
  int32_t   y_off_min;
  int32_t   y_off_max;
  uint32_t  hatswitch_stbit;
  uint32_t  hatswitch_bits;
  uint32_t  button_stbit;
  uint32_t  num_buttons;
} GamePadInfo;

static struct
{
  uint8_t report_count;
  tuh_hid_report_info_t report_info[MAX_REPORT];
  GamePadInfo pad_info;
} hid_info[CFG_TUH_HID];

static uint8_t  sInputKeys[6] = {0};
static bool     sInputKeysHasChanged = false;
static uint32_t sButtons = 0;
static int  sMouseMoveX = 0;
static int  sMouseMoveY = 0;
static int  sMouseMoveWheel = 0;
static uint32_t sMouseButtons = 0;

static void process_kbd_report(hid_keyboard_report_t const *report);
static void process_mouse_report(uint8_t dev_addr, uint8_t instance, uint8_t const* report, uint16_t len);
static void process_generic_report(uint8_t dev_addr, uint8_t instance, uint8_t const* report, uint16_t len);
static uint32_t set_gamepadinfo_from_inputItem(const HIDInputReport *local_item, GamePadInfo *pad_info, uint32_t input_bit_count);
static uint32_t set_mouseinfo_from_inputItem(const HIDInputReport *local_item, GamePadInfo *pad_info, uint32_t input_bit_count);
static uint8_t hid_parse_report_descriptor(tuh_hid_report_info_t* report_info_arr, uint8_t arr_count, uint8_t const* desc_report, uint16_t desc_len, GamePadInfo *pad_info);
static void parse_gamepad_report(uint8_t dev_addr, uint8_t instance, uint8_t const* report, uint16_t len);

void tuh_hid_mount_cb(uint8_t dev_addr, uint8_t instance, uint8_t const* desc_report, uint16_t desc_len)
{
  uint8_t const itf_protocol = tuh_hid_interface_protocol(dev_addr, instance);

  if ( itf_protocol == HID_ITF_PROTOCOL_NONE ) {
    hid_info[instance].report_count = hid_parse_report_descriptor(hid_info[instance].report_info, MAX_REPORT, desc_report, desc_len, &hid_info[instance].pad_info);
  }
  else if ( itf_protocol == HID_ITF_PROTOCOL_MOUSE ) {
    hid_info[instance].report_count = hid_parse_report_descriptor(hid_info[instance].report_info, MAX_REPORT, desc_report, desc_len, &hid_info[instance].pad_info);
    tuh_hid_set_protocol(dev_addr, instance, HID_PROTOCOL_REPORT);
  }
  if ( !tuh_hid_receive_report(dev_addr, instance) )
  {
    // tft.printf("Error: cannot request to receive report\r\n");
  }
}

static uint32_t set_gamepadinfo_from_inputItem(const HIDInputReport *local_item, GamePadInfo *pad_info, uint32_t input_bit_count)
{
  if (local_item->usage_num > 0) {
    for (int i=0; i<local_item->report_count; ++i) {
      if (i < local_item->usage_num) {
        if (local_item->usage[i] == HID_USAGE_DESKTOP_X) {
          const int pp = local_item->logical_maximum - local_item->logical_minimum;
          pad_info->x_axis_bits = local_item->report_size;
          pad_info->x_axis_stbit = input_bit_count;
          pad_info->x_off_min = local_item->logical_minimum + (pp >> 2);
          pad_info->x_off_max = local_item->logical_maximum - (pp >> 2);
        }
        if (local_item->usage[i] == HID_USAGE_DESKTOP_Y) {
          const int pp = local_item->logical_maximum - local_item->logical_minimum;
          pad_info->y_axis_bits = local_item->report_size;
          pad_info->y_axis_stbit = input_bit_count;
          pad_info->y_off_min = local_item->logical_minimum + (pp >> 2);
          pad_info->y_off_max = local_item->logical_maximum - (pp >> 2);
        }
        if (local_item->usage[i] == HID_USAGE_DESKTOP_HAT_SWITCH) {
          pad_info->hatswitch_bits = local_item->report_size;
          pad_info->hatswitch_stbit = input_bit_count;
        }
      }
      input_bit_count += local_item->report_size;
    }
  }
  if (local_item->usage_min < local_item->usage_max) {
    if (local_item->usage_page == HID_USAGE_PAGE_BUTTON) {
      pad_info->button_stbit = input_bit_count;
      pad_info->num_buttons = local_item->report_count;
      pad_info->report_id = local_item->report_id;
    }
    input_bit_count += local_item->report_size * local_item->report_count;
  }
  return input_bit_count;
}

static uint32_t set_mouseinfo_from_inputItem(const HIDInputReport *local_item, GamePadInfo *pad_info, uint32_t input_bit_count)
{
  uint32_t bit_count = input_bit_count;
  if (local_item->usage_num > 0) {
    for (int i=0; i<local_item->report_count; ++i) {
      if (i < local_item->usage_num) {
        if (local_item->usage[i] == HID_USAGE_DESKTOP_X) {
          const int pp = local_item->logical_maximum - local_item->logical_minimum;
          pad_info->x_axis_bits = local_item->report_size;
          pad_info->x_axis_stbit = bit_count;
          // pad_info->x_off_min = local_item->logical_minimum + (pp >> 2);
          // pad_info->x_off_max = local_item->logical_maximum - (pp >> 2);
        }
        if (local_item->usage[i] == HID_USAGE_DESKTOP_Y) {
          const int pp = local_item->logical_maximum - local_item->logical_minimum;
          pad_info->y_axis_bits = local_item->report_size;
          pad_info->y_axis_stbit = bit_count;
          // pad_info->y_off_min = local_item->logical_minimum + (pp >> 2);
          // pad_info->y_off_max = local_item->logical_maximum - (pp >> 2);
        }
        if (local_item->usage[i] == HID_USAGE_DESKTOP_WHEEL) {
          pad_info->hatswitch_bits = local_item->report_size;
          pad_info->hatswitch_stbit = bit_count;
        }
      }
      bit_count += local_item->report_size;
    }
  }
  if (local_item->usage_min < local_item->usage_max) {
    if (local_item->usage_page == HID_USAGE_PAGE_BUTTON) {
      pad_info->button_stbit = bit_count;
      pad_info->num_buttons = local_item->report_count;
      pad_info->report_id = local_item->report_id;
    }
    bit_count += local_item->report_size * local_item->report_count;
  }
  return input_bit_count + local_item->report_count * local_item->report_size;
}

static uint8_t hid_parse_report_descriptor(tuh_hid_report_info_t* report_info_arr, uint8_t arr_count, uint8_t const* desc_report, uint16_t desc_len, GamePadInfo *pad_info)
{
  uint8_t header_byte;

  memset(report_info_arr, 0, arr_count * sizeof(tuh_hid_report_info_t));

  uint8_t report_num = 0;
  tuh_hid_report_info_t* info = report_info_arr;

  uint8_t ri_collection_depth = 0;

  const int max_stack = 4;
  const int max_depth = 8;
  HIDInputReport  local_item;
  HIDInputReport  stacked_item[max_stack];
  int             current_stack = 0;
  uint32_t        usage_stack[max_depth];
  uint32_t        input_bit_count = 0;

  memset(&local_item, 0, sizeof(HIDInputReport));
  for (int i=0; i<max_stack; ++i) {
    memset(&stacked_item[i], 0, sizeof(HIDInputReport));
  }

  while (desc_len && report_num < arr_count) {
    header_byte = *desc_report++;
    desc_len--;

    if (header_byte == 0xfe) {
      desc_report += desc_report[0] + 4;
      desc_len -= desc_report[0] + 4;
      continue;
    }
    uint8_t const tag = (header_byte >> 4) & 0x0f;
    uint8_t const type = (header_byte >> 2) & 0x03;
    uint8_t const size = (1 << (header_byte & 0x03)) >> 1;

    uint32_t data32 = desc_report[0];
    if (size > 1 ) data32 |= desc_report[1] << 8;
    if (size > 2 ) data32 |= (desc_report[2] << 16) | (desc_report[3] << 24);

    switch (type) {
      case RI_TYPE_MAIN:
        switch (tag) {
          case RI_MAIN_INPUT:
            if (usage_stack[0] == HID_USAGE_DESKTOP_JOYSTICK || usage_stack[0] == HID_USAGE_DESKTOP_GAMEPAD) {
              input_bit_count = set_gamepadinfo_from_inputItem(&local_item, pad_info, input_bit_count);
            }
            if (usage_stack[0] == HID_USAGE_DESKTOP_MOUSE) {
              input_bit_count = set_mouseinfo_from_inputItem(&local_item, pad_info, input_bit_count);
            }
            break;
          case RI_MAIN_OUTPUT:
            break;
          case RI_MAIN_FEATURE:
            break;
          case RI_MAIN_COLLECTION:
            ri_collection_depth++;
            break;

          case RI_MAIN_COLLECTION_END:
            ri_collection_depth--;
            if (ri_collection_depth == 0) {
              info++;
              report_num++;
            }
            break;

          default:break;
        }
        local_item.usage_num = 0;
        local_item.usage_min = 0;
        local_item.usage_max = 0;
        break;

      case RI_TYPE_GLOBAL:
        switch (tag) {
          case RI_GLOBAL_USAGE_PAGE:
            local_item.usage_page = data32;
            if (ri_collection_depth == 0) info->usage_page = data32;
            break;

          case RI_GLOBAL_LOGICAL_MIN:
            local_item.logical_minimum = data32;
            break;
          case RI_GLOBAL_LOGICAL_MAX:
            local_item.logical_maximum = data32;
            break;
          case RI_GLOBAL_PHYSICAL_MIN:
            local_item.physical_minimum = data32;
            break;
          case RI_GLOBAL_PHYSICAL_MAX:
            local_item.physical_maximum = data32;
            break;

          case RI_GLOBAL_REPORT_ID:
            local_item.report_id = data32;
            input_bit_count += 8;
            info->report_id = data32;
            break;

          case RI_GLOBAL_REPORT_SIZE:
            local_item.report_size = data32;
            break;

          case RI_GLOBAL_REPORT_COUNT:
            local_item.report_count = data32;
            break;

          case RI_GLOBAL_UNIT_EXPONENT:
            local_item.unit_exponent = data32;
            break;
          case RI_GLOBAL_UNIT:
            local_item.unit = data32;
            break;
          case RI_GLOBAL_PUSH:
            if (current_stack < max_stack) {
              stacked_item[current_stack++] = local_item;
            }
            break;
          case RI_GLOBAL_POP:
            if (current_stack > 0) {
              local_item = stacked_item[--current_stack];
            }
            break;

          default: break;
        }
        break;

      case RI_TYPE_LOCAL:
        switch (tag) {
          case RI_LOCAL_USAGE:
            if (local_item.usage_num < MAX_USAGE) {
              local_item.usage[local_item.usage_num] = data32;
              usage_stack[ri_collection_depth] = local_item.usage[local_item.usage_num];
              local_item.usage_num++;
            }
            if (ri_collection_depth == 0) info->usage = data32;
            break;

          case RI_LOCAL_USAGE_MIN:
            local_item.usage_min = data32;
            break;
          case RI_LOCAL_USAGE_MAX:
            local_item.usage_max = data32;
            break;
          case RI_LOCAL_DESIGNATOR_INDEX:
            break;
          case RI_LOCAL_DESIGNATOR_MIN:
            break;
          case RI_LOCAL_DESIGNATOR_MAX:
            break;
          case RI_LOCAL_STRING_INDEX:
            break;
          case RI_LOCAL_STRING_MIN:
            break;
          case RI_LOCAL_STRING_MAX:
            break;
          case RI_LOCAL_DELIMITER:
            break;
          default: break;
        }
        break;

      // TODO: エラー
      default: break;
    }

    desc_report += size;
    desc_len -= size;
  }

  for (uint8_t i = 0; i < report_num; i++) {
    info = report_info_arr + i;
  }

  return report_num;
}

void tuh_hid_umount_cb(uint8_t dev_addr, uint8_t instance)
{
}

void tuh_hid_report_received_cb(uint8_t dev_addr, uint8_t instance, uint8_t const* report, uint16_t len)
{
  uint8_t const itf_protocol = tuh_hid_interface_protocol(dev_addr, instance);

  switch (itf_protocol) {
    case HID_ITF_PROTOCOL_KEYBOARD:
      process_kbd_report( (hid_keyboard_report_t const*) report );
    break;

    case HID_ITF_PROTOCOL_MOUSE:
      process_mouse_report(dev_addr, instance, report, len);
    break;

    default:
      process_generic_report(dev_addr, instance, report, len);
    break;
  }
  if ( !tuh_hid_receive_report(dev_addr, instance) ) {
    // tft.printf("Error: cannot request to receive report\r\n");
  }
}

bool   getKeyboardKey(uint8_t key[6])
{
  if (sInputKeysHasChanged) {
    sInputKeysHasChanged = false;
    for(uint8_t i=0; i<6; i++) {
      key[i] = sInputKeys[i];
    }
    return true;
  }
  return false;
}

static void process_kbd_report(hid_keyboard_report_t const *report)
{
  for(uint8_t i=0; i<6; i++) {
    bool const is_shift = report->modifier & (KEYBOARD_MODIFIER_LEFTSHIFT | KEYBOARD_MODIFIER_RIGHTSHIFT);
    uint8_t ch = keycode2ascii[report->keycode[i]][is_shift ? 1 : 0];
    sInputKeys[i] = ch;
  }
  sInputKeysHasChanged = true;
}

void cursor_movement(int8_t x, int8_t y, int8_t wheel)
{
  sMouseMoveX += x;
  sMouseMoveY += y;
  sMouseMoveWheel += wheel;
}

static void process_mouse_report(uint8_t dev_addr, uint8_t instance, uint8_t const* report, uint16_t len)
{
  GamePadInfo *padinfo = &hid_info[instance].pad_info;
  int8_t x = 0;
  int8_t y = 0;
  int8_t wheel = 0;
  uint32_t  buttons = 0;

  if (padinfo->report_id != 0) {
    if (*report != padinfo->report_id) return;
  }
  if (padinfo->x_axis_bits != 0) {
    x = report[padinfo->x_axis_stbit >> 3];
    // x >>= padinfo->x_axis_stbit & 0x07;
    // x &= (1 << padinfo->x_axis_bits) - 1;
  }
  if (padinfo->y_axis_bits != 0) {
    y = report[padinfo->y_axis_stbit >> 3];
    // y >>= padinfo->y_axis_stbit & 0x07;
    // y &= (1 << padinfo->y_axis_bits) - 1;
  }
  if (padinfo->hatswitch_bits != 0) {
    wheel = report[padinfo->hatswitch_stbit >> 3];
    // wheel >>= padinfo->hatswitch_stbit & 0x07;
    // wheel &= (1 << padinfo->hatswitch_bits) - 1;
  }
  if (padinfo->num_buttons != 0) {
    memcpy(&buttons, &report[padinfo->button_stbit >> 3], min(4, (padinfo->num_buttons + 7) >> 3));
    buttons >>= padinfo->button_stbit & 0x07;
    buttons &= (1 << padinfo->num_buttons) - 1;
  }
  sMouseButtons = buttons;
  cursor_movement(x, y, wheel);
}

static void process_generic_report(uint8_t dev_addr, uint8_t instance, uint8_t const* report, uint16_t len)
{
  (void) dev_addr;

  uint8_t const rpt_count = hid_info[instance].report_count;
  tuh_hid_report_info_t* rpt_info_arr = hid_info[instance].report_info;
  tuh_hid_report_info_t* rpt_info = NULL;

  if ( rpt_count == 1 ) {
    rpt_info = &rpt_info_arr[0];
  }

  if (!rpt_info) {
    return;
  }

  if ( rpt_info->usage_page == HID_USAGE_PAGE_DESKTOP ) {
    switch (rpt_info->usage)
    {
      case HID_USAGE_DESKTOP_KEYBOARD:
        process_kbd_report( (hid_keyboard_report_t const*) report );
      break;

      case HID_USAGE_DESKTOP_MOUSE:
        process_mouse_report(dev_addr, instance, report, len);
      break;

      case HID_USAGE_DESKTOP_JOYSTICK:
      case HID_USAGE_DESKTOP_GAMEPAD:
      parse_gamepad_report(dev_addr, instance, report, len);
      break;

      default: break;
    }
  }
}

static void parse_gamepad_report(uint8_t dev_addr, uint8_t instance, uint8_t const* report, uint16_t len)
{
  GamePadInfo *padinfo = &hid_info[instance].pad_info;
  int x_axis = 0;
  int y_axis = 0;
  uint32_t  hatsw = 0;
  uint32_t  buttons = 0;
  static uint32_t prev_buttons = 0;
  const uint32_t hatsw_bits[8] = {1 << 4, (1 << 4)|(1 << 6), 1 << 6, (1 << 6)|(1 << 5), 1 << 5, (1 << 5)|(1 << 4), 1 << 3, (1 << 3)|(1 << 4)};

  if (padinfo->report_id != 0) {
    if (*report != padinfo->report_id) return;
  }
  if (padinfo->x_axis_bits != 0) {
    x_axis = report[padinfo->x_axis_stbit >> 3];
    x_axis >>= padinfo->x_axis_stbit & 0x07;
    x_axis &= (1 << padinfo->x_axis_bits) - 1;
  }
  if (padinfo->y_axis_bits != 0) {
    y_axis = report[padinfo->y_axis_stbit >> 3];
    y_axis >>= padinfo->y_axis_stbit & 0x07;
    y_axis &= (1 << padinfo->y_axis_bits) - 1;
  }

  if (padinfo->num_buttons != 0) {
    memcpy(&buttons, &report[padinfo->button_stbit >> 3], min(4, (padinfo->num_buttons + 7) >> 3));
    buttons >>= padinfo->button_stbit & 0x07;
    buttons &= (1 << padinfo->num_buttons) - 1;
    buttons <<= 7;
  }
  if (padinfo->hatswitch_bits != 0) {
    memcpy(&hatsw, &report[padinfo->hatswitch_stbit >> 3], min(4, (padinfo->hatswitch_bits + 7) >> 3));
    hatsw >>= padinfo->hatswitch_stbit & 0x07;
    hatsw &= (1 << padinfo->hatswitch_bits) - 1;
    if (hatsw < 8) {
      buttons |= hatsw_bits[hatsw];
    }
  }

  if (x_axis > padinfo->x_off_max) {
    buttons |= 1 << 6;
  }
  else if (x_axis < padinfo->x_off_min) {
    buttons |= 1 << 3;
  }
  if (y_axis > padinfo->y_off_max) {
    buttons |= 1 << 5;
  }
  else if (y_axis < padinfo->y_off_min) {
    buttons |= 1 << 4;
  }

  sButtons = buttons;
}

uint32_t  getJoypadButtons()
{
  return sButtons;
}

uint32_t  getMouseMove(int *x, int *y, int *wheel)
{
  *x = sMouseMoveX;
  *y = sMouseMoveY;
  *wheel = sMouseMoveWheel;
  return sMouseButtons;
}

void      clearMouseMove()
{
  sMouseMoveX = 0;
  sMouseMoveY = 0;
  sMouseMoveWheel = 0;
}