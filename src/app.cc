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

static void draw_scene();	// both near and infinite parts
static void draw_scene_near();	// near scene: regular objects affected by parallax shift and translation
// infinity scene: objects conceptually at infinity, not affected by parallax shift and translation
static void draw_scene_inf();
static bool parse_args(int argc, char **argv);

static void flip_image(float *pixels, int xsz, int ysz);

static const char *img_fname;
static float cam_theta, cam_phi;

static Texture *pano_tex;
static Mesh *pano_mesh;

static int win_width, win_height;


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

	glEnable(GL_CULL_FACE);

	if(GLEW_ARB_framebuffer_sRGB) {
		glGetError();	// discard previous errors
		glEnable(GL_FRAMEBUFFER_SRGB);
		if(glGetError() != GL_NO_ERROR) {
			fprintf(stderr, "failed to enable sRGB framebuffer\n");
		}
	}

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

	draw_scene();

	app_swap_buffers();
	assert(glGetError() == GL_NO_ERROR);
}

void render_cubemap()
{
	int fbsize = win_width < win_height ? win_width : win_height;
	float *pixels = new float[fbsize * fbsize * 3];

	glViewport(0, 0, fbsize, fbsize);

	Mat4 viewmat[6];
	viewmat[0].rotation_y(deg_to_rad(90));	// +X
	viewmat[1].rotation_x(deg_to_rad(-90));	// +Y
	viewmat[2].rotation_y(deg_to_rad(180));	// +Z
	viewmat[3].rotation_y(deg_to_rad(-90));	// -X
	viewmat[4].rotation_x(deg_to_rad(90));	// -Y

	static const char *fname[] = {
		"cubemap_px.jpg",
		"cubemap_py.jpg",
		"cubemap_pz.jpg",
		"cubemap_nx.jpg",
		"cubemap_ny.jpg",
		"cubemap_nz.jpg"
	};

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45, 1.0, 0.5, 500.0);

	for(int i=0; i<6; i++) {
		glClear(GL_COLOR_BUFFER_BIT);

		glMatrixMode(GL_MODELVIEW);
		glLoadMatrixf(viewmat[i][0]);

		draw_scene();

		glReadPixels(0, 0, fbsize, fbsize, GL_RGB, GL_FLOAT, pixels);
		flip_image(pixels, fbsize, fbsize);

		if(img_save_pixels(fname[i], pixels, fbsize, fbsize, IMG_FMT_RGBF) == -1) {
			fprintf(stderr, "failed to save %dx%d image: %s\n", fbsize, fbsize, fname[i]);
			break;
		}
	}

	glViewport(0, 0, win_width, win_height);

	delete [] pixels;
}

// both near and infinite parts (see below)
static void draw_scene()
{
	draw_scene_inf();
	draw_scene_near();
}

// infinity scene: objects conceptually at infinity, not affected by parallax shift and translation
static void draw_scene_inf()
{
	pano_tex->bind();
	glEnable(GL_TEXTURE_2D);
	pano_mesh->draw();
	glDisable(GL_TEXTURE_2D);
}

// near scene: regular objects affected by parallax shift and translation
static void draw_scene_near()
{
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
	int cubemap_size;

	if(press) {
		switch(key) {
		case 27:
			app_quit();
			break;

		case ' ':
			cubemap_size = pano_tex->get_width() / 4;
			app_resize(cubemap_size, cubemap_size);
			break;

		case 's':
			printf("rendering cubemap\n");
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
