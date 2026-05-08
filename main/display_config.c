#include "globalVar.h"

static const char *TAG = "ORBIT_DISPLAY";

static esp_lcd_panel_handle_t panel_handle = NULL;
static spi_device_handle_t touch_spi;

//=========== Touch Init ===========
static orbit_err_t _display_touch_init(void)
{
    spi_bus_config_t buscfg = {
        .mosi_io_num   = TOUCH_MOSI,
        .miso_io_num   = TOUCH_MISO,
        .sclk_io_num   = TOUCH_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
    };

    if (spi_bus_initialize(SPI3_HOST, &buscfg, SPI_DMA_CH_AUTO) != ESP_OK) {
        ESP_LOGE(TAG, "_display_touch_init: spi_bus_initialize failed");
        return ORBIT_ERR;
    }

    spi_device_interface_config_t devcfg = {
        .clock_speed_hz = 2 * 1000 * 1000,
        .mode           = 0,
        .spics_io_num   = TOUCH_CS,
        .queue_size     = 1,
    };

    if (spi_bus_add_device(SPI3_HOST, &devcfg, &touch_spi) != ESP_OK) {
        ESP_LOGE(TAG, "_display_touch_init: spi_bus_add_device failed");
        return ORBIT_ERR;
    }

    gpio_config_t irq_conf = {
        .pin_bit_mask = (1ULL << TOUCH_IRQ),
        .mode         = GPIO_MODE_INPUT,
        .pull_up_en   = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type    = GPIO_INTR_DISABLE,
    };

    if (gpio_config(&irq_conf) != ESP_OK) {
        ESP_LOGE(TAG, "_display_touch_init: gpio_config failed");
        return ORBIT_ERR;
    }

    ESP_LOGI(TAG, "_display_touch_init: OK");
    return ORBIT_OK;
}

//=========== Display Init ===========
orbit_err_t _display_init(void)
{
    esp_err_t ret;

    spi_bus_config_t buscfg = {
        .sclk_io_num     = PIN_NUM_CLK,
        .mosi_io_num     = PIN_NUM_MOSI,
        .miso_io_num     = PIN_NUM_MISO,
        .quadwp_io_num   = -1,
        .quadhd_io_num   = -1,
        .max_transfer_sz = 320 * 240 * 2 + 8,
    };

    ret = spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "SPI bus init failed");
        return ORBIT_ERR;
    }

    esp_lcd_panel_io_handle_t io_handle = NULL;

    esp_lcd_panel_io_spi_config_t io_config = {
        .dc_gpio_num       = PIN_NUM_DC,
        .cs_gpio_num       = PIN_NUM_CS,
        .pclk_hz           = 40 * 1000 * 1000,
        .lcd_cmd_bits      = 8,
        .lcd_param_bits    = 8,
        .spi_mode          = 0,
        .trans_queue_depth = 10,
    };

    ret = esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)SPI2_HOST, &io_config, &io_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "LCD panel IO init failed");
        return ORBIT_ERR;
    }

    esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = PIN_NUM_RST,
        .color_space    = ESP_LCD_COLOR_SPACE_BGR,
        .bits_per_pixel = 16,
    };

    ret = esp_lcd_new_panel_ili9341(io_handle, &panel_config, &panel_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "ILI9341 panel init failed");
        return ORBIT_ERR;
    }

    esp_lcd_panel_reset(panel_handle);
    esp_lcd_panel_init(panel_handle);
    esp_lcd_panel_disp_on_off(panel_handle, true);

    gpio_set_direction(PIN_NUM_BL, GPIO_MODE_OUTPUT);
    gpio_set_level(PIN_NUM_BL, 1);

    if (_display_touch_init() != ORBIT_OK) {
        ESP_LOGE(TAG, "Touch init failed");
        return ORBIT_ERR;
    }

    ESP_LOGI(TAG, "Display initialized successfully");
    return ORBIT_OK;
}

//=========== Clear Screen ===========
static uint16_t line[LCD_WIDTH];

void _display_clear(uint16_t color)
{
    for (int x = 0; x < LCD_WIDTH; x++) line[x] = color;
    for (int y = 0; y < LCD_HEIGHT; y++) {
        esp_lcd_panel_draw_bitmap(panel_handle, 0, y, LCD_WIDTH, y + 1, line);
    }
}

//=========== Touch Read ===========
#define TOUCH_X_MIN   389
#define TOUCH_X_MAX   3721
#define TOUCH_Y_MIN   276
#define TOUCH_Y_MAX   3687

static bool _display_touch_read(int *out_x, int *out_y)
{
    if (gpio_get_level(TOUCH_IRQ) == 1) return false;

    uint8_t tx[3] = {0};
    uint8_t rx[3] = {0};

    spi_transaction_t t = {
        .length    = 24,
        .tx_buffer = tx,
        .rx_buffer = rx,
    };

    tx[0] = 0xD0;
    spi_device_polling_transmit(touch_spi, &t);
    int raw_x = ((rx[1] << 8) | rx[2]) >> 3;

    tx[0] = 0x90;
    spi_device_polling_transmit(touch_spi, &t);
    int raw_y = ((rx[1] << 8) | rx[2]) >> 3;

    int mapped_x = (raw_x - TOUCH_X_MIN) * LCD_WIDTH  / (TOUCH_X_MAX - TOUCH_X_MIN);
    int mapped_y = (raw_y - TOUCH_Y_MIN) * LCD_HEIGHT / (TOUCH_Y_MAX - TOUCH_Y_MIN);

    *out_x = mapped_x < 0 ? 0 : (mapped_x > LCD_WIDTH  ? LCD_WIDTH  : mapped_x);
    *out_y = mapped_y < 0 ? 0 : (mapped_y > LCD_HEIGHT ? LCD_HEIGHT : mapped_y);

    return true;
}

