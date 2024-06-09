/**
 * @file EscSeqParser.cpp
 * @brief 
 * @author osoumen
 * @date 2019/10/09
 * @copyright 
 */

#include "EscSeqParser.h"

#if defined(__APPLE__) || defined(_WIN32)
#include "edispSDL.h"
#elif defined(ARDUINO)
#include "tftDispSPI.h"
#endif

#if defined(_WIN32)
#include <Windows.h>
#endif

EscSeqParser::EscSeqParser(edispSDL *disp)
: mDisp(disp)
, mEscapeState(0)
, mTwobyte_1st_char(0)
, mFwVersion(0)
{
#if defined(__APPLE__)
	mIconv = iconv_open("UTF-8", "sjis");
#endif
}

EscSeqParser::~EscSeqParser()
{
#if defined(__APPLE__)
	iconv_close(mIconv);
#endif
}

void EscSeqParser::ParseByte(uint8_t inbyte)
{
	if ((mEscapeState == 0) || (inbyte < 0x20) || (inbyte >= 0x80)) {
		mEscapeState = 0;
		if ((mTwobyte_1st_char != 0) && (inbyte >= 0x40) && (inbyte < 0xfd) && (inbyte != 0x7f)) {
#if defined(ARDUINO)
			char outbuf[3];
			outbuf[0] = mTwobyte_1st_char;
			outbuf[1] = inbyte;
			outbuf[2] = 0;
			mDisp->puts_(outbuf);
			mTwobyte_1st_char = 0;
#else
			// SJIS文字をUTF8に変換
			char inbuf[3];
			char outbuf[8];
			char	*pSrc = inbuf;
			char	*pDst = outbuf;
			size_t inbytes = 3;
			size_t outbytes = 8;
			inbuf[0] = mTwobyte_1st_char;
			inbuf[1] = inbyte;
			inbuf[2] = 0;
#if defined(_WIN32)
			wchar_t bufUnicode[16];
			int lenUnicode = MultiByteToWideChar(CP_ACP, 0, inbuf, strlen(inbuf) + 1, bufUnicode, 16);
			WideCharToMultiByte(CP_UTF8, 0, bufUnicode, lenUnicode, outbuf, 8, NULL, NULL);
#elif defined(__APPLE__)
			iconv(mIconv, &pSrc, &inbytes, &pDst, &outbytes);
#endif
			mDisp->puts_(outbuf);
			mTwobyte_1st_char = 0;
#endif
		}
		else {
			mTwobyte_1st_char = 0;
			switch (inbyte) {
				case 0x08:	// TODO: BS
					break;
				case 0x09:	// TODO: TAB
					break;
				case 0x0a:	// LF
					mDisp->curdown();
					break;
				case 0x0c:	// TODO: FF
					break;
				case 0x0d:	// CR
					mDisp->cur_rowtop();
					break;
				case 0x1b:	// ESC
					mEscapeState = 1;
					mEscapeSequence.erase();
					break;
				case 0x7f:	// TODO: DEL
					break;
				default:
					if ((inbyte >= 0x20) && (inbyte < 0x7f)) {
						char str[2] = {0,0};
						str[0] = inbyte;
						mDisp->puts_(str);
					}
					else if (((inbyte >= 0x80) && (inbyte <= 0xa0)) || (inbyte >= 0xe0)) {
						mTwobyte_1st_char = inbyte;
					}
					else if ((inbyte >= 0xa1) && (inbyte < 0xe0)) {
#if defined(ARDUINO)
						char str[2] = { 0,0 };
						str[0] = inbyte;
						mDisp->puts_(str);
						mTwobyte_1st_char = 0;
#else
						// 半角カナ文字をUTF8に変換
						char inbuf[2];
						char outbuf[8];
						char	*pSrc = inbuf;
						char	*pDst = outbuf;
						size_t inbytes = 2;
						size_t outbytes = 8;
						inbuf[0] = inbyte;
						inbuf[1] = 0;
#if defined(_WIN32)
						wchar_t bufUnicode[16];
						int lenUnicode = MultiByteToWideChar(CP_ACP, 0, inbuf, strlen(inbuf) + 1, bufUnicode, 16);
						WideCharToMultiByte(CP_UTF8, 0, bufUnicode, lenUnicode, outbuf, 8, NULL, NULL);
#elif defined(__APPLE__)
						iconv(mIconv, &pSrc, &inbytes, &pDst, &outbytes);
#endif
						mDisp->puts_(outbuf);
						mTwobyte_1st_char = 0;
#endif
					}
			}
		}
	}
	else {
		if (mEscapeState == 1) {
			switch (inbyte) {
				case '[':
				case '$':
				case '(':
				case '@':
					mEscapeSequence.append(1, (char)inbyte);
					mEscapeState = 2;
					break;
				default:
					mEscapeSequence.append(1, (char)inbyte);
					// 受信したエスケープシーケンスを処理する
					ParseEscapeSequence(mEscapeSequence);
					mEscapeState = 0;
			}
		}
		else if (mEscapeState == 2) {
			if (((inbyte >= '0') && (inbyte <= '9')) || (inbyte == ';') || (inbyte == '>')) {
				// 数値、セミコロン、'>'は継続
				mEscapeSequence.append(1, (char)inbyte);
			}
			else if (((inbyte >= 'A') && (inbyte <= 'Z')) || ((inbyte >= 'a') && (inbyte <= 'z')) || (inbyte == '*') || (inbyte == '@')) {
				// アルファベット、'*'、'@'は終端
				mEscapeSequence.append(1, (char)inbyte);
				// 受信したエスケープシーケンスを処理する
				ParseEscapeSequence(mEscapeSequence);
				mEscapeState = 0;
			}
			else {
				// それ以外はエラー
				mEscapeState = 0;
			}
		}
	}
}

