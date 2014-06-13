/* This file was originally generated with usbsnoop2libusb.pl from a usbsnoop log file. */
/* Latest version of the script should be in http://iki.fi/lindi/usb/usbsnoop2libusb.pl */

/*
All giant tables have been moved to a .h file
*/

/*TODO
Add option for other device and vendor IDs?
Figure out how to calibrate properly.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <signal.h>
#include <ctype.h>
#include <usb.h>
#include "hp4600consts.h"

#if 0
 #include <linux/usbdevice_fs.h>
 #define LIBUSB_AUGMENT
 #include "libusb_augment.h"
#endif

#define PRINTREGREADS	1


struct usb_dev_handle *devh;

void release_usb_device(int dummy) {
    int ret;
    ret = usb_release_interface(devh, 0);
    if (!ret)
	printf("failed to release interface: %d\n", ret);
    usb_close(devh);
    if (!ret)
	printf("failed to close interface: %d\n", ret);
    exit(1);
} 

struct usb_device *find_device(int vendor, int product) {
    struct usb_bus *bus;
    
    for (bus = usb_get_busses(); bus; bus = bus->next) {
	struct usb_device *dev;
	
	for (dev = bus->devices; dev; dev = dev->next) {
	    if (dev->descriptor.idVendor == vendor
		&& dev->descriptor.idProduct == product)
		return dev;
	}
    }
    return NULL;
}

void print_bytes(char *bytes, int len) {
    int i;
    if (len > 0) {
	for (i=0; i<len; i++) {
	    if(i > 0 && i % 16 == 0){
		printf("\n");
	    }
	    printf("%02x ", (int)((unsigned char)bytes[i]));
	}
	printf("\"");
        for (i=0; i<len; i++) {
	    printf("%c", isprint(bytes[i]) ? bytes[i] : '.');
        }
        printf("\"");
    }
}

void print_file(char *bytes, int len, FILE *output){
    int i;
    if (len > 0) {
	for (i=0; i<len; i++) {
	    fprintf(output, "%c", bytes[i]);
	}
    }
}

/* buffer consists of register/value pairs */
int writeRegisters(char *buf){
	return usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, REGISTERS, 0x0000000, buf, 0x0000004, 1000);
}

int read_register(char *buf, char reg){
	int ret;
	char reg_command[] = {0x8b, 0x00, 0x8b, 0x00};
	reg_command[1] = reg_command[3] = reg;

	memcpy(buf, reg_command, 0x0000004);
	ret = writeRegisters(buf); //write to status register
	ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE + USB_ENDPOINT_IN, 0x000000c, 0x0000007, 0x0000000, buf, 0x0000001, 1000); //read result

#ifdef PRINTREGREADS
	printf("Register %02x = ", reg);
	print_bytes(buf, ret);
	printf("\n");
#endif
	return ret;

}

int select_register_bank(char *buf, int bank){
	int ret;
	char reg_command[] = {0x5f, 0x00, 0x5f, 0x00};
	if(bank < 0 || bank > 2){
		return 0;
	}
	reg_command[1] = reg_command[3] = bank;
	printf("Switching to register %d\n", bank);
	memcpy(buf, reg_command, 0x0000004);
	ret = writeRegisters(buf);
	return ret;
}

int openscanchip(char *buf){
	int ret;
	printf("Opening chip\n");
	memcpy(buf, "\x64\x64\x64\x64", 0x0000004);
	ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, 0x0000090, 0x0000000, buf, 0x0000004, 1000);
	if(ret != 0){
		return ret;
	}
	memcpy(buf, "\x65\x65\x65\x65", 0x0000004);
	ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, 0x0000090, 0x0000000, buf, 0x0000004, 1000);
	if(ret != 0){
		return ret;
	}
	memcpy(buf, "\x44\x44\x44\x44", 0x0000004);
	ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, 0x0000090, 0x0000000, buf, 0x0000004, 1000);
	if(ret != 0){
		return ret;
	}
	memcpy(buf, "\x45\x45\x45\x45", 0x0000004);
	ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, 0x0000090, 0x0000000, buf, 0x0000004, 1000);
	return ret;
}

int closescanchip(char *buf){
	int ret;
	printf("Closing chip\n");
	memcpy(buf, "\x64\x64\x64\x64", 0x0000004);
	ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, 0x0000090, 0x0000000, buf, 0x0000004, 1000);
	if(ret != 0){
		return ret;
	}
	memcpy(buf, "\x65\x65\x65\x65", 0x0000004);
	ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, 0x0000090, 0x0000000, buf, 0x0000004, 1000);
	if(ret != 0){
		return ret;
	}
	memcpy(buf, "\x16\x16\x16\x16", 0x0000004);
	ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, 0x0000090, 0x0000000, buf, 0x0000004, 1000);
	if(ret != 0){
		return ret;
	}
	memcpy(buf, "\x17\x17\x17\x17", 0x0000004);
	ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, 0x0000090, 0x0000000, buf, 0x0000004, 1000);
	return ret;
}

int do_command_one(char *buf){
	int ret;
	memcpy(buf, command_one, 0x0000010);

	ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, 0x000000a, 0x0000000, buf, 0x0000010, 1000);
	printf("Command One %d\n", ret);
	return ret;
}

int do_command_two(char *buf){
	int ret;
	memcpy(buf, command_two, 0x0000010);

	ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, 0x000000a, 0x0000000, buf, 0x0000010, 1000);
	printf("Command Two %d\n", ret);
	return ret;
}

int do_command_three(char *buf){
	int ret;
	memcpy(buf, command_three, 0x0000010);
	ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, 0x000000a, 0x0000000, buf, 0x0000010, 1000);
	printf("Command Three %d\n", ret);
	ret = usb_bulk_read(devh, 0x00000082, buf, 0x0000200, 1030);
	printf("\t");
	print_bytes(buf, ret);
	printf("\n");
	return ret;
}

int do_command_four(char *buf){
	int ret;
	memcpy(buf, command_four, 0x0000010);
	ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, 0x000000a, 0x0000000, buf, 0x0000010, 1000);
	printf("Command Four %d\n", ret);
	ret = usb_bulk_read(devh, 0x00000082, buf, 0x0000200, 1030);
	print_bytes(buf, ret);
	printf("\n");
	return ret;
}

int do_command_five(char *buf){
	int ret;
	memcpy(buf, command_five, 0x0000010);

	ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, 0x000000a, 0x0000000, buf, 0x0000010, 1000);
	printf("Command Five %d\n", ret);
	return ret;
}

int do_command_six(char *buf){
	int ret;
	memcpy(buf, command_six, 0x0000010);
	ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, 0x000000a, 0x0000000, buf, 0x0000010, 1000);
	printf("Command Six %d\n", ret);
	ret = usb_bulk_read(devh, 0x00000082, buf, 0x0000200, 1030);
	print_bytes(buf, ret);
	printf("\n");
	return ret;
}

int adjustPowerSaveRegisters(char *buf){
	int ret;
	/*Mustek:
	#define ES01_94_PowerSaveControl		0x94
	#define ES01_92_TimerPowerSaveTime		0x92
	*/
	memcpy(buf, "\x94\x31\x92\x57", 0x0000004);

	ret = writeRegisters(buf);
	printf("adjustPowerSaveRegisters? %d\n", ret);
	return ret;
}

int do_command_eight(char *buf){
	int ret;
	/*#define ES01_96_GPIOValue8_15			0x96*/
	memcpy(buf, "\x96\x21\x96\x21", 0x0000004);
	ret = writeRegisters(buf);
	printf("Command Eight - GPIO? %d\n", ret);
	return ret;
}

/*Same as Mustek*/
int clearFIFO(char *buf){
/*ret = clearFIFO(buf);*/
	int ret;
	printf("Clearing FIFO... ");
	memcpy(buf, "\x00\x00\x00\x00", 0x0000004);
	ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, 0x0000005, 0x0000000, buf, 0x0000004, 1000);
	printf("%d", ret);
	memcpy(buf, "\x00\x00\x00\x00", 0x0000004);
	ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, SCANCONTROL, 0x0000000, buf, 0x0000004, 1000);
	printf("%d\n", ret);
	return ret;
}

int readChipString(char *buf){
	int ret;
	/*registers read "S120"*/
	ret = read_register(buf, 0x1f);
	ret = read_register(buf, 0x1e);
	ret = read_register(buf, 0x1d);
	ret = read_register(buf, 0x1c);
	return ret;
}

int readScanState(char *buf){
	int ret;
	printf("Reading State\t");
	ret = read_register(buf, 0x01);
	return ret;
}

int doSDRAMSetup(char *buf){
	int ret;
	/*SDRAM Timing - see mustek_usb2_asic.c line 3592*/
	memcpy(buf, "\x87\xf1\x87\xa5\x87\x91\x87\x81" "\x87\x11\x87\xf0", 0x000000c);
	ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, REGISTERS, 0x0000000, buf, 0x000000c, 1000);
	printf("SDRAM initial sequence.\n");
	return ret;
}

int doCmdSequenceAlpha(char *buf){
	int ret;
	ret = do_command_one(buf);
	ret = do_command_two(buf);
	ret = do_command_three(buf);
	ret = do_command_four(buf);
	ret = readChipString(buf);
	return ret;
}

int setScannerIdle(char *buf){
	/*writing registers, disable trigger, enable clocks*/
	printf("Set Scanner Idle\n");
	memcpy(buf, "\xf4\x00\x86\x00", 0x0000004);
	return writeRegisters(buf);
}

int reg7cReadAndStuff(char *buf){
	int ret;

	/*Set trigger?*/
	memcpy(buf, "\xf8\x02\xf4\x01", 0x0000004);
	ret = writeRegisters(buf);

	usleep(16*1000);
	/*check status? (No idea what failure mode to watch for)*/
	ret = read_register(buf, 0x05);
	ret = read_register(buf, 0x04);
	usleep(30*1000);

	memcpy(buf, "\x7c\xbe\x7d\x7d\x7e\x00\x7f\x00", 0x0000008);
	ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, REGISTERS, 0x0000000, buf, 0x0000008, 1000);
	usleep(16*1000);
	memcpy(buf, "\x7c\xfb\x00\x00", 0x0000004);
	ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, DMAREAD, 0x0000000, buf, 0x0000004, 1000);

	usleep(10*1000);
	ret = usb_bulk_read(devh, 0x00000082, buf, 0x000fa00, 4840);

	usleep(10*1000);
	ret = usb_bulk_read(devh, 0x00000082, buf, 0x0000200, 1030);

	usleep(15*1000);
	ret = setScannerIdle(buf);
	usleep(30*1000);
	ret = readScanState(buf);

	return ret;
}

int setRegisters2xx(char *buf){
	int ret;
	ret = select_register_bank(buf, 2);
	memcpy(buf, 
		"\x60\x00\x61\x00\x62\x00\x63\x00" "\x64\x00\x65\x00\x66\x00\x67\x00" 
		"\x68\x00\x69\x00\x6a\x00\x6b\x00" "\x6c\x00\x6d\x00\x6e\x00\x6f\x00" 
		"\x70\x40\x71\x05\x72\x00\x73\x40" "\x74\x05\x75\x00\x76\x40\x77\x05" 
		"\x78\x00\x79\x40\x7a\x05\x7b\x00" "\x7c\x80\x7d\x0a\x7e\x00\x7f\x80" 
	, 0x0000040);
	ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, REGISTERS, 0x0000000, buf, 0x0000040, 1000);
	memcpy(buf, 
		"\x80\x0a\x81\x00\x82\x80\x83\x0a" "\x84\x00\x85\x80\x86\x0a\x87\x00" 
		"\x88\xc0\x89\x0f\x8a\x00\x8b\xc0" "\x8c\x0f\x8d\x00\x8e\xc0\x8f\x0f" 
		"\x90\x00\x91\xc0\x92\x0f\x93\x00" "\xb0\x00\xb1\x00\xb2\x00\xb3\x00" 
		"\xb4\x00\xb5\x00\xb6\x00\xb7\x00" "\xb8\x00\xb9\x00\xba\x00\xbb\x00" 
	, 0x0000040);
	ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, REGISTERS, 0x0000000, buf, 0x0000040, 1000);
	memcpy(buf, 
		"\xbc\x00\xbd\x00\xbe\x00\xbf\x00" "\xc0\x00\xc1\x00\xc2\x00\xc3\x00" 
		"\xc4\x00\xc5\x00\xc6\x00\xc7\x00" "\xc8\x00\xc9\x00\xca\x00\xcb\x00" 
		"\xcc\x00\xcd\x00\xce\x00\xcf\x00" 
	, 0x0000028);
	ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, REGISTERS, 0x0000000, buf, 0x0000028, 1000);
	ret = select_register_bank(buf, 0);
	return ret;
}

int CCDSetupType2(char *buf){
	int ret;
	printf("CCDSetupType2\n");
	ret = select_register_bank(buf, 1);
	memcpy(buf, 
		"\x60\x08\x61\x0b\x62\x0e\x63\x11" "\x64\x14\x65\x17\x66\x02\x67\x05" 
		"\x68\x08\x69\x50\x6a\x01\x6b\x0c" "\x6d\x48\x6e\x55\x6f\x0e\x70\x50" 
		"\x71\x55\x72\x0e\x73\xe8\x74\x57" "\x75\x0e\x76\xf0\x77\x57\x78\x0e" 
		"\x79\x48\x7a\x2b\x7b\x0e\x7c\x50" "\x7d\x2b\x7e\x0e\x7f\xe8\x80\x2d" 
	, 0x0000040);
	ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, REGISTERS, 0x0000000, buf, 0x0000040, 1000);
	memcpy(buf, 
		"\x81\x0e\x82\xf0\x83\x2d\x84\x0e" "\x85\x48\x86\x7f\x87\x0e\x88\x50" 
		"\x89\x7f\x8a\x0e\x8b\xe8\x8c\x81" "\x8d\x0e\x8e\xf0\x8f\x81\x90\x0e" 
		"\x9a\x00\x9b\x15\x9d\x01\x9e\x00" "\x9f\x00\xa0\x0e\xa1\x00\xa2\x2a" 
		"\xa3\x0e\xa4\x00\xa5\x54\xa6\x0e" "\xa7\xff\xa8\x29\xa9\x0e\xaa\xff" 
	, 0x0000040);
	ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, REGISTERS, 0x0000000, buf, 0x0000040, 1000);

	memcpy(buf, 
		"\xab\x53\xac\x0e\xad\xff\xae\x7d" "\xaf\x0e\xb0\x50\xb1\x01\xb9\xff" 
		"\xba\x14\xcd\x00\xce\x0f\xd4\x00" "\xd5\x8e\xd6\xe3\xd7\x38\xec\x00" 
		"\xed\x00\xee\xc0\xef\x00\xf0\x00" "\xf1\x00\xf2\x00\xf3\x30\xf4\x00" 
		"\xf5\x00\xf6\xfe\xf7\x14\xf8\xaa" "\xf9\x00\xfa\x55\xfb\x00\xfc\x2a" 
	, 0x0000040);
	ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, REGISTERS, 0x0000000, buf, 0x0000040, 1000);
	memcpy(buf, "\xfd\x00\xfd\x00", 0x0000004);
	ret = writeRegisters(buf);
	ret = select_register_bank(buf, 0);
	return ret;
}

int bunchOfConfig(char *buf){
	int ret;
	printf("bunchOfConfig\n");
	ret = select_register_bank(buf, 0);
	memcpy(buf, 
		"\x58\x01\x5e\x01\x82\x00\x83\x38" "\x84\x8e\x85\xe3\x8a\x00\x8e\x00" 
		"\x90\xa0\x91\x00\x94\x30\x95\xa7" "\x96\x23\x9b\x3d\x9d\x3f\x9e\x00" 
		"\x9f\x80\xa0\x00\xa1\x00\xa2\x00" "\xa3\xff\xa4\xfe\xa5\x0d\xa6\x40" 
		"\xab\x00\xae\xdb\xaf\x16\xb0\x08" "\xb1\x07\xb2\x14\xb3\x01\xb4\x7b" 
	, 0x0000040);
	ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, REGISTERS, 0x0000000, buf, 0x0000040, 1000);
	memcpy(buf, 
		"\xb5\x00\xb6\x70\xb7\x05\xc9\x00" "\xca\x00\xcb\x01\xcc\x00\xcd\x3c" 
		"\xce\x3c\xcf\x3c\xd0\x07\xd8\x05" "\xd9\x3c\xda\x54\xdb\x00\xdc\x01" 
		"\xde\x01\xdf\x17\xe0\x01\xe1\x00" "\xe2\x00\xe3\x00\xe4\x00\xe5\x01" 
		"\xe6\x00\xe7\x00\xe8\x00\xe9\x00" "\xea\x2c\xeb\x01\xec\x01\xed\x00" 
	, 0x0000040);
	ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, REGISTERS, 0x0000000, buf, 0x0000040, 1000);
	memcpy(buf, 
		"\xee\x01\xef\x01\xf0\x14\xf1\x00" "\xf2\x00\xf3\x0c\xf5\x12\xf6\x00" 
		"\xf7\x00\xf8\x02\xf9\x03\xfa\x36" "\xfb\xfb\xfc\x00\xfd\xd9\xfe\x16" 
		"\xff\xfc" 
	, 0x0000022);
	ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, REGISTERS, 0x0000000, buf, 0x0000022, 1000);
	return ret;
}

int resetHostStartAddr(char *buf){
	printf("ResetHostStartAddr\n");
	memcpy(buf, "\xa0\x00\xa1\x00\xa2\x00", 0x0000006);
	return usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, REGISTERS, 0x0000000, buf, 0x0000006, 1000);
	
}

