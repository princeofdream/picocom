/*
 * =====================================================================================
 *
 *       Filename:  misc_utils.cpp
 *
 *    Description:  conv encoding
 *
 *        Version:  1.0
 *        Created:  12/07/2019 02:22:40 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Li Jin (JamesL), lij1@xiaopeng.com
 *   Organization:  XPeng
 *
 * =====================================================================================
 */

#include <misc_utils.h>

misc_utils::misc_utils()
{
}

misc_utils::~misc_utils()
{
}

int
misc_utils::conv_gbk_to_utf8(char* src_str, char* dest_str)
{
	int ret;

	char *utf8_src = "小鹏WiFiAP-";
	char dest[1024];

	// memset(dest, 0x0, sizeof(dest));
	// conv_string("gbk", "utf-8", gbk_src, strlen(gbk_src), dest, sizeof(dest));
	// logd("conv result: %s\n", dest);

	memset(dest, 0x0, sizeof(dest));
	conv_string("utf-8", "gbk", utf8_src, strlen(utf8_src), dest, sizeof(dest));
	logd("conv result: %s\n", dest);

	return 0;
}

int
misc_utils::conv_string(const char* from_charset, const char* to_charset,
						   char* src_str, size_t src_len, char* dest_str, size_t dest_len)
{
	int ret;
	iconv_t cd;
	char *pin=src_str;
	char *pout=dest_str;

	cd = iconv_open(to_charset,from_charset);
	if (cd == 0) {
		perror("iconv open");
		loge("error in open iconv");
		return -1;
	}

	ret = iconv(cd, &pin, &src_len, &pout, &dest_len);
	if (ret < 0) {
		perror("conver error");
		loge("error to conv string.\n");
		return ret;
	}
	iconv_close(cd);

	return 0;
}

int
misc_utils::make_path(char* mkpath, int mode)
{
	char *path_str;
	char cur_path[2048];
	char buffer[2048];

	memset(cur_path, 0x0, sizeof(cur_path));
	memset(buffer, 0x0, sizeof(buffer));

	sprintf(buffer, "%s", mkpath );

	path_str = strtok(buffer, "/");
	if (path_str != NULL) {
		if (buffer[0] == '/')
			sprintf(cur_path, "/%s", path_str);
		else
			sprintf(cur_path, "%s", path_str);
		if (strcmp(path_str, ".") != 0 && strcmp(path_str, "..") != 0) {
			mkdir(cur_path, mode);
			logd("mkdir %s", cur_path);
			chmod(cur_path, mode);
		}
	}

	while(path_str != NULL) {
		path_str = strtok(NULL, "/");
		if (path_str != NULL) {
			sprintf(cur_path, "%s/%s", cur_path,path_str);
			if (strcmp(path_str, ".") != 0 && strcmp(path_str, "..") != 0)
				mkdir(cur_path, mode);
				logd("mkdir %s", cur_path);
				chmod(cur_path, mode);
		}
	}

	return 0;
}

