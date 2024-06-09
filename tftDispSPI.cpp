#include "tftDispSPI.h"

#include "shnm8x16r.h"
#include "mplus_j10r_jisx0201.h"
#include "mplus_j10r.h"
#include "shnmk16.h"
#include "misaki_4x8_jisx0201.h"
#include "misaki_gothic.h"
#include "image.h"

const int sFontHeight[3] PROGMEM = {8, 11, 16};
const int sTextHeight[3] PROGMEM = {8, 12, 16};
const int sTextWidth[3] PROGMEM = {4, 5, 8};

tftDispSPI::tftDispSPI()
: mBgSpr(&mTft)
, mTextSpr{TFT_eSprite(&mTft)
, TFT_eSprite(&mTft)
, TFT_eSprite(&mTft)
, TFT_eSprite(&mTft)
, TFT_eSprite(&mTft)
, TFT_eSprite(&mTft)
, TFT_eSprite(&mTft)
, TFT_eSprite(&mTft)
, TFT_eSprite(&mTft)
, TFT_eSprite(&mTft)
, TFT_eSprite(&mTft)
, TFT_eSprite(&mTft)
, TFT_eSprite(&mTft)
, TFT_eSprite(&mTft)
, TFT_eSprite(&mTft)
, TFT_eSprite(&mTft)
, TFT_eSprite(&mTft)
, TFT_eSprite(&mTft)
, TFT_eSprite(&mTft)
, TFT_eSprite(&mTft)
, TFT_eSprite(&mTft)
, TFT_eSprite(&mTft)
, TFT_eSprite(&mTft)
, TFT_eSprite(&mTft)}
, mTextSprPtr{nullptr
, nullptr
, nullptr
, nullptr
, nullptr
, nullptr
, nullptr
, nullptr
, nullptr
, nullptr
, nullptr
, nullptr
, nullptr
, nullptr
, nullptr
, nullptr
, nullptr
, nullptr
, nullptr
, nullptr
, nullptr
, nullptr
, nullptr
, nullptr}
, mUpdateStartY(VIEW_HEIGHT)
, mUpdateEndY(0)
, mTextPosX(0)
, mTextPosY(0)
, mTextColor(9)
, mTextBgColor(TFT_EDISP_TRANSPARENT)
, mFontStyle(0)
, m2ByteGlyph(0)
, mReading2ByteCode(0)
{
}

tftDispSPI::~tftDispSPI()
{
}

void tftDispSPI::init()
{
  mTft.init();
  mTft.invertDisplay(true);
  mTft.setRotation(1);
  mTft.fillScreen(TFT_BLACK);
  mBgSpr.setColorDepth(16);
  mBgSpr.fillSprite(TFT_BLACK);
  mBgSprPtr = (uint16_t*)mBgSpr.createSprite(VIEW_WIDTH, VIEW_HEIGHT);
  set_charsize(kNormalFont);
  mTft.initDMA();
}

uint16_t tftDispSPI::sjisToLiner(uint16_t sjis)
{
  uint16_t linear;
  uint8_t up = sjis >> 8;
  uint8_t lo = sjis & 0xff;
  if (0x81 <= up && up <= 0x9f) {
    linear = up - 0x81;
  }
  else if (0xe0 <= up && up <= 0xef) {
    linear = up - 0xc1;
  }
  else {
    return 0;
  }
  if(0x40 <= lo && lo <= 0x7e) {
    linear *= 188;
    linear += lo - 0x40;
  }
  else if (0x80 <= lo && lo <= 0xfc) {
    linear *= 188;
    linear += 63;
    linear += lo - 0x80;
  }
  else {
    return 0;
  }
  return linear;
}

