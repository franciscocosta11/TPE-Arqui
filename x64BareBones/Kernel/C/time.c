#include <time.h>
#include <stdint.h>

static unsigned long ticks = 0;

#define CMOS_PORT_ADDRESS 0x70
#define CMOS_PORT_DATA 0x71

#define SECONDS_REGISTER 0x00
#define MINUTES_REGISTER 0x02
#define HOURS_REGISTER 0x04
#define DAY_REGISTER 0x07
#define MONTH_REGISTER 0x08
#define YEAR_REGISTER 0x09

extern uint8_t inb(uint16_t port);
extern void outb(uint16_t port, uint8_t value);

// Convierte BCD a decimal
static uint8_t bcd_to_decimal(uint8_t bcd) {
    return ((bcd >> 4) * 10) + (bcd & 0x0F);
}

// Lee un registro RTC específico
static uint8_t read_rtc_register(uint8_t reg) {
    outb(CMOS_PORT_ADDRESS, reg);
    return inb(CMOS_PORT_DATA);
}

void getSystemTime(char *buffer) {
    uint8_t second = bcd_to_decimal(read_rtc_register(SECONDS_REGISTER));
    uint8_t minute = bcd_to_decimal(read_rtc_register(MINUTES_REGISTER));
    uint8_t hour = bcd_to_decimal(read_rtc_register(HOURS_REGISTER));
    uint8_t day = bcd_to_decimal(read_rtc_register(DAY_REGISTER));
    uint8_t month = bcd_to_decimal(read_rtc_register(MONTH_REGISTER));
    uint8_t year = bcd_to_decimal(read_rtc_register(YEAR_REGISTER));
    
    // Formato: "DD/MM/YY HH:MM:SS"
    buffer[0] = '0' + (day / 10);
    buffer[1] = '0' + (day % 10);
    buffer[2] = '/';
    buffer[3] = '0' + (month / 10);
    buffer[4] = '0' + (month % 10);
    buffer[5] = '/';
    buffer[6] = '2';
    buffer[7] = '0';
    buffer[8] = '0' + (year / 10);
    buffer[9] = '0' + (year % 10);
    buffer[10] = ' ';
    buffer[11] = '0' + (hour / 10);
    buffer[12] = '0' + (hour % 10);
    buffer[13] = ':';
    buffer[14] = '0' + (minute / 10);
    buffer[15] = '0' + (minute % 10);
    buffer[16] = ':';
    buffer[17] = '0' + (second / 10);
    buffer[18] = '0' + (second % 10);
    buffer[19] = '\0';
}

void timer_handler() {
	ticks++;
}

int ticks_elapsed() {
	return ticks;
}

int seconds_elapsed() {
	return ticks / 18;
}

