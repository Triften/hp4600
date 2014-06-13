//first strip is 212 pixels (636 bytes)
//333 pixels wide, 4 pixel vertical offset
//also 2 pixel overlap on strips. :P 16 strips, 15 overlaps, 30 pixels lost
//14 999 byte sections
//254 at end of line

//5128 x 7054 start 15384 bytes per line
//5098 x 7050 end 15294 per line

//TODO: Use gray strip to calibrate color
// One cal value for light bands one for dark? :(
// One cal value per band?
// One cal value per column?
// assume that the lighter bands are correct and calibrate based on overlaps?

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

unsigned char confine(float val){
	if(val > 255) {
		return 255;
	}
	if(val < 0){
		return 0;
	}
	return (unsigned char)val;
}

int main(int argc, char **argv) {
	FILE * infile;
	unsigned char inbuf[1024];
	unsigned char line[15384];
	float rcal[16];
	float gcal[16];
	float bcal[16];
	float rdiff;
	float gdiff;
	float bdiff;
	int temppix;
	int x, i, j, m;
	unsigned int rtotal[16];
	unsigned int gtotal[16];
	unsigned int btotal[16];

	int linenum = 0;
	if (argc!=2) {
		printf("usage: %s inputfile\n", argv[0]);
		exit(1);
	}

	if (!(infile = fopen(argv[1], "r")))
    	{
		exit(1);
	}
	
	

	for(x = 0; x < 16; x++){
		rcal[x] = 0.0;
		gcal[x] = 0.0;
		bcal[x] = 0.0;
		rtotal[x] = 0;
		gtotal[x] = 0;
		btotal[x] = 0;
	}

	//find calibration values:
	//sum the values in each band for the first four lines
	for(x = 0; x < 4; x++){

		//strip 0
		fseek(infile, ((7052-8+x)*15384+54), SEEK_SET);
		fread(inbuf, 636, 1, infile);
		for(i = 0; i < 636; i++){
			m = i%3;
			if(m == 0){
				btotal[0] += (unsigned int)inbuf[i];
			}
			if(m == 1){
				gtotal[0] += (unsigned int)inbuf[i];
			}
			if(m == 2){
				rtotal[0] += (unsigned int)inbuf[i];
			}
		}
		
		//strips 1-14
		for(j = 1; j < 15; j++){
			fseek(infile, 999, SEEK_CUR);
			fread(inbuf, 999, 1, infile);
			for(i = 0; i < 999; i++){
				m = i%3;
				if(m == 0){
					btotal[j] += (unsigned int)inbuf[i];
				}
				if(m == 1){
					gtotal[j] += (unsigned int)inbuf[i];
				}
				if(m == 2){
					rtotal[j] += (unsigned int)inbuf[i];
				}
			}
		}
		//strip 15
		fread(inbuf, 762, 1, infile);
		for(i = 0; i < 762; i++){
			m = i%3;
			if(m == 0){
				btotal[15] += (unsigned int)inbuf[i];
			}
			if(m == 1){
				gtotal[15] += (unsigned int)inbuf[i];
			}
			if(m == 2){
				rtotal[15] += (unsigned int)inbuf[i];
			}
		}
	
	}

	//strip 0 is 212 * 4
	rcal[0] = (float) rtotal[0] / 848.0;
	gcal[0] = (float) gtotal[0] / 848.0;
	bcal[0] = (float) btotal[0] / 848.0;

	//strips 1-14 is 333 * 4
	for(j = 1; j < 15; j++){
		rcal[j] = (float) rtotal[j] / 1332.0;
		gcal[j] = (float) gtotal[j] / 1332.0;
		bcal[j] = (float) btotal[j] / 1332.0;
	}
	//strips 15 is 254 * 4
	rcal[15] = (float) rtotal[15] / 1016.0;
	gcal[15] = (float) gtotal[15] / 1016.0;
	bcal[15] = (float) btotal[15] / 1016.0;

/*	for(j = 0; j < 16; j++){
		printf("%2d R: %f\n", j, rcal[j]);
		printf("%2d G: %f\n", j, gcal[j]);
		printf("%2d B: %f\n", j, bcal[j]);
	}
	
	return 0;*/

	//print first 54 bytes
	fwrite(bmp_file_header, 54, 1, stdout);


	//reverse the lines
	for(linenum = 7050; linenum >= 0; linenum--){

		//seek 4 lines ahead
		fseek(infile, ((linenum+4)*15384+54), SEEK_SET);

		//strip 0
		x = 0;
		fread(inbuf, 636, 1, infile);
		rdiff = 220.0 - rcal[x];
		gdiff = 220.0 - gcal[x];
		bdiff = 220.0 - bcal[x];
		for(i = 0; i < 636; i++){
			m = i%3;
			if(m == 0){
				inbuf[i] = confine((float)inbuf[i] + bdiff); //blue
			}
			if(m == 1){
				inbuf[i] = confine((float)inbuf[i] + gdiff); //green
			}
			if(m == 2){
				inbuf[i] = confine((float)inbuf[i] + rdiff); //red
			}
		}
		memcpy(line, inbuf, 636);

		//2 through 14
		for(x = 2; x < 15; x+=2){
			fseek(infile, 999, SEEK_CUR);
			fread(inbuf, 999, 1, infile);
			rdiff = 220.0 - rcal[x];
			gdiff = 220.0 - gcal[x];
			bdiff = 220.0 - bcal[x];
/*
			rdiff = (rcal[x-1] + rcal[x+1])/2 - rcal[x];
			gdiff = (gcal[x-1] + gcal[x+1])/2 - gcal[x];
			bdiff = (bcal[x-1] + bcal[x+1])/2 - bcal[x];
*/
			for(i = 0; i < 999; i++){
				m = i%3;
				if(m == 0){
					inbuf[i] = confine((float)inbuf[i] + bdiff); //blue
				}
				if(m == 1){
					inbuf[i] = confine((float)inbuf[i] + gdiff); //green
				}
				if(m == 2){
					inbuf[i] = confine((float)inbuf[i] + rdiff); //red
				}
			}
			memcpy(line+630+993*(x-1), inbuf, 999);
		}


		fseek(infile, (linenum*15384+54+636), SEEK_SET);
		//1 through 13
		for(x = 1; x < 14; x+=2){
			
			fread(inbuf, 999, 1, infile);
			rdiff = 220.0 - rcal[x];
			gdiff = 220.0 - gcal[x];
			bdiff = 220.0 - bcal[x];
/*
			rdiff = (rcal[x-1] + rcal[x+1])/2 - rcal[x];
			gdiff = (gcal[x-1] + gcal[x+1])/2 - gcal[x];
			bdiff = (bcal[x-1] + bcal[x+1])/2 - bcal[x];
*/
			for(i = 0; i < 999; i++){
				m = i%3;
				if(m == 0){
					inbuf[i] = confine((float)inbuf[i] + bdiff); //blue
				}
				if(m == 1){
					inbuf[i] = confine((float)inbuf[i] + gdiff); //green
				}
				if(m == 2){
					inbuf[i] = confine((float)inbuf[i] + rdiff); //red
				}
			}
			memcpy(line+630+993*(x-1), inbuf, 999);
			fseek(infile, 999, SEEK_CUR);
		}

		//strip 15
		x = 15;
		fread(inbuf, 762, 1, infile);
		rdiff = 220.0 - rcal[x];
		gdiff = 220.0 - gcal[x];
		bdiff = 220.0 - bcal[x];
/*
		rdiff = rcal[x-1] - rcal[x];
		gdiff = gcal[x-1] - gcal[x];
		bdiff = bcal[x-1] - bcal[x];
*/
		for(i = 0; i < 762; i++){
			m = i%3;
			if(m == 0){
				inbuf[i] = confine((float)inbuf[i] + bdiff); //blue
			}
			if(m == 1){
				inbuf[i] = confine((float)inbuf[i] + gdiff); //green
			}
			if(m == 2){
				inbuf[i] = confine((float)inbuf[i] + rdiff); //red
			}
		}
		memcpy(line+630+993*14, inbuf, 762);

		//Are we printing a non-multiple of 3 bytes here?
		//reverse the pixels (fixes color and rotation.)
		for (i=15296; i>= 1; i--) {
		//for (i=1; i<= 15296; i++) {
			printf("%c", line[i]);
		}

	}

	close(infile);
}
