#include QMK_KEYBOARD_H
#include "debug.h"
#include "action_layer.h"
#include "version.h"
#include "raw_hid.h"

#define BASE 0 // default layer
#define SYMB 1 // symbols
#define MDIA 2 // media keys
#define NUM 3 // numeric keyboard

// to make run "make ergodox_ez:pldev"
enum custom_keycodes {
  PLACEHOLDER = SAFE_RANGE, // can always be here
  EPRM,
  VRSN,
  RGB_SLD,
    HID_KEY
};

struct {
    uint16_t lctrl;
    uint16_t rctrl;
    uint16_t lalt;
    uint16_t ralt;
} modTapTimers;


typedef struct {
    uint16_t keycode;
    uint16_t startTime;
    uint16_t keyDown;
    uint16_t keyUp;
} modTapRecord;

enum MOD_TAP_KEYS {
    MT_LCTL = 0,
    MT_LALT,
    MT_RCTL,
    MT_RALT,
    MT_COUNT

};

void resetModTapRecord(modTapRecord* rec) {
    rec->startTime = 0;
    rec->keyDown = 0;
    rec->keyUp = 0;
}

modTapRecord modTapRecords[MT_COUNT];

enum hidEvents {
    HID_EVT_KEYPRESS = 1,
    HID_EVT_TIME
};

void writeState(unsigned char* packet, char layerHint) {
    const char lookedUpLayer =
        layerHint == 0xFF ? get_highest_layer(layer_state) : layerHint;

    const char led = host_keyboard_led_state().raw;
    memset(packet, 0, RAW_EPSIZE);
    packet[0] = lookedUpLayer;
    packet[1] = led;

}

void sendHidState(char layerHint) {
    unsigned char packet[RAW_EPSIZE];
    writeState(packet, layerHint);
    raw_hid_send(packet, RAW_EPSIZE);
}

void sendHidKeypress(uint8_t hidKey) {
    unsigned char packet[RAW_EPSIZE];
    writeState(packet, 0xFF);

    packet[2] = HID_EVT_KEYPRESS;
    packet[3] = hidKey;
    raw_hid_send(packet, RAW_EPSIZE);
};

void sendHidTiming(uint8_t key, uint16_t time) {
    unsigned char packet[RAW_EPSIZE];
    writeState(packet, 0xFF);

    packet[2] = HID_EVT_TIME;
    packet[3] = key;
    packet[4] = time >> 8;
    packet[5] = time & 0xFF;
    packet[6] = (modTapRecords[key].keyDown) >> 8;
    packet[7] = (modTapRecords[key].keyDown) &0xFF;
    packet[8] = (modTapRecords[key].keyUp) >> 8;
    packet[9] = (modTapRecords[key].keyUp) &0xFF;
    raw_hid_send(packet, RAW_EPSIZE);
};

//void timeHidKey(uint8_t id, uint16_t* store, keyrecord_t* record) {
//    if (record->event.pressed) {
//        *store = record->event.time;
//    } else if (*store != 0) {
//        uint16_t elapsed = TIMER_DIFF_16(record->event.time, *store);
//        sendHidTiming(id, elapsed);
//        *store = 0;
//    }
//}