void EscSeqParser::ParseEscapeSequence(const std::string &seq)
{
	char first_byte = seq.front();
	char last_byte = seq.back();
	size_t param_length = seq.length() - 2;
	
	switch (first_byte) {
		case '[':
			switch (last_byte) {
				case 'H':
				case 'f':
					if (param_length > 0) {
						std::string param_str = seq.substr(1, param_length);
						int param[2];
						if (ExtractParamString(param_str, param, 2) == 2) {
							mDisp->move(param[0], param[1]);
						}
					}
					else {
						mDisp->move(0, 0);
					}
					break;
				case 'A':
					if (param_length > 0) {
						std::string param_str = seq.substr(1, param_length);
						int param;
						if (ExtractParamString(param_str, &param, 1) == 1) {
							mDisp->curup(param);
						}
					}
					else {
						mDisp->curup();
					}
					break;
				case 'B':
					if (param_length > 0) {
						std::string param_str = seq.substr(1, param_length);
						int param;
						if (ExtractParamString(param_str, &param, 1) == 1) {
							mDisp->curdown(param);
						}
					}
					else {
						mDisp->curdown();
					}
					break;
				case 'C':
					if (param_length > 0) {
						std::string param_str = seq.substr(1, param_length);
						int param;
						if (ExtractParamString(param_str, &param, 1) == 1) {
							mDisp->curright(param);
						}
					}
					else {
						mDisp->curright();
					}
					break;
				case 'D':
					if (param_length > 0) {
						std::string param_str = seq.substr(1, param_length);
						int param;
						if (ExtractParamString(param_str, &param, 1) == 1) {
							mDisp->curleft(param);
						}
					}
					else {
						mDisp->curleft();
					}
					break;
				case 'J':
					if (param_length > 0) {
						std::string param_str = seq.substr(1, param_length);
						int param;
						if (ExtractParamString(param_str, &param, 1) == 1) {
							if (param == 0) {
								//								mDisp->del_to_screen_end();
							}
							else if (param == 1) {
								//								mDisp->del_from_screen_top();
							}
							else if (param == 2) {
								mDisp->clear();
							}
						}
					}
					else {
						//						mDisp->del_to_screen_end();
					}
					break;
				case '*':
					if (param_length == 0) {
						mDisp->clear();
					}
					break;
				case 'K':
					if (param_length > 0) {
						std::string param_str = seq.substr(1, param_length);
						int param;
						if (ExtractParamString(param_str, &param, 1) == 1) {
							if (param == 0) {
								mDisp->del_to_end();
							}
							else if (param == 1) {
								//								mDisp->del_from_top();
							}
							else if (param == 2) {
								mDisp->del_row();
							}
						}
					}
					else {
						mDisp->del_to_end();
					}
					break;
				case 'm':
					if (param_length > 0) {
						std::string param_str = seq.substr(1, param_length);
						int param[6];
						int param_num = ExtractParamString(param_str, param, 6);
						switch (param_num) {
							case 1:
								mDisp->set_attribute(param[0]);
								break;
							case 2:
								mDisp->set_attribute(param[0], param[1]);
								break;
							case 3:
								mDisp->set_attribute(param[0], param[1], param[2]);
								break;
							case 4:
								mDisp->set_attribute(param[0], param[1], param[2], param[3]);
								break;
							case 5:
								//								mDisp->set_attribute(param[0], param[1], param[2], param[3], param[4]);
								break;
							case 6:
								//								mDisp->set_attribute(param[0], param[1], param[2], param[3], param[4], param[5]);
								break;
							default:
								break;
						}
					}
					else {
						mDisp->set_attribute(0);
					}
					break;
				case 'P':
					if (param_length > 0) {
						std::string param_str = seq.substr(1, param_length);
						int param;
						if (ExtractParamString(param_str, &param, 1) == 1) {
							//							mDisp->del_compact(param);
						}
					}
					else {
						//						mDisp->del_compact(1);
					}
					break;
				case 'X':
					if (param_length > 0) {
						std::string param_str = seq.substr(1, param_length);
						int param;
						if (ExtractParamString(param_str, &param, 1) == 1) {
							mDisp->del(param);
						}
					}
					else {
						mDisp->del(1);
					}
					break;
				case 'M':
					if (param_length > 0) {
						std::string param_str = seq.substr(1, param_length);
						int param;
						if (ExtractParamString(param_str, &param, 1) == 1) {
							//							mDisp->del_row_compact(param);
						}
					}
					else {
						//						mDisp->del_row_compact(1);
					}
					break;
				case 'L':
					if (param_length > 0) {
						std::string param_str = seq.substr(1, param_length);
						int param;
						if (ExtractParamString(param_str, &param, 1) == 1) {
							//							mDisp->insert_row(param);
						}
					}
					else {
						//						mDisp->insert_row(1);
					}
					break;
				case 's':
					if (param_length == 0) {
						mDisp->save_attribute(0);
					}
					break;
				case 'u':
					if (param_length == 0) {
						mDisp->load_attribute(0);
					}
					break;
				case 'l':
					if ((seq.at(1) == '>') && (seq.at(2) == '5')) {
						mDisp->curon();
					}
					break;
				case 'h':
					if ((seq.at(1) == '>') && (seq.at(2) == '5')) {
						mDisp->curoff();
					}
					break;
				case 'g':
					if (param_length > 0) {
						std::string param_str = seq.substr(1, param_length);
						int param;
						if (ExtractParamString(param_str, &param, 1) == 1) {
							if (param == 0) {
								//								mDisp->clear_tabpos();
							}
							else {
								//								mDisp->clear_alltab();
							}
						}
					}
					else {
						//						mDisp->clear_tabpos();
					}
					break;
			}
			break;
		case '$':
			switch (last_byte) {
				case 'B':
				case '@':
					break;
			}
			break;
		case '(':
			switch (last_byte) {
				case 'B':
				case 'J':
					break;
			}
			break;
		case '@':
			switch (last_byte) {
				case 'Z':
					if (param_length > 0) {
						std::string param_str = seq.substr(1, param_length);
						int param;
						if (ExtractParamString(param_str, &param, 1) == 1) {
							switch (param) {
								case 0:
									mDisp->init_disp();
									break;
								case 1:
								case 2:
								case 3:
									//									mDisp->set_charset(param);
									break;
								case 4:
								case 5:
								case 6:
									mDisp->set_charsize(param - 4);
									break;
								case 10:
								case 11:
								case 12:
								case 13:
								case 14:
								case 15:
									// ボーレートの設定
									break;
								case 20:
								case 21:
									mDisp->set_autocr((param==20)?1:0);
									break;
								case 22:
								case 23:
									//									mDisp->set_autoscroll((param==22)?1:0);
									break;
								case 30:
								case 31:
								case 32:
								case 33:
									mDisp->set_bgbuff(param - 30);
									break;
								case 35:
								case 36:
									break;
								case 40:
								case 41:
									//									mDisp->set_lfmode((param==40)?1:0);
									break;
								case 42:
								case 43:
									//									mDisp->set_crmode((param==42)?1:0);
									break;
								case 44:
								case 45:
								case 46:
									//									mDisp->set_delmode(param - 44);
									break;
								case 48:
								case 49:
									//									mDisp->set_bsmode((param==48)?1:0);
									break;
									
								case 70:
								case 71:
								case 72:
								case 73:
									mDisp->clear_bgbuff(param - 70);
									break;
								case 75:
								case 76:
									//									mDisp->set_fullwidth((param==75)?1:0);
									break;
								case 80:
								case 81:
									//									mDisp->set_backlight((param==80)?1:0);
									break;
							}
						}
					}
					break;
				case 'z':
					if (param_length > 0) {
						std::string param_str = seq.substr(1, param_length);
						int param[8];
						int param_num = ExtractParamString(param_str, param, 8);
						if (param_num > 0) {
							switch (param[0]) {
								case 0:
									if (param_num == 0) {
										mDisp->draw_fillrect(0, 320, 240, 0, 0, 32768);
									}
									else {
										mDisp->draw_fillrect(param[1], param[2], param[3], param[4], param[5], param[6]);
									}
									break;
								case 1:
									//									mDisp->draw_background(param[1], param[2], param[3], param[4], param[5]);
									break;
								case 2:
									mDisp->draw_line(param[1], param[2], param[3], param[4], param[5], param[6]);
									break;
								case 3:
									mDisp->draw_ellipse(param[1], param[2], param[3], param[4], param[5], param[6], param[7]);
									break;
								case 4:
									mDisp->draw_rectangle(param[1], param[2], param[3], param[4], param[5], param[6]);
									break;
								case 5:
									//									mDisp->draw_dot(param[1], param[2], param[3], param[4]);
									break;
							}
						}
					}
					break;
				case 'I':
					if (param_length > 0) {
						std::string param_str = seq.substr(1, param_length);
						int param[5];
						int param_num = ExtractParamString(param_str, param, 5);
						switch (param_num) {
							case 2:
								mDisp->draw_image(param[0], param[1], 0, 0);
								break;
							case 3:
								mDisp->draw_image2(param[0], param[1], param[2], 0, 0);
								break;
							case 4:
								mDisp->draw_image(param[0], param[1], param[2], param[3]);
								break;
							case 5:
								mDisp->draw_image2(param[0], param[1], param[2], param[3], param[4]);
								break;
						}
					}
					else {
						mDisp->draw_image(1, 0, 0, 0);
					}
					break;
				case 'V':
					if (param_length > 0) {
						std::string param_str = seq.substr(1, param_length);
						int param[4];
						if (ExtractParamString(param_str, param, 4) == 4) {
							mFwVersion = (param[0] << 24) | (param[1] << 16) | (param[2] << 8) | param[3];
						}
					}
					break;
			}
			break;
		case '7':
			mDisp->save_attribute(0);
			break;
		case '8':
			mDisp->load_attribute(0);
			break;
		case 'D':
			//			mDisp->cur_down2();
			break;
		case 'M':
			//			mDisp->cur_up2();
			break;
		case 'E':
			//			mDisp->cur_lastrow();
			break;
		case 'H':
			//			mDisp->set_tabpos();
			break;
		case 'c':
			//			mDisp->init();
			break;
		case 'T':
			//			mDisp->cur_rowtop();
			break;
		default:
			// エラー
			break;
	}
}

int EscSeqParser::ExtractParamString(const std::string &str, int *out_array, int array_size)
{
	size_t	begin_pos = 0;
	int param_num = 0;
	while ((begin_pos != std::string::npos) && (param_num < array_size)) {
		size_t next_pos = str.find(';', begin_pos);
		size_t len = next_pos;
		if (next_pos != std::string::npos) {
			len -= begin_pos;
		}
		std::string num = str.substr(begin_pos, next_pos);
		if (num.length() > 0) {
			out_array[param_num++] = std::stoi(num);
		}
		begin_pos = next_pos;
		if (next_pos != std::string::npos) {
			++begin_pos;
		}
	}
	return param_num;
}
