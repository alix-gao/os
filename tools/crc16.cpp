#include <stdio.h>
#include <stdlib.h>


unsigned short crc16(unsigned char *data_p,unsigned long length)
{
	unsigned char x;
	unsigned short crc = 0xffff;

	while (length--) {
	    x = crc >> 8 ^ *data_p++;
	    x ^= x >> 4;
	    crc = (crc << 8) ^ ((unsigned short)(x << 12)) ^ ((unsigned short)(x << 5)) ^ ((unsigned short) x);
	}
	return crc;
}

int main()
{
	FILE *fp_src;
    unsigned char data[] = {'1', '2', '3', '4', '5', '6', '7', '8', '9', 0};

    fp_src = fopen("os4.bin", "rb");
    if (NULL == fp_src) {
        printf("cannot open file u.ima\n");
        return 0;
    }

    int len = 6103040;
    unsigned char *c;
    int i;
    c = (unsigned char *) malloc(len);
    for (i = 0; i < len; i++) {
    	fread(c+i, sizeof(char), 1, fp_src);
    }
    if (fp_src) { fclose(fp_src); }

	for (i = 0; i < 0x10; i++) {
		printf("%x", c[i + 0x43cc00]);
	} printf("\n");
    printf("%x %x \n%x %x %x\n%x\n", crc16(data, 10), crc16((unsigned char *)"123456789", 9),
     crc16(c, len), crc16(c, 0x43cc00), crc16(c+0x43cc00, 0x400),
     crc16(c, 0x5d1400));
    system("pause");
}