//=============== DISPLAY MODULES ===============================

//=========== Time Format ===========
static int _time_hour   = 10;
static int _time_minute = 42;

static void _time_get_string(char *buf, size_t len)
{
    if (g_system_prefs.units == 1) {
        int hour12 = _time_hour % 12;
        if (hour12 == 0) hour12 = 12;
        const char *period = (_time_hour < 12) ? "AM" : "PM";
        snprintf(buf, len, "%d:%02d %s", hour12, _time_minute, period);
    } else {
        snprintf(buf, len, "%02d:%02d", _time_hour, _time_minute);
    }
}

//=========== Date Format ===========
static int _date_day     = 5;
static int _date_month   = 5;
static int _date_year    = 2025;
static int _date_weekday = 1;  // 0=SUN, 1=MON ... 6=SAT

static void _date_get_string(char *buf, size_t len)
{
    const char *days[]   = {"SUN","MON","TUE","WED","THU","FRI","SAT"};
    const char *months[] = {"JAN","FEB","MAR","APR","MAY","JUN",
                             "JUL","AUG","SEP","OCT","NOV","DEC"};
    snprintf(buf, len, "%s  %s %d  %d",
        days[_date_weekday],
        months[_date_month - 1],
        _date_day,
        _date_year
    );
}

//=========== Astrophysics & General Facts ===========
#define FACTS_COUNT 200

