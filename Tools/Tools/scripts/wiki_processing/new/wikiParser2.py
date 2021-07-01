#!/usr/bin/env python

from __future__ import with_statement
from contextlib import closing
from StringIO import StringIO
from optparse import OptionParser
from xml.sax import make_parser
from xml.sax.handler import ContentHandler

import sys
import re
import string
import time
import optparse
import pattern, pattern.vector
from pattern.vector import stem,LEMMA,PORTER
import pdb
import math
import time

dictcorpus = {}
debug = False
options = 0
dictlabels  = {}
dictterms = {}
ftitles = open('titles.csv','w')
fdeltitles = open('deltitles.csv','w')
title_log = open("Title_LOG.txt", "w")
nodocs = 0
noterms = 0
nolabels = 0
textlength = 0

""" 
SAX parser code lifted from 
http://jjinux.blogspot.com/2009/01/python-parsing-wikipedia-dumps-using.html
"""

class XMLHandler():

    def __init__(self):
        self.counter = 0
        self.outfile = open('outfile.txt','a')
    
    def _try_calling(self, method_name, *args):
        try:
            f = getattr(self, method_name)
        except AttributeError:
            pass
        else:
            return f(*args)

    def startElement(self, name, attr):
        """Dispatch to methods like _start_tagname."""
        self._characters_buf = None
        self._try_calling('_start_' + name, attr)

    def endElement(self, name):
        """Dispatch to methods like _end_tagname."""
        self._try_calling('_end_' + name)

    def _start_page(self, attr):
        self._redirect = False

    def _start_text(self, attr):
        self._setup_characters()
        self._text_type = 'text'

    def _end_text(self):
        self._teardown_characters()
        
    def _start_title(self, attr):
        self._setup_characters()
        self._text_type = 'title'

    def _end_title(self):
        self._teardown_characters()

    def _start_redirect(self, attr):
        self._redirect = True

    def _end_redirect(self):
        pass

    def _setup_characters(self):
        """Setup the callbacks to receive character data.

The Parser will call the "characters" method to report each chunk of
character data.  SAX parsers may return all contiguous character data
in a single chunk, or they may split it into several chunks.  Hence,
this class has to take care of some buffering.

        """
        self._characters_buf = StringIO()
        
    def characters(self, s):
        """Buffer the given characters."""
        if self._characters_buf is not None:
            self._characters_buf.write(s)

    def _teardown_characters(self):
        """Now that we have the entire string, put it where it needs to go.

Dispatch to methods like _characters_some_stack_of_tags.  Drop strings
that are just whitespace.

        """
        if self._characters_buf is None:
            return
        s = self._characters_buf.getvalue()
        if s.strip() == '':
            return

        method_name = '_characters_' + self._text_type
        self._try_calling(method_name, s)

    def _characters_title(self,s):
        self._title = s

    def tokenize(self,s):
        words = re.split(r'\W+',s)
        return words

    def _characters_text(self,s):

        global fdeltitles

        self._title = self._title.encode("utf8")

        if self._redirect:
            fdeltitles.write(self._title + "\n")
            return
        #
        if re.match(r'Portal:|Template:|Wikipedia:|Category:|File:',self._title) or re.search(r'\(disambiguation\)',self._title or re.findall(r'{{[^{}]*?-stub}}',s,re.DOTALL)) :
            fdeltitles.write(self._title + "\n")
            return
        
        global dictcorpus

        s = s.encode("utf8")
        
        title = self._title
        dictcorpus[title] = {}

        if debug:
            print
            print 'Title: %s' % title
            print
            sys.stdout.flush()
            pdb.set_trace()

        labels = re.findall(r'(\[\[Category:)(.*?)(\]\])',s,re.DOTALL)
        labelvec = []
        for label in labels:
            if label[1][-2:]=="| ":
                label = label[1][:-2]
            else:
                label = label[1]
            labelvec.append(label)

        if debug:
            print
            print 'Labels: '
            print '\n'.join(labelvec)
            print
            sys.stdout.flush()
            pdb.set_trace()
            
        #dictcorpus[title]['labels'] = labelvec

        if debug:
            print
            print 'Removing noisy and command part of the text'
            print
            sys.stdout.flush()
            
        if debug:
            r = ' '.join(re.findall(r'== *?References *?==.*',s,re.DOTALL))
        s = re.sub(r'== *?References *?==.*',r' ',s,0,re.DOTALL)
       
        if debug:
            r = r+' '+' '.join(re.findall(r'== *?External links *?==.*',s,re.DOTALL))
        s = re.sub(r'== *?External links *?==.*',r' ',s,0,re.DOTALL)

        if debug:
            r = r+' '+' '.join(re.findall(r'== *?Further reading *?==.*',s,re.DOTALL))
        s = re.sub(r'== *?Further reading *?==.*',r' ',s,0,re.DOTALL)

        if debug:
            r = r+' '+' '.join(re.findall(r'== *?See also *?==.*',s,re.DOTALL))
        s = re.sub(r'== *?See also *?==.*',r' ',s,0,re.DOTALL)

        if debug:
            print
            print 'Noisy trailing part of text removed:'
            print r
            print
            sys.stdout.flush()
            pdb.set_trace()

        if debug:
            r = '\n'.join(re.findall(r'<math>.*?</math>',s,re.DOTALL))
        s = re.sub(r'<math>.*?</math>',r' ',s,0,re.DOTALL)
        if debug:
            r = r+'\n'+'\n'.join(re.findall(r'<!--.*?-->',s,re.DOTALL))
        s = re.sub(r'<!--.*?-->',r' ',s,0,re.DOTALL)

        if debug:
            print
            print 'Text between <math></math> and <!-- --> removed:'
            print r
            print
            sys.stdout.flush()
            pdb.set_trace()

        anchortext =  ', '.join([item[1] if (item[1].rfind('|')==-1) else item[1][item[1].rfind('|')+1:] for item in re.findall(r'(\[\[)([^\[\]{}]*?)(\]\])',s,re.DOTALL)])
        anchorwords = self.tokenize(anchortext)

        if debug:
            print
            print 'Anchortext retrieved:'
            print anchortext
            print
            sys.stdout.flush()
            pdb.set_trace()

        titletext = title
        titletext = titletext + ', '+', '.join([item[1] for item in re.findall(r'(==+?)([^=]*?)(==+?)',s,re.DOTALL)])
        titlewords = self.tokenize(titletext)

        if debug:
            print
            print 'Titletext retrieved:'
            print titletext
            print
            sys.stdout.flush()
            pdb.set_trace()

        if debug:
            print
            print 'Text between {{ }} {| |} removed'
            print
            sys.stdout.flush()

        if debug:
            r = '\n'.join(re.findall(r'{\|.*?\|}',s,re.DOTALL))
        s = re.sub(r'{\|.*?\|}',r' ',s,0,re.DOTALL)

        if debug:
            print
            print r
            print
            sys.stdout.flush()
            pdb.set_trace()

        itr = re.finditer(r'({{)|(}})',s,re.DOTALL)
        indices = []

        for item in itr:
            if item.start(1)!=-1:
                indices.append([1,item.start(1)])
            if item.end(2)!=-1:
                indices.append([2,item.end(2)])
        
        slist = list(s)

        count = 0
        for i in indices:
            if i[0]==1:
                count = count+1
                if count==1:
                    start = i[1]
            else:
                count = count-1
            
            if count<0:
                count = 0
            elif count==0:
                slist[start:i[1]] = ['*']*(i[1]-start)
                if debug:
                    print
                    print s[start:i[1]]
                    print

        s = ''.join(slist)
        s = string.replace(s,'*',' ')

        if debug:
            print s
            sys.stdout.flush()
            pdb.set_trace()

        if debug:
            print
            print 'Some wiki directive tags removed'
            print
            sys.stdout.flush()

        if debug:
            r = ' '.join(re.findall(r'<ref.*?>',s,re.DOTALL))
        s = re.sub(r'<ref.*?>',r' ',s,0,re.DOTALL)
        if debug:
            r = r+' '+' '.join(re.findall(r'<\w+>|</\w+>|File:|Image:|http:|<\w+/>',s,re.DOTALL))
        s = re.sub(r'<\w+>|</\w+>|File:|Image:|<\w+/>',r' ',s,0,re.DOTALL)

        if debug:
            print
            print r
            print
            print s
            sys.stdout.flush()
            pdb.set_trace()

        text = s

        if debug:
            print
            print 'Sanitizing text by replacing everything other than [a-zA-Z0-9] by space'
            print
            sys.stdout.flush()
            pdb.set_trace()

        words = self.tokenize(text)
        
        vec = words
        
        vec2 = []
        titlewords2 = []
        anchorwords2 = []
        dic2 = {}
        for i in range(len(vec)):
            vec[i] = vec[i].lower()
            vec[i] = vec[i].replace("_", " ")
            vec[i] = re.sub(r'[ ]+', vec[i], " ").strip()
            vec2.extend(vec[i].split(" "))

        for i in range(len(titlewords)):
            titlewords[i] = titlewords[i].lower()
            titlewords[i] = titlewords[i].replace("_", " ")
            titlewords[i] = re.sub(r'[ ]+', titlewords[i], " ").strip()
            if (titlewords[i] != " " or titlewords[i] != ""):
                titlewords2.extend(titlewords[i].split(" "))

        for i in range(len(anchorwords)):
            anchorwords[i] = anchorwords[i].lower()
            anchorwords[i] = anchorwords[i].replace("_", " ")
            anchorwords[i] = re.sub(r'[ ]+', anchorwords[i], " ").strip()
            if (anchorwords[i] != " " or anchorwords[i] != ""):
                anchorwords2.extend(anchorwords[i].split(" "))

        vec = vec2
        titlewords = titlewords2
        anchorwords = anchorwords2
        vec.extend(titlewords)
        vec.extend(anchorwords)

        for i in range(options.boost_title):
            vec.extend(titlewords)
        
        for i in range(options.boost_anchor):
            vec.extend(anchorwords)
        """
        for i in range(len(vec)):
            title_log.write(vec[i])
            title_log.write("\n")
        """
        for i in range(len(vec)):
            if(len(vec[i]) > 1):
                if(vec[i] not in dic2):
                    dic2[vec[i]] =  0
                dic2[vec[i]] += 1
        """
        for key, val in dic2.items():
            title_log.write(str(key) + ":" + str(val))
            title_log.write("\n")
        """
        dic = pattern.vector.count(vec,stemmer=PORTER)
        #dic = dic2
        global dictlabels,dictterms,nodocs

        nodocs = nodocs+1

        labelvec = map(lambda x: re.sub(r'[^\w]',' ',x), labelvec)

        for item in labelvec:
            if dictlabels.has_key(item):
                dictlabels[item] = dictlabels[item]+1
            else:
                dictlabels[item] = 1

        for key,val in dic.items():
            if dictterms.has_key(key):
                dictterms[key] = dictterms[key]+1
            else:
                dictterms[key] = 1
        
        global fout,ftitles
        fout.write(title +"\t" +','.join(labelvec)+'\t'+','.join([(str(key)+':'+str(val)) for key,val in dic.items()]) + "\n" )
        ftitles.write(title + "\n");


        self.counter = self.counter+1
        if self.counter%1000==0:
            print str(self.counter/1000.0)
            sys.stdout.flush()
            
    """ Unnecessary Callback Functions
    """

    def processingInstruction(self, target, data):
        pass

    def setDocumentLocator(self, locator):
        pass

    def startDocument(self):
        pass

    def endDocument(self):
        pass


