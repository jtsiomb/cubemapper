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
#include <imago2.h>
#include "texture.h"
#include "opengl.h"

Texture::Texture()
{
	width = height = tex_width = tex_height = 0;
	tex = 0;
}

Texture::~Texture()
{
	if(tex) {
		glDeleteTextures(1, &tex);
	}
}

static unsigned int next_pow2(unsigned int x)
{
	--x;
	x |= x >> 1;
	x |= x >> 2;
	x |= x >> 4;
	x |= x >> 8;
	x |= x >> 16;
	return x + 1;
}

int Texture::get_width() const
{
	return width;
}

int Texture::get_height() const
{
	return height;
}

bool Texture::load(const char *fname)
{
	img_pixmap img;
	img_init(&img);
	if(img_load(&img, fname) == -1) {
		fprintf(stderr, "failed to load texture: %s\n", fname);
		img_destroy(&img);
		return false;
	}

	unsigned int intfmt = img_glintfmt(&img);
	unsigned int pixfmt = img_glfmt(&img);
	unsigned int pixtype = img_gltype(&img);

	width = img.width;
	height = img.height;
	tex_width = next_pow2(width);
	tex_height = next_pow2(height);

	if(!tex) {
		glGenTextures(1, &tex);
	}
	glBindTexture(GL_TEXTURE_2D, tex);

	if(GLEW_SGIS_generate_mipmap) {
		glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP_SGIS, GL_TRUE);
	} else {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	}

	glTexImage2D(GL_TEXTURE_2D, 0, intfmt, tex_width, tex_height, 0, pixfmt, pixtype, 0);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, pixfmt, pixtype, img.pixels);

	tmat.scaling((float)width / (float)tex_width, (float)height / (float)tex_height, 1);
	return true;
}

const Mat4 &Texture::texture_matrix() const
{
	return tmat;
}

void Texture::bind(bool loadmat) const
{
	if(!tex) return;

	if(loadmat) {
		glMatrixMode(GL_TEXTURE);
		glLoadMatrixf(tmat[0]);
		glMatrixMode(GL_MODELVIEW);
	}

	glBindTexture(GL_TEXTURE_2D, tex);
}