int main(int argc, char **argv) {
    int ret, vendor, product;
    struct usb_device *dev;
    char buf[65535], *endptr;
    FILE * myfile;
    int scanloop, x;
#if 0
    usb_urb *isourb;
    struct timeval isotv;
    char isobuf[32768];
#endif


    if (argc!=2) {
	printf("usage: %s outputfile\n", argv[0]);
	exit(1);
    }

    if (myfile = fopen(argv[1], "r"))
    {
        fclose(myfile);
        printf("File already exists!\n");
	exit(1);
    }


    usb_init();
    usb_set_debug(255);
    usb_find_busses();
    usb_find_devices();

    vendor = 0x03f0;
    product = 0x3005;

    dev = find_device(vendor, product);
    assert(dev);

    devh = usb_open(dev);
    assert(devh);
    
    signal(SIGTERM, release_usb_device);

    ret = usb_get_driver_np(devh, 0, buf, sizeof(buf));
    printf("usb_get_driver_np returned %d\n", ret);
    if (ret == 0) {
	printf("interface 0 already claimed by driver \"%s\", attempting to detach it\n", buf);
	ret = usb_detach_kernel_driver_np(devh, 0);
	printf("usb_detach_kernel_driver_np returned %d\n", ret);
    }
    ret = usb_claim_interface(devh, 0);
    if (ret != 0) {
	printf("claim failed with error %d\n", ret);
		exit(1);
    }
    
    ret = usb_set_altinterface(devh, 0);
    assert(ret >= 0);

ret = usb_get_descriptor(devh, 0x0000001, 0x0000000, buf, 0x0000012);
ret = usb_get_descriptor(devh, 0x0000002, 0x0000000, buf, 0x0000009);
ret = usb_get_descriptor(devh, 0x0000002, 0x0000000, buf, 0x0000027);

ret = usb_release_interface(devh, 0);
if (ret != 0) printf("failed to release interface before set_configuration: %d\n", ret);

ret = usb_set_configuration(devh, 0x0000001);

ret = usb_claim_interface(devh, 0);
if (ret != 0) printf("claim after set_configuration failed with error %d\n", ret);

ret = usb_set_altinterface(devh, 0);

usleep(100*1000);

/*#define		ES01_03_AFEReg3				0x03*/

ret = doCmdSequenceAlpha(buf);

ret = do_command_five(buf);
ret = do_command_one(buf);
ret = read_register(buf, 0x1c);
ret = readScanState(buf);
ret = read_register(buf, 0x03);

ret = closescanchip(buf);
/*------------------------------*/
ret = openscanchip(buf);

/*Power save control commands?*/
ret = adjustPowerSaveRegisters(buf);

ret = do_command_five(buf);

/*this appears to be repeated.*/
ret = doCmdSequenceAlpha(buf);

ret = do_command_five(buf);
ret = do_command_one(buf);
ret = read_register(buf, 0x1c);
ret = readScanState(buf);
ret = read_register(buf, 0x03);

/*close and reopen chip*/
ret = closescanchip(buf);
/*------------------------------*/
ret = openscanchip(buf);

ret = adjustPowerSaveRegisters(buf);
ret = do_command_five(buf);
usleep(40*1000);

ret = usb_interrupt_read(devh, 0x00000083, buf, 0x0000001, 1000);
printf("Interrupt Read: %02x %02x\n", ret, buf[0]);

printf("Initialization Complete\n");

/*Sleep command reduced*/
/*I think this was between starting to record and opening the Windows scanner interface*/
usleep(200*1000);

ret = doCmdSequenceAlpha(buf);

ret = do_command_five(buf);
ret = do_command_one(buf);
ret = read_register(buf, 0x03);

ret = do_command_five(buf);
ret = do_command_one(buf);
ret = read_register(buf, 0x03);

ret = do_command_five(buf);
ret = do_command_one(buf);
ret = read_register(buf, 0x03);

/*GPIO on Mustek_usb2...*/
ret = do_command_eight(buf);

ret = do_command_five(buf);
ret = do_command_one(buf);
ret = read_register(buf, 0x03);

ret = do_command_five(buf);

memcpy(buf, 
	"\x02\xa0\x10\x03\x00\x00\x00\x00" "\x00\x00\x00\x00\x00\x00\x00\x00" 
, 0x0000010);
ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, 0x000000a, 0x0000000, buf, 0x0000010, 1000);
ret = usb_bulk_read(devh, 0x00000082, buf, 0x0000200, 1030);
printf("\t");
print_bytes(buf, ret);
printf("\n");

ret = do_command_six(buf);

ret = do_command_one(buf);

/*writing registers, disable trigger, enable clocks*/
ret = setScannerIdle(buf);

ret = readScanState(buf);

ret = closescanchip(buf);
/*------------------------------*/
ret = openscanchip(buf);

ret = doSDRAMSetup(buf);

/*home scan head? (writing registers)*/
memcpy(buf, "\xf3\x21\xf4\x00", 0x0000004);
ret = writeRegisters(buf);

memcpy(buf, "\xf3\xf3\xf3\xf3", 0x0000004);
ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, 0x0000005, 0x0000000, buf, 0x0000004, 1000);

memcpy(buf, "\xf3\xf3\xf3\xf3", 0x0000004);
ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, SCANCONTROL, 0x0000000, buf, 0x0000004, 1000);

usleep(12*1000);

ret = read_register(buf, 0x00);

ret = readChipString(buf);

ret = closescanchip(buf);
/*------------------------------*/
ret = openscanchip(buf);

ret = adjustPowerSaveRegisters(buf);

/*86 = Disable clocks?*/
memcpy(buf, "\x00\x00\x86\x01", 0x0000004);
ret = writeRegisters(buf);

ret = closescanchip(buf);
/*------------------------------*/
ret = openscanchip(buf);
ret = adjustPowerSaveRegisters(buf);

ret = do_command_five(buf);

/*reading from successive registers?*/
printf("Command 8 sequence\n");
for(x = 0; x < 16; x+=2){
	memcpy(buf, command_eight_base, 0x0000010);
	buf[2] += x << 4;
	ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, 0x000000a, 0x0000000, buf, 0x0000010, 1000);
	ret = usb_bulk_read(devh, 0x00000082, buf, 0x0000200, 1030);
	printf("Command 8\n");
	print_bytes(buf, ret);
	printf("\n");
}

for(x = 0; x < 10; x+=2){
	memcpy(buf, command_eight_base, 0x0000010);
	buf[1] += 2;
	buf[2] += x << 4;
	ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, 0x000000a, 0x0000000, buf, 0x0000010, 1000);
	ret = usb_bulk_read(devh, 0x00000082, buf, 0x0000200, 1030);
	printf("Command 8\n");
	print_bytes(buf, ret);
	printf("\n");
}
/* end successive reads */

memcpy(buf, 
	"\x02\xa0\x2b\x02\x00\x00\x00\x00" "\x00\x00\x00\x00\x00\x00\x00\x00" 
, 0x0000010);
ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, 0x000000a, 0x0000000, buf, 0x0000010, 1000);
ret = usb_bulk_read(devh, 0x00000082, buf, 0x0000200, 1030);
	printf("02 a0 2b...\n");
	print_bytes(buf, ret);
	printf("\n");

ret = do_command_one(buf);

ret = closescanchip(buf);
/*------------------------------*/
ret = openscanchip(buf);

ret = adjustPowerSaveRegisters(buf);

ret = do_command_five(buf);
usleep(2500*1000);
ret = do_command_one(buf);
ret = read_register(buf, 0x03);

ret = do_command_five(buf);
ret = do_command_one(buf);
ret = read_register(buf, 0x03);

ret = do_command_eight(buf);

ret = do_command_five(buf);
ret = do_command_one(buf);

ret = do_command_four(buf);

/*remove trigger enable clocks*/
ret = setScannerIdle(buf);

ret = readScanState(buf);

ret = closescanchip(buf);
/*------------------------------*/
ret = openscanchip(buf);

ret = doSDRAMSetup(buf);

/*motor commands?*/
printf("Motor Initialize?\n");
memcpy(buf, "\xf3\x21\xf4\x00", 0x0000004);
ret = writeRegisters(buf);
memcpy(buf, "\xf3\xf3\xf3\xf3", 0x0000004);
ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, 0x0000005, 0x0000000, buf, 0x0000004, 1000);
memcpy(buf, "\xf3\xf3\xf3\xf3", 0x0000004);
ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, SCANCONTROL, 0x0000000, buf, 0x0000004, 1000);


/*CCDTiming see mustek_usb_asic.c line 2582------------------*/
printf("CCD Setup?\n");
memcpy(buf, "\x82\x00\x83\x38\x84\x8e\x85\xe3", 0x0000008);
ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, REGISTERS, 0x0000000, buf, 0x0000008, 1000);
ret = select_register_bank(buf, 1);
/*or these are all 0x1?? due to register bank select?*/
/*CCDTiming continued*/
/*ES01_160_CHANNEL_A_LATCH_POSITION_HB etc
  ES01_1D0_DUMMY_CYCLE_TIMING_B0
  ES01_1EC_AFEVS_TIMING_ADJ_B0
  ES01_1F0_AFERS_TIMING_ADJ_B0*/
memcpy(buf, 
	"\x60\x08\x61\x0b\x62\x0e\x63\x11" "\x64\x14\x65\x17\x66\x02\x67\x05" 
	"\x68\x08\xd4\x00\xd5\x8e\xd6\xe3" "\xd7\x38\xec\x00\xed\x00\xee\xc0" 
	"\xef\x00\xf0\x00\xf1\x00\xf2\x00" "\xf3\x30" 
, 0x000002a);
ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, REGISTERS, 0x0000000, buf, 0x000002a, 1000);
/*That was URB 220 */

ret = select_register_bank(buf, 0);

for(x = 0x60; x < 0xa0; x+=0x20){
	memcpy(buf, 
	"\x02\xa0\x60\x20\x00\x00\x00\x00" "\x00\x00\x00\x00\x00\x00\x00\x00" 
, 0x0000010);
	buf[2] = x;
	ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, 0x000000a, 0x0000000, buf, 0x0000010, 1000);
	ret = usb_bulk_read(devh, 0x00000082, buf, 0x0000200, 1030);
	printf("02 a0...\n");
	print_bytes(buf, ret);
	printf("\n");
}

for(x = 0xa0; x < 0xd0; x+=0x20){
	memcpy(buf, 
	"\x02\xa6\xa0\x20\x00\x00\x00\x00" "\x00\x00\x00\x00\x00\x00\x00\x00" 
, 0x0000010);
	buf[2] = x;
	ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, 0x000000a, 0x0000000, buf, 0x0000010, 1000);
	ret = usb_bulk_read(devh, 0x00000082, buf, 0x0000200, 1030);
	printf("02 a6...\n");
	print_bytes(buf, ret);
	printf("\n");
}

for(x = 0xa0; x < 0xf0; x+=0x20){
	memcpy(buf, 
	"\x02\xa0\xa0\x20\x00\x00\x00\x00" "\x00\x00\x00\x00\x00\x00\x00\x00" 
, 0x0000010);
	buf[2] = x;
	ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, 0x000000a, 0x0000000, buf, 0x0000010, 1000);
	ret = usb_bulk_read(devh, 0x00000082, buf, 0x0000200, 1030);
	printf("02 a0...\n");
	print_bytes(buf, ret);
	printf("\n");
}

for(x = 0x00; x < 0xf0; x+=0x20){
	memcpy(buf, 
	"\x02\xa2\x00\x20\x00\x00\x00\x00" "\x00\x00\x00\x00\x00\x00\x00\x00" 
, 0x0000010);
	buf[2] = x;
	ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, 0x000000a, 0x0000000, buf, 0x0000010, 1000);
	ret = usb_bulk_read(devh, 0x00000082, buf, 0x0000200, 1030);
	printf("02 a2...\n");
	print_bytes(buf, ret);
	printf("\n");
}
/*That was URB 250 */

for(x = 0x00; x < 0xf0; x+=0x20){
	memcpy(buf, 
	"\x02\xa4\x00\x20\x00\x00\x00\x00" "\x00\x00\x00\x00\x00\x00\x00\x00" 
, 0x0000010);
	buf[2] = x;
	ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, 0x000000a, 0x0000000, buf, 0x0000010, 1000);
	ret = usb_bulk_read(devh, 0x00000082, buf, 0x0000200, 1030);
	printf("02 a4...\n");
	print_bytes(buf, ret);
	printf("\n");
}

for(x = 0x00; x < 0x90; x+=0x20){
	memcpy(buf, 
	"\x02\xa6\x00\x20\x00\x00\x00\x00" "\x00\x00\x00\x00\x00\x00\x00\x00" 
, 0x0000010);
	buf[2] = x;
	ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, 0x000000a, 0x0000000, buf, 0x0000010, 1000);
	ret = usb_bulk_read(devh, 0x00000082, buf, 0x0000200, 1030);
	printf("02 a6...\n");
	print_bytes(buf, ret);
	printf("\n");
}

memcpy(buf, 
	"\x02\xaa\xb0\x01\x00\x00\x00\x00" "\x00\x00\x00\x00\x00\x00\x00\x00" 
, 0x0000010);ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, 0x000000a, 0x0000000, buf, 0x0000010, 1000);
usleep(2*1000);
ret = usb_bulk_read(devh, 0x00000082, buf, 0x0000200, 1030);
printf("02 a6...\n");
print_bytes(buf, ret);
printf("\n");

ret = read_register(buf, 0x00);

/*Setting a bunch of options?*/
/*b8-c3 LED set up. mustek_usb2_asic.c line 2916?*/
/* Also used in LLFMotorMove */
memcpy(buf,
	"\x74\x00\x78\x00\x82\x00\x83\x38" "\x84\x8e\x85\xe3\x86\x01\x88\x80"
	"\x89\x80\x90\x64\x8e\x00\x92\x57" "\x93\x0a\x95\x27\x96\x21\x97\xf0"
	"\x98\xbe\x99\x30\xb2\x14\xb3\x01" "\xb8\x00\xb9\x00\xba\x00\xbb\x00"
	"\xbc\x00\xbd\x00\xbe\x00\xbf\x00" "\xc0\x00\xc1\x00\xc2\x00\xc3\x00", 0x0000040);

ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, REGISTERS, 0x0000000, buf, 0x0000040, 1000);

/*SetLineTimeAndExposure c4 to cb - this is only called in setwindow*/
/*d0-d5 is ccd setup continued*/
/*db-dc used in set window*/
/*de ccd setup register - set extra settings*/
/*ea-eb motor set up*/
/*f8 white shading data format -> ES01_SHADING_3_INT_13_DEC		0x01 same?*/
memcpy(buf, 
	"\xc4\x00\xc5\x00\xc6\x00\xc7\x00" "\xc8\x00\xcc\x00\xd0\x07\xd1\x00" 
	"\xd2\x00\xd3\x00\xd4\x00\xd5\x00" "\xd6\x00\xd7\x00\xd8\x05\xdb\x00" 
	"\xdc\x01\xde\x01\xea\x2c\xeb\x01" "\xf8\x01" 
, 0x000002a);
ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, REGISTERS, 0x0000000, buf, 0x000002a, 1000);

/*168 etc ccd controls?*/
printf("Doing some CCD config stuff.\n");
ret = select_register_bank(buf, 1);
memcpy(buf, 
	"\x60\x08\x61\x0b\x62\x0e\x63\x11" "\x64\x14\x65\x17\x66\x02\x67\x05" 
	"\x68\x08\x6c\x1c\xd0\x00\xd1\x00" "\xd2\x00\xd3\x00\xd4\x00\xd5\x8e" 
	"\xd6\xe3\xd7\x38\xd8\xff\xd9\xff" "\xda\xff\xdb\xff\xdc\xff\xdd\xff" 
	"\xde\xff\xdf\xff\xe0\xff\xe1\xff" "\xe2\xff\xe3\xff\xe4\xff\xe5\xff" 
, 0x0000040);
ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, REGISTERS, 0x0000000, buf, 0x0000040, 1000);

/*
more ccd stuff
*/
memcpy(buf, 
	"\xe6\xff\xe7\xff\xe8\xff\xe9\xff" "\xea\xff\xeb\xff\xec\x00\xed\x00" 
	"\xee\xc0\xef\x00\xf0\x00\xf1\x00" "\xf2\x00\xf3\x30" 
, 0x000001c);ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, REGISTERS, 0x0000000, buf, 0x000001c, 1000);



ret = select_register_bank(buf, 0);

ret = select_register_bank(buf, 2);
memcpy(buf, 
	"\xd0\x01\xd1\x09\xd2\x11\xd3\x19" "\xd4\x21\xd5\x29\xd6\x31\xd7\x39" 
, 0x0000010);ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, REGISTERS, 0x0000000, buf, 0x0000010, 1000);


ret = select_register_bank(buf, 0);
/*That was URB 290 */
memcpy(buf, "\x79\x00\x86\x00", 0x0000004);
ret = writeRegisters(buf);
ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE + USB_ENDPOINT_IN, 0x000000c, 0x00000d0, 0x0000000, buf, 0x0000001, 1000);

printf("Host addr 000000 to 0fffff. 08\n");
memcpy(buf, 
	"\xa0\x00\xa1\x00\xa2\x00\xa3\xff" "\xa4\xff\xa5\x0f\x7c\x00\x7d\x08" 
	"\x7e\x00\x7f\x00" 
, 0x0000014);
ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, REGISTERS, 0x0000000, buf, 0x0000014, 1000);

/*motor table. LLFRamAccess calls----------------------*/
memcpy(buf, "\x00\x08\x00\x00", 0x0000004);
ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, DMAWRITE, 0x0000000, buf, 0x0000004, 1000);
usleep(16*1000);
for(x = 0; x < 2048; x+= 8){
	memcpy(buf+x, "\x01\x02\x04\x08\x10\x20\x40\x80", 0x0000008);
}
ret = usb_bulk_write(devh, 0x00000001, buf, 0x0000800, 1122);
ret = clearFIFO(buf);

usleep(46*1000);

printf("Host addr 000000 to 0fffff. 04 (read?)\n");
memcpy(buf, 
	"\xa0\x00\xa1\x00\xa2\x00\xa3\xff" "\xa4\xff\xa5\x0f\x7c\x00\x7d\x04" 
	"\x7e\x00\x79\x00" 
, 0x0000014);
ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, REGISTERS, 0x0000000, buf, 0x0000014, 1000);

memcpy(buf, "\x00\x08\x00\x00", 0x0000004);
ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, DMAREAD, 0x0000000, buf, 0x0000004, 1000);
ret = usb_bulk_read(devh, 0x00000082, buf, 0x0000800, 1122);
printf("00 08 00 00...\n");
print_bytes(buf, ret);
printf("\n");
/*That was URB 300 */

