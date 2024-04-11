#ifndef _FRAME_BUFFER_SERIAL_PORT_LIB_H_
#define _FRAME_BUFFER_SERIAL_PORT_LIB_H_

typedef struct _FBCON_POSITION {
  INTN x;
  INTN y;
} FBCON_POSITION, *PFBCON_POSITION;

typedef struct _FBCON_COLOR {
  UINTN Foreground;
  UINTN Background;
} FBCON_COLOR, *PFBCON_COLOR;

#endif /* _FRAME_BUFFER_SERIAL_PORT_LIB_H_ */
