/* See LICENSE file for license details. */
#define _XOPEN_SOURCE 500
#if HAVE_SHADOW_H
#include <shadow.h>
#endif

#include <ctype.h>
#include <errno.h>
#include <pwd.h>
#include <unistd.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <crypt.h>
#include <sys/types.h>
#include <X11/keysym.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#if HAVE_BSD_AUTH
#include <login_cap.h>
#include <bsd_auth.h>
#endif

#include "config.h"

typedef struct
{
	int screen;
	Window root, win;
	Pixmap pmap;
	unsigned long colors[2];
} Lock;

static Lock **locks;
static int nscreens;
static Bool running = True;

static void die(const char *errstr, ...)
{
	va_list ap;

	va_start(ap, errstr);
	vfprintf(stderr, errstr, ap);
	va_end(ap);
	exit(EXIT_FAILURE);
}

#ifdef __linux__
#include <fcntl.h>

static void dontkillme(void)
{
	int fd;

	fd = open("/proc/self/oom_score_adj", O_WRONLY);
	if (fd < 0 && errno == ENOENT)
		return;

	if (fd < 0 || write(fd, "-1000\n", 6) != 6 || close(fd) != 0)
		die("cannot disable the out-of-memory killer for this process\n");
}
#endif

#ifndef HAVE_BSD_AUTH
static const char *getpw(void)
{                               /* only run as root */
	const char *rval;
	struct passwd *pw;

	errno = 0;
	pw = getpwuid(getuid());
	if (!pw)
	{
		if (errno)
			die("tomato-lock: getpwuid: %s\n", strerror(errno));
		else
			die("tomato-lock: cannot retrieve password entry (make sure to suid or sgid tomato-lock)\n");
	}
	rval = pw->pw_passwd;

#if HAVE_SHADOW_H
	if (rval[0] == 'x' && rval[1] == '\0')
	{
		struct spwd *sp;
		sp = getspnam(getenv("USER"));
		if (!sp)
			die("tomato-lock: cannot retrieve shadow entry (make sure to suid or sgid tomato-lock)\n");
		rval = sp->sp_pwdp;
	}
#endif

	/* drop privileges */
	if (geteuid() == 0
		&& ((getegid() != pw->pw_gid && setgid(pw->pw_gid) < 0) || setuid(pw->pw_uid) < 0))
		die("tomato-lock: cannot drop privileges\n");
	return rval;
}
#endif

static void
#ifdef HAVE_BSD_AUTH
readpw(Display * dpy)
#else
readpw(Display * dpy, const char *pws)
#endif
{
	char buf[32], passwd[256];
	int num, screen;
	unsigned int len, llen;
	KeySym ksym;
	XEvent ev;

	len = llen = 0;
	running = True;

	/* As "tomato-lock" stands for "Simple X display locker", the DPMS settings
	 * had been removed and you can set it with "xset" or some other
	 * utility. This way the user can easily set a customized DPMS
	 * timeout. */
	while (running && !XNextEvent(dpy, &ev))
	{
		if (ev.type == KeyPress)
		{
			buf[0] = 0;
			num = XLookupString(&ev.xkey, buf, sizeof buf, &ksym, 0);
			if (IsKeypadKey(ksym))
			{
				if (ksym == XK_KP_Enter)
					ksym = XK_Return;
				else if (ksym >= XK_KP_0 && ksym <= XK_KP_9)
					ksym = (ksym - XK_KP_0) + XK_0;
			}
			if (IsFunctionKey(ksym) || IsKeypadKey(ksym)
				|| IsMiscFunctionKey(ksym) || IsPFKey(ksym) || IsPrivateKeypadKey(ksym))
				continue;
			switch (ksym)
			{
			  case XK_Return:
				  passwd[len] = 0;
#ifdef HAVE_BSD_AUTH
				  running = !auth_userokay(getlogin(), NULL, "auth-xlock", passwd);
#else
				  running = ! !strcmp(crypt(passwd, pws), pws);
#endif
				  if (running)
					  XBell(dpy, 100);
				  len = 0;
				  break;
			  case XK_Escape:
				  len = 0;
				  break;
			  case XK_BackSpace:
				  if (len)
					  --len;
				  break;
			  default:
				  if (num && !iscntrl((int)buf[0]) && (len + num < sizeof passwd))
				  {
					  memcpy(passwd + len, buf, num);
					  len += num;
				  }
				  break;
			}
			if (llen == 0 && len != 0)
			{
				for (screen = 0; screen < nscreens; screen++)
				{
					XSetWindowBackground(dpy, locks[screen]->win, locks[screen]->colors[1]);
					XClearWindow(dpy, locks[screen]->win);
				}
			}
			else if (llen != 0 && len == 0)
			{
				for (screen = 0; screen < nscreens; screen++)
				{
					XSetWindowBackground(dpy, locks[screen]->win, locks[screen]->colors[0]);
					XClearWindow(dpy, locks[screen]->win);
				}
			}
			llen = len;
		}
		else
			for (screen = 0; screen < nscreens; screen++)
				XRaiseWindow(dpy, locks[screen]->win);
	}
}

static void unlockscreen(Display * dpy, Lock * lock)
{
	if (dpy == NULL || lock == NULL)
		return;

	XUngrabPointer(dpy, CurrentTime);
	XFreeColors(dpy, DefaultColormap(dpy, lock->screen), lock->colors, 2, 0);
	XFreePixmap(dpy, lock->pmap);
	XDestroyWindow(dpy, lock->win);

	free(lock);
}

