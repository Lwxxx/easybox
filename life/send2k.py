#!/usr/bin/env python
# -*- coding:utf-8 -*-

'''
send2k: Send e-book/document To Kindle

Copyright (C) 2014 by Li Wenxiang <lwx169@gmail.com>

'''

import os, sys
import pickle
import base64
import smtplib
import mimetypes
from email.mime.text import MIMEText
from email.mime.image import MIMEImage
from email.mime.multipart import MIMEMultipart

KINDLE_ACCOUNT_INFO_PATH=os.path.expanduser('~/.send2k_info')

COLOR_RED    = 31
COLOR_GREEN  = 32
COLOR_YELLOE = 33
COLOR_BLUE   = 34


def colorText(text, colorCode):
    return '\033[%dm%s\033[0m' % (colorCode, text)


def userInfoConfig():
    _userInfo = {}
    print colorText('send2k config ...\n', COLOR_GREEN)
    _userInfo['usmtp']      = raw_input('SMTP Server: ')
    _userInfo['uemail']     = raw_input('Your Email Address: ')
    _userInfo['upasswd']    = raw_input('Your Password: ')
    _userInfo['kemail']     = raw_input('Kindle Email Address: ')

    print colorText("\nNotice: send2k save all your info by pickle, that's not safe !!!\n"
                    , COLOR_RED)
    _userInfo['savepasswd'] = raw_input(colorText('Save Password ? (y/n) ', COLOR_GREEN))
    while 'y' != _userInfo['savepasswd'] and 'n' != _userInfo['savepasswd']:
        _userInfo['savepasswd'] = raw_input(colorText('Save Password ? (y/n) ', COLOR_GREEN))

    print ''
    return _userInfo


def sendEmailWithDoc(userInfo, password, docList):

    emailMsg = MIMEMultipart()
    emailMsg.set_charset('utf8')
    emailMsg['from'] = userInfo['uemail']
    emailMsg['to'] = userInfo['kemail']
    emailMsg['subject'] = 'kindle'

    for fileName in docList:
        ctype, encoding = mimetypes.guess_type(fileName)
        if ctype is None or encoding is not None:
            ctype = 'application/octet-stream'
        manitype, subtype = ctype.split('/', 1)
        print colorText('Add %s' % (fileName), COLOR_GREEN)
        attachFile = open(fileName, 'rb')
        attachment = MIMEText(attachFile.read(), 'base64', 'UTF-8')
        attachFile.close()
        baseName = os.path.basename(fileName)
        attachment.add_header('Content-Disposition', 'attachment', filename= '=?utf-8?b?'
                              + base64.b64encode(baseName.encode('UTF-8')) + '?=') 
        emailMsg.attach(attachment)

    smtpServer = smtplib.SMTP(userInfo['usmtp'])
    print colorText('\nLogin...', COLOR_GREEN)
    try:
        smtpServer.login(userInfo['uemail'], userPassword)
    except Exception, e:
        print colorText('Login Failed', COLOR_RED), e

    print colorText('Send Email...', COLOR_GREEN)
    try:
        smtpServer.sendmail(emailMsg['from'], emailMsg['to'], emailMsg.as_string())
    except Exception, e:
        print colorText('Send Failed', COLOR_RED), e

    print colorText('Done!', COLOR_GREEN)
    smtpServer.close()


if __name__ == '__main__':

    reload(sys)
    sys.setdefaultencoding('utf8')

    docList = sys.argv[1:]
    usage = 'send2k FILE1 FILE2 ...'
    if len(sys.argv) < 2:
        print usage
        sys.exit(-1)

    cfgFlag = False
    reCfgFlag = False
    if '--reconfig' == sys.argv[1]:
        docList = docList[1:]
        reCfgFlag = True

    userInfo = None
    infoFile = None
    try:
        infoFile = open(KINDLE_ACCOUNT_INFO_PATH, 'rb')
    except IOError:
        cfgFlag = True

    if infoFile is not None:
        try:
            userInfo = pickle.load(infoFile)
            infoFile.close()
        except Exception, e:
            cfgFlag = True
            print e

    if True == cfgFlag or True == reCfgFlag:
        userInfo = userInfoConfig()

    userPassword = userInfo['upasswd']
    while None == userPassword:
        userPassword = raw_input('Your Password: ')

    if reCfgFlag is False:
        sendEmailWithDoc(userInfo, userPassword, docList)

    if 'n' == userInfo['savepasswd']:
        userInfo['upasswd'] = None

    try:
        infoFile = open(KINDLE_ACCOUNT_INFO_PATH, 'wb')
    except IOError, e:
        print e
        sys.exit(-1)

    if infoFile:
        pickle.dump(userInfo, infoFile, -1)
        infoFile.close()
