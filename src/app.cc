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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <imago2.h>
#include "app.h"
#include "opengl.h"
#include "texture.h"
#include "mesh.h"
#include "meshgen.h"

static void draw_equilateral();
static void draw_cubemap();
static bool parse_args(int argc, char **argv);

static const char *img_fname, *img_suffix;
static float cam_theta, cam_phi;

static Texture *tex;
static Mesh *mesh;

static int win_width, win_height;
static int show_cubemap;

static unsigned int fbo;
static unsigned int cube_tex;
static int cube_size;


bool app_init(int argc, char **argv)
{
	if(!parse_args(argc, argv)) {
		return false;
	}
	if(!img_fname) {
		fprintf(stderr, "please specify an equilateral panoramic image\n");
		return false;
	}

	if(!init_opengl()) {
		return false;
	}

	glEnable(GL_MULTISAMPLE);

	Mesh::use_custom_sdr_attr = false;
	mesh = new Mesh;
	gen_sphere(mesh, 1.0, 80, 40);
	mesh->flip();
	Mat4 xform;
	xform.rotation_y(-M_PI / 2.0);	// rotate the sphere to face the "front" part of the image
	mesh->apply_xform(xform, xform);

	xform.scaling(-1, 1, 1);		// flip horizontal texcoord since we're inside the sphere
	mesh->texcoord_apply_xform(xform);

	tex = new Texture;
	if(!tex->load(img_fname)) {
		return false;
	}
	printf("loaded image: %dx%d\n", tex->get_width(), tex->get_height());

	if(!(img_suffix = strrchr(img_fname, '.'))) {
		img_suffix = ".jpg";
	}

	// create cubemap
	cube_size = tex->get_height();
	glGenTextures(1, &cube_tex);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cube_tex);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	for(int i=0; i<6; i++) {
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, cube_size, cube_size,
				0, GL_RGB, GL_FLOAT, 0);
	}


	// create fbo
	glGenFramebuffers(1, &fbo);

	// tex-gen for cubemap visualization
	glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
	glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
	glTexGeni(GL_R, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
	float planes[][4] = {{1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}};
	glTexGenfv(GL_S, GL_OBJECT_PLANE, planes[0]);
	glTexGenfv(GL_T, GL_OBJECT_PLANE, planes[1]);
	glTexGenfv(GL_R, GL_OBJECT_PLANE, planes[2]);
	return true;
}

void app_cleanup()
{
	delete mesh;
	delete tex;
}

void app_draw()
{
	glClear(GL_COLOR_BUFFER_BIT);

	Mat4 view_matrix;
	view_matrix.pre_rotate_x(deg_to_rad(cam_phi));
	view_matrix.pre_rotate_y(deg_to_rad(cam_theta));

	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf(view_matrix[0]);

	if(show_cubemap) {
		draw_cubemap();

		glColor3f(0, 0, 0);
		app_print_text(10, 10, "cubemap");
		glColor3f(0, 0.8, 1);
		app_print_text(8, 13, "cubemap");
	} else {
		draw_equilateral();

		glColor3f(0, 0, 0);
		app_print_text(10, 10, "equilateral");
		glColor3f(1, 0.8, 0);
		app_print_text(8, 13, "equilateral");
	}
	glColor3f(1, 1, 1);

	app_swap_buffers();
	assert(glGetError() == GL_NO_ERROR);
}