static Lock *lockscreen(Display * dpy, int screen)
{
	char curs[] = { 0, 0, 0, 0, 0, 0, 0, 0 };
	unsigned int len;
	Lock *lock;
	XColor color, dummy;
	XSetWindowAttributes wa;
	Cursor invisible;

	if (dpy == NULL || screen < 0)
	{
		return NULL;
	}

	lock = malloc(sizeof(Lock));
	if (lock == NULL)
	{
		return NULL;
	}

	lock->screen = screen;

	lock->root = RootWindow(dpy, lock->screen);

	/* init */
	wa.override_redirect = 1;
	wa.background_pixel = BlackPixel(dpy, lock->screen);
	lock->win =
		XCreateWindow(dpy, lock->root, 0, 0, DisplayWidth(dpy, lock->screen),
					  DisplayHeight(dpy, lock->screen), 0, DefaultDepth(dpy, lock->screen),
					  CopyFromParent, DefaultVisual(dpy, lock->screen),
					  CWOverrideRedirect | CWBackPixel, &wa);
	XAllocNamedColor(dpy, DefaultColormap(dpy, lock->screen), COLOR2, &color, &dummy);
	lock->colors[1] = color.pixel;
	XAllocNamedColor(dpy, DefaultColormap(dpy, lock->screen), COLOR1, &color, &dummy);
	lock->colors[0] = color.pixel;
	lock->pmap = XCreateBitmapFromData(dpy, lock->win, curs, 8, 8);
	invisible = XCreatePixmapCursor(dpy, lock->pmap, lock->pmap, &color, &color, 0, 0);
	XDefineCursor(dpy, lock->win, invisible);
	XMapRaised(dpy, lock->win);
	for (len = 1000; len; len--)
	{
		if (XGrabPointer
			(dpy, lock->root, False, ButtonPressMask | ButtonReleaseMask | PointerMotionMask,
			 GrabModeAsync, GrabModeAsync, None, invisible, CurrentTime) == GrabSuccess)
			break;
		usleep(1000);
	}
	if (running && (len > 0))
	{
		for (len = 1000; len; len--)
		{
			if (XGrabKeyboard(dpy, lock->root, True, GrabModeAsync, GrabModeAsync, CurrentTime)
				== GrabSuccess)
				break;
			usleep(1000);
		}
	}

	running &= (len > 0);
	if (!running)
	{
		unlockscreen(dpy, lock);
		lock = NULL;
	}
	else
		XSelectInput(dpy, lock->root, SubstructureNotifyMask);

	return lock;
}

static void show_usage(void)
{
	die("usage: tomato-lock [-t XXX]\n");
}

static void show_version(void)
{
	//die("tomato-lock-%s\n", VERSION);
}

#ifndef HAVE_BSD_AUTH
static void take_a_rest(char* pws)
#else
static void take_a_rest(void)
#endif
{
	Display *dpy;
	int screen;

	running = True;

	if (!getpwuid(getuid()))
		die("tomato-lock: no passwd entry for you\n");

	if (!(dpy = XOpenDisplay(0)))
		die("tomato-lock: cannot open display\n");
	/* Get the number of screens in display "dpy" and blank them all. */
	nscreens = ScreenCount(dpy);
	locks = malloc(sizeof(Lock *) * nscreens);
	if (locks == NULL)
		die("tomato-lock: malloc: %s\n", strerror(errno));
	int nlocks = 0;
	for (screen = 0; screen < nscreens; screen++)
	{
		if ((locks[screen] = lockscreen(dpy, screen)) != NULL)
		{
			nlocks++;
		}
	}
	XSync(dpy, False);

	/* Did we actually manage to lock something? */
	if (nlocks == 0)
	{                           // nothing to protect
		free(locks);
		XCloseDisplay(dpy);
		exit(EXIT_FAILURE);
	}

	/* Everything is now blank. Now wait for the correct password. */
#ifdef HAVE_BSD_AUTH
	readpw(dpy);
#else
	readpw(dpy, pws);
#endif

	/* Password ok, unlock everything and quit. */
	for (screen = 0; screen < nscreens; screen++)
		unlockscreen(dpy, locks[screen]);

	free(locks);
	XCloseDisplay(dpy);
}

int main(int argc, char **argv)
{
	int i         = 0;
	int ch        = 0;
	int work_time = 0;
	char cmd[256] = {0};
#ifndef HAVE_BSD_AUTH
	char* pws;
#endif

	while ((ch = getopt(argc, argv, "t:hv")) != -1)
	{
		switch (ch)
		{
		case 't':
			work_time = atoi(optarg);
			break;
		case 'h':
			show_usage();
			break;
		case 'v':
			show_version();
			break;
		default:
			break;
		}
	}

#ifdef __linux__
	dontkillme();
#endif

#ifndef HAVE_BSD_AUTH
	pws = getpw();
#endif

	// daemonize
	daemon(1, 0);

	if (0 == work_time)
	{
		work_time = DFT_WORK_TIME;
	}
	work_time *= 60; // min -> second

	while (1)
	{
		for (i = 0; i < work_time; ++i)
		{
			memset(cmd, 0 , strlen(cmd));
			printf("%d\n", i);
			sprintf(cmd, "export TOMATO_TIME=%d\n", work_time - i);
			system(cmd);
			sleep(1);
		}
#ifndef HAVE_BSD_AUTH
		take_a_rest(pws);
#else
		take_a_rest();
#endif
	}

	return 0;
}