memcpy(buf, "\x79\x10\x86\x00", 0x0000004);
ret = writeRegisters(buf);
ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE + USB_ENDPOINT_IN, 0x000000c, 0x00000d0, 0x0000000, buf, 0x0000001, 1000);

printf("Host addr 000000 to 0fffff. 08 (write?)\n");
memcpy(buf, 
	"\xa0\x00\xa1\x00\xa2\x00\xa3\xff" "\xa4\xff\xa5\x0f\x7c\x00\x7d\x08" 
	"\x7e\x00\x7f\x00" 
, 0x0000014);
ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, REGISTERS, 0x0000000, buf, 0x0000014, 1000);
usleep(26*1000);
memcpy(buf, "\x00\x08\x00\x00", 0x0000004);
ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, DMAWRITE, 0x0000000, buf, 0x0000004, 1000);
usleep(26*1000);
for(x = 0; x < 2048; x+= 8){
	memcpy(buf+x, "\x01\x02\x04\x08\x10\x20\x40\x80", 0x0000008);
}
ret = usb_bulk_write(devh, 0x00000001, buf, 0x0000800, 1122);
ret = clearFIFO(buf);

usleep(46*1000);

printf("Host addr 000000 to 0fffff. 04 (read?) R79 = 0x10 (SDRAM CLK DELAY?)\n");
memcpy(buf, 
	"\xa0\x00\xa1\x00\xa2\x00\xa3\xff" "\xa4\xff\xa5\x0f\x7c\x00\x7d\x04" 
	"\x7e\x00\x79\x10" 
, 0x0000014);
ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, REGISTERS, 0x0000000, buf, 0x0000014, 1000);

memcpy(buf, "\x00\x08\x00\x00", 0x0000004);
ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, DMAREAD, 0x0000000, buf, 0x0000004, 1000);
ret = usb_bulk_read(devh, 0x00000082, buf, 0x0000800, 1122);
printf("00 08 00 00...\n");
print_bytes(buf, ret);
printf("\n");

memcpy(buf, "\x79\x20\x86\x00", 0x0000004);
ret = writeRegisters(buf);
ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE + USB_ENDPOINT_IN, 0x000000c, 0x00000d0, 0x0000000, buf, 0x0000001, 1000);

printf("Host addr 000000 to 0fffff. 08 (write?)\n");
memcpy(buf, 
	"\xa0\x00\xa1\x00\xa2\x00\xa3\xff" "\xa4\xff\xa5\x0f\x7c\x00\x7d\x08" 
	"\x7e\x00\x7f\x00" 
, 0x0000014);
ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, REGISTERS, 0x0000000, buf, 0x0000014, 1000);
memcpy(buf, "\x00\x08\x00\x00", 0x0000004);
ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, DMAWRITE, 0x0000000, buf, 0x0000004, 1000);
for(x = 0; x < 2048; x+= 8){
	memcpy(buf+x, "\x01\x02\x04\x08\x10\x20\x40\x80", 0x0000008);
}
ret = usb_bulk_write(devh, 0x00000001, buf, 0x0000800, 1122);
ret = clearFIFO(buf);

usleep(46*1000);

memcpy(buf, 
	"\xa0\x00\xa1\x00\xa2\x00\xa3\xff" "\xa4\xff\xa5\x0f\x7c\x00\x7d\x04" 
	"\x7e\x00\x79\x20" 
, 0x0000014);
ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, REGISTERS, 0x0000000, buf, 0x0000014, 1000);

usleep(31*1000);

memcpy(buf, "\x00\x08\x00\x00", 0x0000004);
ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, DMAREAD, 0x0000000, buf, 0x0000004, 1000);
ret = usb_bulk_read(devh, 0x00000082, buf, 0x0000800, 1122);


memcpy(buf, "\x79\x30\x86\x00", 0x0000004);
ret = writeRegisters(buf);
ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE + USB_ENDPOINT_IN, 0x000000c, 0x00000d0, 0x0000000, buf, 0x0000001, 1000);

memcpy(buf, 
	"\xa0\x00\xa1\x00\xa2\x00\xa3\xff" "\xa4\xff\xa5\x0f\x7c\x00\x7d\x08" 
	"\x7e\x00\x7f\x00" 
, 0x0000014);
ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, REGISTERS, 0x0000000, buf, 0x0000014, 1000);

usleep(30*1000);

memcpy(buf, "\x00\x08\x00\x00", 0x0000004);
ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, DMAWRITE, 0x0000000, buf, 0x0000004, 1000);
usleep(15*1000);
for(x = 0; x < 2048; x+= 8){
	memcpy(buf+x, "\x01\x02\x04\x08\x10\x20\x40\x80", 0x0000008);
}
ret = usb_bulk_write(devh, 0x00000001, buf, 0x0000800, 1122);
usleep(16*1000);
ret = clearFIFO(buf);
usleep(47*1000);
memcpy(buf, 
	"\xa0\x00\xa1\x00\xa2\x00\xa3\xff" "\xa4\xff\xa5\x0f\x7c\x00\x7d\x04" 
	"\x7e\x00\x79\x30" 
, 0x0000014);ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, REGISTERS, 0x0000000, buf, 0x0000014, 1000);
usleep(31*1000);
memcpy(buf, "\x00\x08\x00\x00", 0x0000004);
ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, DMAREAD, 0x0000000, buf, 0x0000004, 1000);
ret = usb_bulk_read(devh, 0x00000082, buf, 0x0000800, 1122);

/*That was URB 330 */

memcpy(buf, "\x79\x40\x86\x00", 0x0000004);
ret = writeRegisters(buf);
ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE + USB_ENDPOINT_IN, 0x000000c, 0x00000d0, 0x0000000, buf, 0x0000001, 1000);
memcpy(buf, 
	"\xa0\x00\xa1\x00\xa2\x00\xa3\xff" "\xa4\xff\xa5\x0f\x7c\x00\x7d\x08" 
	"\x7e\x00\x7f\x00" 
, 0x0000014);ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, REGISTERS, 0x0000000, buf, 0x0000014, 1000);
usleep(30*1000);
memcpy(buf, "\x00\x08\x00\x00", 0x0000004);
ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, DMAWRITE, 0x0000000, buf, 0x0000004, 1000);
usleep(16*1000);
for(x = 0; x < 2048; x+= 8){
	memcpy(buf+x, "\x01\x02\x04\x08\x10\x20\x40\x80", 0x0000008);
}
ret = usb_bulk_write(devh, 0x00000001, buf, 0x0000800, 1122);
usleep(16*1000);
ret = clearFIFO(buf);
usleep(46*1000);
memcpy(buf, 
	"\xa0\x00\xa1\x00\xa2\x00\xa3\xff" "\xa4\xff\xa5\x0f\x7c\x00\x7d\x04" 
	"\x7e\x00\x79\x40" 
, 0x0000014);ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, REGISTERS, 0x0000000, buf, 0x0000014, 1000);
usleep(32*1000);
memcpy(buf, "\x00\x08\x00\x00", 0x0000004);
ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, DMAREAD, 0x0000000, buf, 0x0000004, 1000);
ret = usb_bulk_read(devh, 0x00000082, buf, 0x0000800, 1122);

/*That was URB 340 */
memcpy(buf, "\x79\x50\x86\x00", 0x0000004);
ret = writeRegisters(buf);

ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE + USB_ENDPOINT_IN, 0x000000c, 0x00000d0, 0x0000000, buf, 0x0000001, 1000);
memcpy(buf, 
	"\xa0\x00\xa1\x00\xa2\x00\xa3\xff" "\xa4\xff\xa5\x0f\x7c\x00\x7d\x08" 
	"\x7e\x00\x7f\x00" 
, 0x0000014);ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, REGISTERS, 0x0000000, buf, 0x0000014, 1000);
usleep(30*1000);
memcpy(buf, "\x00\x08\x00\x00", 0x0000004);
ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, DMAWRITE, 0x0000000, buf, 0x0000004, 1000);
usleep(16*1000);
for(x = 0; x < 2048; x+= 8){
	memcpy(buf+x, "\x01\x02\x04\x08\x10\x20\x40\x80", 0x0000008);
}
ret = usb_bulk_write(devh, 0x00000001, buf, 0x0000800, 1122);
usleep(15*1000);
ret = clearFIFO(buf);
usleep(46*1000);
memcpy(buf, 
	"\xa0\x00\xa1\x00\xa2\x00\xa3\xff" "\xa4\xff\xa5\x0f\x7c\x00\x7d\x04" 
	"\x7e\x00\x79\x50" 
, 0x0000014);ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, REGISTERS, 0x0000000, buf, 0x0000014, 1000);
usleep(31*1000);
memcpy(buf, "\x00\x08\x00\x00", 0x0000004);
ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, DMAREAD, 0x0000000, buf, 0x0000004, 1000);

ret = usb_bulk_read(devh, 0x00000082, buf, 0x0000800, 1122);

/*That was URB 350 */
memcpy(buf, "\x79\x60\x86\x00", 0x0000004);
ret = writeRegisters(buf);
ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE + USB_ENDPOINT_IN, 0x000000c, 0x00000d0, 0x0000000, buf, 0x0000001, 1000);
memcpy(buf, 
	"\xa0\x00\xa1\x00\xa2\x00\xa3\xff" "\xa4\xff\xa5\x0f\x7c\x00\x7d\x08" 
	"\x7e\x00\x7f\x00" 
, 0x0000014);ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, REGISTERS, 0x0000000, buf, 0x0000014, 1000);
usleep(31*1000);
memcpy(buf, "\x00\x08\x00\x00", 0x0000004);
ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, DMAWRITE, 0x0000000, buf, 0x0000004, 1000);
usleep(15*1000);
for(x = 0; x < 2048; x+= 8){
	memcpy(buf+x, "\x01\x02\x04\x08\x10\x20\x40\x80", 0x0000008);
}
ret = usb_bulk_write(devh, 0x00000001, buf, 0x0000800, 1122);
usleep(16*1000);
ret = clearFIFO(buf);
usleep(46*1000);
memcpy(buf, 
	"\xa0\x00\xa1\x00\xa2\x00\xa3\xff" "\xa4\xff\xa5\x0f\x7c\x00\x7d\x04" 
	"\x7e\x00\x79\x60" 
, 0x0000014);ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, REGISTERS, 0x0000000, buf, 0x0000014, 1000);
usleep(31*1000);
memcpy(buf, "\x00\x08\x00\x00", 0x0000004);
ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, DMAREAD, 0x0000000, buf, 0x0000004, 1000);
ret = usb_bulk_read(devh, 0x00000082, buf, 0x0000800, 1122);

/*That was URB 360 */

memcpy(buf, "\x79\x70\x86\x00", 0x0000004);
ret = writeRegisters(buf);
ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE + USB_ENDPOINT_IN, 0x000000c, 0x00000d0, 0x0000000, buf, 0x0000001, 1000);

memcpy(buf, 
	"\xa0\x00\xa1\x00\xa2\x00\xa3\xff" "\xa4\xff\xa5\x0f\x7c\x00\x7d\x08" 
	"\x7e\x00\x7f\x00" 
, 0x0000014);ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, REGISTERS, 0x0000000, buf, 0x0000014, 1000);
usleep(29*1000);
memcpy(buf, "\x00\x08\x00\x00", 0x0000004);
ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, DMAWRITE, 0x0000000, buf, 0x0000004, 1000);
usleep(17*1000);
for(x = 0; x < 2048; x+= 8){
	memcpy(buf+x, "\x01\x02\x04\x08\x10\x20\x40\x80", 0x0000008);
}
ret = usb_bulk_write(devh, 0x00000001, buf, 0x0000800, 1122);
usleep(15*1000);

ret = clearFIFO(buf);

usleep(46*1000);
memcpy(buf, 
	"\xa0\x00\xa1\x00\xa2\x00\xa3\xff" "\xa4\xff\xa5\x0f\x7c\x00\x7d\x04" 
	"\x7e\x00\x79\x70" 
, 0x0000014);ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, REGISTERS, 0x0000000, buf, 0x0000014, 1000);
usleep(32*1000);
memcpy(buf, "\x00\x08\x00\x00", 0x0000004);
ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, DMAREAD, 0x0000000, buf, 0x0000004, 1000);
ret = usb_bulk_read(devh, 0x00000082, buf, 0x0000800, 1122);

/*That was URB 370 */

memcpy(buf, "\x79\x80\x86\x00", 0x0000004);
ret = writeRegisters(buf);
ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE + USB_ENDPOINT_IN, 0x000000c, 0x00000d0, 0x0000000, buf, 0x0000001, 1000);
memcpy(buf, 
	"\xa0\x00\xa1\x00\xa2\x00\xa3\xff" "\xa4\xff\xa5\x0f\x7c\x00\x7d\x08" 
	"\x7e\x00\x7f\x00" 
, 0x0000014);ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, REGISTERS, 0x0000000, buf, 0x0000014, 1000);
usleep(32*1000);
memcpy(buf, "\x00\x08\x00\x00", 0x0000004);
ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, DMAWRITE, 0x0000000, buf, 0x0000004, 1000);
usleep(14*1000);
for(x = 0; x < 2048; x+= 8){
	memcpy(buf+x, "\x01\x02\x04\x08\x10\x20\x40\x80", 0x0000008);
}
ret = usb_bulk_write(devh, 0x00000001, buf, 0x0000800, 1122);
usleep(15*1000);
ret = clearFIFO(buf);
usleep(46*1000);
memcpy(buf, 
	"\xa0\x00\xa1\x00\xa2\x00\xa3\xff" "\xa4\xff\xa5\x0f\x7c\x00\x7d\x04" 
	"\x7e\x00\x79\x80" 
, 0x0000014);ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, REGISTERS, 0x0000000, buf, 0x0000014, 1000);
usleep(31*1000);
memcpy(buf, "\x00\x08\x00\x00", 0x0000004);
ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, DMAREAD, 0x0000000, buf, 0x0000004, 1000);

ret = usb_bulk_read(devh, 0x00000082, buf, 0x0000800, 1122);

/*That was URB 380 */
memcpy(buf, "\x79\x90\x86\x00", 0x0000004);
ret = writeRegisters(buf);
ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE + USB_ENDPOINT_IN, 0x000000c, 0x00000d0, 0x0000000, buf, 0x0000001, 1000);

memcpy(buf, 
	"\xa0\x00\xa1\x00\xa2\x00\xa3\xff" "\xa4\xff\xa5\x0f\x7c\x00\x7d\x08" 
	"\x7e\x00\x7f\x00" 
, 0x0000014);ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, REGISTERS, 0x0000000, buf, 0x0000014, 1000);
usleep(30*1000);
memcpy(buf, "\x00\x08\x00\x00", 0x0000004);
ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, DMAWRITE, 0x0000000, buf, 0x0000004, 1000);
usleep(15*1000);
for(x = 0; x < 2048; x+= 8){
	memcpy(buf+x, "\x01\x02\x04\x08\x10\x20\x40\x80", 0x0000008);
}
ret = usb_bulk_write(devh, 0x00000001, buf, 0x0000800, 1122);
usleep(16*1000);
ret = clearFIFO(buf);

usleep(47*1000);
memcpy(buf, 
	"\xa0\x00\xa1\x00\xa2\x00\xa3\xff" "\xa4\xff\xa5\x0f\x7c\x00\x7d\x04" 
	"\x7e\x00\x79\x90" 
, 0x0000014);ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, REGISTERS, 0x0000000, buf, 0x0000014, 1000);
usleep(31*1000);
memcpy(buf, "\x00\x08\x00\x00", 0x0000004);
ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, DMAREAD, 0x0000000, buf, 0x0000004, 1000);
ret = usb_bulk_read(devh, 0x00000082, buf, 0x0000800, 1122);

/*That was URB 390 */
memcpy(buf, "\x79\xa0\x86\x00", 0x0000004);
ret = writeRegisters(buf);

ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE + USB_ENDPOINT_IN, 0x000000c, 0x00000d0, 0x0000000, buf, 0x0000001, 1000);
memcpy(buf, 
	"\xa0\x00\xa1\x00\xa2\x00\xa3\xff" "\xa4\xff\xa5\x0f\x7c\x00\x7d\x08" 
	"\x7e\x00\x7f\x00" 
, 0x0000014);ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, REGISTERS, 0x0000000, buf, 0x0000014, 1000);
usleep(29*1000);
memcpy(buf, "\x00\x08\x00\x00", 0x0000004);
ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, DMAWRITE, 0x0000000, buf, 0x0000004, 1000);
usleep(16*1000);
for(x = 0; x < 2048; x+= 8){
	memcpy(buf+x, "\x01\x02\x04\x08\x10\x20\x40\x80", 0x0000008);
}
ret = usb_bulk_write(devh, 0x00000001, buf, 0x0000800, 1122);
usleep(16*1000);
ret = clearFIFO(buf);
usleep(47*1000);
memcpy(buf, 
	"\xa0\x00\xa1\x00\xa2\x00\xa3\xff" "\xa4\xff\xa5\x0f\x7c\x00\x7d\x04" 
	"\x7e\x00\x79\xa0" 
, 0x0000014);ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, REGISTERS, 0x0000000, buf, 0x0000014, 1000);
usleep(31*1000);
memcpy(buf, "\x00\x08\x00\x00", 0x0000004);
ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, DMAREAD, 0x0000000, buf, 0x0000004, 1000);
ret = usb_bulk_read(devh, 0x00000082, buf, 0x0000800, 1122);

/*That was URB 400 */
memcpy(buf, "\x79\x40\x86\x00", 0x0000004);
ret = writeRegisters(buf);

ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE + USB_ENDPOINT_IN, 0x000000c, 0x00000d0, 0x0000000, buf, 0x0000001, 1000);
memcpy(buf, 
	"\xa0\x00\xa1\x00\xa2\x00\xa3\xff" "\xa4\xff\xa5\x0f\x7c\x00\x7d\x90" 
	"\x7e\x01\x7f\x00" 
, 0x0000014);ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, REGISTERS, 0x0000000, buf, 0x0000014, 1000);
usleep(30*1000);
memcpy(buf, "\x00\xf0\x00\x00", 0x0000004);
ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, DMAWRITE, 0x0000000, buf, 0x0000004, 1000);
usleep(23*1000);
for(x = 0; x < 61440; x+= 8){
	memcpy(buf+x, "\x01\x02\x04\x08\x10\x20\x40\x80", 0x0000008);
}
ret = usb_bulk_write(devh, 0x00000001, buf, 0x000f000, 4686);