bool tftDispSPI::updateContent()
{
	if (mUpdateStartY < mUpdateEndY) {
    if (mTft.dmaBusy()) return false;
/*
		mTft.startWrite();
    mTft.pushImageDMA(0, mUpdateStartY, VIEW_WIDTH, mUpdateEndY - mUpdateStartY, mBgSprPtr+VIEW_WIDTH*mUpdateStartY);
    int textHeight = sTextHeight[mFontType];
    int updateStartRow = mUpdateStartY / textHeight;
    int updateEndRow = (mUpdateEndY + textHeight - 1) / textHeight;
    for (int i=updateStartRow; i<updateEndRow; ++i) {
      mTextSpr[i].pushSprite(0, textHeight * i, TFT_EDISP_TRANSPARENT);
    }
    mTft.endWrite();
    */
    
    int textHeight = sTextHeight[mFontType];
    TFT_eSprite tmpSpr(&mTft);
    tmpSpr.setColorDepth(16);
    uint16_t* tmpSprPtr = (uint16_t*)tmpSpr.createSprite(VIEW_WIDTH, textHeight);
    int updateStartRow = mUpdateStartY / textHeight;
    int updateEndRow = (mUpdateEndY + textHeight - 1) / textHeight;
    for (int i=updateStartRow; i<updateEndRow; ++i) {
      tmpSpr.pushImage(0, 0, VIEW_WIDTH, textHeight, mBgSprPtr+VIEW_WIDTH*i*textHeight);
      blend4bppImageToRGB565(mTextSprPtr[i], tmpSprPtr, VIEW_WIDTH, textHeight, TFT_EDISP_TRANSPARENT, default_4bit_palette);// mTextSpr[i].pushToSprite(&tmpSpr, 0, 0, TFT_EDISP_TRANSPARENT);
      mTft.startWrite();
      mTft.pushImageDMA(0, i*textHeight, VIEW_WIDTH, textHeight, tmpSprPtr);
      mTft.endWrite();
    }
    tmpSpr.deleteSprite();

    mUpdateStartY = VIEW_HEIGHT;
    mUpdateEndY = 0;
	}
  return true;
}

void  tftDispSPI::blend4bppImageToRGB565(const uint16_t *src, uint16_t *dst, uint16_t width, uint16_t height, uint16_t transpColor, const uint16_t *palette)
{
  const uint8_t *in = (const uint8_t*)src;
  int     nibble = 4;
  int pixels = width * height;
  for (int i=0; i<pixels; ++i) {
    uint16_t  in_pixel = (*in >> nibble) & 0x0f;
    if (in_pixel != transpColor) {
      *dst = (palette[in_pixel] << 11) | ((palette[in_pixel] >> 5)&0x07e0) | ((palette[in_pixel] >> 6)&0x001f);
      // *dst = palette[in_pixel];
    }
    ++dst;
    nibble = (nibble + 4) & 4;
    if (nibble == 4) {
      ++in;
    }
  }
}

void	tftDispSPI::setUpdateArea(int startY, int endY)
{
  if (startY < mUpdateStartY) {
    mUpdateStartY = startY;
  }
  if (endY > mUpdateEndY) {
    mUpdateEndY = endY;
  }
}

int tftDispSPI::getLineLength(const char *str)
{
	int len = 0;
	while (str[len] != '\n' && str[len] != '\r' && str[len] != 0) {
		++len;
	}
	return len;
}