static const char facts[FACTS_COUNT][2][50] = { // this is specific to the gift so yeah... add your own

    //=========== NASA ===========
    {"NASA",    "The Sun contains 99.8% of solar system mass"},
    {"NASA",    "Light from the Sun takes 8 minutes to reach Earth"},
    {"NASA",    "The Milky Way has over 200 billion stars"},
    {"NASA",    "Mars has the tallest volcano: Olympus Mons"},
    {"NASA",    "Venus spins backwards compared to most planets"},
    {"NASA",    "A day on Venus is longer than its year"},
    {"NASA",    "Jupiter has 95 known moons as of 2024"},
    {"NASA",    "The Great Red Spot is a storm older than 350 yrs"},
    {"NASA",    "Saturn could float on water due to low density"},
    {"NASA",    "Uranus rotates on its side at 98 degree tilt"},

    //=========== ISS ===========
    {"ISS",     "The ISS orbits Earth every 90 minutes"},
    {"ISS",     "ISS crew sees 16 sunrises and sunsets per day"},
    {"ISS",     "The ISS is as long as a football field"},
    {"ISS",     "ISS has been continuously occupied since Nov 2000"},
    {"ISS",     "Astronauts float due to free fall not zero G"},
    {"ISS",     "The ISS travels at 28000 km/h around Earth"},
    {"ISS",     "Over 270 people have visited the ISS"},
    {"ISS",     "Water on the ISS is 93% recycled from urine"},
    {"ISS",     "The ISS cost over 150 billion USD to build"},
    {"ISS",     "ISS has 6 sleeping quarters and 2 bathrooms"},

    //=========== HUBBLE ===========
    {"HUBBLE",  "Hubble orbits 547 km above Earths surface"},
    {"HUBBLE",  "Hubble has made over 1.5 million observations"},
    {"HUBBLE",  "Hubble helped determine universe age 13.8 bil yr"},
    {"HUBBLE",  "Hubble discovered dark energy expansion evidence"},
    {"HUBBLE",  "Hubble was launched in 1990 aboard Discovery"},
    {"HUBBLE",  "Hubble mirror was flawed and fixed in 1993"},
    {"HUBBLE",  "Hubble has no thrusters it uses gyroscopes"},

    //=========== JWST ===========
    {"JWST",    "James Webb launched Christmas Day 2021"},
    {"JWST",    "JWST orbits at L2 point 1.5 million km from Earth"},
    {"JWST",    "JWST mirror is 6.5 meters across 18 segments"},
    {"JWST",    "JWST operates in infrared not visible light"},
    {"JWST",    "JWST can see first galaxies after the Big Bang"},
    {"JWST",    "JWST detected CO2 in an exoplanet atmosphere"},
    {"JWST",    "JWST keeps cold at -233C using a sunshield"},
    {"JWST",    "JWST cost approximately 10 billion USD"},
    {"JWST",    "JWST can see through dust clouds in the galaxy"},
    {"JWST",    "JWST has a 20 year design lifetime"},

    //=========== COSMOS ===========
    {"COSMOS",  "The observable universe is 93 billion light yrs wide"},
    {"COSMOS",  "There are more stars than grains of sand on Earth"},
    {"COSMOS",  "The universe is approximately 13.8 billion years old"},
    {"COSMOS",  "Dark matter makes up 27% of the universe"},
    {"COSMOS",  "Dark energy makes up 68% of the universe"},
    {"COSMOS",  "Normal matter is only 5% of the universe"},
    {"COSMOS",  "Space is completely silent no medium for sound"},
    {"COSMOS",  "The coldest known place is Boomerang Nebula -272C"},
    {"COSMOS",  "A neutron star teaspoon weighs 10 million tons"},
    {"COSMOS",  "All atoms in your body were made inside a star"},

    //=========== STARS ===========
    {"STARS",   "The Sun is a G-type main sequence yellow dwarf"},
    {"STARS",   "Betelgeuse could explode as supernova any time"},
    {"STARS",   "Proxima Centauri is 4.2 light years from Earth"},
    {"STARS",   "A white dwarf is the remnant of a dead star"},
    {"STARS",   "Neutron stars can spin 700 times per second"},
    {"STARS",   "Stars are born in nebulae clouds of gas and dust"},
    {"STARS",   "Our Sun will become a red giant in 5 billion years"},
    {"STARS",   "The heaviest elements are forged in supernovae"},
    {"STARS",   "Black holes form when massive stars collapse"},
    {"STARS",   "The Sun fuses 600 million tons of hydrogen per sec"},

    //=========== SPACEX ===========
    {"SPACEX",  "SpaceX Falcon 9 first stage is fully reusable"},
    {"SPACEX",  "SpaceX Starship is the largest rocket ever built"},
    {"SPACEX",  "SpaceX launched first crewed mission in 2020"},
    {"SPACEX",  "Starlink constellation has over 5000 satellites"},
    {"SPACEX",  "Falcon Heavy has 27 Merlin engines at launch"},
    {"SPACEX",  "Starship uses liquid methane and oxygen fuel"},
    {"SPACEX",  "SpaceX aims to make humans multiplanetary"},
    {"SPACEX",  "SpaceX has launched over 250 missions total"},

    //=========== FERRARI F1 ===========
    {"FERRARI", "Ferrari is the oldest active F1 constructor"},
    {"FERRARI", "Ferrari has won 16 F1 constructors championships"},
    {"FERRARI", "Michael Schumacher won 5 titles with Ferrari"},
    {"FERRARI", "Ferrari debuted in F1 at the 1950 Monaco GP"},
    {"FERRARI", "Enzo Ferrari founded the team in Maranello Italy"},
    {"FERRARI", "Ferrari is known as Scuderia Ferrari in F1"},
    {"FERRARI", "The Ferrari F1 car is called the SF series"},
    {"FERRARI", "Charles Leclerc drives the number 16 for Ferrari"},
    {"FERRARI", "Ferrari won back to back titles in 2000 and 2001"},
    {"FERRARI", "Ferrari red is called Rosso Corsa racing red"},
    {"FERRARI", "Kimi Raikkonen won Ferraris last title in 2007"},
    {"FERRARI", "Ferrari has over 240 Grand Prix victories total"},
    {"FERRARI", "The prancing horse logo came from a WW1 pilot"},
    {"FERRARI", "Ferrari headquarters is in Maranello near Modena"},
    {"FERRARI", "Ferraris engine is built with Italian craftsmanship"},

    //=========== MCLAREN F1 ===========
    {"MCLAREN", "McLaren was founded by Bruce McLaren in 1963"},
    {"MCLAREN", "McLaren has won 8 F1 constructors championships"},
    {"MCLAREN", "Ayrton Senna won 3 driver titles with McLaren"},
    {"MCLAREN", "McLaren and Honda dominated F1 in 1988"},
    {"MCLAREN", "The 1988 McLaren MP4/4 won 15 of 16 races"},
    {"MCLAREN", "McLaren uses papaya orange as their signature color"},
    {"MCLAREN", "Lando Norris drives number 4 for McLaren"},
    {"MCLAREN", "McLaren Technology Centre is in Woking England"},
    {"MCLAREN", "McLaren won their last title with Lewis Hamilton"},
    {"MCLAREN", "The McLaren F1 road car had a central driver seat"},
    {"MCLAREN", "McLaren has over 180 Grand Prix victories total"},
    {"MCLAREN", "McLaren uses a carbon fiber chassis since 1981"},
    {"MCLAREN", "Alain Prost won 3 of his 4 titles with McLaren"},
    {"MCLAREN", "Oscar Piastri joined McLaren in 2023"},
    {"MCLAREN", "McLaren won the 2024 constructors championship"},

    //=========== GREEN BAY PACKERS ===========
    {"PACKERS", "The Packers were founded in Green Bay in 1919"},
    {"PACKERS", "Green Bay is the smallest city with an NFL team"},
    {"PACKERS", "The Packers are community owned since 1923"},
    {"PACKERS", "Green Bay has won 13 NFL championships total"},
    {"PACKERS", "The Packers have won 4 Super Bowls"},
    {"PACKERS", "Lambeau Field opened in 1957 in Green Bay WI"},
    {"PACKERS", "Lambeau Field holds over 81000 fans"},
    {"PACKERS", "The Packers are named after the Indian Packing Co"},
    {"PACKERS", "Curly Lambeau co-founded the Packers in 1919"},
    {"PACKERS", "Vince Lombardi coached the Packers to 5 NFL titles"},
    {"PACKERS", "The Ice Bowl was played in -13F in 1967"},
    {"PACKERS", "Bart Starr led the Packers to 5 championships"},
    {"PACKERS", "Brett Favre threw 442 TD passes as a Packer"},
    {"PACKERS", "Aaron Rodgers won MVP 4 times as a Packer"},
    {"PACKERS", "Jordy Nelson had 1519 receiving yards in 2016"},
    {"PACKERS", "Donald Driver is Packers all time receiving leader"},
    {"PACKERS", "The Packers have sold out season tickets since 1960"},
    {"PACKERS", "Reggie White was known as Minister of Defense"},
    {"PACKERS", "Green Bay beat Dallas in Ice Bowl on a QB sneak"},
    {"PACKERS", "The Packers have retired 22 jersey numbers"},
    {"PACKERS", "Ray Nitschke is the greatest Packer LB ever"},
    {"PACKERS", "Packers won Super Bowl XLV over Pittsburgh 31-25"},
    {"PACKERS", "The Tundra Trot is a famous Lambeau Field tradition"},
    {"PACKERS", "Packers fans are called Cheeseheads"},
    {"PACKERS", "Green Bay has a waiting list of 140000 for tickets"},

    //=========== US HISTORY ===========
    {"US HIST", "The Declaration of Independence was signed in 1776"},
    {"US HIST", "George Washington was the first US president"},
    {"US HIST", "The US Constitution was ratified in 1788"},
    {"US HIST", "The Bill of Rights added 10 amendments in 1791"},
    {"US HIST", "The Louisiana Purchase doubled US size in 1803"},
    {"US HIST", "The War of 1812 was fought against Britain"},
    {"US HIST", "The Civil War lasted from 1861 to 1865"},
    {"US HIST", "Abraham Lincoln abolished slavery in 1863"},
    {"US HIST", "The Emancipation Proclamation freed slaves in 1863"},
    {"US HIST", "Women gained voting rights in the US in 1920"},
    {"US HIST", "The stock market crashed in October 1929"},
    {"US HIST", "The US entered WW2 after Pearl Harbor in 1941"},
    {"US HIST", "D-Day invasion of Normandy took place June 6 1944"},
    {"US HIST", "The atomic bomb was dropped on Japan in Aug 1945"},
    {"US HIST", "The Korean War lasted from 1950 to 1953"},
    {"US HIST", "Martin Luther King gave I Have A Dream in 1963"},
    {"US HIST", "JFK was assassinated in Dallas Texas in 1963"},
    {"US HIST", "Neil Armstrong walked on Moon July 20 1969"},
    {"US HIST", "The Vietnam War ended in 1975"},
    {"US HIST", "The Berlin Wall fell on November 9 1989"},
    {"US HIST", "The US has 50 states and 1 federal district"},
    {"US HIST", "The Statue of Liberty was a gift from France 1886"},
    {"US HIST", "The Panama Canal opened in August 1914"},
    {"US HIST", "The Manhattan Project built the first atomic bomb"},
    {"US HIST", "Rosa Parks refused to give up her seat in 1955"},
    {"US HIST", "The Civil Rights Act was signed into law in 1964"},
    {"US HIST", "Watergate scandal led to Nixon resignation in 1974"},
    {"US HIST", "The Internet was born from ARPANET in 1969"},
    {"US HIST", "9/11 attacks changed US security forever in 2001"},
    {"US HIST", "Barack Obama became first Black president in 2009"},

    //=========== WISCONSIN HISTORY ===========
    {"WISC",    "Wisconsin became the 30th US state in 1848"},
    {"WISC",    "Wisconsin is known as America's Dairyland"},
    {"WISC",    "Wisconsin produces more cheese than any US state"},
    {"WISC",    "Milwaukee is the largest city in Wisconsin"},
    {"WISC",    "Madison is the capital city of Wisconsin"},
    {"WISC",    "The Republican Party was founded in Ripon WI 1854"},
    {"WISC",    "Harry Houdini was born in Appleton Wisconsin"},
    {"WISC",    "Wisconsin has over 15000 lakes within its borders"},
    {"WISC",    "The first kindergarten in US opened in Watertown WI"},
    {"WISC",    "Frank Lloyd Wright was born in Richland Center WI"},
    {"WISC",    "Wisconsin had the first state income tax in 1911"},
    {"WISC",    "The USS Wisconsin battleship is named after the state"},
    {"WISC",    "Wisconsin leads the US in butter production"},
    {"WISC",    "Lake Michigan borders Wisconsin to the east"},
    {"WISC",    "The Harley Davidson company was founded in Milwaukee"},
    {"WISC",    "Wisconsin has 72 counties within its borders"},
    {"WISC",    "Door County WI is known as the Cape Cod of the Midwest"},
    {"WISC",    "The Wisconsin Dells is known as the waterpark capital"},
    {"WISC",    "Serial killer Ed Gein was from Plainfield Wisconsin"},
    {"WISC",    "The first US hydroelectric plant opened in Appleton WI"},
};