usleep(42*1000);
memcpy(buf, "\x00\xa0\x00\x00", 0x0000004);
ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, DMAWRITE, 0x0000000, buf, 0x0000004, 1000);
usleep(17*1000);

for(x = 0; x < 40960; x+= 8){
	memcpy(buf+x, "\x01\x02\x04\x08\x10\x20\x40\x80", 0x0000008);
}
ret = usb_bulk_write(devh, 0x00000001, buf, 0x000a000, 3457);
usleep(27*1000);
ret = clearFIFO(buf);

/*END motor table?----------------------*/

usleep(46*1000);

printf("More DRAM stuff\n");
memcpy(buf, 
	"\xa0\x00\xa1\x00\xa2\x00\xa3\xff" "\xa4\xff\xa5\x0f\x7c\x00\x7d\xc8" 
	"\x7e\x00\x79\x40" 
, 0x0000014);
ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, REGISTERS, 0x0000000, buf, 0x0000014, 1000);
/*That was URB 410 */

usleep(32*1000);
memcpy(buf, "\x00\x90\x01\x00", 0x0000004);
ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, DMAREAD, 0x0000000, buf, 0x0000004, 1000);
usleep(7*1000);
ret = usb_bulk_read(devh, 0x00000082, buf, 0x000fc00, 4870);
printf("00 90 00 00...\n");
print_bytes(buf, ret);
printf("\n");
usleep(17*1000);
ret = usb_bulk_read(devh, 0x00000082, buf, 0x0009400, 3273);
printf("00 90 00 00...\n");
print_bytes(buf, ret);
printf("\n");

printf("Gamma Ram access!\n");
/*ON_CHIP_*_GAMMA area never seems to get used by mustek2...
  Commenting this out doesn't seem to affect things.*/
memcpy(buf, 
	"\xa0\x00\xa1\x00\xa2\x80\xa3\xff" "\xa4\xff\xa5\x0f\x7c\x00\x7d\x06" 
	"\x7e\x00\x7f\x00" 
, 0x0000014);
ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, REGISTERS, 0x0000000, buf, 0x0000014, 1000);

usleep(31*1000);

memcpy(buf, "\x00\x06\x00\x00", 0x0000004);
ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, DMAWRITE, 0x0000000, buf, 0x0000004, 1000);

usleep(16*1000);

memcpy(buf, table0x0000600Type3, 0x0000600);
ret = usb_bulk_write(devh, 0x00000001, buf, 0x0000600, 1092);
usleep(15*1000);


memcpy(buf, "\x00\x00\x00\x00", 0x0000004);
ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, 0x0000005, 0x0000000, buf, 0x0000004, 1000);

memcpy(buf, "\x00\x00\x00\x00", 0x0000004);
ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, SCANCONTROL, 0x0000000, buf, 0x0000004, 1000);

usleep(15*1000);

memcpy(buf, 
	"\xa0\x00\xa1\x00\xa2\x80\xa3\xff" "\xa4\xff\xa5\x0f\x7c\x00\x7d\x03" 
	"\x7e\x00\x79\x40" 
, 0x0000014);
ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, REGISTERS, 0x0000000, buf, 0x0000014, 1000);

usleep(31*1000);

memcpy(buf, "\x00\x06\x00\x00", 0x0000004);
ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, DMAREAD, 0x0000000, buf, 0x0000004, 1000);
/*That was URB 420 */

usleep(2*1000);

ret = usb_bulk_read(devh, 0x00000082, buf, 0x0000600, 1092);

memcpy(buf, 
	"\xa0\x00\xa1\x08\xa2\x80\xa3\xff" "\xa4\xff\xa5\x0f\x7c\x00\x7d\x06" 
	"\x7e\x00\x7f\x00" 
, 0x0000014);
ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, REGISTERS, 0x0000000, buf, 0x0000014, 1000);

usleep(29*1000);
memcpy(buf, "\x00\x06\x00\x00", 0x0000004);
ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, DMAWRITE, 0x0000000, buf, 0x0000004, 1000);

usleep(16*1000);
memcpy(buf, table0x0000600Type2, 0x0000600);
ret = usb_bulk_write(devh, 0x00000001, buf, 0x0000600, 1092);

usleep(16*1000);

ret = clearFIFO(buf);

usleep(15*1000);
printf("Gamma RAM, PRE_GAMMA\n");
memcpy(buf, 
	"\xa0\x00\xa1\x08\xa2\x80\xa3\xff" "\xa4\xff\xa5\x0f\x7c\x00\x7d\x03" 
	"\x7e\x00\x79\x40" 
, 0x0000014);
ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, REGISTERS, 0x0000000, buf, 0x0000014, 1000);

usleep(32*1000);

memcpy(buf, "\x00\x06\x00\x00", 0x0000004);
ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, DMAREAD, 0x0000000, buf, 0x0000004, 1000);
ret = usb_bulk_read(devh, 0x00000082, buf, 0x0000600, 1092);

usleep(15*1000);

printf("AFE Control?\n");
memcpy(buf, "\x9a\x00\x9a\x00", 0x0000004);
ret = writeRegisters(buf);
/*That was URB 430 */

memcpy(buf, "\x00\x00\x02\x0f", 0x0000004);
ret = writeRegisters(buf);

usleep(16*1000);

ret = read_register(buf, 0x00);
ret = do_command_eight(buf);

/*
\xf3 ES01_F3_ActionOption UNIFORM_MOTOR_AND_SCAN_SPEED_ENABLE INVERT_MOTOR_DIRECTION_ENABLE MOTOR_BACK_HOME_AFTER_SCAN_ENABLE
*/
printf("Motor related stuff\n");
memcpy(buf, 
	"\xf3\x32\xfd\x28\xfe\x23\xa6\x4d" "\xf6\x00\x96\x21\xe4\x01\xe2\x32" 
	"\xe3\x00\xe4\x00\x95\x27\xf4\x01" 
, 0x0000018);
ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, REGISTERS, 0x0000000, buf, 0x0000018, 1000);

ret = readScanState(buf);
ret = readScanState(buf);

/*That was URB ~440 */
ret = read_register(buf, 0x0b);
ret = read_register(buf, 0x0a);
ret = read_register(buf, 0x09);

ret = do_command_eight(buf);

usleep(14*1000);

ret = setScannerIdle(buf);

usleep(49*1000);

ret = readScanState(buf);
printf("GPIO?\n");
memcpy(buf, "\x96\x23\x96\x23", 0x0000004);
ret = writeRegisters(buf);
/*That was URB 450 */

/*Setting RAM addresses*/
memcpy(buf, 
	"\xa0\x00\xa1\xf4\xa2\x0f\xa3\xff" "\xa4\xff\xa5\x0f\x7c\x00\x7d\x10" 
	"\x7e\x00\x7f\x00" 
, 0x0000014);ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, REGISTERS, 0x0000000, buf, 0x0000014, 1000);

usleep(27*1000);

memcpy(buf, "\x00\x10\x00\x00", 0x0000004);
ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, DMAWRITE, 0x0000000, buf, 0x0000004, 1000);

usleep(16*1000);

memcpy(buf, table0x00001000type5, 0x0001000);
ret = usb_bulk_write(devh, 0x00000001, buf, 0x0001000, 1245);
usleep(16*1000);

ret = clearFIFO(buf);

usleep(31*1000);

memcpy(buf, 
	"\xa0\x00\xa1\x00\xa2\x00\xa3\xff" "\xa4\xff\xa5\x0f\x7c\x00\x7d\x01" 
	"\x7e\x00\x79\x40" 
, 0x0000014);
ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, REGISTERS, 0x0000000, buf, 0x0000014, 1000);

usleep(31*1000);

memcpy(buf, "\x00\x02\x00\x00", 0x0000004);
ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, DMAREAD, 0x0000000, buf, 0x0000004, 1000);
ret = usb_bulk_read(devh, 0x00000082, buf, 0x0000200, 1030);

printf("AFE Control?\n");
memcpy(buf, "\x9a\x00\x9a\x00", 0x0000004);
ret = writeRegisters(buf);

memcpy(buf, 
	"\x00\x01\x02\x0f\x04\x00\x06\x00" "\x08\x00\x0a\x00\x0c\x64\x0e\x64" 
	"\x10\x64\x12\x64" 
, 0x0000014);
ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, REGISTERS, 0x0000000, buf, 0x0000014, 1000);
/*That was URB 460 */

printf("CCD Setup Type 1\n");
ret = select_register_bank(buf, 1);
memcpy(buf, 
	"\x60\x08\x61\x0b\x62\x0e\x63\x11" "\x64\x14\x65\x17\x66\x02\x67\x05" 
	"\x68\x08\x69\x50\x6a\x01\x6b\x0c" "\x6d\x48\x6e\x55\x6f\x0e\x70\x50" 
	"\x71\x55\x72\x0e\x73\xe8\x74\x57" "\x75\x0e\x76\xf0\x77\x57\x78\x0e" 
	"\x79\x48\x7a\x2b\x7b\x0e\x7c\x50" "\x7d\x2b\x7e\x0e\x7f\xe8\x80\x2d" 
, 0x0000040);ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, REGISTERS, 0x0000000, buf, 0x0000040, 1000);
memcpy(buf, 
	"\x81\x0e\x82\xf0\x83\x2d\x84\x0e" "\x85\x48\x86\x7f\x87\x0e\x88\x50" 
	"\x89\x7f\x8a\x0e\x8b\xe8\x8c\x81" "\x8d\x0e\x8e\xf0\x8f\x81\x90\x0e" 
	"\x9a\x00\x9b\x15\x9d\x01\x9e\x00" "\x9f\x00\xa0\x0e\xa1\x00\xa2\x2a" 
	"\xa3\x0e\xa4\x00\xa5\x54\xa6\x0e" "\xa7\xff\xa8\x29\xa9\x0e\xaa\xff" 
, 0x0000040);ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, REGISTERS, 0x0000000, buf, 0x0000040, 1000);
/*This 3rd section differs from type 2*/
memcpy(buf, 
	"\xab\x53\xac\x0e\xad\xff\xae\x7d" "\xaf\x0e\xb0\x50\xb1\x01\xb9\x7f" 
	"\xba\x0a\xcd\x00\xce\x0f\xd4\x00" "\xd5\x8e\xd6\xe3\xd7\x38\xec\x00" 
	"\xed\x00\xee\xc0\xef\x00\xf0\x00" "\xf1\x00\xf2\x00\xf3\x30\xf4\x00" 
	"\xf5\x05\xf6\x35\xf7\x01\xf8\xaa" "\xf9\x00\xfa\x55\xfb\x00\xfc\x2a" 
, 0x0000040);
ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, REGISTERS, 0x0000000, buf, 0x0000040, 1000);
memcpy(buf, "\xfd\x00\xfd\x00", 0x0000004);
ret = writeRegisters(buf);
ret = select_register_bank(buf, 0);

ret = setRegisters2xx(buf);
/*That was URB 470 */

printf("Preparing for launch?\n");
ret = select_register_bank(buf, 0);
memcpy(buf, 
	"\x58\x00\x5e\xff\x82\x00\x83\x38" "\x84\x8e\x85\xe3\x8a\x00\x8e\x00" 
	"\x90\x64\x91\x00\x94\x30\x95\xa7" "\x96\x23\x9b\x3d\x9d\x3f\x9e\x00" 
	"\x9f\x40\xa0\x00\xa1\x00\xa2\x00" "\xa3\xff\xa4\xfe\xa5\x0d\xa6\x41" 
	"\xab\x00\xae\x85\xaf\x0e\xb0\x08" "\xb1\x07\xb2\x14\xb3\x01\xb4\x7b" 
, 0x0000040);
ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, REGISTERS, 0x0000000, buf, 0x0000040, 1000);

memcpy(buf, 
	"\xb5\x00\xb6\x70\xb7\x05\xc9\x00" "\xca\x00\xcb\x00\xcc\x00\xcd\x3c" 
	"\xce\x3c\xcf\x3c\xd0\x07\xd8\x05" "\xd9\x3c\xda\x54\xdb\x00\xdc\x01" 
	"\xde\x01\xdf\x17\xe0\x2c\xe1\x01" "\xe2\x00\xe3\x00\xe4\x00\xe5\xc8" 
	"\xe6\x00\xe7\x01\xe8\x00\xe9\x01" "\xea\x2c\xeb\x01\xec\x20\xed\x00" 
, 0x0000040);ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, REGISTERS, 0x0000000, buf, 0x0000040, 1000);
memcpy(buf, 
	"\xee\x30\xef\x20\xf0\xee\xf1\x01" "\xf2\x00\xf3\x0c\xf5\x11\xf6\x00" 
	"\xf7\x03\xf8\x02\xf9\xdd\xfa\x37" "\xfb\x0e\xfc\x00\xfd\x6c\xfe\x0b" 
	"\xff\xfc" 
, 0x0000022);ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, REGISTERS, 0x0000000, buf, 0x0000022, 1000);
usleep(135*1000);
/*
ES01_F8_WHITE_SHADING_DATA_FORMAT ES01_SHADING_4_INT_12_DEC
ES01_F4_ActiveTriger ACTION_TRIGER_ENABLE
*/
printf("Setting ? and trigger\n");
memcpy(buf, "\xf8\x02\xf4\x01", 0x0000004);
ret = writeRegisters(buf);

do{
	usleep(15*1000);
	ret = read_register(buf, 0x05);
	if(buf[0] != 0){
		break;
	}
	ret = read_register(buf, 0x04);
} while (buf[0] == 0x0);

usleep(30*1000);

/*Set RW size?*/
memcpy(buf, "\x7c\x96\x7d\x00\x7e\x00\x7f\x00", 0x0000008);
ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, REGISTERS, 0x0000000, buf, 0x0000008, 1000);
usleep(16*1000);
memcpy(buf, "\x2c\x01\x00\x00", 0x0000004);
ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, DMAREAD, 0x0000000, buf, 0x0000004, 1000);
ret = usb_bulk_read(devh, 0x00000082, buf, 0x0000200, 1030);

memcpy(buf, "\x7c\x00\x7d\x40\x7e\x00\x7f\x00", 0x0000008);
ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, REGISTERS, 0x0000000, buf, 0x0000008, 1000);
usleep(16*1000);
memcpy(buf, "\x00\x80\x00\x00", 0x0000004);
ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, DMAREAD, 0x0000000, buf, 0x0000004, 1000);
usleep(3*1000);
ret = usb_bulk_read(devh, 0x00000082, buf, 0x0008000, 2966);

usleep(186*1000);
memcpy(buf, "\x7c\x30\x7d\x35\x7e\x00\x7f\x00", 0x0000008);
ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, REGISTERS, 0x0000000, buf, 0x0000008, 1000);
/*That was URB 510 */
usleep(14*1000);
/*DMA reads?*/
memcpy(buf, "\x60\x6a\x00\x00", 0x0000004);
ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, DMAREAD, 0x0000000, buf, 0x0000004, 1000);
usleep(2*1000);
ret = usb_bulk_read(devh, 0x00000082, buf, 0x0006a00, 2628);

usleep(197*1000);
ret = usb_bulk_read(devh, 0x00000082, buf, 0x0000200, 1030);

usleep(4*1000);
ret = setScannerIdle(buf);
usleep(31*1000);
ret = readScanState(buf);
usleep(63*1000);
ret = readScanState(buf);
memcpy(buf, 
	"\x01\xa0\x56\x02\x00\xe5\x00\x00" "\x00\x00\x00\x00\x00\x00\x00\x00" 
, 0x0000010);ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, 0x000000a, 0x0000000, buf, 0x0000010, 1000);
usleep(20*1000);
memcpy(buf, 
	"\x01\xa0\x3e\x01\x01\x00\x00\x00" "\x00\x00\x00\x00\x00\x00\x00\x00" 
, 0x0000010);ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, 0x000000a, 0x0000000, buf, 0x0000010, 1000);
/*That was URB 520 */
usleep(17*1000);
memcpy(buf, 
	"\x01\xa0\x18\x01\x40\x00\x00\x00" "\x00\x00\x00\x00\x00\x00\x00\x00" 
, 0x0000010);ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, 0x000000a, 0x0000000, buf, 0x0000010, 1000);
usleep(41*1000);

ret = read_register(buf, 0x00);
/*
ES01_9D_MotorTableAddrA14_A21
ES01_79_AFEMCLK_SDRAMCLK_DELAY_CONTROL SDRAMCLK_DELAY_8_ns
*/
memcpy(buf, "\x9d\x3f\x79\x40", 0x0000004);
ret = writeRegisters(buf);
/*Could be end of a LLFSetMotorTable call?*/
memcpy(buf, 
	"\xa0\x00\xa1\xfc\xa2\x0f\xa3\xff" "\xa4\xff\xa5\x0f\x7c\x00\x7d\x04" 
	"\x7e\x00\x7f\x00" 
, 0x0000014);ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, REGISTERS, 0x0000000, buf, 0x0000014, 1000);
usleep(29*1000);
memcpy(buf, "\x00\x04\x00\x00", 0x0000004);
ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, DMAWRITE, 0x0000000, buf, 0x0000004, 1000);
usleep(16*1000);
memcpy(buf, table0x00000400, 0x0000400);
ret = usb_bulk_write(devh, 0x00000001, buf, 0x0000400, 1061);
usleep(15*1000);
ret = clearFIFO(buf);
/*That was URB 530 */
usleep(15*1000);
memcpy(buf, 
	"\xa0\x00\xa1\xfc\xa2\x0f\xa3\xff" "\xa4\xff\xa5\x0f\x7c\x00\x7d\x02" 
	"\x7e\x00\x79\x40" 
, 0x0000014);ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, REGISTERS, 0x0000000, buf, 0x0000014, 1000);
usleep(31*1000);
memcpy(buf, "\x00\x04\x00\x00", 0x0000004);
ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, DMAREAD, 0x0000000, buf, 0x0000004, 1000);

ret = usb_bulk_read(devh, 0x00000082, buf, 0x0000400, 1061);

