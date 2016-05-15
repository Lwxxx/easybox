#!/usr/bin/env python

import re
import sys
import urllib2
from optparse import OptionParser


BING_URL   = r'http://bing.com'
RE_BG_URL  = r'g_img=\{url\:\s?["\'](.+?\.jpg)["\']'


''' get url of bing main page background image
'''
def parse_bing_bg_url():
    image_url = None
    req = urllib2.Request(BING_URL)
    req.add_header('User-agent', 'Mozilla/5.0 (X11; Ubuntu; Linux x86_64; rv:39.0) Gecko/20100101 Firefox/39.0')
    try:
        page_html = urllib2.urlopen(req).read()
    except Exception, e:
        print 'request for bing main page failed: ', e
        sys.exit(-1)

    # search backgroud image url by regexp
    bg_url_pattern = re.compile(RE_BG_URL)
    match = re.search(bg_url_pattern, str(page_html))
    if match is not None:
        image_url = match.group(1)
        image_url = image_url.replace(r'\/', '/')

    return image_url

''' http download
'''
def download_file(url, output):

    # set output filename
    if output is None:
        filename = url.split('/')[-1]
    else:
        filename = output

    # download file by http request
    req = urllib2.Request(url)
    req.add_header('User-agent', 'Mozilla/5.0 (X11; Ubuntu; Linux x86_64; rv:39.0) Gecko/20100101 Firefox/39.0')
    try:
        data = urllib2.urlopen(req).read()
    except Exception, e:
        print 'request for file down failed: ', e
        sys.exit(-1)

    try:
        output_file = open(filename, 'wb')
        output_file.write(data)
        output_file.close()
    except IOError, e:
        print 'write file failed: ', e
        sys.exit(-1)


if __name__ == '__main__':

    # parse cli options
    usage = 'Usage: bing_wallpaper [option]'
    option_parser = OptionParser(usage)
    option_parser.add_option('-p', '--print-url', dest = 'only_print',
                             action = 'store_true',
                             help = 'only print url, do not download')
    option_parser.add_option('-o', '--output', dest = 'output',
                             help = 'set output filename')

    (options, args) = option_parser.parse_args()
    only_print  = options.only_print
    output_file = options.output

    bing_bg_url = parse_bing_bg_url()
    if bing_bg_url is None:
        print 'could not found backgroud image on bing.com'
        sys.exit(-1)

    if only_print is True:
        print bing_bg_url
    else:
        download_file(bing_bg_url, output_file)
