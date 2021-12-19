#include "libslavery.h"

#include <fcntl.h>
#include <linux/hidraw.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <unistd.h>

int main() {
	slavery_receiver_list_t *receiver_list = slavery_scan_receivers();

	slavery_receiver_list_print(receiver_list);

	for (slavery_receiver_list_t *receiver_entry = receiver_list; receiver_entry;
	     receiver_entry = receiver_entry->next) {
		slavery_device_list_t *device_list = slavery_receiver_get_devices(receiver_entry->receiver);

		for (slavery_device_list_t *device_entry = device_list; device_entry;
		     device_entry = device_entry->next) {
			// slavery_device_set_config(device_entry->device, config);
		}
	}

	slavery_receiver_list_free(receiver_list);

	return 0;

	/*struct hidraw_report_descriptor desc = {0};
	// struct hidraw_devinfo info = {0};
	int fd = open("/dev/hidraw10", O_RDWR);
	char buf[256];

	if (fd < 0) {
	    perror("open()");

	    return -1;
	}*/

	/*int ret = ioctl(fd, HIDIOCGRDESCSIZE, &desc.size);

	if (ret < 0) {
	    perror("ioctl(HIDIOCGRDESCSIZE)");

	    return -1;
	}

	ret = ioctl(fd, HIDIOCGRDESC, &desc);

	if (ret < 0) {
	    perror("ioctl(HIDIOCGRDESC)");

	    return -1;
	}

	printf("Report Descriptor:\n");
	for (int i = 0; i < desc.size; i++)
	    printf("%hhx ", desc.value[i]);
	puts("\n");

	ret = ioctl(fd, HIDIOCGRAWNAME(256), buf);

	if (ret < 0) {
	    perror("ioctl(HIDIOCGRAWNAME)");

	    return -1;
	}*/

	/*printf("Name: %s\n", buf);

	ssize_t n = 0;

	// ping
	// n = write(fd, "\x10\xFF\x00\x10\x02\x00\xEE", 7);

	n = write(fd, "\x10\xFF\x83\xB5\x40\x00\x00", 7);

	printf("wrote %ld\n", n);

	n = read(fd, buf, 256);

	printf("read %ld\n", n);

	for (int i = 0; i < n; i++) {
	    printf("%hhx ", buf[i]);
	}

	puts("\n");

	for (int i = 0; i < n; i++) {
	    printf("%c ", buf[i]);
	}

	puts("\n");

	perror("read error:");

	return 0;*/
}