void tftDispSPI::puts_(const char* str, uint32_t max_len)
{
  uint16_t	foreColor;
  uint16_t	backColor;
  if (mFontStyle & STYLE_INVERTED) {
    foreColor = mTextBgColor;
    backColor = mTextColor;
  }
  else {
    foreColor = mTextColor;
    backColor = mTextBgColor;
  }
  int startCol = 0;

  int fontHeight = sFontHeight[mFontType];
  int textHeight = sTextHeight[mFontType];
  int textWidth = sTextWidth[mFontType];

  do {
    static char lineText[81];
    int lineLen = getLineLength(&str[startCol]);
    strlcpy(lineText, &str[startCol], (lineLen > 80)?81:(lineLen+1));
    // アップデート領域を設定
    int lineStartPosX = mTextPosX;
    int lineStartY = mTextPosY * textHeight;
    int lineEndY = (mTextPosY + 1) * textHeight;
    if (lineText[0] != 0) {
      setUpdateArea(lineStartY, lineEndY);
    }
    // テキストを描画
    int drawPos = 0;
    while (lineText[drawPos] != 0) {
      if (mReading2ByteCode) {
        m2ByteGlyph |= lineText[drawPos];
        mReading2ByteCode = false;
        int glyph = sjisToLiner(m2ByteGlyph);
        mTextSpr[mTextPosY].drawXBitmap(mTextPosX*textWidth,0,m2ByteGlyphData+m2ByteGlyphBytes*glyph,textWidth*2,fontHeight,foreColor,backColor);
        mTextPosX += 2;
      }
      else {
        if ((0x81 <= lineText[drawPos] && lineText[drawPos] <= 0x9f) || (0xe0 <= lineText[drawPos] && lineText[drawPos] <= 0xef)) {
          m2ByteGlyph = lineText[drawPos] << 8;
          mReading2ByteCode = true;
        }
        else {
          mTextSpr[mTextPosY].drawXBitmap(mTextPosX*textWidth,0,mAsciiGlyphData+mAsciiGlyphBytes*lineText[drawPos],textWidth,fontHeight,foreColor,backColor);
          mTextPosX += 1;
        }
      }
      ++drawPos;
    }
    // 下線を描画
    if (mFontStyle & STYLE_UNDERLINED) {
      mTextSpr[mTextPosY].drawFastHLine(lineStartPosX * textWidth, textHeight-1, (mTextPosX - lineStartPosX) * textWidth, foreColor);
    }
    else if (fontHeight == 11) {
      mTextSpr[mTextPosY].drawFastHLine(lineStartPosX * textWidth, textHeight-1, (mTextPosX - lineStartPosX) * textWidth, backColor);
    }
    // 改行コードの処理
    startCol += lineLen;
    while (str[startCol] == '\r' || str[startCol] == '\n') {
      if (str[startCol] == '\r') {
        mTextPosX = 0;
      }
      if (str[startCol] == '\n') {
        ++mTextPosY;
      }
      ++startCol;
    }
  } while (str[startCol] != 0);
}

void tftDispSPI::printw(const char* fmt, ...)
{
	va_list arg;
	static char buf[100];
	va_start(arg, fmt);
	vsnprintf(buf, 100, fmt, arg);
	va_end(arg);
	puts_(buf);
}

void tftDispSPI::vprintw(const char* fmt, va_list arg)
{
	static char buf[100];
	vsnprintf(buf, 100, fmt, arg);
	puts_(buf);
}

void tftDispSPI::clear(void)
{
	mTextPosX = mTextPosY = 0;
  mBgSpr.fillSprite(TFT_BLACK);
  for (int i=0; i<24; ++i) {
    if (mTextSprPtr[i] != nullptr) {
      mTextSpr[i].fillSprite(TFT_EDISP_TRANSPARENT);
    }
  }
  mUpdateStartY = 0;
  mUpdateEndY = VIEW_HEIGHT;
}

void tftDispSPI::move(int r, int c)
{
	mTextPosY = r;
	mTextPosX = c;
}

void tftDispSPI::curmove(int r, int c)
{
	mTextPosY += r;
	mTextPosX += c;
}

void tftDispSPI::cur_rowtop(void)
{
	mTextPosX = 0;
}

