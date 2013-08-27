/*
 * Author  : lys@allwinnertech.com
 * Verison : 0.1
 */

#include <ctype.h>
#include <dirent.h>
#include <grp.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

static void usage(char *cmd);
int delay;
int iterations;
int fd_dump;
int fd_write;
int ret;

#define DRAM_BW_COMMAND	  0x01c6208c
#define DRAM_BW_COUNTER0  0x01c62090
#define DRAM_BW_LCR       0x01c6209c

typedef struct NodeInfo {
	int id;
	char *name;
} NodeInfo;

NodeInfo nodeInfo[] = {
	{ 32, "ALL " },
	{ 0, "CPUX" },
	{ 1, "GPU0" },
//	{ 2, "GPU1"},
	{ 4, "CPUS" },
	{ 5, "ATH " },
	{ 6, "GMAC" },
	{ 7, "SDC0" },
	{ 8, "SDC1" },
	{ 9, "SDC2" },
	{ 10,"SDC3" },
	{ 11,"USB " },
	{ 15,"NFC1" },
	{ 16, "DMAC" },
	{ 17, "VE  " },
	{ 18, "MP  " },
	{ 19, "NFC0" },
	{ 20, "DRC0" },
	{ 21, "DRC1" },
	{ 22, "DEU0" },
	{ 23, "DEU1" },
	{ 24, "BE0 " },
	{ 25, "FE0 " },
	{ 26, "BE1 " },
	{ 27, "FE1 " },
	{ 28, "CSI0" },
	{ 29, "CSI1" },
	{ 30, "TS  " },
		};

NodeInfo dramRegInfo[] = {
	{ 0x01c62094, "BWC1" },
	{ 0x01c62098, "BWC2" },
	//{ 0x01c6209c, "LCR " },
	{ 0x01c620a0, "FCR0" },
	{ 0x01c620a4, "FCR1" },
	{ 0x01c620a8, "FCR2" },
	{ 0x01c620ac, "FCR3" },
	{ 0x01c620b0, "DCCR" },
};

static int test_times = 0;
static int kByte = 0;
static int history_value[64] = { 0 };
static int history_dccr;
static int kUnit = 1024;
static int old_version = 0;
static int more_detail = 0;

void mtop_write_reg(int reg_addr, int value) {
	char cmd_buffer[128];
	sprintf(cmd_buffer, "0x%x 0x%x\n", reg_addr, value);
	ret = write(fd_write, cmd_buffer, strlen(cmd_buffer));
	lseek(fd_write, 0, SEEK_SET);
	read(fd_write, cmd_buffer, sizeof(cmd_buffer));
	lseek(fd_write, 0, SEEK_SET);
}

int mtop_read_reg(int reg_addr) {
	char cmd_buffer[128];
	char out_buf[128] = { 0 };
	char *str_tmp;
	unsigned long value = 0;

	sprintf(cmd_buffer, "0x%x\n", reg_addr);
	ret = write(fd_dump, cmd_buffer, strlen(cmd_buffer));
	lseek(fd_dump, 0, SEEK_SET);
	ret = read(fd_dump, out_buf, sizeof(out_buf));
	lseek(fd_dump, 0, SEEK_SET);
	//printf("out_buf=%s   ", out_buf);

	if (old_version) { //old version
		str_tmp = strstr(out_buf, ":");
		str_tmp = str_tmp + 1;
		//printf("str_tmp=%s   ", str_tmp);
		value = strtoul(str_tmp, NULL, 16);
		//printf("%s value:%x", out_buf,value);
	} else {
		value = strtoul(out_buf, NULL, 16);
	}

	return value;
}

