#!/usr/bin/env python

''' get terminal size
'''
def get_terminal_size():
    import os
    env = os.environ

    def ioctl_GWINSZ(fd):
        try:
            import fcntl, termios, struct, os
            cr = struct.unpack('hh', fcntl.ioctl(fd, termios.TIOCGWINSZ, '1234'))
        except:
            return
        return cr

    cr = ioctl_GWINSZ(0) or ioctl_GWINSZ(1) or ioctl_GWINSZ(2)
    if not cr:
        try:
            fd = os.open(os.ctermid(), os.O_RDONLY)
            cr = ioctl_GWINSZ(fd)
            os.close(fd)
        except:
            pass
        if not cr:
            cr = (env.get('LINES', 25), env.get('COLUMNS', 80))

    return int(cr[1]), int(cr[0])


''' convert byte to a human readable string
'''
def byte_to_pretty_str(data):
    if data > 1024:
        data = float(data) / 1024
        if data > 1024:
            data = float(data) / 1024
            if data > 1024:
                data = float(data) / 1024
                pretty_str = "%.2f GB" % (data)
            else:
                pretty_str = "%.2f MB" % (data)
        else:
            pretty_str = "%.2f KB" % (data)
    else:
        pretty_str = "%.2f B" % (data)

    return pretty_str
