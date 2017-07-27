#ifndef APP_H_
#define APP_H_

bool app_init(int argc, char **argv);
void app_cleanup();

void app_draw();

void app_reshape(int x, int y);
void app_keyboard(int key, bool press);
void app_mouse_button(int bn, bool press, int x, int y);
void app_mouse_motion(int x, int y);

// functions implemented in main.cc
void app_quit();
void app_redisplay();
void app_swap_buffers();

#endif	// APP_H_
