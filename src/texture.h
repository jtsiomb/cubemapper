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
#ifndef TEXTURE_H_
#define TEXTURE_H_

#include <gmath/gmath.h>

class Texture {
private:
	int width, height;
	int tex_width, tex_height;
	unsigned int tex;
	Mat4 tmat;

public:
	Texture();
	~Texture();

	int get_width() const;
	int get_height() const;

	bool load(const char *fname);

	const Mat4 &texture_matrix() const;
	void bind(bool loadmat = true) const;
};

#endif	// TEXTURE_H_
