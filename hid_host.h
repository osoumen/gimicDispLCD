#ifndef hid_host_h
#define hid_host_h

enum Keys {
		KEY_CURSOR_UP = 0x80,
		KEY_CURSOR_DOWN,
		KEY_CURSOR_LEFT,
		KEY_CURSOR_RIGHT,
		KEY_RET,
		KEY_BS,
		KEY_HOME,
		KEY_END,
		KEY_PAGE_UP,
		KEY_PAGE_DOWN,
		KEY_FUNC_F1,
		KEY_FUNC_F2,
		KEY_FUNC_F3,
		KEY_FUNC_F4,
		KEY_FUNC_F5,
		KEY_FUNC_F6,
		KEY_FUNC_F7,
		KEY_FUNC_F8,
		KEY_FUNC_F9,
		KEY_FUNC_F10,
		KEY_FUNC_F11,
		KEY_FUNC_F12,
};

#ifdef __cplusplus
extern "C" {
#endif
uint32_t  getNumInputKeys();
bool   getKeyboardKey(uint8_t *key);
uint32_t  getJoypadButtons();
uint32_t  getMouseMove(int *x, int *y, int *wheel);
void      clearMouseMove();
#ifdef __cplusplus
}
#endif

#endif  // hid_host_h
