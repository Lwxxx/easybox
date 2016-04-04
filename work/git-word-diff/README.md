# git-word-diff

git-word-diff is a tool to compare word documents in git repository. git diff can only handle ascii files, actually, word is best tool for word documents compare, so git-word-diff just checkout different revisions of the document and control word to compare them.

## usage

```
Usage: git-word-diff [options] <commit> [--] [<path> ...]

OPTIONS:
-v, --version      show version
-h, --help         show this usage message
```

git-word-diff compare HEAD with <commit>, is no path was provided, it will compare all different doc/docx fils.

## dependence

- Python 2.x
- PyWin32
- GitPython
- Word (only office 2007 was tested)
