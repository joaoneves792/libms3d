/*
 *	LXSR Load textures
 * 	adapted from multiple sources by E 
 *	Texture.cpp
 *
 *  Created on: Jun 2, 2012
 */

#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <string>
#include <iostream>
#include <GL/glut.h>
#include <jpeglib.h>
#include <png.h>

/*****************************************/
/*  Load Bitmaps, Jpegs, Pngs And Convert To Textures */
/*  Big mess of C and C++ code */


typedef struct {
    int width;
    int height;
    bool alpha;
    unsigned char *data;
}textureImage;

int loadPNG(const char* filename, textureImage* texture){
	png_structp png_ptr;
	png_infop info_ptr;
	unsigned int sig_read = 0;
	int color_type, interlace_type;
	FILE *fp;

	if ((fp = fopen(filename, "rb")) == NULL)
        	return 0;
	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (png_ptr == NULL) {
		fclose(fp);
		return 0;
	}
	info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == NULL) {
		fclose(fp);
		png_destroy_read_struct(&png_ptr, NULL, NULL);
		return 0;
	}
	if (setjmp(png_jmpbuf(png_ptr))) {
		png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
		fclose(fp);
		return 0;
	}
	png_init_io(png_ptr, fp);

	png_set_sig_bytes(png_ptr, sig_read);
	png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_STRIP_16 | PNG_TRANSFORM_PACKING | PNG_TRANSFORM_EXPAND, NULL);



	texture->width = png_get_image_width(png_ptr,  info_ptr);
	texture->height = png_get_image_height(png_ptr, info_ptr);

	switch (png_get_color_type(png_ptr, info_ptr)) {
		case PNG_COLOR_TYPE_RGBA:
			texture->alpha = true;
			break;
		case PNG_COLOR_TYPE_RGB:
			texture->alpha = false;
			break;
		default:
		std::cout << "Color type " << png_get_color_type(png_ptr, info_ptr) << " not supported" << std::endl;
		png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
		fclose(fp);
		return 0;
	}
	unsigned int row_bytes = png_get_rowbytes(png_ptr, info_ptr);
	texture->data = (unsigned char*)malloc(row_bytes * texture->height);

	png_bytepp row_pointers = png_get_rows(png_ptr, info_ptr);

	for (int i = 0; i < texture->height; i++) {// note that png is ordered top to
		memcpy(texture->data+(row_bytes * (texture->height-1-i)), row_pointers[i], row_bytes);
	}

	png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
	fclose(fp);
	return 1;
}

