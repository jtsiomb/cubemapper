/*
Cubemapper - a program for converting panoramic images into cubemaps
Copyright (C) 2017  John Tsiombikas <nuclear@member.fsf.org>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
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
void app_resize(int x, int y);
void app_print_text(int x, int y, const char *str);

#endif	// APP_H_
