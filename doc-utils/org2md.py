#!/usr/bin/env python

'''
org2md: convert org-mode docment to markdown format

Copyright (C) 2014 by Li Wenxiang <lwx169@gmail.com>

'''

import sys
import re
from optparse import OptionParser


RE_ORG_TABLE=r'^\s*\|.*\|.*'
RE_ORG_BLOCK_BEGIN=r'#\+BEGIN_.*?'
RE_ORG_BLOCK_END=r'#\+END_.*?'
RE_ORG_BOLD=r'(\*[^*]+?\*)'
RE_ORG_ITALIC=r'(/[^/]+?/)'
RE_ORG_UNDERLINE=r'(_[^_]+?_)'
RE_ORG_LINK=r'\[\[[^\[\]]+?\]\[[^\[\]]+?\]\]'
RE_ORG_IMAGE=r'\[\[file:.+?\]\]'

''' Common org-mode line object
'''
class OrgLineObj(object):

    def __init__(self, lineList, objType):
        self.lineList = lineList
        self.objType  = objType

    def export(self, outFile, outFormat):
        if 'markdown' == outFormat:
            self._export2Markdown(outFile)


    def _export2Markdown(self, outFile):
        markdownLine = self._org2md()
        try:
            outFile.write(markdownLine)
        except IOError, e:
            print 'Export Error: ', e
            sys.exit(-1)


    def _org2md(self):
        return ''.join(self.lineList)


    def show(self):
        for line in self.lineList:
            print '[%6s]: %s' % (self.objType, line),


''' org-mode text (single line) object
'''
class OrgTextLine(OrgLineObj):

    def __init__(self, line):
        super(OrgTextLine, self).__init__([line], 'text')
        self.text = self.lineList[0]
        self.level = 0
        self._parseLine()


    def _parseLine(self):
        for char in self.text:
            if '*' == char:
                self.level = self.level + 1
            else:
                break
        self.text = self.text[self.level:]

        if 0 != self.level:
            self.objType = "h%d" % self.level
            return

        # remove all white space at head
        preSpaceCount = 0
        for char in self.text:
            if ' ' == char or '\t' == char:
                preSpaceCount = preSpaceCount + 1
            else:
                break
        self.text = self.text[preSpaceCount:]



    def _org2md(self):
        markdownLine = ''
        orgText = self.text

        for i in range(self.level):
            markdownLine = markdownLine + '#'

        # replace bold
        matches = re.findall(RE_ORG_BOLD, orgText)
        for item in matches:
            orgItem = item
            markdownItem = '*' + item +  '*'
            orgText = orgText.replace(orgItem, markdownItem, 1)

        # replace italic
        matches = re.findall(RE_ORG_ITALIC, orgText)
        for item in matches:
            orgItem = item
            markdownItem = '*' + item[1:-1] + '*'
            orgText = orgText.replace(orgItem, markdownItem, 1)

        # replace underline
        matches = re.findall(RE_ORG_UNDERLINE, orgText)
        for item in matches:
            orgItem = item
            markdownItem = '++' + item[1:-1] + '++'
            orgText = orgText.replace(orgItem, markdownItem, 1)

        # replace link
        matches = re.findall(RE_ORG_LINK, orgText)
        for item in matches:
            orgItem = item
            subMatch = re.match(r'\[\[([^\[\]]+?)\]\[([^\[\]]+?)\]\]', item)
            if subMatch:
                link = subMatch.group(1)
                desc = subMatch.group(2)
                markdownItem = '[%s](%s)' % (desc, link)
                orgText = orgText.replace(orgItem, markdownItem, 1)

        # repalce image
        matches = re.findall(RE_ORG_IMAGE, orgText)
        for item in matches:
            orgItem = item
            imagePath = item.split("file:")[-1][:-2]
            markdownItem = "![img](%s)" % (imagePath)
            orgText = orgText.replace(orgItem, markdownItem, 1)

        markdownLine = markdownLine + orgText
        return markdownLine


''' org-mode table object
'''
class OrgTable(OrgLineObj):

    def __init__(self, lineList):
        super(OrgTable, self).__init__(lineList, 'table')


''' org-mode block object
'''
class OrgBlock(OrgLineObj):

    def __init__(self, lineList):
        super(OrgBlock, self).__init__(lineList, 'block')
        self.srcLanguage = ''
        self._parseBlock()


    def _parseBlock(self):
        match = re.match(r'#\+BEGIN_SRC\s+(.+)', self.lineList[0])
        if match:
            self.srcLanguage = match.group(1)

    def _org2md(self):
        self.lineList[0] = '```%s' % (self.srcLanguage)
        self.lineList[-1] = '```'
        return ''.join(self.lineList)