void timeHidKey(int key, keyrecord_t* record) {
    if (record->event.pressed) {
        modTapRecords[key].startTime = record->event.time;
    } else if (modTapRecords[key].startTime != 0) {
        uint16_t elapsed = TIMER_DIFF_16(record->event.time, modTapRecords[key].startTime);
        sendHidTiming(key, elapsed);
        resetModTapRecord(&(modTapRecords[key]));
    }
}

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
/* Keymap 0: Basic layer
 *
 * ,--------------------------------------------------.           ,--------------------------------------------------.
 * |   =    |   1  |   2  |   3  |   4  |   5  | LEFT |           | RIGHT|   6  |   7  |   8  |   9  |   0  |   -    |
 * |--------+------+------+------+------+-------------|           |------+------+------+------+------+------+--------|
 * | Del    |   Q  |   W  |   E  |   R  |   T  |  L1  |           |  L1  |   Y  |   U  |   I  |   O  |   P  |   \    |
 * |--------+------+------+------+------+------|      |           |      |------+------+------+------+------+--------|
 * | BkSp   |   A  |   S  |   D  |   F  |   G  |------|           |------|   H  |   J  |   K  |   L  |; / L2|' / Cmd |
 * |--------+------+------+------+------+------| Hyper|           | Meh  |------+------+------+------+------+--------|
 * | LShift |Z/Ctrl|   X  |   C  |   V  |   B  |      |           |      |   N  |   M  |   ,  |   .  |//Ctrl| RShift |
 * `--------+------+------+------+------+-------------'           `-------------+------+------+------+------+--------'
 *   |Grv/L1|  '"  |AltShf| Left | Right|                                       |  Up  | Down |   [  |   ]  | ~L1  |
 *   `----------------------------------'                                       `----------------------------------'
 *                                        ,-------------.       ,-------------.
 *                                        | App  | LGui |       | Alt  |Ctrl/Esc|
 *                                 ,------|------|------|       |------+--------+------.
 *                                 |      |      | Home |       | PgUp |        |      |
 *                                 | Space|Backsp|------|       |------|  Tab   |Enter |
 *                                 |      |ace   | End  |       | PgDn |        |      |
 *                                 `--------------------'       `----------------------'
 */
// If it accepts an argument (i.e, is a function), it doesn't need KC_.
// Otherwise, it needs KC_*
[BASE] = LAYOUT_ergodox(  // layer 0 : default
        // left hand
        KC_ESC,         KC_1,            KC_2,    KC_3,          KC_4,        KC_5, KC_CAPSLOCK,
        KC_TAB,         KC_Q,            KC_W,    KC_E,          KC_R,        KC_T, LSFT(KC_INSERT),
        KC_LSHIFT,      KC_A,            KC_S,    ALT_T(KC_D),   CTL_T(KC_F), KC_G,
        KC_LCTRL,       KC_Z,            KC_X,    KC_C,          KC_V,        KC_B, KC_MINUS,
        KC_APPLICATION, OSM(MOD_HYPR),   HID_KEY, KC_NONUS_HASH, TT(SYMB),
                                                        KC_MEDIA_PREV_TRACK, KC_MEDIA_NEXT_TRACK,
                                                                                       KC_INSERT,
                                                                    KC_SPACE, TT(SYMB), KC_DELETE,
        // right hand
        KC_PSCREEN,  KC_6, KC_7,        KC_8,        KC_9,    KC_0,          TG(NUM),
        KC_HOME,     KC_Y, KC_U,        KC_I,        KC_O,    KC_P,          KC_BSPACE,
                     KC_H, CTL_T(KC_J), ALT_T(KC_K), KC_L,    KC_SCLN,       KC_RSHIFT,
        KC_END ,     KC_N, KC_M,        KC_COMM,     KC_DOT,  KC_SLSH,       KC_LALT,
                                        TT(SYMB),    KC_LEFT, KC_DOWN,KC_UP, KC_RIGHT,
        KC_MEDIA_PLAY_PAUSE,  KC_RGUI,
        KC_PGUP,
        KC_PGDN, KC_BSPACE, KC_ENT
    ),
/* Keymap 1: Symbol Layer
 *
 * ,---------------------------------------------------.           ,--------------------------------------------------.
 * |Version  |  F1  |  F2  |  F3  |  F4  |  F5  |      |           |      |  F6  |  F7  |  F8  |  F9  |  F10 |   F11  |
 * |---------+------+------+------+------+------+------|           |------+------+------+------+------+------+--------|
 * |         |   !  |   @  |   {  |   }  |   |  |      |           |      |   Up |   7  |   8  |   9  |   *  |   F12  |
 * |---------+------+------+------+------+------|      |           |      |------+------+------+------+------+--------|
 * |         |   #  |   $  |   (  |   )  |   `  |------|           |------| Down |   4  |   5  |   6  |   +  |        |
 * |---------+------+------+------+------+------|      |           |      |------+------+------+------+------+--------|
 * |         |   %  |   ^  |   [  |   ]  |   ~  |      |           |      |   &  |   1  |   2  |   3  |   \  |        |
 * `---------+------+------+------+------+-------------'           `-------------+------+------+------+------+--------'
 *   | EPRM  |      |      |      |      |                                       |      |    . |   0  |   =  |      |
 *   `-----------------------------------'                                       `----------------------------------'
 *                                        ,-------------.       ,-------------.
 *                                        |Animat|      |       |Toggle|Solid |
 *                                 ,------|------|------|       |------+------+------.
 *                                 |Bright|Bright|      |       |      |Hue-  |Hue+  |
 *                                 |ness- |ness+ |------|       |------|      |      |
 *                                 |      |      |      |       |      |      |      |
 *                                 `--------------------'       `--------------------'
 */
