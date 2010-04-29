/* lsusb-freebsd.c: a simple lsusb utility for FreeBSD, possibly others */
/*
Copyright (c) 2009, Kirn Gill <segin2005@gmail.com> 
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY KIRN GILL ''AS IS'' AND ANY
EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL KIRN GILL BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <dev/usb/usb.h>

void usage(void)
{
	printf(	"usage: lspci [-sbh]\n\n"
		"\t-s\tAlternate \"short form\" output\n"
		"\t-b\tPrint device bandwidth\n"
		"\t-h\tPrint this help.\n"
	);
}

void print_usb_device(stuct usb_device_info *dev, int bus, int addr, int flag) 
{
	switch(flag) { 
		case 0:
			printf("Bus %03d Device %03d: "
					       		"ID %04x:%04x %s %s\n",
					       		bus, addr,
							dev->udi_vendorNo,
					       		dev->udi_productNo, 
					       		dev->udi_vendor,
					       		dev->udi_product);
						break;
					case 's':	
						printf("%03d:%03d %04x:%04x "
							"%s %s\n",
							bus, addr,
							dev->udi_vendorNo,
							dev->udi_productNo,
							dev->udi_vendor,
							dev->udi_product);
						break;
					case 'b':	
						printf("Bus %03d Device %03d: ",
							bus, addr);
						switch (dev->udi_speed) {
						case USB_SPEED_LOW:
							printf("1.5MB/s\n");
							break;
						case USB_SPEED_FULL:
							printf("12 MB/s\n");
							break;
						case USB_SPEED_HIGH:
							printf("480MB/s\n");
							break;
						}
						break;	
				}
}

int main(int argc, char *argv[])
{
	struct usb_device_info *dev = malloc(sizeof(struct usb_device_info));
	int fd, bus, addr, ret;
	int flag, ch;

	ch = getopt(argc, argv, "sbh");
	switch (ch) {
		case 's':
		case 'b':
			flag = ch;
			break;
		case -1:
			flag = 0;
			break;
		default:
			usage();
			exit(1);
			break;
	}


	char *dev_fname;
	
	for (bus = 0; bus < 8; bus++) { 
		asprintf(&dev_fname, "/dev/usb%d", bus);
		fd = open(dev_fname, O_RDWR | O_NONBLOCK);
		if (fd != -1) {
			for (addr = 1; addr < 128; addr++) {
				dev->udi_addr = addr;
				ret = ioctl(fd, USB_DEVICEINFO, dev);
				if (ret != -1) print_usb_device(dev, bus, 
								addr, flag);
			}
		}
		free(dev_fname); 
	}
	free(dev);
}