//=========== Font ===========
static const uint8_t font5x7[128][5] = {
    [' '] = {0x00,0x00,0x00,0x00,0x00},
    ['-'] = {0x08,0x08,0x08,0x08,0x08},
    [':'] = {0x00,0x36,0x36,0x00,0x00},
    ['.'] = {0x00,0x60,0x60,0x00,0x00},
    ['/'] = {0x20,0x10,0x08,0x04,0x02},
    ['%'] = {0x23,0x13,0x08,0x64,0x62},
    ['('] = {0x00,0x1C,0x22,0x41,0x00},
    [')'] = {0x00,0x41,0x22,0x1C,0x00},
    ['!'] = {0x00,0x5F,0x00,0x00,0x00},
    ['#'] = {0x14,0x7F,0x14,0x7F,0x14},
    ['0'] = {0x3E,0x51,0x49,0x45,0x3E},
    ['1'] = {0x00,0x42,0x7F,0x40,0x00},
    ['2'] = {0x42,0x61,0x51,0x49,0x46},
    ['3'] = {0x21,0x41,0x45,0x4B,0x31},
    ['4'] = {0x18,0x14,0x12,0x7F,0x10},
    ['5'] = {0x27,0x45,0x45,0x45,0x39},
    ['6'] = {0x3C,0x4A,0x49,0x49,0x30},
    ['7'] = {0x01,0x71,0x09,0x05,0x03},
    ['8'] = {0x36,0x49,0x49,0x49,0x36},
    ['9'] = {0x06,0x49,0x49,0x29,0x1E},
    ['A'] = {0x7E,0x11,0x11,0x11,0x7E},
    ['B'] = {0x7F,0x49,0x49,0x49,0x36},
    ['C'] = {0x3E,0x41,0x41,0x41,0x22},
    ['D'] = {0x7F,0x41,0x41,0x22,0x1C},
    ['E'] = {0x7F,0x49,0x49,0x49,0x41},
    ['F'] = {0x7F,0x09,0x09,0x09,0x01},
    ['G'] = {0x3E,0x41,0x49,0x49,0x7A},
    ['H'] = {0x7F,0x08,0x08,0x08,0x7F},
    ['I'] = {0x00,0x41,0x7F,0x41,0x00},
    ['J'] = {0x20,0x40,0x41,0x3F,0x01},
    ['K'] = {0x7F,0x08,0x14,0x22,0x41},
    ['L'] = {0x7F,0x40,0x40,0x40,0x40},
    ['M'] = {0x7F,0x02,0x0C,0x02,0x7F},
    ['N'] = {0x7F,0x04,0x08,0x10,0x7F},
    ['O'] = {0x3E,0x41,0x41,0x41,0x3E},
    ['P'] = {0x7F,0x09,0x09,0x09,0x06},
    ['Q'] = {0x3E,0x41,0x51,0x21,0x5E},
    ['R'] = {0x7F,0x09,0x19,0x29,0x46},
    ['S'] = {0x46,0x49,0x49,0x49,0x31},
    ['T'] = {0x01,0x01,0x7F,0x01,0x01},
    ['U'] = {0x3F,0x40,0x40,0x40,0x3F},
    ['V'] = {0x1F,0x20,0x40,0x20,0x1F},
    ['W'] = {0x3F,0x40,0x38,0x40,0x3F},
    ['X'] = {0x63,0x14,0x08,0x14,0x63},
    ['Y'] = {0x07,0x08,0x70,0x08,0x07},
    ['Z'] = {0x61,0x51,0x49,0x45,0x43},
    ['a'] = {0x20,0x54,0x54,0x54,0x78},
    ['b'] = {0x7F,0x48,0x44,0x44,0x38},
    ['c'] = {0x38,0x44,0x44,0x44,0x20},
    ['d'] = {0x38,0x44,0x44,0x48,0x7F},
    ['e'] = {0x38,0x54,0x54,0x54,0x18},
    ['f'] = {0x08,0x7E,0x09,0x01,0x02},
    ['g'] = {0x0C,0x52,0x52,0x52,0x3E},
    ['h'] = {0x7F,0x08,0x04,0x04,0x78},
    ['i'] = {0x00,0x44,0x7D,0x40,0x00},
    ['j'] = {0x20,0x40,0x44,0x3D,0x00},
    ['k'] = {0x7F,0x10,0x28,0x44,0x00},
    ['l'] = {0x00,0x41,0x7F,0x40,0x00},
    ['m'] = {0x7C,0x04,0x18,0x04,0x7C},
    ['n'] = {0x7C,0x08,0x04,0x04,0x78},
    ['o'] = {0x38,0x44,0x44,0x44,0x38},
    ['p'] = {0x7C,0x14,0x14,0x14,0x08},
    ['q'] = {0x08,0x14,0x14,0x18,0x7C},
    ['r'] = {0x7C,0x08,0x04,0x04,0x08},
    ['s'] = {0x48,0x54,0x54,0x54,0x20},
    ['t'] = {0x04,0x3F,0x44,0x40,0x20},
    ['u'] = {0x3C,0x40,0x40,0x40,0x3C},
    ['v'] = {0x1C,0x20,0x40,0x20,0x1C},
    ['w'] = {0x3C,0x40,0x30,0x40,0x3C},
    ['x'] = {0x44,0x28,0x10,0x28,0x44},
    ['y'] = {0x0C,0x50,0x50,0x50,0x3C},
    ['z'] = {0x44,0x64,0x54,0x4C,0x44},
};