// SYMBOLS
[SYMB] = LAYOUT_ergodox(
       // left hand
       VRSN,    KC_F1,   KC_F2,          KC_F3,          KC_F4,           KC_F5,                 KC_TRNS,
       KC_TRNS, KC_EXLM, KC_AT,          KC_LCBR,        KC_RCBR,         KC_PIPE,               KC_TRNS,
       KC_TRNS, KC_HASH, KC_DLR,         KC_TRNS,        KC_TRNS,         KC_GRV,
       KC_TRNS, KC_PERC, LSFT(KC_MINUS), LSFT(KC_EQUAL), KC_NONUS_BSLASH, LSFT(KC_NONUS_BSLASH), KC_TRNS,
          EPRM, KC_TRNS, KC_TRNS,        KC_TRNS,        KC_TRNS,
                                                                          RGB_MOD,               KC_TRNS,
                                                                                                 KC_TRNS,
                                                         RGB_VAD,         RGB_VAI,               KC_TRNS,
       // right hand
       KC_TRNS,  KC_F6,                KC_F7,       KC_F8,         KC_F9,         KC_F10,              KC_F11,
       KC_TRNS,  KC_LBRACKET,          KC_RBRACKET, KC_UP,         KC_MINUS,      KC_EQUAL,            KC_F12,
                 KC_LEFT_CURLY_BRACE,  KC_LEFT,     KC_DOWN,       KC_RIGHT,      KC_TRNS,             KC_TRNS,
       KC_TRNS,  KC_RIGHT_CURLY_BRACE, KC_QUOT,     KC_NONUS_HASH, LSFT(KC_QUOT), KC_PIPE/*uk tilde*/, KC_TRNS,
                                       KC_PGUP,     KC_PGDN,       KC_TRNS,       KC_TRNS,             KC_TRNS,
       KC_TRNS, KC_TRNS,
       KC_TRNS,
       KC_TRNS,  KC_TRNS,  KC_TRNS
),
/* Keymap 2: Media and mouse keys
 *
 * ,--------------------------------------------------.           ,--------------------------------------------------.
 * |        |      |      |      |      |      |      |           |      |      |      |      |      |      |        |
 * |--------+------+------+------+------+-------------|           |------+------+------+------+------+------+--------|
 * |        |      |      | MsUp |      |      |      |           |      |      |      |      |      |      |        |
 * |--------+------+------+------+------+------|      |           |      |------+------+------+------+------+--------|
 * |        |      |MsLeft|MsDown|MsRght|      |------|           |------|      |      |      |      |      |  Play  |
 * |--------+------+------+------+------+------|      |           |      |------+------+------+------+------+--------|
 * |        |      |      |      |      |      |      |           |      |      |      | Prev | Next |      |        |
 * `--------+------+------+------+------+-------------'           `-------------+------+------+------+------+--------'
 *   |      |      |      | Lclk | Rclk |                                       |VolUp |VolDn | Mute |      |      |
 *   `----------------------------------'                                       `----------------------------------'
 *                                        ,-------------.       ,-------------.
 *                                        |      |      |       |      |      |
 *                                 ,------|------|------|       |------+------+------.
 *                                 |      |      |      |       |      |      |Brwser|
 *                                 |      |      |------|       |------|      |Back  |
 *                                 |      |      |      |       |      |      |      |
 *                                 `--------------------'       `--------------------'
 */