int mtop_process() {
	int ret = -1;
	int i, curr_value, diff;

	printf("===================== statics begin ===================== time:%d\n", test_times);

	if (kByte)
		printf("== module   (KB) ==\n");
	else
		printf("== module   (MB) ==\n", test_times);

	for (i = 0; i < (sizeof(nodeInfo) / sizeof(NodeInfo)); i++) {
		printf(" %2d: %s  ", nodeInfo[i].id, nodeInfo[i].name);
		mtop_write_reg(DRAM_BW_COMMAND, nodeInfo[i].id << 1 | 1);
		curr_value = mtop_read_reg(DRAM_BW_COUNTER0);
		diff = curr_value - history_value[i];

		if (test_times > 0) {
			if (!strcmp(nodeInfo[i].name, "GPU0"))
				printf(" %d X 2\n", diff / kUnit);
			else
				printf(" %d \n", diff / kUnit);
		} else {
			printf("\n");
		}

		history_value[i] = curr_value;
	}

	if (more_detail) {
		int value;

		printf("\n----------- more detail ---------\n");
		for (i = 0; i < (sizeof(dramRegInfo) / sizeof(NodeInfo)); i++) {
			printf(" 0x%x: %s  ", dramRegInfo[i].id, dramRegInfo[i].name);
			value = curr_value = mtop_read_reg(dramRegInfo[i].id);

			if (!strcmp(dramRegInfo[i].name, "DCCR")) {
				value = curr_value - history_dccr;
				history_dccr = curr_value;
			}

			if (test_times > 0) {
				printf(" 0x%x \n", value);
			} else {
				printf("\n");
			}
		}

		printf("\n----------- more detail LCR---------\n");

		curr_value = mtop_read_reg(DRAM_BW_COMMAND);
		curr_value &= ~(1<<8);
		mtop_write_reg(DRAM_BW_COMMAND, curr_value);


		for(i = 0; i < 12; i++) {
			curr_value &= ~(0xf<<9);
			curr_value |= i<<9;
			mtop_write_reg(DRAM_BW_COMMAND, curr_value);
			value = mtop_read_reg(DRAM_BW_LCR);
			printf(" LCR with sel:%2d value: 0x%x  \n", i, value);

			if(i==4) {
				i=7;
			}
		}

		curr_value |= 1<<8;
		mtop_write_reg(DRAM_BW_COMMAND, curr_value);
	}

	printf("\n\n");

	test_times++;

	return 0;
}

int main(int argc, char *argv[]) {
	int i;

	delay = 1;
	iterations = -1;

	for (i = 1; i < argc; i++) {
		if (!strcmp(argv[i], "-n")) {
			if (i + 1 >= argc) {
				fprintf(stderr, "Option -n expects an argument.\n");
				usage(argv[0]);
				exit(EXIT_FAILURE);
			}
			iterations = atoi(argv[++i]);
			continue;
		}

		if (!strcmp(argv[i], "-d")) {
			if (i + 1 >= argc) {
				fprintf(stderr, "Option -d expects an argument.\n");
				usage(argv[0]);
				exit(EXIT_FAILURE);
			}
			delay = atoi(argv[++i]);
			continue;
		}

		if (!strcmp(argv[i], "-k")) {
			kByte = 1;
			kUnit = 1;
			continue;
		}

		if (!strcmp(argv[i], "-o")) {
			old_version = 1;
			continue;
		}

		if (!strcmp(argv[i], "-m")) {
			more_detail = 1;
			continue;
		}

		if (!strcmp(argv[i], "-h")) {
			usage(argv[0]);
			exit(0);
		}
		fprintf(stderr, "Invalid argument \"%s\".\n", argv[i]);
		usage(argv[0]);
		exit(-1);
	}

	fd_dump = open("/sys/class/sunxi_dump/dump", O_RDWR);
	if (fd_dump <= 0) {
		printf("%s(%d): open failed, err!\n", __func__, __LINE__);
		exit(0);
	}

	fd_write = open("/sys/class/sunxi_dump/write", O_RDWR);
	if (fd_write <= 0) {
		printf("%s(%d): open failed, err!\n", __func__, __LINE__);
		exit(0);
	}

	//mtop_write_reg(DRAM_BW_COMMAND, 0); //stop
	mtop_write_reg(DRAM_BW_COMMAND, 32 << 1 | 1); //start
	while ((iterations == -1) || (iterations-- > 0)) {
		//mtop_write_reg(DRAM_BW_COMMAND, 32<<1 | 1); //start
		sleep(delay);
		//mtop_write_reg(DRAM_BW_COMMAND, 32<<1 | 0); //stop
		mtop_process();
	}
	mtop_write_reg(DRAM_BW_COMMAND, 0); //stop

	if (fd_dump)
		close(fd_dump);

	if (fd_write)
		close(fd_write);

	return 0;
}

static void usage(char *cmd) {
	fprintf(stderr,
			"Usage: %s [ -n iterations ] [ -d delay ] [ -m ] [ -h ]\n"
					"    -n num  Updates to show before exiting.\n"
					"    -d num  Seconds to wait between updates.\n"
					"    -k KB.\n"
					"    -m more detail of DRAM parameter.\n"
					"    -h Display this help screen.\n", cmd);
}
