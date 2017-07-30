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
#ifndef MESHGEN_H_
#define MESHGEN_H_

#include <gmath/gmath.h>

class Mesh;

void gen_sphere(Mesh *mesh, float rad, int usub, int vsub, float urange = 1.0, float vrange = 1.0);
void gen_geosphere(Mesh *mesh, float rad, int subdiv, bool hemi = false);
void gen_torus(Mesh *mesh, float mainrad, float ringrad, int usub, int vsub, float urange = 1.0, float vrange = 1.0);
void gen_cylinder(Mesh *mesh, float rad, float height, int usub, int vsub, int capsub = 0, float urange = 1.0, float vrange = 1.0);
void gen_cone(Mesh *mesh, float rad, float height, int usub, int vsub, int capsub = 0, float urange = 1.0, float vrange = 1.0);
void gen_plane(Mesh *mesh, float width, float height, int usub = 1, int vsub = 1);
void gen_heightmap(Mesh *mesh, float width, float height, int usub, int vsub, float (*hf)(float, float, void*), void *hfdata = 0);
void gen_box(Mesh *mesh, float xsz, float ysz, float zsz, int usub = 1, int vsub = 1);

void gen_revol(Mesh *mesh, int usub, int vsub, Vec2 (*rfunc)(float, float, void*), void *cls = 0);
void gen_revol(Mesh *mesh, int usub, int vsub, Vec2 (*rfunc)(float, float, void*), Vec2 (*nfunc)(float, float, void*), void *cls);

/* callback args: (float u, float v, void *cls) -> Vec2 XZ offset u,v in [0, 1] */
void gen_sweep(Mesh *mesh, float height, int usub, int vsub, Vec2 (*sfunc)(float, float, void*), void *cls = 0);

#endif	// MESHGEN_H_
