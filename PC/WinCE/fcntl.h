/*
 *	On Windows/CE we need to have a dummy fcntl.h,
 *	because even though we #undef HAVE_FCNTL_H some of the
 *	code ignores this (e.g. socketmodule.c)
 */
