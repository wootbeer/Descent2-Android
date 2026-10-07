//
//
#include <jni.h>
#include "mouse.h"

extern void mouse_handler(short x, short y, bool down);

JNIEXPORT void JNICALL Java_wootbeer_descent2_DescentView_mouseHandler(JNIEnv *env, jclass thiz, jshort x,
												   jshort y, jboolean down) {
	mouse_handler(x, y, down);
}

JNIEXPORT void JNICALL Java_wootbeer_descent2_DescentView_mouseSetPos(JNIEnv *env, jclass thiz, jshort x, jshort y) {
	mouse_set_pos(x, y);
}
