#ifndef __TIMER_H__
#define __TIMER_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

void          timer_init       ();
unsigned long timer_millis_get ();
void          timer_delay      (double ms);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __TIMER_H__ */
