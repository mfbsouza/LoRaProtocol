#ifndef __TIMER_INTERFACE_H__
#define __TIMER_INTERFACE_H__

typedef struct {
	void (*delay)(double milliseconds);
} TimerInterface_t;

#endif /* __TIMER_INTERFACE_H__ */
