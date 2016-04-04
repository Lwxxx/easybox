#!/usr/bin/env python

import os
import sys
import shutil
from git import Repo
import win32com
from win32com.client import Dispatch, constants

GIT_WORD_DIFF_VERSION = '0.0.1'


def show_usage():
    print '\n Usage: git-word-diff [options] <commit> [--] [<path> ...] \
    \n \
    \n OPTIONS: \
    \n \
    \n -v, --version      show version \
    \n -h, --help         show this usage message \
    '
    sys.exit(0)


def show_version():
    print '\n git-word-diff version: %s' % (GIT_WORD_DIFF_VERSION)
    sys.exit(0)


def find_git_root():
    ''' if we're in a git repository, return it's root path, else
    return None. '''

    prev_path = None

    while True:
        curr_path = os.getcwd()

        # we have already come to root path
        if prev_path == curr_path:
            return None

        # found a .git
        if '.git' in os.listdir('.'):
            return curr_path

        # goto upper directory
        prev_path = curr_path
        os.chdir('..')


def main(args):
    # check if current directory is in a git repository
    git_root = find_git_root()
    if git_root is None:
        print 'fatal: Not a git repository'
        sys.exit(-1)

    # parse argument list
    if '--help' in args or '-h' in args:
        show_usage()

    if '--version' in args or '-v' in args:
        show_version()

    args = [item for item in args if '-' != item[0]]
    if 0 == len(args):
        show_usage()

    rev_1     = 'HEAD'
    rev_2     = args[0]
    file_list = args[1:]

    # create git repository instance
    repo = Repo(git_root)
    orig_head = repo.head.ref

    if 0 == len(file_list):
        #+TODO: support diff all different files
        print 'ERROR: not target file provided!'
        show_usage()

    # control word by COM
    word = win32com.client.DispatchEx('Word.Application')
    word.Visible = 0
    word.DisplayAlerts = 0

    # compare documents
    for item in file_list:
        file_1_path = git_root + '\\__1'
        file_2_path = git_root + '\\__2'

        # copy doc 1
        repo.git.checkout(rev_1)
        shutil.copy(item, file_1_path)
        repo.git.checkout(orig_head)

        # copy doc 2
        repo.git.checkout(rev_2)
        shutil.copy(item, file_2_path)
        repo.git.checkout(orig_head)

        # open doc 1
        print 'open %s @ %s' % (item, rev_1)
        doc_1 = word.Documents.Open(file_1_path)

        # open doc 2
        print 'open %s @ %s' % (item, rev_2)
        doc_2 = word.Documents.Open(file_2_path)

        # compare doc
        print 'compare ...'
        word.Application.CompareDocuments(doc_2, doc_1)

        # close docs and delete them
        doc_1.Close()
        doc_2.Close()
        os.remove(file_1_path)
        os.remove(file_2_path)

    # show word window
    word.Visible = 1


if __name__ == '__main__':
    main(sys.argv[1:])