memcpy(buf, 
	"\xa0\x00\xa1\xfe\xa2\x0f\xa3\xff" "\xa4\xff\xa5\x0f\x7c\x00\x7d\x02" 
	"\x7e\x00\x7f\x00" 
, 0x0000014);ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, REGISTERS, 0x0000000, buf, 0x0000014, 1000);
usleep(31*1000);
memcpy(buf, "\x00\x02\x00\x00", 0x0000004);
ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, DMAWRITE, 0x0000000, buf, 0x0000004, 1000);
usleep(15*1000);
memcpy(buf, table0x00000200, 0x0000200);
ret = usb_bulk_write(devh, 0x00000001, buf, 0x0000200, 1030);
usleep(16*1000);
ret = clearFIFO(buf);
usleep(15*1000);
memcpy(buf, 
	"\xa0\x00\xa1\xfe\xa2\x0f\xa3\xff" "\xa4\xff\xa5\x0f\x7c\x00\x7d\x01" 
	"\x7e\x00\x79\x40" 
, 0x0000014);ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, REGISTERS, 0x0000000, buf, 0x0000014, 1000);
usleep(32*1000);
memcpy(buf, "\x00\x02\x00\x00", 0x0000004);
ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, DMAREAD, 0x0000000, buf, 0x0000004, 1000);
/*That was URB 540 */
ret = usb_bulk_read(devh, 0x00000082, buf, 0x0000200, 1030);

/*Motor setup stuff*/
/*Do motor 0, enable home sensor*/
/*Speed = 1 pixel per unit time*/
printf("Motor Configuration\n");
memcpy(buf, 
	"\xa6\x41\xf6\x00\xe0\x00\xe1\x01" "\xe2\x58\xe3\x02\xe4\x00\xe5\x20" 
	"\xf3\x02\xea\x00\xeb\x00\x95\x87" "\xfd\xd0\xfe\x07\x96\x21\xf4\x01" 
, 0x0000020);
ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, REGISTERS, 0x0000000, buf, 0x0000020, 1000);

do {
	ret = readScanState(buf);
	usleep(61*1000);
} while (buf[0] != 0x0);
/*That was URB 550 */

/*That was URB 560 */
printf("GPIO Pins\n");
memcpy(buf, "\x96\x21\x95\xa7", 0x0000004);
ret = writeRegisters(buf);

/*some address register?*/
ret = read_register(buf, 0x0b);
ret = read_register(buf, 0x0a);
ret = read_register(buf, 0x09);

memcpy(buf, "\x00\x00\x86\x01", 0x0000004);
ret = writeRegisters(buf);

memcpy(buf, 
	"\x01\xa0\x05\x01\x01\x00\x00\x00" "\x00\x00\x00\x00\x00\x00\x00\x00" 
, 0x0000010);
ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, 0x000000a, 0x0000000, buf, 0x0000010, 1000);

ret = closescanchip(buf);
/*-----------------------------*/
ret = openscanchip(buf);

ret = adjustPowerSaveRegisters(buf);
usleep(7*1000);
ret = do_command_five(buf);
ret = do_command_one(buf);
/*That was URB 580 */

printf("AFE_ADCCLK_TIMING_ADJ\n");
memcpy(buf, "\x82\x00\x83\x38\x84\x8e\x85\xe3", 0x0000008);
ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, REGISTERS, 0x0000000, buf, 0x0000008, 1000);

printf("CCD Config again\n");
ret = select_register_bank(buf, 1);
memcpy(buf, 
	"\x60\x08\x61\x0b\x62\x0e\x63\x11" "\x64\x14\x65\x17\x66\x02\x67\x05" 
	"\x68\x08\xd4\x00\xd5\x8e\xd6\xe3" "\xd7\x38\xec\x00\xed\x00\xee\xc0" 
	"\xef\x00\xf0\x00\xf1\x00\xf2\x00" "\xf3\x30" 
, 0x000002a);ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, REGISTERS, 0x0000000, buf, 0x000002a, 1000);

ret = select_register_bank(buf, 0);
ret = readScanState(buf);

ret = setScannerIdle(buf);
usleep(16*1000);
ret = readScanState(buf);

printf("Set Home to First Line, Uniform speed\n");
memcpy(buf, "\xf3\x21\xf4\x00", 0x0000004);
ret = writeRegisters(buf);
/*That was URB 590 */
memcpy(buf, "\xf3\xf3\xf3\xf3", 0x0000004);
ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, 0x0000005, 0x0000000, buf, 0x0000004, 1000);
memcpy(buf, "\xf3\xf3\xf3\xf3", 0x0000004);
ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, SCANCONTROL, 0x0000000, buf, 0x0000004, 1000);
usleep(15*1000);

memcpy(buf, command_four, 0x0000010);
ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, 0x000000a, 0x0000000, buf, 0x0000010, 1000);
usleep(2*1000);
ret = usb_bulk_read(devh, 0x00000082, buf, 0x0000200, 1030);

usleep(14*1000);
ret = read_register(buf, 0x00);
ret = read_register(buf, 0x00);

ret = do_command_eight(buf);
memcpy(buf, 
	"\xf3\x32\xfd\x94\xfe\x11\xa6\x4d" "\xf6\x00\x96\x21\xe4\x01\xe2\x32" 
	"\xe3\x00\xe4\x00\x95\xa7\xf4\x01" 
, 0x0000018);ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, REGISTERS, 0x0000000, buf, 0x0000018, 1000);
/*That was URB 600 */
ret = readScanState(buf);
usleep(60*1000);
ret = readScanState(buf);

ret = read_register(buf, 0x0b);
ret = read_register(buf, 0x0a);
ret = read_register(buf, 0x09);

/*That was URB 610 */
ret = do_command_eight(buf);

printf("GPIO pins\n");
memcpy(buf, "\x96\x01\x96\x01", 0x0000004);
ret = writeRegisters(buf);
printf("AFEControl\n");
memcpy(buf, "\x9a\x00\x9a\x00", 0x0000004);
ret = writeRegisters(buf);
memcpy(buf, 
	"\x00\x01\x02\x0f\x04\x00\x06\x00" "\x08\x00\x0a\x00\x0c\x64\x0e\x64" 
	"\x10\x64\x12\x64" 
, 0x0000014);
ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, REGISTERS, 0x0000000, buf, 0x0000014, 1000);

ret = setScannerIdle(buf);
usleep(27*1000);
ret = readScanState(buf);

printf("GPIO pins\n");
memcpy(buf, "\x96\x03\x96\x03", 0x0000004);
ret = writeRegisters(buf);
memcpy(buf, 
	"\xa0\x00\xa1\xf4\xa2\x0f\xa3\xff" "\xa4\xff\xa5\x0f\x7c\x00\x7d\x10" 
	"\x7e\x00\x7f\x00" 
, 0x0000014);
ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, REGISTERS, 0x0000000, buf, 0x0000014, 1000);
usleep(31*1000);
memcpy(buf, "\x00\x10\x00\x00", 0x0000004);
ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, DMAWRITE, 0x0000000, buf, 0x0000004, 1000);
/*That was URB 620 */

usleep(15*1000);
memcpy(buf, table0x00001000type4, 0x0001000);
ret = usb_bulk_write(devh, 0x00000001, buf, 0x0001000, 1245);
usleep(16*1000);
ret = clearFIFO(buf);
usleep(47*1000);
printf("Setting address\n");
memcpy(buf, 
	"\xa0\x00\xa1\x00\xa2\x00\xa3\xff" "\xa4\xff\xa5\x0f\x7c\x00\x7d\x01" 
	"\x7e\x00\x79\x40" 
, 0x0000014);
ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, REGISTERS, 0x0000000, buf, 0x0000014, 1000);
usleep(31*1000);
memcpy(buf, "\x00\x02\x00\x00", 0x0000004);
ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, DMAREAD, 0x0000000, buf, 0x0000004, 1000);
ret = usb_bulk_read(devh, 0x00000082, buf, 0x0000200, 1030);


memcpy(buf, "\x9a\x00\x9a\x00", 0x0000004);
ret = writeRegisters(buf);
memcpy(buf, 
	"\x00\x01\x02\x0f\x04\x00\x06\x00" "\x08\x00\x0a\x00\x0c\x64\x0e\x64" 
	"\x10\x64\x12\x64" 
, 0x0000014);ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, REGISTERS, 0x0000000, buf, 0x0000014, 1000);

ret = CCDSetupType2(buf);

ret = setRegisters2xx(buf);

//2 3 but not 1
printf("bunchOfConfigv2\n");
ret = select_register_bank(buf, 0);
memcpy(buf, 
	"\x58\x01\x5e\x01\x82\x00\x83\x38" "\x84\x8e\x85\xe3\x8a\x00\x8e\x00" 
	"\x90\xa0\x91\x00\x94\x30\x95\xa7" "\x96\x03\x9b\x3d\x9d\x3f\x9e\x00" 
	"\x9f\x80\xa0\x00\xa1\x00\xa2\x00" "\xa3\xff\xa4\xfe\xa5\x0d\xa6\x40" 
	"\xab\x00\xae\xdb\xaf\x16\xb0\x08" "\xb1\x07\xb2\x14\xb3\x01\xb4\x7b" 
, 0x0000040);ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, REGISTERS, 0x0000000, buf, 0x0000040, 1000);
/*That was URB 640 */
memcpy(buf, 
	"\xb5\x00\xb6\x70\xb7\x05\xc9\x00" "\xca\x00\xcb\x01\xcc\x00\xcd\x3c" 
	"\xce\x3c\xcf\x3c\xd0\x07\xd8\x05" "\xd9\x3c\xda\x54\xdb\x00\xdc\x01" 
	"\xde\x01\xdf\x17\xe0\x01\xe1\x00" "\xe2\x00\xe3\x00\xe4\x00\xe5\x01" 
	"\xe6\x00\xe7\x00\xe8\x00\xe9\x00" "\xea\x2c\xeb\x01\xec\x01\xed\x00" 
, 0x0000040);ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, REGISTERS, 0x0000000, buf, 0x0000040, 1000);

memcpy(buf, 
	"\xee\x01\xef\x01\xf0\x14\xf1\x00" "\xf2\x00\xf3\x0c\xf5\x12\xf6\x00" 
	"\xf7\x00\xf8\x02\xf9\x03\xfa\x36" "\xfb\xfb\xfc\x00\xfd\xd9\xfe\x16" 
	"\xff\xfc" 
, 0x0000022);ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, REGISTERS, 0x0000000, buf, 0x0000022, 1000);
usleep(149*1000);


ret = reg7cReadAndStuff(buf);


printf("GPIO and AFE Control(?)\n");
memcpy(buf, "\x96\x03\x96\x03", 0x0000004);
ret = writeRegisters(buf);
memcpy(buf, "\x9a\x00\x9a\x00", 0x0000004);
ret = writeRegisters(buf);

memcpy(buf, 
	"\x00\x01\x02\x0f\x04\x00\x06\x00" "\x08\x00\x0a\x00\x0c\xfa\x0e\xfa" 
	"\x10\xfa\x12\xfa" 
, 0x0000014);ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, REGISTERS, 0x0000000, buf, 0x0000014, 1000);
ret = setScannerIdle(buf);
usleep(30*1000);
ret = readScanState(buf);
/*That was URB 660 */
ret = resetHostStartAddr(buf);
usleep(31*1000);

ret = reg7cReadAndStuff(buf);

usleep(2*1000);
memcpy(buf, "\x9a\x02\x9a\x02", 0x0000004);
ret = writeRegisters(buf);
memcpy(buf, "\x00\x01\x02\x0f", 0x0000004);
ret = writeRegisters(buf);

printf("Lamp settings?\n");
memcpy(buf, "\x90\xa0\x91\xa0", 0x0000004);
ret = writeRegisters(buf);

printf("GPIO\n");
memcpy(buf, "\x96\x23\x96\x23", 0x0000004);
ret = writeRegisters(buf);

printf("Long wait\n");
usleep(1982*1000);

ret = setScannerIdle(buf);
usleep(31*1000);
ret = readScanState(buf);
/*That was URB 680 */
memcpy(buf, "\x96\x23\x96\x23", 0x0000004);
ret = writeRegisters(buf);
memcpy(buf, 
	"\xa0\x00\xa1\xf4\xa2\x0f\xa3\xff" "\xa4\xff\xa5\x0f\x7c\x00\x7d\x10" 
	"\x7e\x00\x7f\x00" 
, 0x0000014);ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, REGISTERS, 0x0000000, buf, 0x0000014, 1000);
usleep(31*1000);
memcpy(buf, "\x00\x10\x00\x00", 0x0000004);
ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, DMAWRITE, 0x0000000, buf, 0x0000004, 1000);
usleep(15*1000);
memcpy(buf, table0x00001000type4, 0x0001000);
ret = usb_bulk_write(devh, 0x00000001, buf, 0x0001000, 1245);
usleep(16*1000);
ret = clearFIFO(buf);

usleep(32*1000);
memcpy(buf, 
	"\xa0\x00\xa1\x00\xa2\x00\xa3\xff" "\xa4\xff\xa5\x0f\x7c\x00\x7d\x01" 
	"\x7e\x00\x79\x40" 
, 0x0000014);ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, REGISTERS, 0x0000000, buf, 0x0000014, 1000);
usleep(30*1000);
memcpy(buf, "\x00\x02\x00\x00", 0x0000004);
ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, DMAREAD, 0x0000000, buf, 0x0000004, 1000);

ret = usb_bulk_read(devh, 0x00000082, buf, 0x0000200, 1030);

ret = select_register_bank(buf, 2);
/*That was URB 690 */
memcpy(buf, "\xa0\x01\xa0\x01", 0x0000004);
ret = writeRegisters(buf);
ret = select_register_bank(buf, 0);

ret = select_register_bank(buf, 2);
memcpy(buf, 
	"\xa1\x00\xa2\x19\xa1\x00\xa2\x1b" "\xa1\x00\xa2\x16\xa1\x00\xa2\x18" 
	"\xa1\x00\xa2\x19\xa1\x00\xa2\x1b" "\xa1\x00\xa2\x16\xa1\x00\xa2\x18" 
	"\xa1\x00\xa2\x19\xa1\x00\xa2\x1b" "\xa1\x00\xa2\x16\xa1\x00\xa2\x18" 
	"\xa1\x00\xa2\x19\xa1\x00\xa2\x1b" "\xa1\x00\xa2\x16\xa1\x00\xa2\x18" 
, 0x0000040);
ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, REGISTERS, 0x0000000, buf, 0x0000040, 1000);

memcpy(buf, 
	"\xa1\x00\xa2\x19\xa1\x00\xa2\x1b" "\xa1\x00\xa2\x16\xa1\x00\xa2\x18" 
	"\xa1\x00\xa2\x19\xa1\x00\xa2\x1b" "\xa1\x00\xa2\x16\xa1\x00\xa2\x18" 
	"\xa1\x00\xa2\x19\xa1\x00\xa2\x1b" "\xa1\x00\xa2\x16\xa1\x00\xa2\x18" 
	"\xa1\x00\xa2\x19\xa1\x00\xa2\x1b" "\xa1\x00\xa2\x16\xa1\x00\xa2\x18" 
, 0x0000040);
ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, REGISTERS, 0x0000000, buf, 0x0000040, 1000);
memcpy(buf, 
	"\xa1\x00\xa2\x19\xa1\x00\xa2\x1b" "\xa1\x00\xa2\x16\xa1\x00\xa2\x18" 
	"\xa1\x00\xa2\x19\xa1\x00\xa2\x1b" "\xa1\x00\xa2\x16\xa1\x00\xa2\x18" 
	"\xa1\x00\xa2\x19\xa1\x00\xa2\x1b" "\xa1\x00\xa2\x16\xa1\x00\xa2\x18" 
	"\xa1\x00\xa2\x19\xa1\x00\xa2\x1b" "\xa1\x00\xa2\x16\xa1\x00\xa2\x18" 
, 0x0000040);
ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, REGISTERS, 0x0000000, buf, 0x0000040, 1000);


//ret = select_register_bank(buf, 0);
//ret = select_register_bank(buf, 2);

memcpy(buf, "\xa0\x00\xa0\x00", 0x0000004);
ret = writeRegisters(buf);


//ret = select_register_bank(buf, 0);
/*That was URB 700 */

ret = CCDSetupType2(buf);

ret = setRegisters2xx(buf);
/*That was URB 710 */
ret = bunchOfConfig(buf);

usleep(147*1000);
printf("Set trigger\n");

ret = reg7cReadAndStuff(buf);


printf("GPIO\n");
memcpy(buf, "\x96\x23\x96\x23", 0x0000004);
ret = writeRegisters(buf);

usleep(2001*1000);

ret = setScannerIdle(buf);
usleep(31*1000);
ret = readScanState(buf);
/*That was URB 730 */

printf("Set start Address\n");
ret = resetHostStartAddr(buf);
usleep(31*1000);

ret = reg7cReadAndStuff(buf);

memcpy(buf, "\x96\x23\x96\x23", 0x0000004);
ret = writeRegisters(buf);
usleep(2001*1000);
ret = setScannerIdle(buf);
usleep(30*1000);
ret = readScanState(buf);
ret = resetHostStartAddr(buf);
usleep(31*1000);

ret = reg7cReadAndStuff(buf);

/*That was URB 760 */
memcpy(buf, "\x96\x23\x96\x23", 0x0000004);
ret = writeRegisters(buf);
usleep(2001*1000);
ret = setScannerIdle(buf);
usleep(31*1000);
ret = readScanState(buf);
ret = resetHostStartAddr(buf);
usleep(31*1000);

ret = reg7cReadAndStuff(buf);


memcpy(buf, "\x96\x23\x96\x23", 0x0000004);
ret = writeRegisters(buf);
usleep(1984*1000);
ret = setScannerIdle(buf);

usleep(31*1000);
ret = readScanState(buf);
/*That was URB 780 */

ret = resetHostStartAddr(buf);
usleep(31*1000);

ret = reg7cReadAndStuff(buf);

memcpy(buf, "\x96\x23\x96\x23", 0x0000004);
ret = writeRegisters(buf);
usleep(1969*1000);
ret = setScannerIdle(buf);
usleep(31*1000);
ret = readScanState(buf);
ret = resetHostStartAddr(buf);
usleep(31*1000);

ret = reg7cReadAndStuff(buf);

/*That was URB 810 */

