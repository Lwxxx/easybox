#!/usr/bin/env python

import re

OrgLineObjType = ["head", ""]

''' One line of a org-mode document
'''
class OrgLine(object):

    lineObjType = {}

    def __init__(self, text):
        pass


class OrgMultiLine(object):

    def __init__(self, text):
        pass


class OrgHead(object):

    def __init__(self, content):
        self.content = content
        self.headObj = {}
        self.parse()


    def parse(self):

        for line in self.content:
            matchResult = re.match(r"(#\+.+?):(.*)" ,line)
            if matchResult:
                self.headObj[matchResult.group(1)] = matchResult.group(2) 
        
        
    def show(self):

        for key in self.headObj:
            print key, "=>", self.headObj[key]

        
class OrgBody(object):

    def __init__(self, content):
        self.content = content
        self.bodyObj = []
        self.parse()
        
    def parse(self):
        pass
        
    def show(self):
        for line in self.content:
            print line,

        print


'''
Org-mode Document Object
'''
class OrgDoc(object):

    def __init__(self, filePath):

        try:
            fp = open(filePath, 'r')
            self.document = fp.readlines()
        except IOError:
            print "Program terminating."
            sys.exit(1)

        self.head = None
        self.body = None
        self.parse()

    ''' parse a org-mode document to OrgHead Object and OrgBody Object
    '''
    def parse(self):

        headLines= []
        popCount = 0

        for line in self.document:
            if re.match(r"#\+.+?:.*", line):
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
    def export2Markdown(self, outPath=None):

        pass

    
    def showHead(self):
        self.head.show()

    def showBody(self):
        self.body.show()


# Main
if __name__ == "__main__":
    orgDoc = OrgDoc("test.org")
    orgDoc.showHead()
    print "==================================================="
    orgDoc.showBody()
    