void tftDispSPI::set_attribute( int n )
{
	switch (n) {
		case 0:
			mTextColor = 9;//TFT_WHITE;
			mTextBgColor = TFT_EDISP_TRANSPARENT;
			mFontStyle &= ~STYLE_BOLD;
			mFontStyle &= ~STYLE_UNDERLINED;
			mFontStyle &= ~STYLE_BLINKED;
			mFontStyle &= ~STYLE_INVERTED;
			break;
		case 1:
			mFontStyle |= STYLE_BOLD;
			break;
		case 4:
			mFontStyle |= STYLE_UNDERLINED;
			break;
		case 5:
			mFontStyle |= STYLE_BLINKED;
			break;
		case 7:
			mFontStyle |= STYLE_INVERTED;
			break;
		case 8:
		case 16:
			mTextColor = TFT_EDISP_TRANSPARENT;
			mTextBgColor = TFT_EDISP_TRANSPARENT;
			break;
		case 30:
			mTextColor = 0;//TFT_BLACK;
			break;
		case 17:
		case 31:
			mTextColor = 2;//TFT_RED;
			break;
		case 18:
		case 32:
			mTextColor = 5;//TFT_GREEN;
			break;
		case 19:
		case 33:
			mTextColor = 4;//TFT_YELLOW;
			break;
		case 20:
		case 34:
			mTextColor = 6;//TFT_BLUE;
			break;
		case 21:
		case 35:
			mTextColor = 11;//TFT_MAGENTA;
			break;
		case 22:
		case 36:
			mTextColor = 10;//TFT_CYAN;
			break;
		case 23:
		case 37:
		case 39:
			mTextColor = 9;//TFT_WHITE;
			break;
			
		case 40:
			mTextBgColor = 0;//TFT_BLACK;
			break;
		case 41:
			mTextBgColor = 2;//TFT_RED;
			break;
		case 42:
			mTextBgColor = 5;//TFT_GREEN;
			break;
		case 43:
			mTextBgColor = 4;//TFT_YELLOW;
			break;
		case 44:
			mTextBgColor = 6;//TFT_BLUE;
			break;
		case 45:
			mTextBgColor = 11;//TFT_MAGENTA;
			break;
		case 46:
			mTextBgColor = 10;//TFT_CYAN;
			break;
		case 47:
			mTextBgColor = 9;//TFT_WHITE;
			break;
		case 49:
			mTextBgColor = TFT_EDISP_TRANSPARENT;
			break;
		case 24:
			mFontStyle &= ~STYLE_UNDERLINED;
			break;
		case 25:
			mFontStyle &= ~STYLE_BLINKED;
			break;
		case 27:
			mFontStyle &= ~STYLE_INVERTED;
			break;
		case 29:
			mFontStyle &= ~STYLE_BOLD;
			break;
		case 60:
			mTextColor = TFT_EDISP_TRANSPARENT;
			break;
		case 61:
			mTextBgColor = TFT_EDISP_TRANSPARENT;
			break;
	}
}
void tftDispSPI::set_attribute( int n1, int n2 )
{
	set_attribute(n1);
	set_attribute(n2);
}

void tftDispSPI::set_attribute( int n1, int n2, int n3 )
{
	set_attribute(n1);
	set_attribute(n2);
	set_attribute(n3);
}

void tftDispSPI::set_attribute( int n1, int n2, int n3, int n4 )
{
	set_attribute(n1);
	set_attribute(n2);
	set_attribute(n3);
	set_attribute(n4);
}

void tftDispSPI::del_to_end()
{
  int textHeight = sTextHeight[mFontType];
  int textWidth = sTextWidth[mFontType];
  mTextSpr[mTextPosY].fillRect(textWidth * mTextPosX, 0, VIEW_WIDTH - textWidth * mTextPosX, textHeight, TFT_EDISP_TRANSPARENT);
  setUpdateArea(textHeight * mTextPosY, textHeight * (mTextPosY + 1));
}

void tftDispSPI::del_row()
{
  int textHeight = sTextHeight[mFontType];
  mTextSpr[mTextPosY].fillRect(0, 0, VIEW_WIDTH, textHeight, TFT_EDISP_TRANSPARENT);
  setUpdateArea(textHeight * mTextPosY, textHeight * (mTextPosY + 1));
}

void tftDispSPI::del(int n)
{
  int textHeight = sTextHeight[mFontType];
  int textWidth = sTextWidth[mFontType];
  mTextSpr[mTextPosY].fillRect(mTextPosX * textWidth, 0, textWidth * n, textHeight, TFT_EDISP_TRANSPARENT);
  setUpdateArea(textHeight * mTextPosY, textHeight * (mTextPosY + 1));
}

void tftDispSPI::save_attribute(int n)
{
	
}