memcpy(buf, "\x96\x23\x96\x23", 0x0000004);
ret = writeRegisters(buf);
usleep(1985*1000);
ret = setScannerIdle(buf);
usleep(31*1000);
ret = readScanState(buf);
ret = resetHostStartAddr(buf);
usleep(31*1000);

ret = reg7cReadAndStuff(buf);


memcpy(buf, "\x96\x23\x96\x23", 0x0000004);
ret = writeRegisters(buf);
usleep(2000*1000);
ret = setScannerIdle(buf);
/*That was URB 830 */
usleep(31*1000);
ret = readScanState(buf);

ret = resetHostStartAddr(buf);
usleep(30*1000);

ret = reg7cReadAndStuff(buf);


memcpy(buf, "\x96\x23\x96\x23", 0x0000004);
ret = writeRegisters(buf);
usleep(1998*1000);
ret = setScannerIdle(buf);
usleep(46*1000);
ret = readScanState(buf);
ret = resetHostStartAddr(buf);
/*That was URB 850 */
usleep(31*1000);

ret = reg7cReadAndStuff(buf);


memcpy(buf, "\x96\x23\x96\x23", 0x0000004);
ret = writeRegisters(buf);
usleep(2000*1000);
ret = setScannerIdle(buf);
usleep(31*1000);
ret = readScanState(buf);
ret = resetHostStartAddr(buf);
usleep(31*1000);

ret = reg7cReadAndStuff(buf);


memcpy(buf, "\x96\x23\x96\x23", 0x0000004);
ret = writeRegisters(buf);
/*That was URB 880 */
usleep(1969*1000);
ret = setScannerIdle(buf);
usleep(31*1000);
ret = readScanState(buf);
ret = resetHostStartAddr(buf);
usleep(31*1000);

ret = reg7cReadAndStuff(buf);


memcpy(buf, "\x96\x23\x96\x23", 0x0000004);
ret = writeRegisters(buf);
usleep(2001*1000);
ret = setScannerIdle(buf);
usleep(30*1000);
ret = readScanState(buf);
/*That was URB 900 */

ret = resetHostStartAddr(buf);
usleep(30*1000);

ret = reg7cReadAndStuff(buf);


memcpy(buf, "\x96\x23\x96\x23", 0x0000004);
ret = writeRegisters(buf);
usleep(1984*1000);
ret = setScannerIdle(buf);
usleep(31*1000);
ret = readScanState(buf);
ret = resetHostStartAddr(buf);
usleep(31*1000);

ret = reg7cReadAndStuff(buf);

/*That was URB 930 */
memcpy(buf, "\x96\x23\x96\x23", 0x0000004);
ret = writeRegisters(buf);
usleep(1969*1000);
ret = setScannerIdle(buf);
usleep(31*1000);
ret = readScanState(buf);
ret = resetHostStartAddr(buf);
usleep(31*1000);

ret = reg7cReadAndStuff(buf);

memcpy(buf, "\x96\x23\x96\x23", 0x0000004);
ret = writeRegisters(buf);
usleep(1999*1000);
ret = setScannerIdle(buf);
usleep(31*1000);

ret = readScanState(buf);
/*That was URB 950 */

ret = resetHostStartAddr(buf);
usleep(31*1000);

ret = reg7cReadAndStuff(buf);

memcpy(buf, "\x96\x23\x96\x23", 0x0000004);
ret = writeRegisters(buf);
usleep(1985*1000);
ret = setScannerIdle(buf);
usleep(31*1000);
ret = readScanState(buf);

ret = resetHostStartAddr(buf);
usleep(30*1000);

ret = reg7cReadAndStuff(buf);


memcpy(buf, "\x90\x64\x91\x64", 0x0000004);
ret = writeRegisters(buf);
usleep(1000*1000);
memcpy(buf, "\x96\x23\x96\x23", 0x0000004);
ret = writeRegisters(buf);
usleep(1985*1000);
ret = setScannerIdle(buf);
usleep(31*1000);
ret = readScanState(buf);
memcpy(buf, "\x96\x23\x96\x23", 0x0000004);
ret = writeRegisters(buf);

memcpy(buf, 
	"\xa0\x00\xa1\xf4\xa2\x0f\xa3\xff" "\xa4\xff\xa5\x0f\x7c\x00\x7d\x10" 
	"\x7e\x00\x7f\x00" 
, 0x0000014);ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, REGISTERS, 0x0000000, buf, 0x0000014, 1000);
usleep(30*1000);
memcpy(buf, "\x00\x10\x00\x00", 0x0000004);
ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, DMAWRITE, 0x0000000, buf, 0x0000004, 1000);
usleep(16*1000);
memcpy(buf, table0x00001000type4, 0x0001000);
ret = usb_bulk_write(devh, 0x00000001, buf, 0x0001000, 1245);
/*That was URB 990 */
usleep(15*1000);
ret = clearFIFO(buf);
usleep(31*1000);

memcpy(buf, 
	"\xa0\x00\xa1\x00\xa2\x00\xa3\xff" "\xa4\xff\xa5\x0f\x7c\x00\x7d\x01" 
	"\x7e\x00\x79\x40" 
, 0x0000014);ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, REGISTERS, 0x0000000, buf, 0x0000014, 1000);
usleep(31*1000);
memcpy(buf, "\x00\x02\x00\x00", 0x0000004);
ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, DMAREAD, 0x0000000, buf, 0x0000004, 1000);
ret = usb_bulk_read(devh, 0x00000082, buf, 0x0000200, 1030);

ret = select_register_bank(buf, 2);
memcpy(buf, "\xa0\x01\xa0\x01", 0x0000004);
ret = writeRegisters(buf);
ret = select_register_bank(buf, 0);

ret = select_register_bank(buf, 2);
memcpy(buf, 
	"\xa1\x00\xa2\x19\xa1\x00\xa2\x1b" "\xa1\x00\xa2\x16\xa1\x00\xa2\x18" 
	"\xa1\x00\xa2\x19\xa1\x00\xa2\x1b" "\xa1\x00\xa2\x16\xa1\x00\xa2\x18" 
	"\xa1\x00\xa2\x19\xa1\x00\xa2\x1b" "\xa1\x00\xa2\x16\xa1\x00\xa2\x18" 
	"\xa1\x00\xa2\x19\xa1\x00\xa2\x1b" "\xa1\x00\xa2\x16\xa1\x00\xa2\x18" 
, 0x0000040);ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, REGISTERS, 0x0000000, buf, 0x0000040, 1000);
/*That was URB 1000 */
memcpy(buf, 
	"\xa1\x00\xa2\x19\xa1\x00\xa2\x1b" "\xa1\x00\xa2\x16\xa1\x00\xa2\x18" 
	"\xa1\x00\xa2\x19\xa1\x00\xa2\x1b" "\xa1\x00\xa2\x16\xa1\x00\xa2\x18" 
	"\xa1\x00\xa2\x19\xa1\x00\xa2\x1b" "\xa1\x00\xa2\x16\xa1\x00\xa2\x18" 
	"\xa1\x00\xa2\x19\xa1\x00\xa2\x1b" "\xa1\x00\xa2\x16\xa1\x00\xa2\x18" 
, 0x0000040);ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, REGISTERS, 0x0000000, buf, 0x0000040, 1000);

memcpy(buf, 
	"\xa1\x00\xa2\x19\xa1\x00\xa2\x1b" "\xa1\x00\xa2\x16\xa1\x00\xa2\x18" 
	"\xa1\x00\xa2\x19\xa1\x00\xa2\x1b" "\xa1\x00\xa2\x16\xa1\x00\xa2\x18" 
	"\xa1\x00\xa2\x19\xa1\x00\xa2\x1b" "\xa1\x00\xa2\x16\xa1\x00\xa2\x18" 
	"\xa1\x00\xa2\x19\xa1\x00\xa2\x1b" "\xa1\x00\xa2\x16\xa1\x00\xa2\x18" 
, 0x0000040);ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, REGISTERS, 0x0000000, buf, 0x0000040, 1000);


ret = select_register_bank(buf, 0);


ret = select_register_bank(buf, 2);

memcpy(buf, "\xa0\x00\xa0\x00", 0x0000004);
ret = writeRegisters(buf);


ret = select_register_bank(buf, 0);

ret = CCDSetupType2(buf);

ret = setRegisters2xx(buf);

ret = bunchOfConfig(buf);
/*That was URB 1020 */
usleep(147*1000);

ret = reg7cReadAndStuff(buf);

printf("GPIO again\n");
memcpy(buf, "\x96\x23\x96\x23", 0x0000004);
ret = writeRegisters(buf);

usleep(1985*1000);

ret = setScannerIdle(buf);
usleep(30*1000);
ret = readScanState(buf);

//host start address
ret = resetHostStartAddr(buf);

usleep(31*1000);

ret = reg7cReadAndStuff(buf);

memcpy(buf, "\x96\x23\x96\x23", 0x0000004);
ret = writeRegisters(buf);
/*That was URB 1050 */
usleep(1969*1000);
ret = setScannerIdle(buf);
usleep(30*1000);

ret = readScanState(buf);

ret = resetHostStartAddr(buf);
usleep(31*1000);

ret = reg7cReadAndStuff(buf);

memcpy(buf, "\x96\x23\x96\x23", 0x0000004);
ret = writeRegisters(buf);
usleep(1969*1000);
ret = setScannerIdle(buf);
usleep(31*1000);
ret = readScanState(buf);
/*That was URB 1070 */
ret = resetHostStartAddr(buf);
usleep(48*1000);

ret = reg7cReadAndStuff(buf);

/*That was URB 1090 */
memcpy(buf, "\x96\x23\x96\x23", 0x0000004);
ret = writeRegisters(buf);

memcpy(buf, 
	"\xa0\x00\xa1\xf4\xa2\x0f\xa3\xff" "\xa4\xff\xa5\x0f\x7c\x00\x7d\x10" 
	"\x7e\x00\x7f\x00" 
, 0x0000014);ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, REGISTERS, 0x0000000, buf, 0x0000014, 1000);
usleep(30*1000);
memcpy(buf, "\x00\x10\x00\x00", 0x0000004);
ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, DMAWRITE, 0x0000000, buf, 0x0000004, 1000);
usleep(16*1000);
memcpy(buf, table0x00001000type3, 0x0001000);
ret = usb_bulk_write(devh, 0x00000001, buf, 0x0001000, 1245);

usleep(15*1000);
ret = clearFIFO(buf);
usleep(31*1000);

memcpy(buf, 
	"\xa0\x00\xa1\x00\xa2\x00\xa3\xff" "\xa4\xff\xa5\x0f\x7c\x00\x7d\x01" 
	"\x7e\x00\x79\x40" 
, 0x0000014);ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, REGISTERS, 0x0000000, buf, 0x0000014, 1000);
usleep(32*1000);
memcpy(buf, "\x00\x02\x00\x00", 0x0000004);
ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, DMAREAD, 0x0000000, buf, 0x0000004, 1000);
ret = usb_bulk_read(devh, 0x00000082, buf, 0x0000200, 1030);

ret = select_register_bank(buf, 2);
/*That was URB 1100 */

memcpy(buf, "\xa0\x01\xa0\x01", 0x0000004);
ret = writeRegisters(buf);
ret = select_register_bank(buf, 0);
ret = select_register_bank(buf, 2);
memcpy(buf, 
	"\xa1\x00\xa2\x19\xa1\x00\xa2\x1b" "\xa1\x00\xa2\x16\xa1\x00\xa2\x18" 
	"\xa1\x00\xa2\x19\xa1\x00\xa2\x1b" "\xa1\x00\xa2\x16\xa1\x00\xa2\x18" 
	"\xa1\x00\xa2\x19\xa1\x00\xa2\x1b" "\xa1\x00\xa2\x16\xa1\x00\xa2\x18" 
	"\xa1\x00\xa2\x19\xa1\x00\xa2\x1b" "\xa1\x00\xa2\x16\xa1\x00\xa2\x18" 
, 0x0000040);ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, REGISTERS, 0x0000000, buf, 0x0000040, 1000);

memcpy(buf, 
	"\xa1\x00\xa2\x19\xa1\x00\xa2\x1b" "\xa1\x00\xa2\x16\xa1\x00\xa2\x18" 
	"\xa1\x00\xa2\x19\xa1\x00\xa2\x1b" "\xa1\x00\xa2\x16\xa1\x00\xa2\x18" 
	"\xa1\x00\xa2\x19\xa1\x00\xa2\x1b" "\xa1\x00\xa2\x16\xa1\x00\xa2\x18" 
	"\xa1\x00\xa2\x19\xa1\x00\xa2\x1b" "\xa1\x00\xa2\x16\xa1\x00\xa2\x18" 
, 0x0000040);ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, REGISTERS, 0x0000000, buf, 0x0000040, 1000);
memcpy(buf, 
	"\xa1\x00\xa2\x19\xa1\x00\xa2\x1b" "\xa1\x00\xa2\x16\xa1\x00\xa2\x18" 
	"\xa1\x00\xa2\x19\xa1\x00\xa2\x1b" "\xa1\x00\xa2\x16\xa1\x00\xa2\x18" 
	"\xa1\x00\xa2\x19\xa1\x00\xa2\x1b" "\xa1\x00\xa2\x16\xa1\x00\xa2\x18" 
	"\xa1\x00\xa2\x19\xa1\x00\xa2\x1b" "\xa1\x00\xa2\x16\xa1\x00\xa2\x18" 
, 0x0000040);ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, REGISTERS, 0x0000000, buf, 0x0000040, 1000);

ret = select_register_bank(buf, 0);
ret = select_register_bank(buf, 2);
memcpy(buf, "\xa0\x00\xa0\x00", 0x0000004);
ret = writeRegisters(buf);

ret = select_register_bank(buf, 0);
/*That was URB 1110 */
ret = CCDSetupType2(buf);

ret = setRegisters2xx(buf);
/*That was URB 1120 */

ret = select_register_bank(buf, 0);
memcpy(buf, 
	"\x58\x01\x5e\x01\x82\x00\x83\x38" "\x84\x8e\x85\xe3\x8a\x00\x8e\x00" 
	"\x90\x64\x91\x00\x94\x30\x95\xa7" "\x96\x23\x9b\x3d\x9d\x3f\x9e\x00" 
	"\x9f\x80\xa0\x00\xa1\x00\xa2\x00" "\xa3\xff\xa4\xfe\xa5\x0d\xa6\x41" 
	"\xab\x00\xae\x08\xaf\x00\xb0\x08" "\xb1\x07\xb2\x14\xb3\x01\xb4\x7b" 
, 0x0000040);ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, REGISTERS, 0x0000000, buf, 0x0000040, 1000);

memcpy(buf, 
	"\xb5\x00\xb6\x70\xb7\x05\xc9\x00" "\xca\x00\xcb\x01\xcc\x00\xcd\x3c" 
	"\xce\x3c\xcf\x3c\xd0\x07\xd8\x05" "\xd9\x3c\xda\x54\xdb\x00\xdc\x01" 
	"\xde\x01\xdf\x17\xe0\x2c\xe1\x01" "\xe2\x00\xe3\x00\xe4\x00\xe5\xc8" 
	"\xe6\x00\xe7\x00\xe8\x00\xe9\x00" "\xea\x2c\xeb\x01\xec\x08\xed\x00" 
, 0x0000040);ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, REGISTERS, 0x0000000, buf, 0x0000040, 1000);
memcpy(buf, 
	"\xee\x18\xef\x08\xf0\x6a\xf1\x00" "\xf2\x00\xf3\x0c\xf5\x12\xf6\x00" 
	"\xf7\x00\xf8\x02\xf9\x03\xfa\x36" "\xfb\xfb\xfc\x00\xfd\xd9\xfe\x16" 
	"\xff\xfc" 
, 0x0000022);ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, REGISTERS, 0x0000000, buf, 0x0000022, 1000);
usleep(147*1000);
memcpy(buf, "\xf8\x02\xf4\x01", 0x0000004);
ret = writeRegisters(buf);

printf("checking 4 and 5 again\n");
usleep(16*1000);

ret = read_register(buf, 0x05);
ret = read_register(buf, 0x04);

usleep(15*1000);

ret = read_register(buf, 0x05);
ret = read_register(buf, 0x04);

usleep(15*1000);

ret = read_register(buf, 0x05);
ret = read_register(buf, 0x04);

usleep(15*1000);

ret = read_register(buf, 0x05);
ret = read_register(buf, 0x04);
/*That was URB 1140 */

usleep(15*1000);

ret = read_register(buf, 0x05);
ret = read_register(buf, 0x04);

usleep(15*1000);

ret = read_register(buf, 0x05);
ret = read_register(buf, 0x04);

usleep(30*1000);

memcpy(buf, "\x7c\x00\x7d\x00\x7e\x04\x7f\x00", 0x0000008);
ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, REGISTERS, 0x0000000, buf, 0x0000008, 1000);
/*That was URB 1150 */
/*calibration scanning?*/
printf("reading 30 lines\n");
memcpy(buf, "\x00\x00\x08\x00", 0x0000004);
ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, DMAREAD, 0x0000000, buf, 0x0000004, 1000);
ret = usb_bulk_read(devh, 0x00000082, buf, 0x000fc00, 4870);
ret = usb_bulk_read(devh, 0x00000082, buf, 0x000fc00, 4870);
ret = usb_bulk_read(devh, 0x00000082, buf, 0x000fc00, 4870);
ret = usb_bulk_read(devh, 0x00000082, buf, 0x000fc00, 4870);
ret = usb_bulk_read(devh, 0x00000082, buf, 0x000fc00, 4870);
ret = usb_bulk_read(devh, 0x00000082, buf, 0x000fc00, 4870);
ret = usb_bulk_read(devh, 0x00000082, buf, 0x000fc00, 4870);
ret = usb_bulk_read(devh, 0x00000082, buf, 0x000fc00, 4870);
ret = usb_bulk_read(devh, 0x00000082, buf, 0x0002000, 1491);