''' org-mode head object
'''
class OrgHead(object):

    def __init__(self, content):
        self.content = content
        self.headObj = {}
        self._parseText()


    def _parseText(self):

        for line in self.content:
            matchResult = re.match(r'(#\+.+?):(.*)' ,line)
            if matchResult:
                key = matchResult.group(1)[2:]
                value = matchResult.group(2)
                self.headObj[key] = value


    def show(self):

        for key in self.headObj:
            print '[  head]: ', key, '=>', self.headObj[key]


''' org-mode body object
'''
class OrgBody(object):

    def __init__(self, content):
        self.content = content
        self.bodyObj = []
        self._parseText()

    def _parseText(self):

        # Parse State
        parseState = {
            "table": False,
            "block": False
        }

        def isParsingMultiLineObj():
            for key in parseState:
                if True == parseState[key]:
                    return True

            return False

        def getMLOType():
            for key in parseState:
                if True == parseState[key]:
                    return key
            return None

        def beginMLOParse(objType):
            parseState[objType] = True

        def stopMLOParse(objType):
            parseState[objType] = False

        # Temp multiline object
        tmpMLO = []

        for line in self.content:

            if isParsingMultiLineObj():
                objType = getMLOType()

                if 'table' == objType:
                    if re.match(RE_ORG_TABLE, line):
                        tmpMLO.append(line)
                        continue
                    else:
                        multiLineObj = OrgTable(tmpMLO)
                        self._addBodyObj(multiLineObj)
                        stopMLOParse(objType)

                elif 'block' == objType:
                    if re.match(RE_ORG_BLOCK_END, line):
                        tmpMLO.append(line)
                        multiLineObj = OrgBlock(tmpMLO)
                        self._addBodyObj(multiLineObj)
                        stopMLOParse(objType)
                    else:
                        tmpMLO.append(line)
                    continue

                else:
                    pass


            if re.match(RE_ORG_TABLE, line):
                beginMLOParse("table")
                tmpMLO = [line]
            elif re.match(RE_ORG_BLOCK_BEGIN, line):
                beginMLOParse("block")
                tmpMLO = [line]
            else:
                lineObj = OrgTextLine(line)
                self._addBodyObj(lineObj)


    def _addBodyObj(self, obj):
        self.bodyObj.append(obj)


    def export2Markdown(self, outFile):
        for obj in self.bodyObj:
            obj.export(outFile, 'markdown')


    def show(self):
        for obj in self.bodyObj:
            obj.show()


''' org-mode document object
'''
class OrgDoc(object):

    def __init__(self, fileObj):
        self.document = fileObj.readlines()
        self.head = None
        self.body = None
        self._parseText()

    ''' parse a org-mode document to OrgHead Object and OrgBody Object
    '''
    def _parseText(self):

        headLines= []
        popCount = 0

        for line in self.document:
            if re.match(r'#\+.+?:.*', line):
                headLines.append(line)
                popCount = popCount + 1
            else:
                break;

        for i in range(0, popCount):
            self.document.pop(0)

        self.head = OrgHead(headLines)
        self.body = OrgBody(self.document)

    ''' export to markdown document
    '''
    def export2Markdown(self, outPath):
        if None == outPath:
            outPath = sys.stdout
        self.body.export2Markdown(outPath)


    def showHead(self):
        self.head.show()

    def showBody(self):
        self.body.show()


# Main
if __name__ == '__main__':
    usage = 'usage: org2md -f XXX.org -o XXX.md'
    optParser = OptionParser(usage)
    optParser.add_option('-f', dest='orgFile', help='org-mode file')
    optParser.add_option('-o', dest='mdFile', help='markdown file')
    (options, args) = optParser.parse_args()
    orgFilePath = options.orgFile
    mdFilePath  = options.mdFile

    if None == orgFilePath:
        print usage
        sys.exit(-1)

    try:
        orgFileObj = open(orgFilePath, 'r')
    except IOError, e:
        print e
        sys.exit(-1)

    mdFileObj = None
    if None != mdFilePath:
        try:
            mdFileObj = open(mdFilePath, 'w')
        except IOError, e:
            print e

    orgDoc = OrgDoc(orgFileObj)
    orgDoc.export2Markdown(mdFileObj)

    if orgFileObj: orgFileObj.close()
    if mdFileObj: mdFileObj.close()
