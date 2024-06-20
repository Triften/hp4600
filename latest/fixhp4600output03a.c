//first strip is 212 pixels (636 bytes)
//333 pixels wide, 4 pixel offset
//also 2 pixel overlap on strips. :P 16 strips, 15 overlaps, 30 pixels lost
//14 999 byte sections
//254 at end of line

//5128 x 7054 start 15384 bytes per line
//5098 x 7050 end 15294 per line

//TODO: Use gray strip to calibrate color

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char bmp_file_header[] = {
	0x42, 0x4d, 0xf6, 0x26, 0xbe, 0x06, 0x00, 0x00,
	0x00, 0x00, 0x36, 0x00, 0x00, 0x00, 0x28, 0x00,
	0x00, 0x00, 0xea, 0x13, 0x00, 0x00, 0x8a, 0x1b,
	0x00, 0x00, 0x01, 0x00, 0x18, 0x00, 0x00, 0x00,
	0x00, 0x00, 0xc0, 0x26, 0xbe, 0x06, 0x46, 0x5c,
	0x00, 0x00, 0x46, 0x5c, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

void print_file(char *bytes, int len, FILE *output){
    int i;
    if (len > 0) {
	for (i=0; i<len; i++) {
	    fprintf(output, "%c", bytes[i]);
	}
    }
}

int main(int argc, char **argv) {
	FILE * infile;
	char inbuf[1024];
	char line[15384];
	float rcal[15];
	float gcal[15];
	float bcal[15];
	int x, i, m;
	int rtotal, gtotal, btotal;
	int linenum = 0;
	if (argc!=2) {
		printf("usage: %s inputfile\n", argv[0]);
		exit(1);
	}

	if (!(infile = fopen(argv[1], "r")))
    	{
		exit(1);
	}
	
	
	//print first 54 bytes
	//fwrite(bmp_file_header, 54, 1, stdout);

	//find calibration values:
	//strip 0
	fseek(infile, ((7052-4)*15384+54), SEEK_SET);
	fread(inbuf, 636, 1, infile);
	for(i = 0; i < 636; i++){
		m = i%3;
		if(m == 0){
			btotal += inbuf[i];
		}
		if(m == 1){
			gtotal += inbuf[i];
		}
		if(m == 2){
			rtotal += (unsigned int)inbuf[i];
		}
		printf("R: %d\n", rtotal);
	}

	rcal[0] = 255.0/((float) rtotal / 212.0);
	gcal[0] = 255.0/((float) gtotal / 212.0);
	bcal[0] = 255.0/((float) btotal / 212.0);
	
	printf("R: %d %f\n", rtotal,rcal[0]);
	printf("G: %d %f\n", gtotal,gcal[0]);
	printf("B: %d %f\n", btotal,bcal[0]);
	
return 0;

//reverse the lines
for(linenum = 7052; linenum >= 0; linenum--){

	//seek 4 lines ahead
	fseek(infile, ((linenum)*15384+54), SEEK_SET);

	//strip 0
	fread(inbuf, 636, 1, infile);
	memcpy(line, inbuf, 636);
	//strip 2
	fseek(infile, 999, SEEK_CUR);
	fread(inbuf, 999, 1, infile);
	memcpy(line+630+993, inbuf, 999);
	//strip 4
	fseek(infile, 999, SEEK_CUR);
	fread(inbuf, 999, 1, infile);
	memcpy(line+630+993*3, inbuf, 999);
	//strip 6
	fseek(infile, 999, SEEK_CUR);
	fread(inbuf, 999, 1, infile);
	memcpy(line+630+993*5, inbuf, 999);
	//strip 8
	fseek(infile, 999, SEEK_CUR);
	fread(inbuf, 999, 1, infile);
	memcpy(line+630+993*7, inbuf, 999);
	//strip 10
	fseek(infile, 999, SEEK_CUR);
	fread(inbuf, 999, 1, infile);
	memcpy(line+630+993*9, inbuf, 999);

	//strip 12
	fseek(infile, 999, SEEK_CUR);
	fread(inbuf, 999, 1, infile);
	memcpy(line+630+993*11, inbuf, 999);

	//strip 14
	fseek(infile, 999, SEEK_CUR);
	fread(inbuf, 999, 1, infile);
	memcpy(line+630+993*13, inbuf, 999);


	fseek(infile, (linenum*15384+54+636), SEEK_SET);

	//do odd strips.
	//strip 1
	fread(inbuf, 999, 1, infile);
	memcpy(line+630, inbuf, 999);
	//strip 3
	fseek(infile, 999, SEEK_CUR);
	fread(inbuf, 999, 1, infile);
	memcpy(line+630+993*2, inbuf, 999);
	//strip 5
	fseek(infile, 999, SEEK_CUR);
	fread(inbuf, 999, 1, infile);
	memcpy(line+630+993*4, inbuf, 999);
	//strip 7
	fseek(infile, 999, SEEK_CUR);
	fread(inbuf, 999, 1, infile);
	memcpy(line+630+993*6, inbuf, 999);
	//strip 9
	fseek(infile, 999, SEEK_CUR);
	fread(inbuf, 999, 1, infile);
	memcpy(line+630+993*8, inbuf, 999);
	//strip 11
	fseek(infile, 999, SEEK_CUR);
	fread(inbuf, 999, 1, infile);
	memcpy(line+630+993*10, inbuf, 999);
	//strip 13
	fseek(infile, 999, SEEK_CUR);
	fread(inbuf, 999, 1, infile);
	memcpy(line+630+993*12, inbuf, 999);
	//strip 15
	fseek(infile, 999, SEEK_CUR);
	fread(inbuf, 762, 1, infile);
	memcpy(line+630+993*14, inbuf, 999);

	//Are we printing a non-multiple of 3 bytes here?
	//reverse the pixels (fixes color and rotation.)
	for (i=15296; i>= 1; i--) {
		printf("%c", line[i]);
	}
	

	

}

close(infile);
}
