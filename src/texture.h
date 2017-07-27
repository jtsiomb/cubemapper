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

	bool load(const char *fname);

	const Mat4 &texture_matrix() const;
	void bind(bool loadmat = true) const;
};

#endif	// TEXTURE_H_