int
misc_utils::generate_random_words(int count, char** str, int mode)
{
	char rndm_str[1024];
	struct timeval tv;
	unsigned long seed;
	int val;
	int i0;

	memset(rndm_str, 0x0, sizeof(rndm_str));
	i0 = 0;
	while (i0 < count) {
		gettimeofday(&tv, NULL);
		seed = (tv.tv_sec << 2) + tv.tv_usec;
		srandom(seed);
		val = random()%('z'+1);
		if ((mode & RANDOM_MODE_NUMBER) == RANDOM_MODE_NUMBER &&
			(mode & RANDOM_MODE_CAP_ALPHABET) == RANDOM_MODE_CAP_ALPHABET &&
			(mode & RANDOM_MODE_SMA_ALPHABET) == RANDOM_MODE_SMA_ALPHABET) {
			// number and alphabet
			if (val < '0' || val > 'z' || (val > '9' && val < 'A') || (val > 'Z' && val < 'a') )
				continue;
		} else if ((mode & RANDOM_MODE_NUMBER) == RANDOM_MODE_NUMBER &&
			(mode & RANDOM_MODE_CAP_ALPHABET) == RANDOM_MODE_CAP_ALPHABET &&
			(mode & RANDOM_MODE_SMA_ALPHABET) != RANDOM_MODE_SMA_ALPHABET) {
			// number and cap alphabet
			if (val < '0' || val > 'Z' || (val > '9' && val < 'A'))
				continue;
		} else if ((mode & RANDOM_MODE_NUMBER) == RANDOM_MODE_NUMBER &&
			(mode & RANDOM_MODE_CAP_ALPHABET) != RANDOM_MODE_CAP_ALPHABET &&
			(mode & RANDOM_MODE_SMA_ALPHABET) == RANDOM_MODE_SMA_ALPHABET) {
			// number and small alphabet
			if (val < '0' || val > 'z' || (val > '9' && val < 'a'))
				continue;
		} else if ((mode & RANDOM_MODE_NUMBER) != RANDOM_MODE_NUMBER &&
			(mode & RANDOM_MODE_CAP_ALPHABET) == RANDOM_MODE_CAP_ALPHABET &&
			(mode & RANDOM_MODE_SMA_ALPHABET) == RANDOM_MODE_SMA_ALPHABET) {
			// alphabet only
			if (val < 'A' || val > 'z' || (val > 'Z' && val < 'a'))
				continue;
		} else if ((mode  & RANDOM_MODE_NUMBER) == RANDOM_MODE_NUMBER  &&
			(mode  & RANDOM_MODE_CAP_ALPHABET) != RANDOM_MODE_CAP_ALPHABET  &&
			(mode  & RANDOM_MODE_SMA_ALPHABET) != RANDOM_MODE_SMA_ALPHABET) {
			// number only
			if (val < '0' || val > '9')
				continue;
		} else if ((mode  & RANDOM_MODE_NUMBER) != RANDOM_MODE_NUMBER  &&
			(mode  & RANDOM_MODE_CAP_ALPHABET) == RANDOM_MODE_CAP_ALPHABET  &&
			(mode  & RANDOM_MODE_SMA_ALPHABET) != RANDOM_MODE_SMA_ALPHABET) {
			// cap alphabet only
			if (val < 'A' || val > 'Z')
				continue;
		} else if ((mode & RANDOM_MODE_NUMBER) != RANDOM_MODE_NUMBER &&
			(mode & RANDOM_MODE_CAP_ALPHABET) != RANDOM_MODE_CAP_ALPHABET &&
			(mode & RANDOM_MODE_SMA_ALPHABET) == RANDOM_MODE_SMA_ALPHABET) {
			// small alphabet only
			if (val < 'a' || val > 'z')
				continue;
		} else {
			if (val < '0' || val > 'z' || (val > '9' && val < 'A') || (val > 'Z' && val < 'a') )
				continue;
		}
		sprintf(rndm_str, "%s%c", rndm_str, val);
		i0++;
	}
	logd("generated random string: %s", rndm_str);
	if (*str != NULL) {
		sprintf(*str, "%s", rndm_str);
	} else {
		*str = (char*)malloc(strlen(rndm_str) + 1);
		sprintf(*str, "%s", rndm_str);
	}

	return 0;
}

void
misc_utils::flag_to_string(uint32_t flag, const char* value)
{
	char flag_str[MAXLEN];

	memset(flag_str, 0x0, sizeof(flag_str));
	if (flag == FLAG_DEFAULT)
		sprintf(flag_str, "FLAG_DEFAULT");

	if ((flag & FLAG_SYNC_MUTEX) == FLAG_SYNC_MUTEX)
		sprintf(flag_str, "%s FLAG_SYNC_MUTEX |", flag_str);
	if ((flag & FLAG_BLOCK) == FLAG_BLOCK)
		sprintf(flag_str, "%s FLAG_BLOCK | ", flag_str);
	if ((flag & FLAG_WITH_PROCESS) == FLAG_WITH_PROCESS)
		sprintf(flag_str, "%s FLAG_WITH_PROCESS |", flag_str);
	if ((flag & FLAG_WITH_PTHREAD) == FLAG_WITH_PTHREAD)
		sprintf(flag_str, "%s FLAG_WITH_PTHREAD |", flag_str);
	if ((flag & FLAG_WITH_IP) == FLAG_WITH_IP)
		sprintf(flag_str, "%s FLAG_WITH_IP |", flag_str);
	if ((flag & FLAG_WITH_REPLY) == FLAG_WITH_REPLY)
		sprintf(flag_str, "%s FLAG_WITH_REPLY |", flag_str);
	if ((flag & FLAG_MSG) == FLAG_MSG)
		sprintf(flag_str, "%s FLAG_MSG |", flag_str);
	if ((flag & FLAG_MSG_ASYNC) == FLAG_MSG_ASYNC)
		sprintf(flag_str, "%s FLAG_MSG_ASYNC |", flag_str);

	if (value != NULL)
		logd("%s flags: < %s >", value, flag_str);
	else
		logd("flags: < %s >", flag_str);
}