// MEDIA AND MOUSE
[MDIA] = LAYOUT_ergodox(
       KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
       KC_TRNS, KC_TRNS, KC_TRNS, KC_MS_U, KC_TRNS, KC_TRNS, KC_TRNS,
       KC_TRNS, KC_TRNS, KC_MS_L, KC_TRNS, KC_TRNS, KC_TRNS,
       KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
       KC_TRNS, KC_TRNS, KC_TRNS, KC_BTN1, KC_BTN2,
                                           KC_TRNS, KC_TRNS,
                                                    KC_TRNS,
                                  KC_TRNS, KC_TRNS, KC_TRNS,
    // right hand
       KC_TRNS,  KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
       KC_TRNS,  KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
                 KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_MPLY,
       KC_TRNS,  KC_TRNS, KC_TRNS, KC_MPRV, KC_MNXT, KC_TRNS, KC_TRNS,
                          KC_VOLU, KC_VOLD, KC_MUTE, KC_TRNS, KC_TRNS,
       KC_TRNS, KC_TRNS,
       KC_TRNS,
       KC_TRNS, KC_TRNS, KC_WBAK
),
[NUM] = LAYOUT_ergodox(
       KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
       KC_TRNS, RGB_TOG, RGB_SLD, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
       KC_TRNS, RGB_HUD, RGB_HUI, KC_TRNS, KC_TRNS, KC_TRNS,
       KC_TRNS, RGB_VAD, RGB_VAI, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
       KC_TRNS, RGB_MOD, KC_TRNS, KC_TRNS, KC_TRNS,
                                                    KC_TRNS, KC_TRNS,
                                                             KC_TRNS,
                                           KC_TRNS, KC_TRNS, KC_TRNS,
    // right hand
       KC_TRNS, KC_TRNS, KC_TRNS, KC_SLASH, KC_ASTERISK, KC_MINUS, KC_TRNS,
       KC_TRNS, KC_TRNS, KC_7, KC_8,  KC_9,     KC_PLUS,  KC_TRNS,
                KC_TAB, KC_4, KC_5,  KC_6,     KC_PLUS,   KC_TRNS,
       KC_TRNS, KC_TRNS, KC_1, KC_2,  KC_3,     KC_ENTER, KC_TRNS,
                         KC_0, KC_0,  KC_DOT,     KC_TRNS,  KC_TRNS,
       KC_TRNS, KC_TRNS,
       KC_TRNS,
       KC_TRNS, KC_TRNS, KC_TRNS
),
};

const uint16_t PROGMEM fn_actions[] = {
    [1] = ACTION_LAYER_TAP_TOGGLE(SYMB)                // FN1 - Momentary Layer 1 (Symbols)
};

const macro_t *action_get_macro(keyrecord_t *record, uint8_t id, uint8_t opt)
{
  // MACRODOWN only works in this function
  switch(id) {
    case 0:
      if (record->event.pressed) {
        SEND_STRING (QMK_KEYBOARD "/" QMK_KEYMAP " @ " QMK_VERSION);
      }
      break;
    case 1:
      if (record->event.pressed) { // For resetting EEPROM
        eeconfig_init();
      }
      break;
  }
  return MACRO_NONE;
};

bool process_record_user(uint16_t keycode, keyrecord_t *record) {

    for (int i = 0; i < MT_COUNT; i++) {
        if (modTapRecords[i].keycode != keycode && modTapRecords[i].startTime != 0) {
            if (record->event.pressed) {
                if (modTapRecords[i].keyDown == 0) {
                    modTapRecords[i].keyDown = keycode;
                }
            } else {
                if (modTapRecords[i].keyUp == 0) {
                    modTapRecords[i].keyUp = keycode;
                }
            }
        }
    }
  switch (keycode) {
    // dynamically generate these.
    case EPRM:
      if (record->event.pressed) {
        eeconfig_init();
      }
      return false;
      break;
    case VRSN:
      if (record->event.pressed) {
        SEND_STRING (QMK_KEYBOARD "/" QMK_KEYMAP " @ " QMK_VERSION);
      }
      return false;
      break;
    case RGB_SLD:
      if (record->event.pressed) {
        #ifdef RGBLIGHT_ENABLE
          rgblight_mode(1);
        #endif
      }
      return false;
      break;
    case HID_KEY:
        if (record->event.pressed) {
            sendHidKeypress(0);
        }
        return false;
        break;
      case CTL_T(KC_F):
          timeHidKey(MT_LCTL, record);
          break;
      case ALT_T(KC_D):
          timeHidKey(MT_LALT, record);
          break;
      case CTL_T(KC_J):
          timeHidKey(MT_RCTL, record);
          break;
      case ALT_T(KC_K):
          timeHidKey(MT_RALT, record);
          break;
  }

  return true;
}

