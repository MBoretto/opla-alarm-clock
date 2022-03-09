
//https://www.mischianti.org/images-to-byte-array-online-converter-cpp-arduino/
const unsigned char wifi [] PROGMEM = {
  // 'wifi, 15x15px
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0xe0, 0x38, 0x38, 0xe0, 0x0e, 0x4f, 0xe4, 0x3c, 0x78, 
  0x10, 0x10, 0x07, 0xc0, 0x04, 0x40, 0x00, 0x00, 0x03, 0x80, 0x03, 0x80, 0x01, 0x00
};

const unsigned char nowifi [] PROGMEM = {
  // 'nowifi, 15x15px
  0x03, 0x80, 0x03, 0x80, 0x03, 0x80, 0x0f, 0xe0, 0x3b, 0xb8, 0xe1, 0x0e, 0x4d, 0x64, 0x19, 0x30, 
  0x11, 0x10, 0x07, 0xc0, 0x05, 0x40, 0x00, 0x00, 0x03, 0x80, 0x03, 0x80, 0x01, 0x00
};


const unsigned char clean [] PROGMEM = {
  // 'clean, 15x15px
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
};

const unsigned char alarmon [] PROGMEM = {
	// 'alarmon, 15x15px
	0x00, 0x00, 0x30, 0x18, 0x69, 0x2c, 0x59, 0x34, 0xb6, 0xda, 0xa8, 0x2a, 0xa8, 0x2a, 0xa8, 0x2a, 
	0x08, 0x20, 0x08, 0x20, 0x10, 0x10, 0x30, 0x18, 0x1f, 0xf0, 0x03, 0x80, 0x00, 0x00
};