/*That was URB 1160 */
usleep(2*1000);
printf("reading 30 lines\n");
memcpy(buf, "\x7c\x01\x7d\x9d\x7e\x03\x7f\x00", 0x0000008);
ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, REGISTERS, 0x0000000, buf, 0x0000008, 1000);
memcpy(buf, "\x02\x3a\x07\x00", 0x0000004);
ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, DMAREAD, 0x0000000, buf, 0x0000004, 1000);
ret = usb_bulk_read(devh, 0x00000082, buf, 0x000fc00, 4870);
ret = usb_bulk_read(devh, 0x00000082, buf, 0x000fc00, 4870);
ret = usb_bulk_read(devh, 0x00000082, buf, 0x000fc00, 4870);
ret = usb_bulk_read(devh, 0x00000082, buf, 0x000fc00, 4870);
ret = usb_bulk_read(devh, 0x00000082, buf, 0x000fc00, 4870);
ret = usb_bulk_read(devh, 0x00000082, buf, 0x000fc00, 4870);
ret = usb_bulk_read(devh, 0x00000082, buf, 0x000fc00, 4870);
ret = usb_bulk_read(devh, 0x00000082, buf, 0x0005600, 2320);
ret = usb_bulk_read(devh, 0x00000082, buf, 0x0000200, 1030);

usleep(12*1000);

ret = setScannerIdle(buf);

usleep(30*1000);

ret = readScanState(buf);

usleep(29*1000);

ret = read_register(buf, 0x0b);
ret = read_register(buf, 0x0a);
ret = read_register(buf, 0x09);
/*That was URB 1180 */
ret = read_register(buf, 0x00);

ret = do_command_eight(buf);

memcpy(buf, 
	"\xf3\x21\xfd\xb8\xfe\x0b\xa6\x41" "\xf6\x00\xea\x00\xeb\xf0\xe2\x3d" 
	"\xe3\x00\xe4\x00\xe0\x00\xe1\x00" "\xe5\x00\x96\x21\xf4\x01" 
, 0x000001e);
ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, REGISTERS, 0x0000000, buf, 0x000001e, 1000);

ret = readScanState(buf);
usleep(61*1000);

ret = readScanState(buf);
usleep(62*1000);

ret = readScanState(buf);
/*That was URB 1190 */

ret = read_register(buf, 0x0b);
ret = read_register(buf, 0x0a);
ret = read_register(buf, 0x09);

ret = do_command_eight(buf);

ret = do_command_eight(buf);
usleep(28*1000);

ret = setScannerIdle(buf);

usleep(31*1000);

ret = readScanState(buf);
/*That was URB 1200 */

memcpy(buf, "\x96\x23\x96\x23", 0x0000004);
ret = writeRegisters(buf);

memcpy(buf, 
	"\xa0\x00\xa1\xf4\xa2\x0f\xa3\xff" "\xa4\xff\xa5\x0f\x7c\x00\x7d\x10" 
	"\x7e\x00\x7f\x00" 
, 0x0000014);
ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, REGISTERS, 0x0000000, buf, 0x0000014, 1000);
usleep(30*1000);
memcpy(buf, "\x00\x10\x00\x00", 0x0000004);
ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, DMAWRITE, 0x0000000, buf, 0x0000004, 1000);
usleep(15*1000);
memcpy(buf, table0x00001000type2, 0x0001000);
ret = usb_bulk_write(devh, 0x00000001, buf, 0x0001000, 1245);
usleep(16*1000);

ret = clearFIFO(buf);

usleep(31*1000);

memcpy(buf, 
	"\xa0\x00\xa1\x00\xa2\x00\xa3\xff" "\xa4\xff\xa5\x0f\x7c\x00\x7d\x01" 
	"\x7e\x00\x79\x40" 
, 0x0000014);ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, REGISTERS, 0x0000000, buf, 0x0000014, 1000);

usleep(31*1000);

memcpy(buf, "\x00\x02\x00\x00", 0x0000004);
ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, DMAREAD, 0x0000000, buf, 0x0000004, 1000);
ret = usb_bulk_read(devh, 0x00000082, buf, 0x0000200, 1030);


/*That was URB 1210 */

ret = select_register_bank(buf, 2);
memcpy(buf, "\xa0\x01\xa0\x01", 0x0000004);
ret = writeRegisters(buf);

ret = select_register_bank(buf, 0);

ret = select_register_bank(buf, 2);
memcpy(buf, 
	"\xa1\x00\xa2\x19\xa1\x00\xa2\x1b" "\xa1\x00\xa2\x16\xa1\x00\xa2\x18" 
	"\xa1\x00\xa2\x19\xa1\x00\xa2\x1b" "\xa1\x00\xa2\x16\xa1\x00\xa2\x18" 
	"\xa1\x00\xa2\x19\xa1\x00\xa2\x1b" "\xa1\x00\xa2\x16\xa1\x00\xa2\x18" 
	"\xa1\x00\xa2\x19\xa1\x00\xa2\x1b" "\xa1\x00\xa2\x16\xa1\x00\xa2\x18" 
, 0x0000040);ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, REGISTERS, 0x0000000, buf, 0x0000040, 1000);
memcpy(buf, 
	"\xa1\x00\xa2\x19\xa1\x00\xa2\x1b" "\xa1\x00\xa2\x16\xa1\x00\xa2\x18" 
	"\xa1\x00\xa2\x19\xa1\x00\xa2\x1b" "\xa1\x00\xa2\x16\xa1\x00\xa2\x18" 
	"\xa1\x00\xa2\x19\xa1\x00\xa2\x1b" "\xa1\x00\xa2\x16\xa1\x00\xa2\x18" 
	"\xa1\x00\xa2\x19\xa1\x00\xa2\x1b" "\xa1\x00\xa2\x16\xa1\x00\xa2\x18" 
, 0x0000040);ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, REGISTERS, 0x0000000, buf, 0x0000040, 1000);

memcpy(buf, 
	"\xa1\x00\xa2\x19\xa1\x00\xa2\x1b" "\xa1\x00\xa2\x16\xa1\x00\xa2\x18" 
	"\xa1\x00\xa2\x19\xa1\x00\xa2\x1b" "\xa1\x00\xa2\x16\xa1\x00\xa2\x18" 
	"\xa1\x00\xa2\x19\xa1\x00\xa2\x1b" "\xa1\x00\xa2\x16\xa1\x00\xa2\x18" 
	"\xa1\x00\xa2\x19\xa1\x00\xa2\x1b" "\xa1\x00\xa2\x16\xa1\x00\xa2\x18" 
, 0x0000040);ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, REGISTERS, 0x0000000, buf, 0x0000040, 1000);

ret = select_register_bank(buf, 0);
ret = select_register_bank(buf, 2);
memcpy(buf, "\xa0\x00\xa0\x00", 0x0000004);
ret = writeRegisters(buf);
/*That was URB 1220 */
ret = select_register_bank(buf, 0);

ret = CCDSetupType2(buf);

ret = setRegisters2xx(buf);

ret = select_register_bank(buf, 0);
memcpy(buf, 
	"\x58\x01\x5e\x01\x82\x00\x83\x38" "\x84\x8e\x85\xe3\x8a\x00\x8e\x00" 
	"\x90\x64\x91\x00\x94\x30\x95\xa7" "\x96\x23\x9b\x3d\x9d\x3f\x9e\x00" 
	"\x9f\x80\xa0\x00\xa1\x00\xa2\x00" "\xa3\xff\xa4\xfe\xa5\x0d\xa6\x41" 
	"\xab\x00\xae\x08\xaf\x00\xb0\x10" "\xb1\x0e\xb2\x14\xb3\x01\xb4\x7b" 
, 0x0000040);ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, REGISTERS, 0x0000000, buf, 0x0000040, 1000);
memcpy(buf, 
	"\xb5\x00\xb6\x70\xb7\x05\xc9\x00" "\xca\x00\xcb\x01\xcc\x00\xcd\x3c" 
	"\xce\x3c\xcf\x3c\xd0\x07\xd8\x05" "\xd9\x3c\xda\x54\xdb\x00\xdc\x01" 
	"\xde\x01\xdf\x17\xe0\x2c\xe1\x01" "\xe2\x00\xe3\x00\xe4\x00\xe5\xc8" 
	"\xe6\x00\xe7\x00\xe8\x00\xe9\x00" "\xea\x2c\xeb\x01\xec\x08\xed\x00" 
, 0x0000040);ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, REGISTERS, 0x0000000, buf, 0x0000040, 1000);
memcpy(buf, 
	"\xee\x18\xef\x08\xf0\xa6\xf1\x00" "\xf2\x00\xf3\x0c\xf5\x12\xf6\x00" 
	"\xf7\x00\xf8\x02\xf9\x03\xfa\x36" "\xfb\xfb\xfc\x00\xfd\xf1\xfe\x2b" 
	"\xff\xfc" 
, 0x0000022);ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, REGISTERS, 0x0000000, buf, 0x0000022, 1000);
usleep(147*1000);

memcpy(buf, "\xf8\x02\xf4\x01", 0x0000004);
ret = writeRegisters(buf);
usleep(15*1000);

printf("Checking 4 and 5 over and over...\n");
ret = read_register(buf, 0x05);
ret = read_register(buf, 0x04);
/*That was URB 1240 */
usleep(15*1000);

ret = read_register(buf, 0x05);
ret = read_register(buf, 0x04);
usleep(15*1000);

ret = read_register(buf, 0x05);
ret = read_register(buf, 0x04);
usleep(15*1000);

ret = read_register(buf, 0x05);
/*That was URB 1250 */
ret = read_register(buf, 0x04);
usleep(15*1000);

ret = read_register(buf, 0x05);
ret = read_register(buf, 0x04);
usleep(14*1000);

ret = read_register(buf, 0x05);
ret = read_register(buf, 0x04);
/*That was URB 1260 */
usleep(15*1000);

ret = read_register(buf, 0x05);
ret = read_register(buf, 0x04);
usleep(15*1000);

ret = read_register(buf, 0x05);
ret = read_register(buf, 0x04);
usleep(14*1000);

ret = read_register(buf, 0x05);
/*That was URB 1270 */
ret = read_register(buf, 0x04);
usleep(15*1000);

ret = read_register(buf, 0x05);
ret = read_register(buf, 0x04);
usleep(15*1000);

ret = read_register(buf, 0x05);
ret = read_register(buf, 0x04);
/*That was URB 1280 */

printf("Moving along.\n");

usleep(31*1000);

memcpy(buf, "\x7c\x00\x7d\x00\x7e\x04\x7f\x00", 0x0000008);
ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, REGISTERS, 0x0000000, buf, 0x0000008, 1000);

usleep(16*1000);
/*calibration scanning?*/
printf("reading 30 lines\n");
memcpy(buf, "\x00\x00\x08\x00", 0x0000004);
ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, DMAREAD, 0x0000000, buf, 0x0000004, 1000);
ret = usb_bulk_read(devh, 0x00000082, buf, 0x000fc00, 4870);
ret = usb_bulk_read(devh, 0x00000082, buf, 0x000fc00, 4870);
ret = usb_bulk_read(devh, 0x00000082, buf, 0x000fc00, 4870);
ret = usb_bulk_read(devh, 0x00000082, buf, 0x000fc00, 4870);
ret = usb_bulk_read(devh, 0x00000082, buf, 0x000fc00, 4870);
ret = usb_bulk_read(devh, 0x00000082, buf, 0x000fc00, 4870);
ret = usb_bulk_read(devh, 0x00000082, buf, 0x000fc00, 4870);
ret = usb_bulk_read(devh, 0x00000082, buf, 0x000fc00, 4870);
ret = usb_bulk_read(devh, 0x00000082, buf, 0x0002000, 1491);

/*calibration scanning?*/
printf("reading 30 lines\n");
memcpy(buf, "\x00\x00\x08\x00", 0x0000004);
ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, DMAREAD, 0x0000000, buf, 0x0000004, 1000);
ret = usb_bulk_read(devh, 0x00000082, buf, 0x000fc00, 4870);
ret = usb_bulk_read(devh, 0x00000082, buf, 0x000fc00, 4870);
ret = usb_bulk_read(devh, 0x00000082, buf, 0x000fc00, 4870);
ret = usb_bulk_read(devh, 0x00000082, buf, 0x000fc00, 4870);
ret = usb_bulk_read(devh, 0x00000082, buf, 0x000fc00, 4870);
ret = usb_bulk_read(devh, 0x00000082, buf, 0x000fc00, 4870);
ret = usb_bulk_read(devh, 0x00000082, buf, 0x000fc00, 4870);
ret = usb_bulk_read(devh, 0x00000082, buf, 0x000fc00, 4870);
ret = usb_bulk_read(devh, 0x00000082, buf, 0x0002000, 1491);

/*calibration scanning?*/
memcpy(buf, "\x00\x00\x08\x00", 0x0000004);
ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, DMAREAD, 0x0000000, buf, 0x0000004, 1000);

usleep(17*1000);
ret = usb_bulk_read(devh, 0x00000082, buf, 0x000fc00, 4870);

usleep(28*1000);
ret = usb_bulk_read(devh, 0x00000082, buf, 0x000fc00, 4870);

usleep(32*1000);
ret = usb_bulk_read(devh, 0x00000082, buf, 0x000fc00, 4870);

usleep(26*1000);
ret = usb_bulk_read(devh, 0x00000082, buf, 0x000fc00, 4870);

usleep(19*1000);
ret = usb_bulk_read(devh, 0x00000082, buf, 0x000fc00, 4870);

usleep(26*1000);
ret = usb_bulk_read(devh, 0x00000082, buf, 0x000fc00, 4870);

usleep(20*1000);
ret = usb_bulk_read(devh, 0x00000082, buf, 0x000fc00, 4870);

usleep(21*1000);
ret = usb_bulk_read(devh, 0x00000082, buf, 0x000fc00, 4870);
/*That was URB 1310 */

usleep(13*1000);
ret = usb_bulk_read(devh, 0x00000082, buf, 0x0002000, 1491);

usleep(2*1000);
memcpy(buf, "\x7c\x23\x7d\xfb\x7e\x02\x7f\x00", 0x0000008);
ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, REGISTERS, 0x0000000, buf, 0x0000008, 1000);

usleep(12*1000);
memcpy(buf, "\x46\xf6\x05\x00", 0x0000004);
ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, DMAREAD, 0x0000000, buf, 0x0000004, 1000);

usleep(8*1000);
ret = usb_bulk_read(devh, 0x00000082, buf, 0x000fc00, 4870);

usleep(21*1000);
ret = usb_bulk_read(devh, 0x00000082, buf, 0x000fc00, 4870);

usleep(20*1000);
ret = usb_bulk_read(devh, 0x00000082, buf, 0x000fc00, 4870);

usleep(20*1000);
ret = usb_bulk_read(devh, 0x00000082, buf, 0x000fc00, 4870);

usleep(20*1000);
ret = usb_bulk_read(devh, 0x00000082, buf, 0x000fc00, 4870);

usleep(20*1000);
ret = usb_bulk_read(devh, 0x00000082, buf, 0x000fc00, 4870);

usleep(13*1000);
ret = usb_bulk_read(devh, 0x00000082, buf, 0x0000e00, 1215);
/*That was URB 1320 */

ret = usb_bulk_read(devh, 0x00000082, buf, 0x0000200, 1030);

usleep(19*1000);

ret = setScannerIdle(buf);
usleep(30*1000);

ret = readScanState(buf);

memcpy(buf, "\x96\x23\x96\x23", 0x0000004);
ret = writeRegisters(buf);

usleep(151*1000);

ret = read_register(buf, 0x00);

ret = do_command_eight(buf);

memcpy(buf, 
	"\xf3\x22\xfd\x94\xfe\x11\xa6\x41" "\xf6\x00\x96\x21\xe4\x01\xe2\x40" 
	"\xe3\x1f\xe4\x00\x95\xa7\xf4\x01" 
, 0x0000018);ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, REGISTERS, 0x0000000, buf, 0x0000018, 1000);

ret = readScanState(buf);
/*That was URB 1330 */

usleep(50*1000);
ret = readScanState(buf);

usleep(62*1000);
ret = readScanState(buf);

usleep(62*1000);
ret = readScanState(buf);

usleep(62*1000);
ret = readScanState(buf);

usleep(62*1000);
ret = readScanState(buf);

usleep(62*1000);
ret = readScanState(buf);

usleep(62*1000);
ret = readScanState(buf);

ret = read_register(buf, 0x0b);
ret = read_register(buf, 0x0a);
ret = read_register(buf, 0x09);

ret = do_command_eight(buf);
usleep(64*1000);

ret = read_register(buf, 0x00);
ret = do_command_eight(buf);

memcpy(buf, 
	"\xf3\x32\xfd\x94\xfe\x11\xa6\x4d" "\xf6\x00\x96\x21\xe4\x01\xe2\x32" 
	"\xe3\x00\xe4\x00\x95\xa7\xf4\x01" 
, 0x0000018);ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, REGISTERS, 0x0000000, buf, 0x0000018, 1000);
ret = readScanState(buf);
usleep(58*1000);
ret = readScanState(buf);
/*That was URB 1360 */

ret = read_register(buf, 0x0b);
ret = read_register(buf, 0x0a);
ret = read_register(buf, 0x09);

ret = do_command_eight(buf);
usleep(76*1000);

/*set start address for DMA*/
memcpy(buf, 
	"\xa0\x00\xa1\x00\xa2\x80\xa3\xff" "\xa4\xff\xa5\x0f\x7c\x00\x7d\x06" 
	"\x7e\x00\x7f\x00" 
, 0x0000014);ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, REGISTERS, 0x0000000, buf, 0x0000014, 1000);
usleep(31*1000);
memcpy(buf, "\x00\x06\x00\x00", 0x0000004);
ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, DMAWRITE, 0x0000000, buf, 0x0000004, 1000);
memcpy(buf, table0x00000600, 0x0000600);
ret = usb_bulk_write(devh, 0x00000001, buf, 0x0000600, 1092);
/*That was URB 1370 */
usleep(16*1000);

ret = clearFIFO(buf);

usleep(34*1000);

//addresses for DMA write.
memcpy(buf, 
	"\xa0\x00\xa1\x40\xa2\x0f\xa3\xff" "\xa4\xff\xa5\x0f\x7c\x6c\x7d\x0c" 
	"\x7e\x01\x7f\x00" 
, 0x0000014);ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, REGISTERS, 0x0000000, buf, 0x0000014, 1000);
usleep(28*1000);