// Runs just one time when the keyboard initializes.
void matrix_init_user(void) {
    for (int i = 0; i < MT_COUNT; i++) {
        resetModTapRecord(&(modTapRecords[i]));
    }
    modTapRecords[MT_LCTL].keycode = CTL_T(KC_F);
    modTapRecords[MT_LALT].keycode = ALT_T(KC_D);
    modTapRecords[MT_RCTL].keycode = CTL_T(KC_J);
    modTapRecords[MT_RALT].keycode = ALT_T(KC_K);

#ifdef RGBLIGHT_COLOR_LAYER_0
  rgblight_setrgb(RGBLIGHT_COLOR_LAYER_0);
#endif
  ergodox_right_led_1_off();

};

// Runs constantly in the background, in a loop.
void matrix_scan_user(void) {

};


void led_set_user(uint8_t usb_led) {
    if (usb_led & (1<<USB_LED_CAPS_LOCK)) {
        ergodox_right_led_1_on();
    } else {
        ergodox_right_led_1_off();
    }
    sendHidState(0xFF);
}

// Runs whenever there is a layer state change.
uint32_t layer_state_set_user(uint32_t state) {
  ergodox_board_led_off();
  ergodox_right_led_2_off();
  ergodox_right_led_3_off();

  uint8_t layer = biton32(state);
  switch (layer) {
      case 0:
        #ifdef RGBLIGHT_COLOR_LAYER_0
          rgblight_setrgb(RGBLIGHT_COLOR_LAYER_0);
        #else
        #ifdef RGBLIGHT_ENABLE
          rgblight_init();
        #endif
        #endif
        break;
      case 1:
        ergodox_right_led_2_on();
        #ifdef RGBLIGHT_COLOR_LAYER_1
          rgblight_setrgb(RGBLIGHT_COLOR_LAYER_1);
        #endif
        break;
      case 2:
        ergodox_right_led_2_on();
        ergodox_right_led_3_on();
        #ifdef RGBLIGHT_COLOR_LAYER_2
          rgblight_setrgb(RGBLIGHT_COLOR_LAYER_2);
        #endif
        break;
      case 3:
        ergodox_right_led_3_on();
        #ifdef RGBLIGHT_COLOR_LAYER_3
          rgblight_setrgb(RGBLIGHT_COLOR_LAYER_3);
        #endif
        break;
      case 4:
        ergodox_right_led_1_on();
        ergodox_right_led_2_on();
        #ifdef RGBLIGHT_COLOR_LAYER_4
          rgblight_setrgb(RGBLIGHT_COLOR_LAYER_4);
        #endif
        break;
      case 5:
        ergodox_right_led_1_on();
        ergodox_right_led_3_on();
        #ifdef RGBLIGHT_COLOR_LAYER_5
          rgblight_setrgb(RGBLIGHT_COLOR_LAYER_5);
        #endif
        break;
      case 6:
        ergodox_right_led_2_on();
        ergodox_right_led_3_on();
        #ifdef RGBLIGHT_COLOR_LAYER_6
          rgblight_setrgb(RGBLIGHT_COLOR_LAYER_6);
        #endif
        break;
      case 7:
        ergodox_right_led_1_on();
        ergodox_right_led_2_on();
        ergodox_right_led_3_on();
        #ifdef RGBLIGHT_COLOR_LAYER_7
          rgblight_setrgb(RGBLIGHT_COLOR_LAYER_6);
        #endif
        break;
      default:
        break;
    }

  sendHidState(get_highest_layer(state));

  return state;
};

void oneshot_mods_changed_user(uint8_t mods) {
  if (mods & MOD_MASK_CTRL) {
    rgblight_enable();
    rgblight_setrgb(0x00, 0x10, 0x10);
  }
  if (!mods) {
    rgblight_disable();
  }
}

void raw_hid_receive(uint8_t *data, uint8_t length) {
    // Your code goes here. data is the packet received from host.
    sendHidState(0xFF);
}


