#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/fb.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <qrencode.h>
#include <getopt.h>

void printUsage(){
	fputs("pwhxyv\n",stderr);
	exit(1);
}
void printInvalidArgumentValue(char * argument, char * value){
	fprintf(stderr,"%s is not a valid value for %s\n",value,argument);
	exit(1);
}
void printInvalidInputLength(){
	fputs("QR input data must be less than 256 bytes.",stderr);
	exit(1);
}

static int module_pixels = 3, border_width_modules = 4, border_height_modules = 4, x_offset = 0, y_offset = 0, verbose = 0;

const char * short_options = "+p:w:h:x:y:v";

const struct option long_options[] = {
		{"module_pixels",required_argument,NULL,'p'},
		{"border_width_modules",required_argument,NULL,'w'},
		{"border_height_modules",required_argument,NULL,'h'},
		{"x_offset",required_argument,NULL,'x'},
		{"y_offset",required_argument,NULL,'y'},
		{"verbose",no_argument,NULL,'v'},
		{NULL,0,0,0}
};

int main(int argc, char **argv) {
	int longindex;
	int option;
	while ((option = getopt_long(argc,argv,short_options,long_options, &longindex)) != -1){
		switch (option){
		case 'p':
			if ((module_pixels = atoi(optarg)) < 1 || module_pixels > 9)
				printInvalidArgumentValue("the number of module pixels.",optarg);
			break;
		case 'w':
			if ((border_width_modules = atoi(optarg)) < 0 || border_width_modules > 9)
				printInvalidArgumentValue("the border module width.",optarg);
			break;
		case 'h':
			if ((border_height_modules = atoi(optarg)) < 0 || border_height_modules > 9)
				printInvalidArgumentValue("the border module height.",optarg);
			break;
		case 'x': x_offset = atoi(optarg); break;
		case 'y': y_offset = atoi(optarg); break;
		case 'v': verbose = 1; break;
		case '?': exit(1); break;
		}
	}
	
	if (verbose){
		printf("module_pixels %d\n",module_pixels);
		printf("border_width_modules %d\n",border_width_modules);
		printf("border_height_modules %d\n",border_height_modules);
		printf("x_offset %d\n",x_offset);
		printf("y_offset %d\n",y_offset);
	}
	
	const int input_buffer_length = 257;
	char input[input_buffer_length];
	
	if (isatty(STDIN_FILENO)){
		int nonoptions = argc - optind;
		if (nonoptions != 1) printUsage();
		if (strlen(argv[argc - 1]) < input_buffer_length) strcpy(input,argv[argc - 1]);
		else printInvalidInputLength();
	} else {
		int input_length = read(STDIN_FILENO,input,input_buffer_length - 1);
		if (input_length < 0){
			perror("read");
			return 1;
		} else if (input_length == input_buffer_length) printInvalidInputLength();
		input[input_length] = 0;
	}
	
	if (verbose) printf("Encoding %s\n",input);
	//if (verbose) printf("argc %d optind %d longindex %d\n",argc,optind,longindex);

	int bpp; //bytes per pixel
	int bpl; //bytes per line
	int x_max; //maximum x
	int y_min; //minimum y
	int qr_width;

	struct fb_var_screeninfo var_info;
	struct fb_fix_screeninfo fix_info;

	char *buffer;
	size_t buffer_length;

	int fd = fd = open("/dev/fb0", O_RDWR);
	if (fd < 0){
		perror("open");
		return 1;
	}
	if (ioctl(fd, FBIOGET_VSCREENINFO, &var_info) < 0 || ioctl(fd, FBIOGET_FSCREENINFO, &fix_info) < 0){
		perror("ioctl");
		return 1;
	}

	if (verbose){
		printf("X RES %d\n",var_info.xres);
		printf("Y RES %d\n",var_info.yres);
		printf("X VRES %d\n",var_info.xres_virtual);
		printf("Y VRES %d\n",var_info.yres_virtual);
		printf("X OFF %d\n",var_info.xoffset);
		printf("Y OFF %d\n",var_info.yoffset);
		printf("Pixel Bits %d\n",var_info.bits_per_pixel);
		printf("Line Bytes %d\n",fix_info.line_length);
	}

	bpp = var_info.bits_per_pixel >> 3;
	bpl = fix_info.line_length;
	x_max = var_info.xres; // use real not virtual buffer size
	y_min = 0;
	
	buffer_length = var_info.yres_virtual * fix_info.line_length;
	buffer = mmap(NULL, buffer_length, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
	if (buffer == MAP_FAILED){
		perror("mmap");
		return 1;
	}

	QRcode *qr = QRcode_encodeString(input, 0, QR_ECLEVEL_H,QR_MODE_8,1);
	
	if (verbose) printf("QR Width %d\n",qr->width);
	qr_width = qr->width;

	for (int i = -border_height_modules; i < qr_width + border_height_modules; i++){
		for (int j = -border_width_modules; j < qr_width + border_width_modules; j++){
			int black = (i < 0 || j < 0 || i >= qr_width || j >= qr_width) ? 0 : qr->data[i * qr_width + j] & 1;
			int x = x_max - x_offset + ((j - qr_width - border_width_modules) * module_pixels);
			int y = y_min + y_offset + ((i + border_height_modules) * module_pixels);
			for (int k = 0; k < module_pixels; k++){
				for (int l = 0; l < module_pixels; l++){
					int x_r = x + k;
					int y_r = y + l;
					if (x_r < 0 || x_r >= var_info.xres || y_r < 0 || y_r >= var_info.yres) continue;
					for (int m = 0; m < bpp; m++) buffer[y_r * bpl + x_r * bpp + m] = black ? 0x00 : 0xff;
				}
			}
		}
	}

	if (buffer && buffer != MAP_FAILED) munmap(buffer, buffer_length);
	if (fd >= 0) close(fd);
}
