/**
 * @file EscSeqParser.h
 * @brief 
 * @author osoumen
 * @date 2019/10/09
 * @copyright 
 */

#ifndef EscSeqParser_h
#define EscSeqParser_h

#if defined(ARDUINO)
#define edispSDL tftDispSPI
#endif

#include <string>
#include <stdint.h>

#ifndef _WIN32
#include <iconv.h>
#endif

class edispSDL;

class EscSeqParser {
public:
	EscSeqParser(edispSDL *disp);
	virtual ~EscSeqParser();
	
	void ParseByte(uint8_t inbyte);
	uint32_t GetFwVersion() const { return mFwVersion; }

private:
	EscSeqParser();

	edispSDL	*mDisp;
	int 		mEscapeState;
	char		mTwobyte_1st_char;
	std::string	mEscapeSequence;
#if defined(__APPLE__)
	iconv_t 	mIconv;
#endif
	uint32_t	mFwVersion;

	void ParseEscapeSequence(const std::string &seq);
	int ExtractParamString(const std::string &str, int *out_array, int array_size);

};

#endif /* EscSeqParser_h */