void tftDispSPI::load_attribute(int n)
{
	
}

void tftDispSPI::curon(void)
{
	
}

void tftDispSPI::curoff(void)
{
	
}

void tftDispSPI::init_disp(void)
{
  mBgSpr.fillSprite(TFT_BLACK);
	clear();
	set_attribute(0);
}

void tftDispSPI::set_charsize(int x)
{
  // TODO: 動かない原因を調査
  if (x==0) return;

  for (int i=0; i<24; ++i) {
    if (mTextSprPtr[i] != nullptr) {
      mTextSpr[i].deleteSprite();
      mTextSprPtr[i] = nullptr;
    }
  }
  int textHeight = sTextHeight[x];
  int textLines = VIEW_HEIGHT / textHeight;
  for (int i=0; i<textLines; ++i) {
    mTextSpr[i].setColorDepth(4);
    mTextSprPtr[i] = (uint16_t*)mTextSpr[i].createSprite(VIEW_WIDTH, textHeight);
    mTextSpr[i].fillSprite(TFT_EDISP_TRANSPARENT);
  }
  mFontType = x;
  switch(x) {
    case kTinyFont:
      mAsciiGlyphBytes = 8;
      m2ByteGlyphBytes = 8;
      mAsciiGlyphData = misaki_4x8_jisx0201;
      m2ByteGlyphData = misaki_gothic;
      break;
    case kSmallFont:
      mAsciiGlyphBytes = 11;
      m2ByteGlyphBytes = 22;
      mAsciiGlyphData = mplus_j10r_jisx0201;
      m2ByteGlyphData = mplus_j10r;
      break;
    case kNormalFont:
      mAsciiGlyphBytes = 16;
      m2ByteGlyphBytes = 32;
      mAsciiGlyphData = shnm8x16r;
      m2ByteGlyphData = shnmk16;
      break;
  }
}

void tftDispSPI::set_bgbuff(int x)
{
	// mCurrentBg = x;
}

void tftDispSPI::clear_bgbuff(int x)
{
	mBgSpr.fillSprite(TFT_BLACK);
  mUpdateStartY = 0;
  mUpdateEndY = VIEW_HEIGHT;
  setUpdateArea(0, VIEW_HEIGHT);
}

void tftDispSPI::draw_fillrect(int bg, int w, int h, int x, int y, int col )
{
  uint16_t tft_col = conv555To565(col);
  mBgSpr.fillRect(x, y, w, h, tft_col);
  setUpdateArea(y, y+h);
}

void tftDispSPI::draw_line(int bg, int x0, int y0, int x1, int y1, int col )
{
  uint16_t tft_col = conv555To565(col);
  mBgSpr.drawLine(x0, y0, x1, y1, tft_col);
  setUpdateArea(y0, y1);
}

void tftDispSPI::draw_ellipse(int bg, int fill, int cx, int cy, int xw, int yh, int col )
{
  uint16_t tft_col = conv555To565(col);
  if (fill) {
    mBgSpr.fillEllipse(cx, cy, xw, yh, tft_col);
  }
  else {
    mBgSpr.drawEllipse(cx, cy, xw, yh, tft_col);
  }
  setUpdateArea(max(0, cy - yh), min(VIEW_HEIGHT, cy + yh + 1));
}

void tftDispSPI::draw_rectangle(int bg, int w, int h, int x, int y, int col)
{
  uint16_t tft_col = conv555To565(col);
  mBgSpr.drawRect(x, y, w, h, tft_col);
  setUpdateArea(y, y+h);
}

void tftDispSPI::draw_image(int file, int bg, int x, int y)
{
  const uint16_t  *imagePtr = imgArray[file];
  if (imagePtr != nullptr) {
    mBgSpr.pushImage(x, y, imagePtr[0], imagePtr[1], &imagePtr[2]);
    setUpdateArea(y, y+imagePtr[1]);
  }
}

void tftDispSPI::draw_image2(int file, int bg, int fol, int x, int y)
{
  // image2は未使用
}
