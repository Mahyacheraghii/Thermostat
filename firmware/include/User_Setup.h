#define ILI9341_DRIVER
#define TFT_MOSI 23
#define TFT_MISO 19
#define TFT_SCLK 18
#define TFT_CS 15
#define TFT_DC 2
#define TFT_RST 4
#define TOUCH_CS 5
#define TOUCH_IRQ 33
// برای سرعت عالی و روان بودن (۲۷ مگاهرتز)
#define SPI_FREQUENCY 27000000

// برای خواندن دیتا (۲۰ مگاهرتز)
#define SPI_READ_FREQUENCY 20000000

// برای لمس (بسیار مهم: این عدد باید کم باشد تا نویز نگیرد - ۲.۵ مگاهرتز)
#define SPI_TOUCH_FREQUENCY 2500000