//this is the size of the DMA write incoming
memcpy(buf, "\x00\xf0\x00\x00", 0x0000004);
ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, DMAWRITE, 0x0000000, buf, 0x0000004, 1000);
memcpy(buf, table0x000f000, 0x000f000);
ret = usb_bulk_write(devh, 0x00000001, buf, 0x000f000, 4686);
usleep(57*1000);

memcpy(buf, "\x6c\x1c\x00\x00", 0x0000004);
ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, DMAWRITE, 0x0000000, buf, 0x0000004, 1000);
memcpy(buf, table0x00001c6c, 0x0001c6c);
ret = usb_bulk_write(devh, 0x00000001, buf, 0x0001c6c, 1436);

usleep(16*1000);
memcpy(buf, "\x6c\x6c\x6c\x6c", 0x0000004);
ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, 0x0000005, 0x0000000, buf, 0x0000004, 1000);
memcpy(buf, "\x6c\x6c\x6c\x6c", 0x0000004);
ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, SCANCONTROL, 0x0000000, buf, 0x0000004, 1000);
usleep(16*1000);
ret = setScannerIdle(buf);
/*That was URB 1380 */
usleep(31*1000);
ret = readScanState(buf);
memcpy(buf, "\x96\x23\x96\x23", 0x0000004);
ret = writeRegisters(buf);

memcpy(buf, 
	"\xa0\x00\xa1\xf4\xa2\x0f\xa3\xff" "\xa4\xff\xa5\x0f\x7c\x00\x7d\x10" 
	"\x7e\x00\x7f\x00" 
, 0x0000014);ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, REGISTERS, 0x0000000, buf, 0x0000014, 1000);
usleep(30*1000);
memcpy(buf, "\x00\x10\x00\x00", 0x0000004);
ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, DMAWRITE, 0x0000000, buf, 0x0000004, 1000);
usleep(16*1000);
memcpy(buf, table0x00001000 , 0x0001000);
ret = usb_bulk_write(devh, 0x00000001, buf, 0x0001000, 1245);

usleep(15*1000);

/*Clear FIFIO command*/
ret = clearFIFO(buf);

usleep(32*1000);

memcpy(buf, 
	"\xa0\x00\xa1\x00\xa2\x00\xa3\xff" "\xa4\xff\xa5\x0f\x7c\x00\x7d\x01" 
	"\x7e\x00\x79\x40" 
, 0x0000014);ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, REGISTERS, 0x0000000, buf, 0x0000014, 1000);

usleep(31*1000);

memcpy(buf, "\x00\x02\x00\x00", 0x0000004);
ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, DMAREAD, 0x0000000, buf, 0x0000004, 1000);
ret = usb_bulk_read(devh, 0x00000082, buf, 0x0000200, 1030);
/*That was URB 1390 */

ret = select_register_bank(buf, 2);
memcpy(buf, "\xa0\x01\xa0\x01", 0x0000004);
ret = writeRegisters(buf);
ret = select_register_bank(buf, 0);

printf("Repeated register 2ax stuff\n");
ret = select_register_bank(buf, 2);
for(x = 0; x < 3; x++){
	memcpy(buf, 
		"\xa1\x00\xa2\x19\xa1\x00\xa2\x1b" "\xa1\x00\xa2\x16\xa1\x00\xa2\x18" 
		"\xa1\x00\xa2\x19\xa1\x00\xa2\x1b" "\xa1\x00\xa2\x16\xa1\x00\xa2\x18" 
		"\xa1\x00\xa2\x19\xa1\x00\xa2\x1b" "\xa1\x00\xa2\x16\xa1\x00\xa2\x18" 
		"\xa1\x00\xa2\x19\xa1\x00\xa2\x1b" "\xa1\x00\xa2\x16\xa1\x00\xa2\x18" 
	, 0x0000040);
	ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, REGISTERS, 0x0000000, buf, 0x0000040, 1000);
}
ret = select_register_bank(buf, 0);

ret = select_register_bank(buf, 2);
/*That was URB 1400 */
memcpy(buf, "\xa0\x00\xa0\x00", 0x0000004);
ret = writeRegisters(buf);

ret = select_register_bank(buf, 0);
ret = select_register_bank(buf, 1);
memcpy(buf, 
	"\x60\x08\x61\x0b\x62\x0e\x63\x11" "\x64\x14\x65\x17\x66\x02\x67\x05" 
	"\x68\x08\x69\x50\x6a\x01\x6b\x0c" "\x6d\x48\x6e\x6a\x6f\x0e\x70\x50" 
	"\x71\x6a\x72\x0e\x73\xe8\x74\x6c" "\x75\x0e\x76\xf0\x77\x6c\x78\x0e" 
	"\x79\x48\x7a\x2b\x7b\x0e\x7c\x50" "\x7d\x40\x7e\x0e\x7f\xe8\x80\x2d" 
, 0x0000040);ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, REGISTERS, 0x0000000, buf, 0x0000040, 1000);

memcpy(buf, 
	"\x81\x0e\x82\xf0\x83\x42\x84\x0e" "\x85\x48\x86\xa9\x87\x0e\x88\x50" 
	"\x89\xbe\x8a\x0e\x8b\xe8\x8c\xab" "\x8d\x0e\x8e\xf0\x8f\xc0\x90\x0e" 
	"\x9a\x00\x9b\x15\x9d\x02\x9e\x00" "\x9f\x00\xa0\x0e\xa1\x00\xa2\x3f" 
	"\xa3\x0e\xa4\x00\xa5\x7e\xa6\x0e" "\xa7\xff\xa8\x3e\xa9\x0e\xaa\xff" 
, 0x0000040);ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, REGISTERS, 0x0000000, buf, 0x0000040, 1000);
memcpy(buf, 
	"\xab\x7d\xac\x0e\xad\xff\xae\xbc" "\xaf\x0e\xb0\x4d\xb1\x01\xb9\xcf" 
	"\xba\x14\xcd\x00\xce\x0f\xd4\x00" "\xd5\x8e\xd6\xe3\xd7\x38\xec\x00" 
	"\xed\x00\xee\xc0\xef\x00\xf0\x00" "\xf1\x00\xf2\x00\xf3\x30\xf4\x75" 
	"\xf5\x00\xf6\x11\xf7\x14\xf8\xaa" "\xf9\x00\xfa\x55\xfb\x00\xfc\x3f" 
, 0x0000040);ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, REGISTERS, 0x0000000, buf, 0x0000040, 1000);

memcpy(buf, "\xfd\x00\xfd\x00", 0x0000004);
ret = writeRegisters(buf);

ret = select_register_bank(buf, 0);


// 2 3 but not 1

/*DO NOT REPLACE!! (yet)*/
/*section 1 is slightly different from setRegisters2xx*/
printf("register 2xx type 2\n");
ret = select_register_bank(buf, 2);
memcpy(buf, 
	"\x60\x30\x61\x03\x62\x30\x63\x03" "\x64\x30\x65\x03\x66\x30\x67\x03" 
	"\x68\x30\x69\x03\x6a\x30\x6b\x03" "\x6c\x30\x6d\x03\x6e\x30\x6f\x03" 
	"\x70\x40\x71\x05\x72\x00\x73\x40" "\x74\x05\x75\x00\x76\x40\x77\x05" 
	"\x78\x00\x79\x40\x7a\x05\x7b\x00" "\x7c\x80\x7d\x0a\x7e\x00\x7f\x80" 
, 0x0000040);ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, REGISTERS, 0x0000000, buf, 0x0000040, 1000);
/*That was URB 1410 */
memcpy(buf, 
	"\x80\x0a\x81\x00\x82\x80\x83\x0a" "\x84\x00\x85\x80\x86\x0a\x87\x00" 
	"\x88\xc0\x89\x0f\x8a\x00\x8b\xc0" "\x8c\x0f\x8d\x00\x8e\xc0\x8f\x0f" 
	"\x90\x00\x91\xc0\x92\x0f\x93\x00" "\xb0\x00\xb1\x00\xb2\x00\xb3\x00" 
	"\xb4\x00\xb5\x00\xb6\x00\xb7\x00" "\xb8\x00\xb9\x00\xba\x00\xbb\x00" 
, 0x0000040);ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, REGISTERS, 0x0000000, buf, 0x0000040, 1000);
memcpy(buf, 
	"\xbc\x00\xbd\x00\xbe\x00\xbf\x00"  "\xc0\x00\xc1\x00\xc2\x00\xc3\x00" 
	"\xc4\x00\xc5\x00\xc6\x00\xc7\x00" "\xc8\x00\xc9\x00\xca\x00\xcb\x00" 
	"\xcc\x00\xcd\x00\xce\x00\xcf\x00" 
, 0x0000028);ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, REGISTERS, 0x0000000, buf, 0x0000028, 1000);

ret = select_register_bank(buf, 0);
ret = select_register_bank(buf, 1);

memcpy(buf, 
	"\xb3\x00\xb4\x00\xb5\x00\xb6\x00" "\xb7\x01\xb8\x00" 
, 0x000000c);ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, REGISTERS, 0x0000000, buf, 0x000000c, 1000);

ret = select_register_bank(buf, 0);
memcpy(buf, 
	"\x58\x00\x5e\xff\x82\x00\x83\x38" "\x84\x8e\x85\xe3\x8a\x01\x8e\x00" 
	"\x90\x64\x91\x00\x94\x30\x95\xa7" "\x96\x23\x9b\x3d\x9d\x3f\x9e\x00" 
	"\x9f\x80\xa0\x00\xa1\x00\xa2\x00" "\xa3\xff\xa4\xfe\xa5\x0d\xa6\x41" 
	"\xab\x00\xae\x0f\xaf\x16\xb0\x08" "\xb1\x07\xb2\x14\xb3\x01\xb4\x7b" 
, 0x0000040);
ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, REGISTERS, 0x0000000, buf, 0x0000040, 1000);
memcpy(buf, 
	"\xb5\x00\xb6\x70\xb7\x05\xc9\x00" "\xca\x00\xcb\x00\xcc\x00\xcd\x3c" 
	"\xce\x3c\xcf\x3c\xd0\x07\xd8\x05" "\xd9\x3c\xda\x54\xdb\x00\xdc\x01" 
	"\xde\x01\xdf\x17\xe0\x2c\xe1\x01" "\xe2\x00\xe3\x00\xe4\x00\xe5\xc8" 
	"\xe6\x00\xe7\x01\xe8\x00\xe9\x01" "\xea\x2c\xeb\x01\xec\x80\xed\x00" 
, 0x0000040);
ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, REGISTERS, 0x0000000, buf, 0x0000040, 1000);

/*
f3 SCAN_ENABLE SCAN_BACK_TRACKING_ENABLE
f5 scan_data_format?
*/
memcpy(buf, 
	"\xee\x46\xef\x80\xf0\xc0\xf1\x38" "\xf2\x00\xf3\x0c\xf5\x10\xf6\x00" 
	"\xf7\x00\xf8\x02\xf9\x19\xfa\x36" "\xfb\xf0\xfc\x00\xfd\x6c\xfe\x0b" 
	"\xff\x50" 
, 0x0000022);ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, REGISTERS, 0x0000000, buf, 0x0000022, 1000);
usleep(146*1000);
memcpy(buf, "\xf8\x02\xf4\x01", 0x0000004);
ret = writeRegisters(buf);
/*That was URB 1420 */

usleep(16*1000);
/*This repeats alot!*/
printf("Checking 4 and 5 over and over\n");
/*checking for lamp readiness?*/
do{
	ret = read_register(buf, 0x05);
	ret = read_register(buf, 0x04);
	usleep(15*1000);
} while(ret == 0x0);

printf("Beginning actual scan...\n");
printf("Setting up DMA Read?\n");
memcpy(buf, "\x7c\x00\x7d\x00\x7e\x04\x7f\x00", 0x0000008);
ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, REGISTERS, 0x0000000, buf, 0x0000008, 1000);

myfile = fopen(argv[1],"w");
if (myfile == NULL){
    printf("Error opening file! (%s) \n", argv[1]);
    exit(1);
}
//print bmp header - 24-bit 5128 pixels wide
memcpy(buf, bmp_file_header, 0x0000036);
print_file(buf, 0x36, myfile);

for(scanloop = 0; scanloop < 207; scanloop++)
{
//153617 bytes per loop? about 30 lines
	memcpy(buf, "\x00\x00\x08\x00", 0x0000004);
	ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, DMAREAD, 0x0000000, buf, 0x0000004, 1000);

	ret = usb_bulk_read(devh, 0x00000082, buf, 0x000fc00, 4870);
	printf("Writing %d bytes\n", ret);
	print_file(buf, ret, myfile);

	ret = usb_bulk_read(devh, 0x00000082, buf, 0x000fc00, 4870);
	printf("Writing %d bytes\n", ret);
	print_file(buf, ret, myfile);

	ret = usb_bulk_read(devh, 0x00000082, buf, 0x000fc00, 4870);
	printf("Writing %d bytes\n", ret);
	print_file(buf, ret, myfile);

	ret = usb_bulk_read(devh, 0x00000082, buf, 0x000fc00, 4870);
	printf("Writing %d bytes\n", ret);
	print_file(buf, ret, myfile);

	ret = usb_bulk_read(devh, 0x00000082, buf, 0x000fc00, 4870);
	printf("Writing %d bytes\n", ret);
	print_file(buf, ret, myfile);

	ret = usb_bulk_read(devh, 0x00000082, buf, 0x000fc00, 4870);
	printf("Writing %d bytes\n", ret);
	print_file(buf, ret, myfile);

	ret = usb_bulk_read(devh, 0x00000082, buf, 0x000fc00, 4870);
	printf("Writing %d bytes\n", ret);
	print_file(buf, ret, myfile);

	ret = usb_bulk_read(devh, 0x00000082, buf, 0x000fc00, 4870);
	printf("Writing %d bytes\n", ret);
	print_file(buf, ret, myfile);

	ret = usb_bulk_read(devh, 0x00000082, buf, 0x0002000, 1491); //5265 bytes?
	printf("Writing %d bytes\n", ret);
	print_file(buf, ret, myfile);
	//removed extra sleeps. Presumably, the USB functions will wait for responses from the scanner.
}
printf("Done scanning image. shutting down scanner.\n");

ret = setScannerIdle(buf);
usleep(31*1000);
ret = readScanState(buf);

memcpy(buf, command_six, 0x0000010);
ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, 0x000000a, 0x0000000, buf, 0x0000010, 1000);
ret = usb_bulk_read(devh, 0x00000082, buf, 0x0000200, 1030);

/*That was URB 3640 */
memcpy(buf, 
	"\x01\xa0\x00\x04\x00\x00\x00\x1e" "\x00\x00\x00\x00\x00\x00\x00\x00" 
, 0x0000010);
ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, 0x000000a, 0x0000000, buf, 0x0000010, 1000);
usleep(20*1000);

ret = do_command_four(buf);

memcpy(buf, 
	"\x01\xa0\x05\x01\x81\x00\x00\x00" "\x00\x00\x00\x00\x00\x00\x00\x00" 
, 0x0000010);
ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, 0x000000a, 0x0000000, buf, 0x0000010, 1000);
usleep(17*1000);

memcpy(buf, 
	"\x02\xa0\x36\x02\x00\x00\x00\x00" "\x00\x00\x00\x00\x00\x00\x00\x00" 
, 0x0000010);
ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, 0x000000a, 0x0000000, buf, 0x0000010, 1000);
ret = usb_bulk_read(devh, 0x00000082, buf, 0x0000200, 1030);


memcpy(buf, 
	"\x01\xa0\x46\x02\x00\x11\x00\x00" "\x00\x00\x00\x00\x00\x00\x00\x00" 
, 0x0000010);ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, 0x000000a, 0x0000000, buf, 0x0000010, 1000);
usleep(19*1000);

ret = closescanchip(buf);
/*------------------*/

ret = openscanchip(buf);

ret = adjustPowerSaveRegisters(buf);

usleep(6*1000);

ret = read_register(buf, 0x0b);
ret = read_register(buf, 0x0a);
ret = read_register(buf, 0x09);

usleep(499*1000);

ret = read_register(buf, 0x00);

ret = do_command_eight(buf);

memcpy(buf, 
	"\xf3\x22\xfd\xc4\xfe\x09\xa6\x41" "\xf6\x00\x96\x21\xe4\x01\xe2\x40" 
	"\xe3\x1f\xe4\x00\x95\xa7\xf4\x01" 
, 0x0000018);ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, REGISTERS, 0x0000000, buf, 0x0000018, 1000);

memcpy(buf, "\x00\x00\x86\x01", 0x0000004);
ret = writeRegisters(buf);
ret = do_command_five(buf);

usleep(2500*1000);

ret = do_command_one(buf);

ret = read_register(buf, 0x1c);
/*That was URB 3670 */
usleep(18*1000);

ret = readScanState(buf);

ret = read_register(buf, 0x03);

/*Timing adjustments?*/
memcpy(buf, "\x82\x00\x83\x00\x84\x00\x85\x00", 0x0000008);
ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, REGISTERS, 0x0000000, buf, 0x0000008, 1000);

printf("Reseting CCD config\n");
ret = select_register_bank(buf, 1);
usleep(27*1000);
memcpy(buf, 
	"\x60\xff\x61\xff\x62\xff\x63\xff" "\x64\xff\x65\xff\x66\xff\x67\xff" 
	"\x68\xff\xd4\xff\xd5\xff\xd6\xff" "\xd7\xff\xec\x00\xed\x00\xee\x00" 
	"\xef\x00\xf0\x00\xf1\x00\xf2\x00" "\xf3\x00" 
, 0x000002a);ret = usb_control_msg(devh, USB_TYPE_VENDOR + USB_RECIP_DEVICE, 0x0000004, REGISTERS, 0x0000000, buf, 0x000002a, 1000);

ret = select_register_bank(buf, 0);

ret = closescanchip(buf);
/*------------------*/

ret = openscanchip(buf);

ret = adjustPowerSaveRegisters(buf);
ret = do_command_five(buf);

printf("Complete\n");

ret = usb_release_interface(devh, 0);
assert(ret == 0);
ret = usb_close(devh);
assert(ret == 0);

printf("Closing file...\n");
fclose(myfile);

return 0;

}
