/*
 * =====================================================================================
 *
 *       Filename:  misc_utils.h
 *
 *    Description:  misc_utils
 *
 *        Version:  1.0
 *        Created:  12/07/2019 02:22:57 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Li Jin (JamesL), princeofdream@outlook.com
 *   Organization:  XPeng
 *
 * =====================================================================================
 */

#ifndef __MISC_UTILS_HEADER__
#define __MISC_UTILS_HEADER__

#include "procmgr_basic.h"

#ifndef ANDROID
#include <iconv.h>
#endif

/*! \enum random_word_mode
 */
enum random_word_mode {
	RANDOM_MODE_DEFAULT = 0,
	RANDOM_MODE_NUMBER = 0x1,
	RANDOM_MODE_CAP_ALPHABET = 0x2,
	RANDOM_MODE_SMA_ALPHABET = 0x4,
};

enum process_flags_t {
	FLAG_DEFAULT      = 0,
	FLAG_SYNC_MUTEX   = 0x1 << 0,
	FLAG_BLOCK        = 0x1 << 1,
	FLAG_WITH_PROCESS = 0x1 << 2,
	FLAG_WITH_PTHREAD = 0x1 << 3,
	FLAG_WITH_IP      = 0x1 << 4,
	FLAG_WITH_REPLY   = 0x1 << 5,
	FLAG_MSG          = 0x1 << 6,
	FLAG_MSG_ASYNC    = 0x1 << 7
};

class misc_utils
{
public:
	misc_utils ();
	virtual ~misc_utils ();

	int conv_gbk_to_utf8(char*, char*);
	int conv_string(const char*, const char*,
					char*, size_t, char*, size_t);
	int make_path(char*, int);
	int generate_random_words(int, char**, int);

	void flag_to_string(uint32_t, const char*);
private:
	/* data */
};

#endif /* ifndef __MISC_UTILS_HEADER__ */

