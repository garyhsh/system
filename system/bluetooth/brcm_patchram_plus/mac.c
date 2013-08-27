/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2000-2001  Qualcomm Incorporated
 *  Copyright (C) 2002-2003  Maxim Krasnyansky <maxk@qualcomm.com>
 *  Copyright (C) 2002-2010  Marcel Holtmann <marcel@holtmann.org>
 *
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef ANDROID
#include <cutils/properties.h>
#define LOG_TAG "mac"
#include <cutils/log.h>
#undef printf
#define printf ALOGD
#endif //ANDROID

#define BT_MAC_PREFIX	"41:57:73"	//ASCII for AWs
static const char bt_mac_file_path[] = "/data/misc/bluetooth/bt.mac";
static int brcm_read_mac_from_file(char* mac)
{
	FILE* fp = NULL;
	char *tmp = NULL;
	char buf[64] = {0};
	int ret = 0;

	printf("===== mac brcm_read_mac_from_file start open %s =====\n", bt_mac_file_path);
	fp = fopen(bt_mac_file_path, "r");
	if (fp == NULL) {
	    printf( "open 'bt.mac' failed\n");
	    ret = -1;
	    goto out;
	}
	printf("===== mac brcm_read_mac_from_file get from %s =====\n", bt_mac_file_path);
	if (fgets(buf, 64, fp)) {
		tmp = buf;
		printf("===== mac brcm_read_mac_from_file read %s from bt.mac, cp to tmp %s =====\n", buf, tmp);
		while((*tmp != 0) && (*tmp) == ' ')
		    tmp ++;

		printf("===== mac brcm_read_mac_from_file parse btaddr =====\n");
		if (strncmp(tmp, "btaddr", 5) == 0) {
		    tmp += 5;
			while((*tmp != 0) && (*tmp != '='))
				tmp++;
		    if (*tmp == 0) {
				printf("===== mac brcm_read_mac_from_file parse btaddr fail =====\n");
		        ret = -1;
		        goto out;
		    } else {
		        tmp++;
		    }
		    while((*tmp != 0) && (*tmp == ' '))
				tmp++;
			if ((*tmp != 0) && (*tmp != ' ')) {
				printf("===== mac brcm_read_mac_from_file copy mac =====\n");
				strncpy(mac, tmp, 17);
			}
		}
	}
	
	printf("brcm_read_mac_from_file, mac %s, ret %d\n", mac, ret);
out:
    if (fp != NULL) {
        fclose(fp);
    	fp = NULL;
    }
    
	return ret;
}

int brcm_random_btmac(char* bdaddr_str)
{
    unsigned char addr[3] = {0};

    srand((unsigned)time(0));
    addr[0] = 0x100*rand()%(0xff);
    addr[1] = 0x100*rand()%(0xff);
    addr[2] = 0x100*rand()%(0xff);

    sprintf(bdaddr_str, "%s:%02x:%02x:%02x", BT_MAC_PREFIX, 0xff&addr[2], 0xff&addr[1], 0xff&addr[0]);
    printf("mac %s\n", bdaddr_str);
    return 0;
}

int brcm_write_mac_file(char* bdaddr_str)
{
    FILE* fp = NULL;
    int ret = 0;
    
	fp = fopen(bt_mac_file_path, "w+");
	if (fp == NULL) {
	    printf( "open 'bt.mac' failed for write\n");
	    ret = -1;
	    goto out;
	}
    
    fprintf(fp, "btaddr=%s", bdaddr_str);

out:
    if (fp != NULL) {
        fclose(fp);
    	fp = NULL;
    }
	return ret;
}

int brcm_get_btaddrstr(char* bdaddr_str)
{
	int ret;
	char addr[18] = {0};
	
	/* read bt mac address from file "/data/misc/bluetooth/bt.mac" */
	printf("===== mac brcm_get_btaddrstr start =====\n");
	ret = brcm_read_mac_from_file(addr);

	printf("===== mac brcm_read_mac_from_file return %d, addr: %s =====\n", ret, addr);
	
	if (!ret) {
	    printf( "found mac file 'bt.mac', mac %s\n", addr);
	    strncpy(bdaddr_str, addr, strlen(addr));
	    goto out;
	}

	printf("===== mac brcm_random_btmac start =====\n");
	ret = brcm_random_btmac(addr);
	if (!ret)  {
	    printf( "generate mac file 'bt.mac'\n" );
	    strncpy(bdaddr_str, addr, strlen(addr));
	}

	printf("===== mac brcm_write_mac_file start =====\n");
	brcm_write_mac_file(bdaddr_str);
	
out:
	return ret;
}
