#ifndef _KILO_PLATFORM_H_
#define _KILO_PLATFORM_H_

void kilo_display_init();
void kilo_draw_rows();
void kilo_draw_status_bar();
void kilo_draw_message_bar();
void kilo_refresh_screen();

void kilo_die();
void kilo_write(const char *s, int len);

#endif // _KILO_PLATFORM_H_

