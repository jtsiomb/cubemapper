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

static void flip_image(float *pixels, int xsz, int ysz);

static const char *img_fname;
static float cam_theta, cam_phi;

static Texture *pano_tex;
static Mesh *pano_mesh;

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
	pano_mesh = new Mesh;
	gen_sphere(pano_mesh, 1.0, 80, 40);
	pano_mesh->flip();
	Mat4 xform;
	xform.rotation_y(-M_PI / 2.0);	// rotate the sphere to face the "front" part of the image
	pano_mesh->apply_xform(xform, xform);

	xform.scaling(-1, 1, 1);		// flip horizontal texcoord since we're inside the sphere
	pano_mesh->texcoord_apply_xform(xform);

	pano_tex = new Texture;
	if(!pano_tex->load(img_fname)) {
		return false;
	}
	printf("loaded image: %dx%d\n", pano_tex->get_width(), pano_tex->get_height());

	// create cubemap
	cube_size = pano_tex->get_height();
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
	delete pano_mesh;
	delete pano_tex;
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
	static const char *fname[] = {
		"cubemap_px.jpg",
		"cubemap_nx.jpg",
		"cubemap_py.jpg",
		"cubemap_ny.jpg",
		"cubemap_pz.jpg",
		"cubemap_nz.jpg"
	};

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

		//glReadPixels(0, 0, cube_size, cube_size, GL_RGB, GL_FLOAT, pixels);
		glGetTexImage(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, GL_FLOAT, pixels);
		//flip_image(pixels, cube_size, cube_size);

		if(img_save_pixels(fname[i], pixels, cube_size, cube_size, IMG_FMT_RGBF) == -1) {
			fprintf(stderr, "failed to save %dx%d image: %s\n", cube_size, cube_size, fname[i]);
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
	pano_tex->bind();
	glEnable(GL_TEXTURE_2D);
	pano_mesh->draw();
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

	pano_mesh->draw();

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
			/*
			} else if(strcmp(argv[i], "-help") == 0) {
				printf("usage: %s [options]\noptions:\n", argv[0]);
				printf(" -help: print usage information and exit\n");
				exit(0);
			} else {*/
				fprintf(stderr, "invalid option: %s\n", argv[i]);
				return false;
			//}
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

static void flip_image(float *pixels, int xsz, int ysz)
{
	float *top_ptr = pixels;
	float *bot_ptr = pixels + xsz * (ysz - 1) * 3;
	float *line = new float[xsz * 3];
	int scansz = xsz * 3 * sizeof(float);

	for(int i=0; i<ysz / 2; i++) {
		memcpy(line, top_ptr, scansz);
		memcpy(top_ptr, bot_ptr, scansz);
		memcpy(bot_ptr, line, scansz);
		top_ptr += xsz * 3;
		bot_ptr -= xsz * 3;
	}

	delete [] line;
}