void render_cubemap()
{
	printf("rendering cubemap %dx%d\n", cube_size, cube_size);

	float *pixels = new float[cube_size * cube_size * 3];

	glViewport(0, 0, cube_size, cube_size);

	Mat4 viewmat[6];
	viewmat[0].rotation_y(deg_to_rad(90));	// +X
	viewmat[1].rotation_y(deg_to_rad(-90));	// -X
	viewmat[2].rotation_x(deg_to_rad(90));	// +Y
	viewmat[2].rotate_y(deg_to_rad(180));
	viewmat[3].rotation_x(deg_to_rad(-90));	// -Y
	viewmat[3].rotate_y(deg_to_rad(180));
	viewmat[4].rotation_y(deg_to_rad(180));	// +Z

	// this must coincide with the order of GL_TEXTURE_CUBE_MAP_* values
	static const char *fname_pattern[] = {
		"cubemap_px%s",
		"cubemap_nx%s",
		"cubemap_py%s",
		"cubemap_ny%s",
		"cubemap_pz%s",
		"cubemap_nz%s"
	};
	static char fname[64];

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluPerspective(90, 1.0, 0.5, 500.0);
	glScalef(-1, -1, 1);

	glBindFramebuffer(GL_FRAMEBUFFER, fbo);

	for(int i=0; i<6; i++) {
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
				GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, cube_tex, 0);

		glClear(GL_COLOR_BUFFER_BIT);

		glMatrixMode(GL_MODELVIEW);
		glLoadMatrixf(viewmat[i][0]);

		draw_equilateral();

		glGetTexImage(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, GL_FLOAT, pixels);

		sprintf(fname, fname_pattern[i], img_suffix);
		if(img_save_pixels(fname, pixels, cube_size, cube_size, IMG_FMT_RGBF) == -1) {
			fprintf(stderr, "failed to save %dx%d image: %s\n", cube_size, cube_size, fname);
		}
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, win_width, win_height);

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	delete [] pixels;

	glBindTexture(GL_TEXTURE_CUBE_MAP, cube_tex);
	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
}

static void draw_equilateral()
{
	tex->bind();
	glEnable(GL_TEXTURE_2D);
	mesh->draw();
	glDisable(GL_TEXTURE_2D);
}

static void draw_cubemap()
{
	glPushAttrib(GL_ENABLE_BIT);

	glBindTexture(GL_TEXTURE_CUBE_MAP, cube_tex);
	glEnable(GL_TEXTURE_CUBE_MAP);
	glEnable(GL_TEXTURE_GEN_S);
	glEnable(GL_TEXTURE_GEN_T);
	glEnable(GL_TEXTURE_GEN_R);

	mesh->draw();

	glPopAttrib();
}

void app_reshape(int x, int y)
{
	glViewport(0, 0, x, y);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(50.0, (float)x / (float)y, 0.5, 500.0);

	win_width = x;
	win_height = y;
}

void app_keyboard(int key, bool press)
{
	if(press) {
		switch(key) {
		case 27:
			app_quit();
			break;

		case ' ':
			show_cubemap = !show_cubemap;
			app_redisplay();
			break;

		case 'c':
			render_cubemap();
			show_cubemap = 1;
			app_redisplay();
			break;
		}
	}
}

static float prev_x, prev_y;
static bool bnstate[16];

void app_mouse_button(int bn, bool press, int x, int y)
{
	if(bn < (int)(sizeof bnstate / sizeof *bnstate)) {
		bnstate[bn] = press;
	}
	prev_x = x;
	prev_y = y;
}

void app_mouse_motion(int x, int y)
{
	float dx = x - prev_x;
	float dy = y - prev_y;
	prev_x = x;
	prev_y = y;

	if(!dx && !dy) return;

	if(bnstate[0]) {
		cam_theta += dx * 0.5;
		cam_phi += dy * 0.5;

		if(cam_phi < -90) cam_phi = -90;
		if(cam_phi > 90) cam_phi = 90;
		app_redisplay();
	}
}

static bool parse_args(int argc, char **argv)
{
	for(int i=1; i<argc; i++) {
		if(argv[i][0] == '-') {
			fprintf(stderr, "invalid option: %s\n", argv[i]);
			return false;
		} else {
			if(img_fname) {
				fprintf(stderr, "unexpected option: %s\n", argv[i]);
				return false;
			}
			img_fname = argv[i];
		}
	}

	return true;
}
