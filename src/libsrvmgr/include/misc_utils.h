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
 *         Author:  Li Jin (JamesL), lij1@xiaopeng.com
 *   Organization:  XPeng
 *
 * =====================================================================================
 */

#ifndef __MISC_UTILS_HEADER__
#define __MISC_UTILS_HEADER__
#include <procmgr_basic.h>
#include <iconv.h>

/*! \enum random_word_mode
 */
enum random_word_mode {
	RANDOM_MODE_DEFAULT = 0,
	RANDOM_MODE_NUMBER = 0x1,
	RANDOM_MODE_CAP_ALPHABET = 0x2,
	RANDOM_MODE_SMA_ALPHABET = 0x4,
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