//=========== Draw Pixel ===========
static void _draw_pixel(int x, int y, uint16_t color)
{
    if (x < 0 || x >= LCD_WIDTH || y < 0 || y >= LCD_HEIGHT) return;
    esp_lcd_panel_draw_bitmap(panel_handle, x, y, x + 1, y + 1, &color);
}

//=========== Draw Text ===========
static void _display_draw_text(const char *text, int x, int y, uint16_t color, uint16_t bg, int scale)
{
    int len = strlen(text);
    int cursor_x = x + (len - 1) * 6 * scale;  // start from right, draw left

    for (int i = 0; text[i]; i++) {
        uint8_t c = (uint8_t)text[i];
        if (c >= 128) { cursor_x -= 6 * scale; continue; }

        for (int col = 0; col < 5; col++) {
            uint8_t bits = font5x7[c][4 - col];  // mirror columns within glyph
            for (int row = 0; row < 7; row++) {
                uint16_t pixel = (bits & (1 << row)) ? color : bg;
                for (int sy = 0; sy < scale; sy++) {
                    for (int sx = 0; sx < scale; sx++) {
                        _draw_pixel(cursor_x + col * scale + sx, y + row * scale + sy, pixel);
                    }
                }
            }
        }
        cursor_x -= 6 * scale;  // move left for next char

        // let FreeRTOS/IDLE run
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

//=========== Draw Horizontal Line ===========
static void _display_draw_hline(int x, int y, int w, uint16_t color)
{
    for (int i = x; i < x + w; i++) _draw_pixel(i, y, color);
}

//=========== Draw Vertical Line ===========
static void _display_draw_vline(int x, int y, int h, uint16_t color)
{
    for (int i = y; i < y + h; i++) _draw_pixel(x, i, color);
}

//=========== Draw Filled Rect ===========
static void _display_draw_rect(int x, int y, int w, int h, uint16_t color)
{
    for (int row = y; row < y + h; row++)
        for (int col = x; col < x + w; col++)
            _draw_pixel(col, row, color);
}


//=============== WIDGETS ===============================
// Each widget takes (int x, int y) as its top-left anchor.
// To move a widget just change where it is called in the page layout.

static void _widget_clock(int x, int y)
{
    char time_str[12];
    _time_get_string(time_str, sizeof(time_str));
    int text_w  = strlen(time_str) * 6 * 3;
    int start_x = x - text_w / 2;
    _display_draw_text(time_str, start_x, y, UI_WHITE, UI_BG, 3);
}

static void _widget_clock_update(int x, int y) //update just the clock text without redrawing the whole widget (to avoid flicker)
{
    if(update_time_flag == 1){
        char time_str[12];
        _time_get_string(time_str, sizeof(time_str));

        int text_w  = strlen(time_str) * 6 * 3;
        int text_h  = 7 * 3;
        int start_x = x - text_w / 2;

        // erase only the clock area
        _display_draw_rect(start_x, y, text_w + 4, text_h + 4, UI_BG);

        // redraw clock
        _display_draw_text(time_str, start_x, y, UI_WHITE, UI_BG, 3);
        update_time_flag = 0; // reset flag until next update is needed
    }
}

//=========== Widget: Date ===========
static void _widget_date(int x, int y)
{
    char date_str[32];
    _date_get_string(date_str, sizeof(date_str));
    int text_w  = strlen(date_str) * 6;
    int start_x = x - text_w / 2;
    _display_draw_text(date_str, start_x, y, UI_WHITE, UI_BG, 1);
}


//=========== Widget: Weather ===========
static void _widget_weather(int x, int y)
{
    _display_draw_text("WEATHER",   x, y,      UI_ACCENT, UI_BG, 1);
    _display_draw_hline(x, y + 9, 50, UI_ACCENT);
    _display_draw_text("Montreal",  x, y + 14, UI_WHITE,  UI_BG, 1);
    _display_draw_text("-6 C",      x, y + 26, UI_WHITE,  UI_BG, 2);
    _display_draw_text("Feels -12", x, y + 44, UI_GRAY,   UI_BG, 1);
    _display_draw_text("Snowing",   x, y + 54, UI_GRAY,   UI_BG, 1);

    _display_draw_text("MON", x,      y + 70, UI_GRAY,  UI_BG, 1);
    _display_draw_text("-4",  x,      y + 80, UI_WHITE, UI_BG, 1);
    _display_draw_text("TUE", x + 30, y + 70, UI_GRAY,  UI_BG, 1);
    _display_draw_text("-2",  x + 30, y + 80, UI_WHITE, UI_BG, 1);
    _display_draw_text("WED", x + 60, y + 70, UI_GRAY,  UI_BG, 1);
    _display_draw_text("1",   x + 60, y + 80, UI_WHITE, UI_BG, 1);
}


//=============== WEATHER ICONS ===============================
// icon types
#define WEATHER_SUNNY        0
#define WEATHER_CLOUDY       1
#define WEATHER_RAINING      2
#define WEATHER_SNOWING      3
#define WEATHER_CLEAR_NIGHT  4

static void _draw_weather_icon(int x, int y, int type)
{
    switch (type)
    {
        case WEATHER_SUNNY:
        {
            // sun circle center
            _display_draw_rect(x + 12, y + 12, 12, 12, UI_YELLOW);
            // rays top/bottom/left/right
            _display_draw_rect(x + 15, y,      6,  6,  UI_YELLOW);
            _display_draw_rect(x + 15, y + 30, 6,  6,  UI_YELLOW);
            _display_draw_rect(x,      y + 15, 6,  6,  UI_YELLOW);
            _display_draw_rect(x + 30, y + 15, 6,  6,  UI_YELLOW);
            // rays diagonal
            _display_draw_rect(x + 3,  y + 3,  3,  3,  UI_YELLOW);
            _display_draw_rect(x + 30, y + 3,  3,  3,  UI_YELLOW);
            _display_draw_rect(x + 3,  y + 30, 3,  3,  UI_YELLOW);
            _display_draw_rect(x + 30, y + 30, 3,  3,  UI_YELLOW);
            break;
        }

        case WEATHER_CLOUDY:
        {
            // cloud body
            _display_draw_rect(x + 6,  y + 15, 24, 12, UI_GRAY);
            _display_draw_rect(x + 12, y + 9,  12,  9, UI_GRAY);
            // small sun peeking top right
            _display_draw_rect(x + 27, y + 3,  3,  3,  UI_YELLOW);
            _display_draw_rect(x + 30, y + 6,  3,  3,  UI_YELLOW);
            _display_draw_rect(x + 27, y + 9,  3,  3,  UI_YELLOW);
            break;
        }

        case WEATHER_RAINING:
        {
            // cloud
            _display_draw_rect(x + 6,  y + 6,  24, 12, UI_GRAY);
            _display_draw_rect(x + 12, y,      12,  9, UI_GRAY);
            // rain drops
            _display_draw_rect(x + 9,  y + 24, 3,  6,  UI_ACCENT);
            _display_draw_rect(x + 18, y + 27, 3,  6,  UI_ACCENT);
            _display_draw_rect(x + 27, y + 24, 3,  6,  UI_ACCENT);
            break;
        }

        case WEATHER_SNOWING:
        {
            // cloud
            _display_draw_rect(x + 6,  y + 6,  24, 12, UI_GRAY);
            _display_draw_rect(x + 12, y,      12,  9, UI_GRAY);
            // snowflake dots
            _display_draw_rect(x + 9,  y + 24, 3,  3,  UI_WHITE);
            _display_draw_rect(x + 15, y + 27, 3,  3,  UI_WHITE);
            _display_draw_rect(x + 9,  y + 30, 3,  3,  UI_WHITE);
            _display_draw_rect(x + 21, y + 24, 3,  3,  UI_WHITE);
            _display_draw_rect(x + 27, y + 27, 3,  3,  UI_WHITE);
            _display_draw_rect(x + 21, y + 30, 3,  3,  UI_WHITE);
            // snowflake crosses
            _display_draw_rect(x + 12, y + 27, 3,  3,  UI_WHITE);
            _display_draw_rect(x + 24, y + 27, 3,  3,  UI_WHITE);
            break;
        }

        case WEATHER_CLEAR_NIGHT:
        {
            // crescent moon
            _display_draw_rect(x + 9,  y + 6,  12, 24, UI_YELLOW);
            _display_draw_rect(x + 15, y + 3,  6,  30, UI_YELLOW);
            // carve out right side to make crescent
            _display_draw_rect(x + 18, y + 6,  12, 24, UI_BG);
            _display_draw_rect(x + 15, y + 9,  12, 18, UI_BG);
            // stars
            _display_draw_rect(x + 3,  y + 3,  3,  3,  UI_WHITE);
            _display_draw_rect(x + 30, y + 9,  3,  3,  UI_WHITE);
            _display_draw_rect(x + 27, y + 27, 3,  3,  UI_WHITE);
            break;
        }
    }
}


//=========== Widget: Agenda ===========
static void _widget_agenda(int x, int y)
{
    _display_draw_text("AGENDA", x, y, UI_ACCENT, UI_BG, 1);
    _display_draw_hline(x, y + 9, 78, UI_ACCENT);

    _display_draw_rect(x,     y + 14, 5, 5, UI_GREEN);
    _display_draw_text("9:00  Standup", x + 8, y + 13, UI_WHITE, UI_BG, 1);

    _display_draw_rect(x,     y + 28, 5, 5, UI_GREEN);
    _display_draw_text("12:00 Lunch",   x + 8, y + 27, UI_WHITE, UI_BG, 1);

    _display_draw_rect(x,     y + 42, 5, 5, UI_WARN);
    _display_draw_text("14:00 Review",  x + 8, y + 41, UI_WHITE, UI_BG, 1);

    _display_draw_rect(x,     y + 56, 5, 5, UI_GRAY);
    _display_draw_text("17:00 Gym",     x + 8, y + 55, UI_GRAY,  UI_BG, 1);

    _display_draw_rect(x,     y + 70, 5, 5, UI_GRAY);
    _display_draw_text("19:00 Dinner",  x + 8, y + 69, UI_GRAY,  UI_BG, 1);
}

//=========== Widget: Stats Bar (HUM + UV only) ===========
static void _widget_stats(int x, int y)
{
    _display_draw_text("HUM",   x,       y,      UI_GRAY,  UI_BG, 1);
    _display_draw_text("62PCT", x,       y + 10, UI_WHITE, UI_BG, 1);
    _display_draw_text("UV",    x + 80,  y,      UI_GRAY,  UI_BG, 1);
    _display_draw_text("3",     x + 80,  y + 10, UI_WHITE, UI_BG, 1);
}

//=========== Widget: Packers ===========
static void _widget_packers(int x, int y)
{
    _display_draw_text("PACKERS",   x, y, UI_PACKER_GREEN, UI_BG, 1);
    _display_draw_hline(x, y + 9, 50, UI_PACKER_GREEN);
    _display_draw_text("NFL 2025-26",  x, y + 14, UI_WHITE,  UI_BG, 1);
    _display_draw_text("11-6",      x, y + 26, UI_WHITE,  UI_BG, 2);
    _display_draw_text("NFC north", x, y + 44, UI_GRAY,   UI_BG, 1);
    _display_draw_text("3rd place",   x, y + 54, UI_GRAY,   UI_BG, 1);

    _display_draw_text("LIO", x,      y + 70, UI_GRAY,  UI_BG, 1);
    _display_draw_text("W",  x,      y + 80, UI_PACKER_GREEN, UI_BG, 1);
    _display_draw_text("VIK", x + 30, y + 70, UI_GRAY,  UI_BG, 1);
    _display_draw_text("L",  x + 30, y + 80, UI_RED, UI_BG, 1);
    _display_draw_text("BEA", x + 60, y + 70, UI_GRAY,  UI_BG, 1);
    _display_draw_text("W",   x + 60, y + 80, UI_PACKER_GREEN, UI_BG, 1);
}

//=========== Widget: Ferrari ===========
static void _widget_ferrari(int x, int y)
{
    _display_draw_text("FERRARI",   x, y, UI_RED, UI_BG, 1);
    _display_draw_hline(x, y + 9, 50, UI_RED);
    _display_draw_text("F1 2025",  x, y + 14, UI_WHITE,  UI_BG, 1);
    _display_draw_text("2nd",      x, y + 26, UI_WARN,  UI_BG, 2);
    _display_draw_text("Constructor", x, y + 44, UI_GRAY,   UI_BG, 1);
    _display_draw_text("187 pts",   x, y + 54, UI_WHITE,   UI_BG, 1);

    _display_draw_text("LEC", x,      y + 70, UI_GRAY,  UI_BG, 1);
    _display_draw_text("P2",  x,      y + 80, UI_ACCENT, UI_BG, 1);
    _display_draw_text("SAI", x + 30, y + 70, UI_GRAY,  UI_BG, 1);
    _display_draw_text("P4",  x + 30, y + 80, UI_ACCENT, UI_BG, 1);
}

//=========== Widget: News ===========
static void _widget_news(int x, int y, const char *article, const char *headline)
{
    // === ERASE AREA FIRST ===
    int width  = LCD_WIDTH;   // full width (or tune later)
    int height = 40;          // adjust based on your text size

    _display_draw_rect(0, y, width, height, UI_BG);

    // === DRAW NEW CONTENT ===
    _display_draw_text(article, (LCD_WIDTH / 2) - 20, y, UI_ACCENT, UI_BG, 1);
    _display_draw_text(headline, 10, y + 15, UI_WHITE, UI_BG, 1);
}


//=========== Widget: Button ===========
static void _widget_button(int x, int y, int w, int h, uint16_t color, const char *label)
{
    _display_draw_rect(x, y, w, h, color);
    int label_x = x + (w - strlen(label) * 6) / 5.5;
    int label_y = y + (h - 7) / 2.5;
    _display_draw_text(label, label_x, label_y, UI_WHITE, color, 2);
}


//================ POSTIONS ==============================
// Each postion depends on users prefs
// certain posistion are not changeable like time, date, news.

static void _display_pos_top_middle(){
    
    _widget_clock(DISPLAY_POSITION_TOP_MIDDLE_X_LV0, DISPLAY_POSITION_TOP_MIDDLE_Y_LV0);
    _widget_date(DISPLAY_POSITION_TOP_MIDDLE_X_LV1, DISPLAY_POSITION_TOP_MIDDLE_Y_LV1);
    
}


static void _display_pos_mid_left(int x, int y){
        _widget_weather(x, y);
}

static void _display_pos_mid_center(int x, int y){
        _widget_ferrari(x, y);
}

static void _display_pos_mid_right(int x, int y){
        _widget_packers(x, y);
}


static void _display_pos_single_point(int x , int y){
    
    static bool seeded = false;
    if (!seeded) {
        srand((unsigned int)esp_timer_get_time());
        seeded = true;
    }

    int current_fact = rand() % FACTS_COUNT;

     if(update_fun_fact_flag == 1){
        _display_draw_hline(0, 150, LCD_WIDTH, UI_GRAY);
        _widget_news(x, y, facts[current_fact][0], facts[current_fact][1]);
        _display_draw_hline(0, 190, LCD_WIDTH, UI_GRAY);
        update_fun_fact_flag = 0; // reset flag to avoid repeated updates
    }
}


static void _display_pos_bottom_middle(){
    
    _widget_button(
        DISPLAY_POSITION_BOTTOM_MIDDLE_X,
        DISPLAY_POSITION_BOTTOM_MIDDLE_Y,
        70, 30,
        UI_ORANGE,
        "ORBIT"
    );
    
}

//=============== PAGE LAYOUTS ===============================
// This is where you arrange widgets on screen.
// Move any widget call to a different x,y to reposition it.
// Swap widgets between pages freely.

//=========== Page 1 ===========
void _draw_page1_task(void *vpParam)
{
    _display_clear(UI_BG);   // only once at startup

    _display_pos_top_middle();
    // draw static stuff once
    _display_draw_hline(0, 50, LCD_WIDTH, UI_WHITE);
    _display_pos_mid_left(DISPLAY_POSITION_MIDDLE_LEFT_X, DISPLAY_POSITION_MIDDLE_LEFT_Y);
    _display_pos_mid_center(DISPLAY_POSITION_MIDDLE_CENTER_X, DISPLAY_POSITION_MIDDLE_CENTER_Y);
    _display_pos_mid_right(DISPLAY_POSITION_MIDDLE_RIGHT_X, DISPLAY_POSITION_MIDDLE_RIGHT_Y);
    _display_pos_bottom_middle();

    while (1)
    {

        // update only changing stuff
        _widget_clock_update(DISPLAY_POSITION_TOP_MIDDLE_X_LV0,
                             DISPLAY_POSITION_TOP_MIDDLE_Y_LV0);

        _display_pos_single_point(DISPLAY_POSITION_SINGLE_POINT_X,
                                  DISPLAY_POSITION_SINGLE_POINT_Y);

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

//=========== Main UI ===========
void _display_main_UI(void)
{

    while (1)
    {
        int tx, ty;
        if (_display_touch_read(&tx, &ty))
        {

            if ((tx>240 && tx< 299) && (ty > 90 && ty < 140))
            {
                printf("Hello\n"); // for debugging, shows touch coordinates
                
                //call on settings
                if(is_settings_portal_on == 0){
                    if(_network_settings_mode() == ORBIT_OK)start_dns_server(); // start captive portal DNS server to redirect to settings page
                    is_settings_portal_on = 1; // set flag to indicate portal is active
                }else{
                    stop_dns_server(); // stop DNS server when exiting settings
                    is_settings_portal_on = 0; // reset flag
                }

            }else{
                printf("Touched at (%d, %d)\n", tx, ty); // for debugging, shows touch coordinates
            }

            vTaskDelay(pdMS_TO_TICKS(300));
        }

        vTaskDelay(pdMS_TO_TICKS(20));
    }
}