int loadJPEG(const char* filename, textureImage* texture){
	FILE* infile;
	struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr jerr;
	int row_stride;     /* physical row width in output buffer */
	long counter = 0;
	int buffer_height = 1;

	texture->alpha = false;
	
	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_decompress(&cinfo);
	if ((infile = fopen(filename, "rb")) == NULL) {
            fprintf(stderr, "can't open %s\n", filename);
            exit(1);
        }
        jpeg_stdio_src(&cinfo, infile);

	jpeg_read_header(&cinfo, TRUE);
	jpeg_start_decompress(&cinfo);
	
	texture->width = cinfo.output_width;
	texture->height = cinfo.output_height;
	texture->data = new unsigned char[cinfo.output_width*cinfo.output_height*cinfo.output_components];

	row_stride = cinfo.output_width * cinfo.output_components;
    	

	JSAMPARRAY buffer = (JSAMPARRAY)malloc(sizeof(JSAMPROW)*buffer_height);


	/* Make a one-row-high sample array that will go away when done with image */
    	buffer = (*cinfo.mem->alloc_sarray) ((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);
	buffer[0] = (JSAMPROW)malloc(sizeof(JSAMPLE) * row_stride);

	while (cinfo.output_scanline < cinfo.output_height) {
		jpeg_read_scanlines(&cinfo, buffer, 1);
		memcpy(texture->data+counter, buffer[0], row_stride);
		counter += row_stride;
	}

	jpeg_finish_decompress (&cinfo);
	jpeg_destroy_decompress (&cinfo);
	fclose(infile);	
	return 1;
}


/* simple loader for 24bit bitmaps (data is in rgb-format) */
int loadBMP(const char *filename, textureImage *texture)
{
    FILE *file;
    unsigned short int bfType;
    long int bfOffBits;
    short int biPlanes;
    short int biBitCount;
    long int biSizeImage;
    int i;
    unsigned char temp;

	texture->alpha = false;

    /* make sure the file is there and open it read-only (binary) */
    if ((file = fopen(filename, "rb")) == NULL)
    {
        printf("File not found : %s\n", filename);
        return 0;
    }
    if(!fread(&bfType, sizeof(short int), 1, file))
    {
        printf("Error reading file!\n");
        return 0;
    }
    /* check if file is a bitmap */
    if (bfType != 19778)
    {
        printf("Not a Bitmap-File!\n");
        return 0;
    }        
    /* get the file size */
    /* skip file size and reserved fields of bitmap file header */
    fseek(file, 8, SEEK_CUR);
    /* get the position of the actual bitmap data */
    if (!fread(&bfOffBits, sizeof(long int), 1, file))
    {
        printf("Error reading file!\n");
        return 0;
    }
    printf("Data at Offset: %ld\n", bfOffBits);
    /* skip size of bitmap info header */
    fseek(file, 4, SEEK_CUR);
    /* get the width of the bitmap */
    fread(&texture->width, sizeof(int), 1, file);
    printf("Width of Bitmap: %d\n", texture->width);
    /* get the height of the bitmap */
    fread(&texture->height, sizeof(int), 1, file);
    printf("Height of Bitmap: %d\n", texture->height);
    /* get the number of planes (must be set to 1) */
    fread(&biPlanes, sizeof(short int), 1, file);
    if (biPlanes != 1)
    {
        printf("Error: number of Planes not 1!\n");
        return 0;
    }
    /* get the number of bits per pixel */
    if (!fread(&biBitCount, sizeof(short int), 1, file))
    {
        printf("Error reading file!\n");
        return 0;
    }
    printf("Bits per Pixel: %d\n", biBitCount);
    if (biBitCount != 24)
    {
        printf("Bits per Pixel not 24\n");
        return 0;
    }
    /* calculate the size of the image in bytes */
    biSizeImage = texture->width * texture->height * 3;
    printf("Size of the image data: %ld\n", biSizeImage);
    texture->data = (unsigned char *)malloc(biSizeImage);
    /* seek to the actual data */
    fseek(file, bfOffBits, SEEK_SET);
    if (!fread(texture->data, biSizeImage, 1, file))
    {
        printf("Error loading file!\n");
        return 0;
    }
    /* swap red and blue (bgr -> rgb) */
    for (i = 0; i < biSizeImage; i += 3)
    {
        temp = texture->data[i];
        texture->data[i] = texture->data[i + 2];
        texture->data[i + 2] = temp;
    }
    return 1;
}

GLuint LoadGLTexture(const char* name){
	bool status;
	textureImage *texti;
	GLuint texPntr[1];
	std::string fn(name);

	status = false;
	texti = (textureImage *)malloc(sizeof(textureImage));
	
	if(!fn.substr(fn.find_last_of(".")+1).compare("bmp"))
		loadBMP(name, texti);
	else if(!fn.substr(fn.find_last_of(".")+1).compare("jpg") || !fn.substr(fn.find_last_of(".")+1).compare("jpeg") )   
		loadJPEG(name, texti);
	else if(!fn.substr(fn.find_last_of(".")+1).compare("png"))
		loadPNG(name, texti);
	
     if(texti){
        status = true;
        glGenTextures(1, &texPntr[0]);   /* create the texture */
        glBindTexture(GL_TEXTURE_2D, texPntr[0]);
        /* actually generate the texture */
        glTexImage2D(GL_TEXTURE_2D, 0, 3, texti->width, texti->height, 0,
            (texti->alpha)?GL_RGBA:GL_RGB, GL_UNSIGNED_BYTE, texti->data);
        /* enable linear filtering */
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	/* free the ram we used in our texture generation process */
        if (texti->data)
            free(texti->data);
        free(texti);
    }    
    return texPntr[0];
}