def parsexml(file):
    parser = make_parser()
    xmlhandler = XMLHandler()
    parser.setContentHandler(xmlhandler)
    parser.parse(file)


def main(options):
    parsexml(options.input_file)
    """
    global fout,ftitles,fdeltitles
    fout.close()
    ftitles.close()
    fdeltitles.close()

    if options.debug:
        print
        print 'xml is parsed'
        print
        sys.stdout.flush()
        pdb.set_trace()

    global nodocs,noterms,nolabels,dictterms,dictlabels

    f = open('delterms.csv','w')
   
    for key,val in dictterms.items():
        #print key, val
        if val<=2:
            f.write('%s,%d' % (key,dictterms[key]) + "\n")
            del dictterms[key]

    f.close()

    noterms = len(dictterms.keys())
    nolabels = len(dictlabels.keys())

    sumterms = 0
    sumlabels = 0
    #print "terms in dictionary "+ "\n"
    for key,val in dictterms.items():
        #print key, val
        sumterms = sumterms+val
    #print "labels in dictionary" + "\n"
    for key,val in dictlabels.items():
        #print key, val
        sumlabels = sumlabels+val

    dperl = float(sumlabels)/nolabels
    lperd = float(sumlabels)/nodocs
    dpert = float(sumterms)/noterms
    tperd = float(sumterms)/nodocs


    finfo = open('info.txt','w')
    finfo.write('Number of Documents : %d' % nodocs + "\n")
    finfo.write('Number of Labels : %d' % nolabels + "\n")
    finfo.write('Number of Terms : %d' % noterms + "\n")
    finfo.write('Avg Documents per Label : %f' % dperl + "\n")
    finfo.write('Avg Labels per Document : %f' % lperd + "\n")
    finfo.write('Avg Terms per Document : %f' % tperd + "\n")
    finfo.write('Avg Documents per Term : %f' % dpert + "\n")
    finfo.close()
    
    labels = dictlabels.keys()
    labels.sort()
    dictlabelinds = dict(zip(labels,range(len(labels))))
    flabels = open('labels.csv','w')
    for item in labels:
        flabels.write('%s,%d' % (item,dictlabels[item]) + "\n")
    flabels.close()

    dictidfs = {}
    for key,val in dictterms.items():
        dictidfs[key] = -math.log(float(val)/nodocs)
    
    terms = dictterms.keys()
    terms.sort()
    dictterminds = dict(zip(terms,range(len(terms))))
    f = open('terms.csv','w')
    for item in terms:
        f.write('%s,%d,%f' % (item,dictterms[item],dictidfs[item]) + "\n")
    f.close()

    fin = open('outfile.txt','r')
    fftout = open('ftMat.csv','w')
    flblout = open('lblMat.csv','w')

    #fftout.write('%d %d' % (nodocs,noterms) + "\n")
    #flblout.write('%d %d' % (nodocs,nolabels) + "\n")

    for line in fin:
        items = line.rstrip().split('\t')
        #print line
        labels = items[0].split(',')
        
        print labels
        
        if(len(items) < 2 ):
            continue
        terms = items[1].split(',')
        
        print terms
        print
        
        labels.sort()
        if labels!=['']:
            t = map(lambda x: str(dictlabelinds[x]),labels)
            #print labels
            print t
            print zip(t,['1.0']*len(t))
            flblout.write(' '.join(map(lambda x: x[0]+':'+x[1],zip(labels,['1.0']*len(labels)))))
        flblout.write("\n")
        dic = {}
        #print terms
        for item in terms:
            pair = item.split(':')
            if dictterminds.has_key(pair[0]):
                dic[pair[0]] = float(pair[1])
        terms = dic.keys()
        terms.sort()
        #print terms
        #print dictterminds.keys()
        fftout.write(' '.join(map(lambda x: str(dictterminds[x])+':'+str(dic[x]*dictidfs[x]),terms)) + "\n")
    fin.close()
    fftout.close()
    flblout.close()
    """
    
def parse_commandline():
    parser = optparse.OptionParser()
    parser.add_option('-i', '--input-file', dest = 'input_file',
            default = None)
    parser.add_option('-t', '--boost-title', dest = 'boost_title',
            default = 0, type = 'int')
    parser.add_option('-a', '--boost-anchor', dest = 'boost_anchor',
            default = 0, type = 'int')
    parser.add_option('-d', '--debug', dest = 'debug',
            default = False, action = 'store_true')
    return parser.parse_args()

  
if __name__ == "__main__":
    global fout
    fout = open('outfile.txt','w')
    tic = time.time()

    options, args = parse_commandline()

    if options.debug:
        print
        print 'Entered Options:'
        print '%s : %s' % ('input_file',options.input_file)
        print '%s : %s' % ('boost_title',options.boost_title)
        print '%s : %s' % ('boost_anchor',options.boost_anchor)
        print '%s : %s' % ('debug',options.debug)
        print
        sys.stdout.flush()
        pdb.set_trace()

    if options.debug:
        debug = True
    main(options)

    title_log.close()

    toc = time.time()
    
    fout.close()

    finfo = open('info.txt','a')
    finfo.write('Total time taken : %f'  % ((toc-tic)/60.0) + "\n")
    finfo.close()

