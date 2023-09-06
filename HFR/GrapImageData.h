#pragma once

#include <QImage>

class GrapImageData {
public:
	GrapImageData() {
		faceNum = 0;
	}

	GrapImageData(const GrapImageData &other) {
		this->faceNum = other.faceNum;
		this->img = other.img.copy();
		this->time = other.time;
	}

	int faceNum;
	QImage img;
	QString time;

};
